/*	Copyright  (c)	Günter Woigk 2001 - 2019
					mailto:kio@little-bat.de

	This file is free software

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	• Redistributions of source code must retain the above copyright notice,
	  this list of conditions and the following disclaimer.
	• Redistributions in binary form must reproduce the above copyright notice,
	  this list of conditions and the following disclaimer in the documentation
	  and/or other materials provided with the distribution.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
	PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
	CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
	EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
	OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
	WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
	OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
	ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	2002-04-28 kio	changed Stream io[] to Stream* io[] to allow use of subclasses of Stream
					added OpenStream() and CloseStream()
	2003-07... kio	changes to use UCS4String library
*/

#define SAFE 3
#define LOG 1
#include "kio/kio.h"
#include "unix/os_utilities.h"
#include "Stream.h"
#include "SystemIO.h"
#include "VScript.h"
#include "Thread.h"
#include "Execute.h"
#include "token.h"
#include "globals.h"
#include "serrors.h"
DEBUG_INIT_MSG


Var* root;


/* ----	read and merge preferences from prefs file ------------------
		typically called twice:
		1 - read user prefs
		2 - read and fill in system prefs
*/
static void MergePrefs ( Var& z, cstr path )
{
	String s;
	ptr a,e,p,eol,eof;

	s.ReadFromFile(path); if(s.Len()==0) return;
	a   = s.Text();
	eof = a+s.Len();

	for( ; a<eof; a=eol+1 )
	{
		eol = a; while( eol<eof && *eol!=10 && *eol!=13 ) { eol++; }
				 while( a<eol && uchar(*a)<=' ' ) { a++; } if(a==eol||*a=='#') continue; 	// a -> start of name
		p = a;   while( p<eol && uchar(*p)>' ' )  { p++; }									// p -> end of name

		NameHandle nh = NewNameHandle( substr(a,p) );
		if(z.FindItem(nh)) goto u;			// name ex. schon -> skip

		e = eol; while( uchar(e[-1])<=' ') { e++; } if(e<=p) goto u;	// e -> end of data
				 while( uchar(*p)<=' ')    { p++; }						// p -> start of data

		z.AppendItem( new Var(substr(p,e),nh) );

	u:	UnlockNameHandle(nh);
	}
}


/* ----	UP: get and evaluate pref for stream coloring -----------------------
*/
static VT_Color EvalColorPref ( cVar* prefs, cstr idf, VT_Color result /*default*/ )
{
	xlogIn("GetColorPref()");

	Var* v = prefs->FindItem(idf);
	if( v && v->IsText() && v->TextLen() )
	{
		String& ps = v->Text();

		int i = String("ack red een low lue nta yan ite").Find(ps.RightString(3));

		if(i>=0)	// color name found ?
		{
			result = VT_black + i/4;
		//	if(ps.Find("dark")>=0) result |= VT_dark;		dark == standard
			if(ps.Find("ligh")>=0) result |= VT_bright;
		}
		else
		{
			result = VT_normal;
		}
		if(ps.Find("bold")>=0)  result |= VT_bold;
		if(ps.Find("inver")>=0) result |= VT_inverse;
		if(ps.Find("under")>=0) result |= VT_underline;
	}

	xlogline("return: %x",(int)result);
	return result;
}


/* ----	setup new VScript instance -------------------------------------
*/
VScript::VScript ( rt runtyp, cString& applname, cString& filename, Var* argv, Stream*i, Stream*o, Stream*e )
{
	xlogIn("VScript()");

	assert(tokens<=255);
	assert( o->OutputEncoding()>=UCS1 && o->OutputEncoding()<=UTF8 );

	root = new Var(isList,"root");		// same name as token: "root"
	root->Lock();

	globals = argv;
	globals->SetName("globals");

	prefs   = new Var( isList, "prefs" );
	rsrc    = new Var( isList, "resources" );
	threads = new Var( isList, "threads" );
	appl    = new Var( isList, "interpreter" );
	stats   = new Var( isList, "stats" );
	job     = new Var( isList, "job" ) ;
	stdio   = new Var( isList, "stdio" );
	shell	= new Var( isList, "shell" );
	env     = NewEnvironmentVar(0/*not lowercase*/); env->SetName("env");

// root.env:

	user    = env->FindItem("user");    if(!user)     env->AppendItem( user     = new Var(getUser(),"user") );
	group   = env->FindItem("group");   if(!group)    env->AppendItem( group    = new Var("(unknown)","group") );
	hostname= env->FindItem("hostname");if(!hostname) env->AppendItem( hostname = new Var(hostName(),"hostname") );

// root.stdio:

	stdin0  = new Var(stdio,0, i, "stdin"  );
	stdout0 = new Var(stdio,1, o, "stdout" );
	stderr0 = new Var(stdio,2, e, "stderr" );

// root.job:

	job->AppendItems(
				  new Var( applname,  	"invocation" ),
		script	= new Var( filename,	"script" ),
		runtype	= new Var( runtyp,		"runtype" ),
		verbose	= new Var( ::verbose,	"verbose")
		);

// root.interpreter:

	appl->AppendItems(
		new Var( applName,		"name"  	  ),
		new Var( applLongName,  "longName" 	  ),
		new Var( applVersion,   "version"     ),
		new Var( applCopyright, "copyright"   ),
		new Var( applEmail,     "email"       ),
	//	new Var( buildUser,     "buildUser"   ),
		new Var( buildDate,     "buildDate"   )
		);

	appl->AppendItems(
	//	new Var( _COMPILER,   	"compiler"        ),
		new Var( _PLATFORM,   	"targetPlatform"  ),
		new Var( _PROCESSOR,   	"targetProcessor" ),
		new Var( _BYTEORDER,	"targetByteorder" ),
		new Var( sizeof(Double),"sizeofNumbers"   ),
		new Var( tokens, 		"opcodes"         ) );

	#if _ALIGNMENT_REQUIRED
		appl->AppendItem( new Var( _MAX_ALIGNMENT,  "alignment"  ) );
		#if 0
		_ALIGNMENT_REQUIRED
		_SHORT_ALIGNMENT
		_INT_ALIGNMENT
		_LONG_ALIGNMENT
		_LONG_LONG_ALIGNMENT
		_DOUBLE_ALIGNMENT
		_POINTER_ALIGNMENT
		#endif
	#endif

	#if defined(DEBUG)
		appl->AppendItem( new Var( "Debug",   "buildStyle" ) );
	#elif defined(DEVELOP)
		appl->AppendItem( new Var( "Develop", "buildStyle" ) );
	#elif defined(RELEASE)
		appl->AppendItem( new Var( "Release", "buildStyle" ) );
	#else
		#error hm hm ...
	#endif

	#if defined(OPCODE_PROFILING)
		appl->AppendItem( new Var( 1.0, "opcodeProfiling" ) );
	#endif

// root.stats:

	#ifdef OPCODE_PROFILING
		stats->AppendItem( opcode_profile = new Var( isList, "opcodeProfiles" ) );
	#endif

// cgi?

	str request_method = getenv("REQUEST_METHOD");
	if (request_method&&*request_method)
	{
		*runtype = runtyp = rt_cgi;

		// merge stdout and stderr => error messages go into cgi output
		if ( dup2(1/*stdout*/,2/*stderr*/) < 0 )
		{
			str s = usingstr("VScript::VScript(): dup2() failed: %s",errorstr());
			fputs (s,stdout);
			fputs (s,stderr);
		}

		get_cgi_arguments ( globals, request_method );
	}

// root.prefs:

	MergePrefs( *prefs, runtyp==rt_cgi ? "/etc/vipsi.cgi.conf" : runtyp==rt_shell ? "~/.vipsi/shell.prefs" : "~/.vipsi/script.prefs" );

	if(runtyp!=rt_cgi)
	{
		MergePrefs( *prefs, runtyp==rt_shell ? "/usr/local/lib/vipsi/shell.prefs" : "/usr/local/lib/vipsi/script.prefs" );

		if( stdin0->GetStream()->IsTTY() )	stdin0->GetStream()->SetTTYAttr ( EvalColorPref( prefs, "stdin_attr",  VT_blue   ) );
		if( stdout0->GetStream()->IsTTY() )	stdout0->GetStream()->SetTTYAttr( EvalColorPref( prefs, "stdout_attr", VT_normal ) );
		if( stderr0->GetStream()->IsTTY() )	stderr0->GetStream()->SetTTYAttr( EvalColorPref( prefs, "stderr_attr", VT_red | VT_inverse ) );
	}
	errno=ok;

// root:

	root->AppendItems(appl,stats,job,stdio);
	root->DeepLockData();
	root->UnlockData();

	root->AppendItems(shell,rsrc,threads,env,prefs,globals);
	root->DeepLockType();
	root->LockData();

	assert(errno==ok);
	assert(stdout0->IsLocked());


// compile script

	compiler.CompileFile(filename);
	if(errno==ok)
	{
		Var* r = compiler.GetBundle();

		assert( r->IsList() );
		assert( r->ListSize()>=3 );
		assert( r->List().array[0]->IsText() );	// csz1: core
		assert( r->List().array[1]->IsList() );	// constants
		assert( r->List().array[2]->IsText() );	// csz4: rowcolxref

		assert( r->DataIsLocked() );
		assert( r->List().array[0]->DataIsLocked() );
		assert( r->List().array[1]->DataIsLocked() );
		assert( r->List().array[2]->DataIsLocked() );

		r->SetName(filename);
		rsrc->AppendItem(r);

		assert(globals->TypeIsLocked());
		Thread* t = new Thread(compiler.GetProc(), globals, runtyp==rt_shell ? 0.05 : 1.0 );
		threads->AppendItem( new Var(t) );
	}
}


VScript::~VScript()
{
	root->Unlock();
	root = nullptr;
}




void VScript::get_cgi_arguments ( Var* globals, cstr request_method )
{
/*	REQUEST_METHOD	= "GET"
	QUERY_STRING	= <arguments>
	encoding		= application/x-www-form-urlencoded		(( "+" == " "; "%ab" = sonderzeichen ))

	<form enctype="text/plain" method="POST">  				(( z.b. für mailto:x@y ))
	<form enctype="multipart/form-data" method="POST">		(( z.b. für datei-upload ))
	encoding		= application/x-www-form-urlencoded		(( default? ))

	REQUEST_METHOD	= "POST"
	stdin			= <arguments>
	CONTENT_LENGTH	= <number>
	CONTENT_TYPE	= multipart/form-data; boundary=---------------------------136913306911258981671059961393
*/
	String data;

	if( eq(request_method,"GET") )
	{
		data = getenv("QUERY_STRING");		// 'GET' arguments
		if (data.Len()==0) return;			// no or empty QUERY_STRING
	}

	else if ( eq(request_method,"POST") )
	{
	// ---- get input data size: ----
		cstr content_length = getenv("CONTENT_LENGTH");
		cptr p = content_length;
		if (!p)
		{
			SetError("vipsi: POST: no CONTENT_LENGTH");
			return;
		}

		uint len = 0; while( is_dec_digit(*p) ) { len = len*10 + *p-'0'; p++; }
		if (*p)
		{
			SetError( usingstr("vipsi: POST: bogus CONTENT_LENGTH: %s",quotedstr(content_length)) );
			return;
		}

	// ---- get input data: ----
		data = String(len,csz1);			// UCS1 string
		uint n = 0;							// bytes read

		/*	note: we _must_ read all data from stdin or else our Apache child thread will get blocked
			and thus will not read our output to stdout and thus won't serve our page!
		*/
		while (n<len)
		{
			uint r = read ( 0/*stdin*/, data.Text()+n, len-n );
			if (r>0) n+=r;
		}

		if(XXLOG)
			globals->AppendItem(new Var(data,"CONTENT_DATA"));
	}

	else
	{
		SetError( usingstr("vipsi: CGI: unknown REQUEST_METHOD: %s",quotedstr(request_method)) );
		return; 		// unknown/bogus request method
	}


// ----	get arguments ----

	cstr content_type = getenv("CONTENT_TYPE");
	if ( !content_type )
	{
	// no CONTENT_TYPE => assume application/x-www-form-urlencoded
		goto nct;
	}


	else if ( find(content_type,"multipart/form-data;") )
	{
	//	CONTENT_TYPE = multipart/form-data; boundary=---------------------------147461239920539999321264095060

	/* example DATA from POST fileupload form:
		--<boundary><0d><0a>
		Content-Disposition: form-data; name="<inputfieldname>"; filename="<uploadedfilename>"<0d><0a>
		Content-Type: application/octett-stream<0d><0a>
		<0d><0a>
		<binary data from file><0d><0a>
		--<boundary><0d><0a>
		Content-Disposition: form-data; name="<inputfieldname>"<0d><0a>
		<0d><0a>
		<valueofinputfield><0d><0a>
		--<boundary>--<0d><0a>
	*/

	/*	conclusio:	fields are delimited by either "; " or "<0d><0a>"

		normal arguments have name and data		=> append normal text variable
		upload files have additional filename	=> append list variable: { filename="..", data=".." }
	*/

	/*	security note:
		<uploadfilename> is not encoded and MAY CONTAIN «"<0d><0a>»  OR  «"; »  !!!
	*/

		content_type = find(content_type,"boundary=");
		if (content_type==nullptr) goto uct;	// unknown content type
		String multipart_boundary = String("--") + (content_type+9);

		String fieldname,filename,fielddata;
		uint32 a=0,e;

		while( a<(uint32)data.Len() )
		{
			if( data.MidString(a,multipart_boundary.Len())==multipart_boundary )
			{
				if(a!=0)	// if not initial <boundary> then store variable
				{
					if(filename.Len())
					{
						Var* newvar = new Var(isList,fieldname);
						newvar->AppendItem( new Var(filename, "filename" ) );
						newvar->AppendItem( new Var(fielddata,"data"     ) );
						globals->AppendItem(newvar);
					}
					else
					{
						globals->AppendItem(new Var(fielddata,fieldname));
					}
				}

				fieldname = filename = fielddata = emptyString;
				a += multipart_boundary.Len();
				if( data.MidString(a,2)=="\x0d\x0a" ) { a+=2; continue; }
				else return; // assume "--<0d><0a>"  =>  end ok
			}

			if( data.MidString(a,13)=="Content-Type:" )
			{
				// just assume "application/octett-stream"
				// => skip field
				goto sf;
			}

			if( data.MidString(a,20)=="Content-Disposition:" )
			{
				// just assume "form-data"
				// => skip field
				goto sf;
			}

			if( data.MidString(a,6)=="name=\"" )
			{
				a += 6;
				e = min( (uint32)data.Find("\"; ",a), (uint32)data.Find("\"\x0d\x0a",a) );
				if (e==(uint32)-1) return;	// bogus
				fieldname = data.SubString(a,e);
				a = e+3; continue;
			}

			if( data.MidString(a,10)=="filename=\"" )
			{
				a += 10;
				e = min( (uint32)data.Find("\"; ",a), (uint32)data.Find("\"\x0d\x0a",a) );
				if (e==(uint32)-1) return;	// bogus
				filename = data.SubString(a,e);
				a = e+3; continue;
			}

			if( data.MidString(a,2)=="\x0d\x0a" )
			{
				e = data.Find(multipart_boundary,a);
				if (e==(uint32)-1) return;	// bogus
				fielddata = data.SubString(a+2,e-2);
				a = e; continue;
			}

			SetError( "vipsi: CGI: unknown field in multipart_data");
			// => skip field
	sf:		a = min( (uint32)data.Find("; ",a), (uint32)data.Find("\x0d\x0a",a) );
			if (a==(uint32)-1) return;	// bogus
			a+=2; continue;
		}
	}


	else	// unknown encoding or application/x-www-form-urlencoded
	{
uct:	SetError( usingstr("vipsi: CGI: unknown CONTENT_TYPE: %s",quotedstr(content_type)) );
nct:	// no content type

	//	CONTENT_TYPE = application/x-www-form-urlencoded

	//	data = "anton=berta&c%C3%A4sar=ist+ein+D%C3%B6del"
	//	non-ascii and special chars escaped with "%"	((utf-8 characters encoded byte-by-byte))
	//	spaces replaced with "+" or "%20"
	//	"=" between name and value						((unnamed args are possible))
	//	"&" between multiple arguments

		data.Replace('+',' ');							// replace <+> with <space>
		data += '&';									// final argument separator for easier splitting
		String next_arg,arg_name;

		while(data.Len())
		{
			int n    = data.Find('&');				// argument separator
			next_arg = data.LeftString(n);
			data     = data.MidString(n+1);

			n        = next_arg.Find('=');			// name-value separator
			arg_name = next_arg.LeftString(n);		// note: -1 for not_found works ok!
			next_arg = next_arg.MidString(n+1);		// ""

			globals->AppendItem( new Var(next_arg.FromUrl(),arg_name.FromUrl()) );
		}
	}
}








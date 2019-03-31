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
*/

#define SAFE 3
#define LOG 1
#include "config.h"
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <termios.h>
#include "unix/os_utilities.h"
#define _include_version_as_source_
#include "version.h"
#undef TAB1	// #define in <termios.h>
#undef TAB2	// #define in <termios.h>
#undef TAB3	// #define in <termios.h>
#include "token.h"
#include "globals.h"
#include "Stream.h"
#include "VScript.h"
#include "Thread.h"

INIT_MSG


// serrors.cpp:
inline	String	ErrorString	( int/*OSErr*/ e )				{ return errorstr(e); }
extern	String	ErrorString	( );
extern	void	ForceError	( int/*OSErr*/e, cString& msg );
inline	void	SetError	( int/*OSErr*/e, cString& msg )	{ if(errno==ok) ForceError(e,msg); }
inline	void	ForceError	( cString& msg )				{ ForceError(-1,msg); }
inline	void	SetError	( cString& msg )				{ if(errno==ok) ForceError(-1,msg); }
extern	void	AppendToError	( cString& msg );
extern	void	PrependToError	( cString& msg );


cstr appl_name = applName;		// for abort.cp

//Stream*	stdStream[3]	= { nullptr,nullptr,nullptr };

static struct termios	old_stdin_termios;

uint verbose = 0;



/* ----	restore old settings for stdin ---------------------------
		sheduled via atexit()
*/
static void restore_stdin ( void )
{
	xlogIn("RestoreStdin()");

	char bu[]="\033[0m"; write(0,bu,4);			// reset text attributes
	tcsetattr(0,TCSADRAIN,&old_stdin_termios);	// drain output, then change
}


/* ---- show help summary -----------------------------------
		display optimized for 80 character terminals
*/
static void ShowHelp ( void )
{
	fputs( "\n"
"______________________________________________________________________________\n"
"" applLongName " " applVersion " " applCopyright ".\n"
"  built by " buildUser " at " buildDate " for " buildTarget ".\n"
"  send bug reports to: " applEmail "\n"
"  development release. please set your terminal to utf-8 and vt100.\n"
"  see <http://k1.spdns.de/Develop/projects/vipsi/> for latest documentation.\n"
"\n"
"vipsi -v        => verbose mode\n"
"vipsi -p        => pretend - check syntax only\n"
"vipsi -h        => print help screen\n"
"vipsi -t        => run internal test\n"
"vipsi -i file   => set stdin\n"
"vipsi -o file   => set stdout\n"
"vipsi -l file   => set stderr\n"
"\n"
"vipsi filename  => execute script.\n"
"vipsi           => interactive shell. help available from within.\n"
"______________________________________________________________________________\n"
"", stderr );
}



/* ---- program entry ---------------------------
*/
int main( int argc, cstr argv[] )
{	int i=0;

	openLogfile("/var/log/vipsi/", DAILY, 30/*max_logfiles*/, no/*log2console*/, no/*with_date*/, no/*with_msec*/);
	xlogIn("main()");
	assert(argc>0);
	assert(argv!=nullptr && argv[0]!=nullptr);

	#ifndef NDEBUG
	{
		for (int i=0;i<8;i++)
		{
			static uchar bu[16];
			write_double(bu+i,123.456e78);
			assert(read_double(bu+i) == 123.456e78);
		}
	}
	#endif

	#ifdef _MACOSX
		cstr path = getenv("PATH");
		if( find(path,"/sw/")==nullptr )
		setenv( "PATH", catstr(path,":/sw/bin:/sw/sbin"), yes );
		if( find(path,"/opt/local/")==nullptr )
		setenv( "PATH", catstr(path,":/opt/local/bin:/opt/local/sbin"), yes );
	#endif

	cstr stdinName  = nullptr;
	cstr stdoutName = nullptr;
	cstr stderrName = nullptr;
	cstr argvName   = argv[i++];
	cstr argvFile	= nullptr;
	bool pretend 	= no;
	verbose    		= no;
	bool test       = no;
	bool shell		= no;
	bool cgi		= no;
	srandom(intCurrentTime());
	errno=ok;


// scan for options
	while ( i<argc )
	{
		cstr s = argv[i];
		if (!s||s[0]!='-') break;			// first non-option
		i++;

		for ( int j=1; s[j]!=0; j++ )
		{
			switch(s[j])
			{
			case 'v':
				verbose=yes;
				log2console=yes;
				continue;

			case 't':
				test=yes;
				continue;

			case 'p':
				pretend=yes;
				continue;

			case 'i':
			{	if( i>=argc||stdinName ) goto h;
				stdinName = FullPath(argv[i++],1); if(errno) goto x;
				int newfd = open(stdinName,O_RDONLY);
				if(newfd==-1) goto x;	// errno set
				if(close(0/*stdin*/)==-1) goto x;
				if(dup(newfd)!=0) { SetError("dup(fd(0)) failed"); goto x; }
				close(newfd);
			}	continue;

			case 'o':
			{	if( i>=argc||stdoutName ) goto h;
				stdoutName = FullPath(argv[i++],1); if(errno) goto x;
				int newfd = open(stdoutName,O_WRONLY|O_CREAT|O_TRUNC,0660/*muss angegeben werden!*/);
				if(newfd==-1) goto x;	// errno set
				errno=ok;			// errno=ENOENT if file did not exist!
				if(close(1/*stdout*/)==-1) goto x;
				if(dup(newfd)!=1) { SetError("dup(fd(1)) failed"); goto x; }
				close(newfd);
			}	continue;

			case 'l':
			{	if( i>=argc||stderrName ) goto h;
				stderrName = FullPath(argv[i++],1); if(errno) goto x;
				int newfd = open(stderrName,O_WRONLY|O_CREAT|O_TRUNC,0660/*muss angegeben werden!*/);
				if(newfd==-1) goto x;	// errno set
				errno=ok;			// errno=ENOENT if file did not exist!
				if(close(2/*stderr*/)==-1) goto x;
				if(dup(newfd)!=2) { SetError("dup(fd(2)) failed"); /* printing errorstr will probably fail... */ goto x; }
				close(newfd);
			}	continue;

			default:
			h:	ShowHelp();					// argv[] error => print help text
				const uint commandlineerror = 125;
				return commandlineerror;	// and return error code, don't print error message
			}
		}
	}

	if(errno)
	{
x:		logline( "\n%s: %s\n", argvName, errorstr() );
		return errno;
	}


	if (verbose)
	{
		logNl();
		logline( "Compiled with %s",    _COMPILER);
		logline( "Compiled for %s",     _PLATFORM);
		logline( "Running on a %s CPU", _PROCESSOR);
		logline( "Byte order is %s",    _BYTEORDER);

		logNl();
		logline( "Memory alignment is %s", _ALIGNMENT_REQUIRED ? "required" : "not required" );
		logline( "max. alignment is %i", _MAX_ALIGNMENT);
		logline( "%s alignments:", _ALIGNMENT_REQUIRED ? "Required" : "Preferred" );
//		logline( "    short ....... %i", _SHORT_ALIGNMENT);
//		logline( "    int ......... %i", _INT_ALIGNMENT);
//		logline( "    long ........ %i", _LONG_ALIGNMENT);
//		logline( "    llong ....... %i", _LONG_LONG_ALIGNMENT);
//		logline( "    double ...... %i", _DOUBLE_ALIGNMENT);
//		logline( "    pointer ..... %i", _POINTER_ALIGNMENT);
		logline( "sizeof(size_t)  = %i",   int(sizeof(size_t)));
		logline( "sizeof(float)   = %i",   int(sizeof(float)));
		logline( "sizeof(double)  = %i\n", int(sizeof(double)));
		if(sizeof(Double)!=sizeof(double))
		logline( "sizeof(Double)  = %i\n", int(sizeof(Double)));


	// info about current environment:
		logNl();
		logline("argvName = %s", argvName );
		logline("stdin    = %s", stdinName );
		logline("stdout   = %s", stdoutName);
		logline("stderr   = %s", stderrName);

		for (int j=0;j<3;j++)
		{
			if (ClassifyFile(j)!=s_tty) continue;
			int rows,cols; errno=ok;
			if( TerminalSize ( j, rows, cols ) == -1 ) logline("TerminalSize() failed: %s", errorstr());
			else logline( "Terminal size: rows,cols = %u,%u",rows,cols );
			break;
		}

		logNl();
		assert(errno==ok);
	}


// run internal test ?
	if (verbose&&test)
	{
		#ifdef INCLUDE_VAR_TEST_SUITE
			errno=ok;
			TestStringClass();
			if (errno) logline("*** String Test error: %s ***\n",errorstr()); else logNl();
		#else
			logline("class Var test suite was not compiled in.\n");
		#endif

		#ifdef INCLUDE_STRING_TEST_SUITE
			errno=ok;
			TestVarClass();
			if (errno) logline("*** Var Test error: %s ***\n",errorstr()); else logNl();
		#else
			logline("class String test suite was not compiled in.\n");
		#endif
		errno=ok;
	}

// setup tty
	if (ClassifyFile(0)==s_tty)
	{
		struct termios new_stdin_termios;
		if( tcgetattr(0,&old_stdin_termios) != -1 && 	// for restore
			tcgetattr(0,&new_stdin_termios) != -1 )		// for vipsi
		{
			assert(errno==ok);
			atexit(restore_stdin);		// ***TODO*** also on error exit
			assert(errno==ok);

			new_stdin_termios.c_lflag    &= ~ulong(ICANON);	// noncanonical input: no wait for nl, no erase, no kill processing
			new_stdin_termios.c_lflag    &= ~ulong(ECHO);	// no echo on input
			new_stdin_termios.c_lflag    &= ~ulong(ECHONL);	// also no echo for nl on input
			new_stdin_termios.c_cc[VMIN]  = 1;			// min. input bytes for read(); note: 0 does not work with FILE* !!
			new_stdin_termios.c_cc[VTIME] = 0/*1*/;  	// max. wait time in 0.1s for read(); 0==off
			IFDEBUG(int err =) tcsetattr(0,TCSADRAIN,&new_stdin_termios);	// drain output, then change
			assert(err==0);
			SetBlocking(0,yes);
			assert(errno==ok);
		}
	}


// run test script ?
	if (test)				// vipsi -t
	{
		cstr fname = "00_test.vs";
		cstr pp[] =
		{
			"../Libs/test_suite/", 				// libs in installation directory
			"../../Libs/test_suite/", 			// libs in installation directory	(MacOS)
			"../../../Libs/test_suite/", 		// libs in installation directory	(MacOS)
			"/usr/local/lib/vipsi/test_suite/",	// system-wide libs
			"/opt/local/lib/vipsi/test_suite/",	// system-wide libs
			"~/.vipsi/test_suite/", 			// libs in user preferences
		};
		cstr p   = nullptr;
		time_t t = 0;
		for(uint j=0;j<NELEM(pp);j++)
		{
			pp[j] = FullPath(pp[j],1);
			if (!IsDir(pp[j])) continue;
			if (!IsFile(catstr(pp[j],fname))) continue;
			time_t tt = FileMTime(pp[j]);
			if (tt>t) { t = tt; p=pp[j]; }
		}
		errno=ok;
		if (!p)
		{
			logline("file \"test_suite/00_test.vs\" not found.");
			return 1;
		}
		else
		{
			argvFile = newcopy(catstr(p,fname));				// durable copy; memleak ignored
			logline("Loading test script \"%s\"",argvFile);
			chdir(p);
		}
	}

// shell mode ?
	else if (i==argc)
	{
		shell=yes;

		cstr fname = "shell.vs";
		cstr pp[] =
		{
			"Libs/",					// libs in project directory
			"../Libs/",					// libs in installation directory
			"../../Libs/", 				// libs in installation directory  (Mac OSX)
			"../../../Libs/", 			// libs in installation directory  (Mac OSX)
			"/usr/local/lib/vipsi/",	// system-wide libs
			"/opt/local/lib/vipsi/",	// system-wide libs
			"~/.vipsi/", 				// libs in user preferences
		};
		cstr p = nullptr;
		for(uint j=0;j<NELEM(pp);j++)
		{
			pp[j] = FullPath(pp[j],1);
			if (!IsDir(pp[j])) { xxlog("-D"); continue; }
			if (!IsFile(catstr(pp[j],fname))) { xxlog("-F"); continue; }
			p = pp[j];
		}
		errno=ok;
		if (!p)
		{
			logline("file \"shell.vs\" not found.");
			xlogline("pwd: %s",getwd(nullptr));
			return 1;
		}
		else
		{
			argvFile = newcopy(catstr(p,fname));				// durable copy; memleak ignored
			xlogline("Loading shell script \"%s\"",argvFile);
		}
	}

// run [cgi] script
	else
	{
		argvFile = argv[i++];
	}


// result cell
	Var* v = nullptr;

	if(pretend)
	{
		Compiler c;
		c.CompileFile(argvFile);
	}
	else
	{
	// create instances:
		Stream* io0 = new Stream( 0, stdinName  ? stdinName  : "stdin://" );
		Stream* io1 = new Stream( 1, stdoutName ? stdoutName : "stdout://" );
		Stream* io2 = new Stream( 2, stderrName ? stderrName : "stderr://" );

		VScript vscript( shell?rt_shell:rt_script,	// shell ?
						 String(argvName),			// invocation name
						 String(argvFile), 			// script
						 new Var(argv+i,argc-i),	// arguments to script
						 io0, io1, io2 );			// streams

	// *** DO IT ***
		if(errno==ok) { v = vscript.Execute(); if(v) v->Lock(); }
		cgi = vscript.RunType()==rt_cgi;
	}

// result:
	// bash: 126 = command found but not executable
	//		 127 = command not found
	//		 128+N command terminated by signal N
	//		 all other codes possible too!
	static const uint errno_notrepresentable  = 125;
	static const uint result_notrepresentable = 125;

	int result =
		errno
		?	uint(errno) < errno_notrepresentable
			? int(errno)
			: errno_notrepresentable
		:	v==nullptr
			? 0
			: v->IsNumber() && uint(v->Value()) < result_notrepresentable
				? int(v->Value())
				: result_notrepresentable;


// error ?
	if(errno)
	{
		cstr errstr = dupstr(errorstr());
		errno=ok;

		if(cgi)
		{
			cstr serverAdmin = getenv("SERVER_ADMIN");
			if ( !serverAdmin||strchr(serverAdmin,'@')==nullptr )
			{
				serverAdmin = "please inform someone who can help.";
			}
			else
			{
				serverAdmin = catstr(
					"please inform the system administrator <a href='mailto:", serverAdmin, "'>",
					serverAdmin, "</a>" );
			}
			printf(
			"content-type: text/html; charset=utf-8\n\n"
			"<!doctype html public \"-//w3c//dtd html 4.01 strict//en\">\n"
			"<html><head><title>vipsi error message</title></head>\n"
			"<body style='font-family:sans-serif;font-size:14pt;padding:30px;'>\n"
			"<table bgcolor='#ff7766' align=center cellpadding=30 border=2 width='90%%'>"
			"<tr><td align=center>"
			"<p>" applLongName " version " applVersion
			"<br>" applCopyright
			"<span style='font-size:10pt;'><br>vipsi - shell and script language: "
			"<a href='https://k1.spdns.de/Develop/Projects/vipsi/'>https://k1.spdns.de/Develop/Projects/vipsi/</a>"
			"</span>\n"
			"<p>An error occured while processing the script"
			"<br>%s:\n"
			"<p>%s\n"
			"<p style='font-size:10pt;'>Maybe the owner of this site is moving things around."
			"<br>If this problem persists over a prolongued time, e.g. more than an hour, "
			"<br>%s"
			"</td></tr></table></body>\n",
			tohtmlstr(quotedstr((argvFile))), tohtmlstr(errstr), serverAdmin
			);
		}
		else
		{
			logNl();
			logline( "%s", CString( VT_FGColor(catstr("while executing script ",quotedstr(argvFile),":"),VT_red) ) );
			logline( "%s", CString( VT_FGColor(errstr,VT_red) ) );
		}
	}


// stats:
	if (verbose)
	{
	#ifdef USE_KIOS_MALLOC
	#ifdef MALLOC_STATISTICS
		logline("  internal memory requests:     %li total", malloc_requests );
   		logline("  external memory allocations:  %li total", malloc_xrequests );
		logline("  internal memory still in use: %li",       MallocUsedBlocks() );
	#endif
	#endif

	#ifdef OPCODE_PROFILING
		logline("\ntokens defined: %i", int(tokens));

		logline("\nopcode singles:");
		for (int i=0; i<40; i++)
		{
			uint32 m=0;
			for (uint j=0;j<tokens;j++)
			{
				if (opcodeProfile[j]>opcodeProfile[m]) m=j;
			}
			logline ( "    %s%8u",
					 (TokenName(m)+"           ").LeftString(12).CString(),
					 opcodeProfile[m] );
			opcodeProfile[m]=0;
		}

		logline("\nopcode tupels:");
		for (int i=0; i<40; i++)
		{
			uint32 m=0,n=0;
			for (uint j=0;j<tokens;j++)
			for (uint k=0;k<tokens;k++)
			{
				if (opcodeTupels[j][k]>opcodeTupels[m][n]) { m=j; n=k; }
			}
			logline ( "    %s%s%8u",
					 (TokenName(m)+"           ").LeftString(12).CString(),
					 (TokenName(n)+"           ").LeftString(12).CString(),
					 opcodeTupels[m][n] );
			opcodeTupels[m][n]=0;
		}
	#endif
	}

	xlogline("/*main*/");
	return result;
}


















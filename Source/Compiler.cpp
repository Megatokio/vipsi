/*	Copyright  (c)	Günter Woigk 2003 - 2019
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

	2005-07-07 kio	rework Streams
	2005-07-10 kio	fixed tCONVERT_TO|FROM: v_down(1) for conversion type @<string>
*/

#define SAFE 3
#define LOG 0
#include	"kio/kio.h"
#include	"kio/peekpoke.h"
INIT_MSG
extern void InitHR();
static struct INIT_HR { INIT_HR(){InitHR();} } dummyname;

#define		TOKEN_PASS	1
#include	"token.h"
#define		TOKEN_PASS	2
#include	"token.h"
#define		TOKEN_PASS	3
#include	"token.h"
#define		TOKEN_PASS	4
#include	"token.h"
#define		TOKEN_PASS	5
#include	"token.h"
#define		TOKEN_PASS	6
#include	"token.h"
#define		TOKEN_PASS	7
#include	"token.h"

#include	"Compiler.h"
#include	"VScript.h"
#include	"Var/Var.h"
#include	<math.h>
#include	"Templates/Stack.h"
#include	"SystemIO.h"


// serrors.cpp:
inline	String	ErrorString	( int/*OSErr*/ e )				{ return errorstr(e); }
extern	String	ErrorString	( );
extern	void	ForceError	( int/*OSErr*/e, cString& msg );
inline	void	SetError	( int/*OSErr*/e, cString& msg )	{ if(errno==ok) ForceError(e,msg); }
inline	void	ForceError	( cString& msg )				{ ForceError(-1,msg); }
inline	void	SetError	( cString& msg )				{ if(errno==ok) ForceError(-1,msg); }
extern	void	AppendToError	( cString& msg );
extern	void	PrependToError	( cString& msg );


#ifndef TABWIDTH
#define TABWIDTH	4
#endif


const Double	d_pi	=	3.141592653589793238462;		// constant pi
const Double	d_ec	=	2.718281828459045235360;		// constant e



/* ----	disassemble proc/script ----------------------------
		used for Var::ToString( String(*disass_proc)(cVar&) )
*/
String DisassString ( cVar& /*q*/ )
{
	return "(( NIMP:DisassString() ))";		// TODO
#if 0
	XXTRAP(q.Csz()!=1);
	xxlog("Disass(cString&)");

	String s = TokenName(tPROC)+" ";
	uptr   p = (uptr)q.Text();
	uptr   e = p + q.Len();

	while ( p<e )
	{
		switch(*p)
		{
		case tNUM:		s += NumString( read_double(p+1) );			break;
		case tBYTE:		s += NumString( (int)(int8)p[1] );	break;
		case tSHORT:	s += NumString( (int)(int16)p[1] );	break;
		case tSTR:		s += String((ptr)p+3,p[2],CharSize(p[1])).ToQuoted(); break;
		case tSTR2:		s += String((ptr)p+4,*(uint16*)(p+2),CharSize(p[1])).ToQuoted();   break;
		case tIDF:		s += GetNameForHandle(*(NameHandle*)(p+1)).ToEscaped(); break;
		case tEOF:		s += TokenName(tEOF);						goto x;
		default:		s += TokenName(*p);							break;
		}
		s += String(' ');
		p=SkipToken(p);
	}

x:	XXXLogOut("");
	return s;
#endif
}

String TokenName ( uint t )
{
	return t<tokens ? tokenname[t] : CharString('$')+HexString(t);
}

String TokenInfo ( uint t )
{
	return t<tokens ? tokeninfo[t] : "not a valid token";
}

String TokenSyntax ( uint t )
{
	String z = String(' ')+(t<tokens?tokensyntax[t]:"")+' ';

	for ( int i=z.Len()-2; i>=1; i-- )
	{
		cstr r;
		switch(z[i])
		{
			case 'x':	r = "value";	break;
			case 'n':	r = "number";	break;
			case 't':	r = "text";		break;
			case 'l':	r = "list";		break;
			case 'v':	r = "variable";	break;
			case 'i':	r = "name";		break;
			default: 	continue;
		}
		if ( isalnum(z[i-1]) || is_letter(z[i+1]) ) continue;
		z = z.LeftString(i) + r + z.MidString(i+1);
	}

	return z.MidString(1,z.Len()-2);
}




enum { arg_void, arg_double, arg_namehandle, arg_long, arg_destination, arg_two_short };

struct Token
{
	uint32	rowcol;			// col + (1<<12) * row
	uchar	token;			// the token
	uchar	argument_type;
	union					// immediate argument
	{		int32		index;			// number or text literal, constant
			NameHandle	namehandle;		// identifiers
			uint32		destination;	// target rowcol for branch offset: if, else, loop, etc.
			int32		arguments;		// for tokens with varying amount of arguments
			int32		type;
			struct{int16 n,m;};
	}		argument;

			Token		( );
			~Token		( );
			Token		( const Token& q );
	Token&	operator=	( const Token& q );

			Token		( uint32 rowcol, uchar t );
			Token		( uint32 rowcol, uchar t, NameHandle nh );
			//Token		( uint32 rowcol, uchar t, int args );
			Token		( uint32 rowcol, uchar t, int32 args );
			Token		( uint32 rowcol, uchar t, int16, int16 );
};


cstr RowColStr(uint row, uint col)
{
	if(row)	return usingstr("at row %u, col %u:\n",row+1,col+1);
	else	return usingstr("at col %u: ",col+1);
}
String RowColString (uint row, uint col){ return RowColStr(row,col); }
String RowColString ( uint32 rowcol )	{ return RowColStr(Row(rowcol),Col(rowcol)); }
String RowColString ( Token* token )	{ return RowColString(token->rowcol); }
void SyntaxError 	( cstr s )			{ SetError(syntaxerror,s); }
void SyntaxError 	( cString& s )		{ SetError(syntaxerror,s); }




inline Token::Token ( )
{
	argument_type=arg_void;
}

inline Token::~Token ( )
{
	xxlogIn("~Token()");
	if(argument_type==arg_namehandle) UnlockNameHandle(argument.namehandle);
}

Token::Token ( const Token& q )
{
	token			= q.token;
	rowcol			= q.rowcol;
	argument		= q.argument;
	argument_type 	= q.argument_type;
	if(argument_type==arg_namehandle) LockNameHandle(argument.namehandle);
}

Token& Token::operator= ( const Token& q )
{
	assert(this != &q);

	if(argument_type==arg_namehandle) UnlockNameHandle(argument.namehandle);
	token			= q.token;
	rowcol			= q.rowcol;
	argument		= q.argument;
	argument_type 	= q.argument_type;
	if(argument_type==arg_namehandle) LockNameHandle(argument.namehandle);
	return *this;
}

inline Token::Token ( uint32 row_col, uchar t )
{
	rowcol			= row_col;
	token			= t;
	argument_type 	= arg_void;
}

Token::Token ( uint32 row_col, uchar t, NameHandle nh )
{
	rowcol				= row_col;
	token				= t;
	argument_type 		= arg_namehandle;
	argument.namehandle	= nh;
	LockNameHandle(nh);
}

/*Token::Token ( uint32 row_col, uchar t, int type )
{
	rowcol			= row_col;
	token			= t;
	argument_type 	= arg_long;
	argument.type 	= type;
}*/

Token::Token ( uint32 row_col, uchar t, int32 args )
{
	rowcol			= row_col;
	token			= t;
	argument_type 	= arg_long;
	argument.type 	= args;
}

Token::Token ( uint32 row_col, uchar t, int16 n, int16 m )
{
	rowcol			= row_col;
	token			= t;
	argument_type 	= arg_two_short;
	argument.n 		= n;
	argument.m 		= m;
}


void Compiler::resize_zbu()
{
	Token* nbu = new Token[zlen+1+zlen/2];		// make sure it actually grows!
	for(uint i=0;i<zlen;i++) nbu[i] = zbu[i];
	delete[] zbu; zbu = nbu; zlen += 1+zlen/2;
}

void Compiler::store_token ( Token const* q )
{
	if(zi==zlen) resize_zbu();
	zbu[zi++] = *q;
	xxlog(" %s",tostr(zbu+zi-1));
	v_delta(vstacksaldo[q->token]);
}

void Compiler::store_token ( Token const& q )
{
	if(zi==zlen) resize_zbu();
	zbu[zi++] = q;
	xxlog(" %s",tostr(zbu+zi-1));
	v_delta(vstacksaldo[q.token]);
}

void Compiler::store_token ( uint32 rowcol, uchar token )
{
	if(zi==zlen) resize_zbu();
	zbu[zi++] = Token(rowcol,token);
	xxlog(" %s",tostr(zbu+zi-1));
	v_delta(vstacksaldo[token]);
}

void Compiler::store_token ( uint32 rowcol, uchar token, int32 n )
{
	if(zi==zlen) resize_zbu();
	zbu[zi++] = Token(rowcol,token,n);
	xxlog(" %s",tostr(zbu+zi-1));
	v_delta(vstacksaldo[token]);
}

/*void Compiler::store_token ( uint32 rowcol, uchar token, int n )
{
	if(zi==zlen) resize_zbu();
	zbu[zi++] = Token(rowcol,token,n);
	xxlog(" %s",tostr(zbu+zi-1));
	v_delta(vstacksaldo[token]);
}*/

void Compiler::store_token ( uint32 rowcol, uchar token, int16 n, int16 m )
{
	if(zi==zlen) resize_zbu();
	zbu[zi++] = Token(rowcol,token,n,m);
	xxlog(" %s",tostr(zbu+zi-1));
	v_delta(vstacksaldo[token]);
}

void Compiler::store_token ( uint32 rowcol, uchar token, NameHandle n )
{
	if(zi==zlen) resize_zbu();
	zbu[zi++] = Token(rowcol,token,n);
	xxlog(" %s",tostr(zbu+zi-1));
	v_delta(vstacksaldo[token]);
}





/* ========================================================================================

									The Compiler

======================================================================================== */

void Compiler::init()
{
	sourcefile = cachefile = source = core = rowcolxref = emptyString;
	constants  = nullptr;
	bundle     = new Var(isList);	bundle->Lock();

	qa = qp = qe = nullptr;
	zbu = nullptr; zi = zlen = 0;

	rowcol = lfdnr = 0;
	v_now = v_max = r_now = r_max = 0;
}

void Compiler::kill()
{
	delete[]qa;
	bundle->Unlock();
}

void Compiler::Reset()
{
	kill();
	init();
}



int Compiler::add_constant(Double n)
{
	Var** a = constants->List().array;
	uint  l = constants->ListSize();
	for( uint i=l; i--; ) if(*(a[i])==n) return i;
	new Var(constants,l,n); return l;
}

int Compiler::add_constant(cString& s)
{
	Var** a = constants->List().array;
	uint  l = constants->ListSize();
	for( uint i=l; i--; ) if(*(a[i])==s) return i;
	new Var(constants,l,s); return l;
}

cstr Compiler::tostr(Token*tok)
{
	switch(tok->token)
	{
//	case tNOP:	return usingstr("(%u,%u)",Row(tok->rowcol),Col(tok->rowcol));
	case tNUM0:	return "0";
	case tNUM1: return "1";
	case tNUM:  return ::tostr((*constants)[tok->argument.index].Value());
	case tSTR:  return quotedstr(CString((*constants)[tok->argument.index].Text()));
	case tIDF:	return CString(GetNameForHandle(tok->argument.namehandle));
	default:	return CString(tokenname[tok->token]);
	}
}




/* ====	tokenize source text ============================================
		"+" 	preserved as operator+ tADD or used as number sign or omitted prefix operator
		"-"		preserved as operator- tSUB or used as number sign or stored as prefix operator tNEG
		"%"		preserved as operator% tMOD or binary number
		"++"	preserved as postfix operator++ tPOSTINCR or stored as prefix operator tPREINCR
		"--"	preserved as postfix operator-- tPOSTDECR or stored as prefix operator tPREDECR
		"else"	expanded to tELSE + tNOP		=> jump target of tIF.dist will be the nop
		"loop"	expanded to tLOOP + tNOP		=> jump target of tEXIT.dist etc. will be the nop
		"elif"	expanded to tELIF + tNOP + tIF	=> jump target of tIF.dist will be the nop
*/
void Compiler::Tokenize ( )
{
	assert(errno==ok);
	xlogIn("Compiler::Tokenize()");

	v_now = 99999;		// damit store_token nicht bummert
	r_now = 99999;

// convert to ucs4, append final char(0)
	source += CharString(0xFFFFFFFF);
	*(source.UCS4Text()+source.Len()-1) = 0;

// convert line breaks to nl
	{
		ucs4char* a = source.UCS4Text();
		ucs4char* e = a+source.Len() -1;
		ucs4char* z = a;
		while (z<e)
		{
			ucs4char c = *z++;
			if (c>13) continue;
			if (c!=0&&c!=10&&c!=13) continue;
			z[-1] = nl;
			if (c==0) continue;
			if (c+*z!=23) continue;

			// DOS text file detected: cr+lf -> nl  =>  text size shrinks, text moves
			*--z=c;
			for ( ucs4char* q=z; q<e; )
			{
				c = *q++;
				if (c!=10&&c!=13) { *z++ = c; continue; }	// move char
				q += c+*q==23; *z++ = nl;					// replace line break
			}
			*z++ = 0;
			source.Truncate(z-a);
			break;
		}
	}

	int32 qlen = source.Len() -1;
	const ucs4char*	qa = source.UCS4Text();
	const ucs4char*	qe = qa + qlen;
	const ucs4char*	qp = qa;
	const ucs4char*	a  = qp;

	zlen= qlen/10 +100;			// estimation
	zbu = new Token[zlen+3];	// note: 2 security spares for 'elif' and a final 'tEOF'
	zi	= 0;

/*	za,zp,ze,zlen  ->  destination	((zp ~ "ziel"))
	qa,qp,qe,qlen  ->  source    	((qp ~ "quelle"))
*/
				rowcol = 0;
	#define		nextrow()	rowcol  = ((rowcol>>12)+1)<<12
	#define		nexttab()	rowcol += TABWIDTH - Col(rowcol)%TABWIDTH
	uchar 		token;
	int   		comment = 0;				// block comment flag / block comment nesting level

	#define expect_operator 0				// mode: require none, expect value or operator
	#define require_value 	1				// mode: require value, reject operator
	bool  mode = require_value;				// mode: expect_operator <-> require_value


// skip line 1: "#!/usr/local/bin/vipsi" or the like
	if ( qa[0]=='#' && qa[1]=='!' ) { int32 n=source.Find('\n'); qp = n<0 ? qe : qa+n+1; }

// tokenize
	while( errno==ok )
	{
	// update col+row
		while ( a<qp )
		{
			if(*a=='\n') { nextrow(); } else
			if(*a=='\t') { nexttab(); } else
			{ rowcol++; } a++;
		}

	// find start of next word
		while ( a<qe && *a<=' ' )
		{
			if(*a=='\n') { nextrow(); } else
			if(*a=='\t') { nexttab(); } else
			{ rowcol++; } a++;
		}

	// end of source?
		if (a==qe) break;
		qp = a+1;

	// convert word -> token
		switch(*a)
		{
		#define T1(C,T)				case C: token=T; break;
		#define T2(C,D,T,U)			case C: if (*qp!=D) { token=T; } else { token=U; qp++; } break;
		#define T3(C,D,E,T,U,V)		case C: if (*qp==D) { token=U; qp++; } else if (*qp==E) { token=V; qp++; } else token=T; break;
		#define T4(C,D,E,F,T,U,V,W)	case C: if (*qp==D) { token=U; qp++; } else if (*qp==E) { token=V; qp++; } else if (*qp==F) { token=W; qp++; } else token=T; break;
		#define TG(C,D,DD,DGL,DDGL) case C: if (*qp=='=') { token=DGL; qp++; break; } if (*qp!=C)   { token=D; break; } qp++; if (*qp!='=') { token=DD; break; } qp++; token=DDGL; break;

		T1(',',tKOMMA)		T1(';',tSEMIK)		T1('(',tRKauf)		T1(')',tRKzu)
		T1('{',tGKauf)		T1('}',tGKzu)		T1('[',tEKauf)		T1(']',tEKzu)
		T1('?',tQMARK)		T1(':',tCOLON)		T1('$',tNUM)		T1('_',tIDF)
		T1('~',tTILDE)		T1('\'',tNUM)		T1('.',tDOT)		T1('@',tAT)
		T1('"',tSTR)		T1(0xab,tSTR)

		T2('^','=',tXOR,tXORGL)					T2('%','=',tMOD,tMODGL)
		T2('!','=',tNOT,tNE)					T2('=','=',tGL,tEQ)

		TG('|', tOR,tOROR,tORGL,tORORGL)
		TG('&', tAND,tANDAND,tANDGL,tANDANDGL)
		TG('>', tGT,tGTGT,tGE,tGTGTGL)
		TG('<', tLT,tLTLT,tLE,tLTLTGL)
		TG('#', tHASH,tHASHHASH,tHASHGL,tHASHHASHGL)

		T3('+','+','=',tADD,tPOSTINCR,tPLGL)
		T3('*','/','=',tMUL,tKEND,tMUGL)

		T4('-','-','=','>',tSUB,tPOSTDECR,tMIGL,tARROW)
		T4('/','/','*','=',tDIV,tKOMM,tKANF,tDVGL)

		default:
			if (ucs4::is_dec_digit(*a))	{ token=tNUM; break; }
			if (ucs4::is_letter(*a)) 		{ token=tIDF; break; }
			if (comment) continue;			// ill. chars e.g. '\' in comments are legal ...
			SetError ( String("Illegal character: '")+(*a<32?'?':*a)+"', ascii=$"+HexString(*a) );
			goto x;
		}

/*	Note:

	"+" "-" "%"	sind entweder ein OPERATOR oder Beginn einer ZAHL oder ein PREFIXOPERATOR (außer "%")

	Im Status "expect operator"
		werden "+" "-" und "%" als OPERATOR interpretiert und tADD, tSUB und tMOD gespeichert.
	Im Status "expect value"
		wird untersucht, ob eine ZAHL folgt:
		"%":	Folgt "0" oder "1" wird "%" als Einleitung einer Binärzahl interpretiert.
				Andernfalls ist hier ein Syntaxfehler.
		"+" und "-":
				Folgt keine Zahl:
					"+" und "-" werden als PREFIXOPERATOR interpretiert und tADD und tSUB gespeichert.
				Folgt eine Zahl:
					"+" und "-" werden als Vorzeichen interpretier und gehen in die Zahl ein.

	Zahlen:		123
				123e+4
				%1010
				$abcd
				'QW'

	=>			+123		-123
				+ + +123	- - -123	- + - + 123			(evtl.)
				+%1010		-%1010
				+$ab		-$ab
				+'QW'		-'QW'

	Der Tokenizer startet im Modus "require value"

	Ist das Token ein Non-Operator?  (=>Modus egal!)

		Ist das Token ein User-defined Word?
			=> Keine Info über Argumente!!
			=> Status wird auf "expect operator" gesetzt
			   In diesem Modus sind auch Non_Ops ok!

		Das Token ist ein Built-in Word:
			Instruction?             =>  Status "require value" beibehalten (Args egal: Instr have no return value!)
			Hat es Argumente?        =>  Status "require value" beibehalten
			Hat es keine Argumente?  =>  Status auf "expect operator" ändern

	Das Token ist ein Operator:

		Ist der Modus "expect operator" ?
			"++" und "--"  =>  Modus "expect operator" beibehalten. (Postfix operator)
			alle anderen   =>  Modus auf "require value" ändern.

		Ist der Modus "require value" ?
			"++" "--"		=> Prefixoperator: Status "require value" beibehalten (Prefix operator)
			"%"				=> Binärzahl
			"+" "-" 		=> Prefixoperator oder Vorzeichen für folgende Zahl
			"("				=> Status "require value" beibehalten
			alle anderen    => Syntaxfehler.
*/
		switch(token)
		{
		case tKOMM:		// comment to end of line starting with '//'
			while(qp<qe&&*qp!='\n') qp++;
			continue;

		case tKANF:		// block comment starting with '/*'
			comment++; store_token(rowcol,tKANF);
			continue;

		case tKEND:		// block comment ending with '*/'
			if(comment) { comment--; zi--; }
			else errno = errKEND;
			continue;

		default:			// single char word: operators
			if(comment) continue;
			if (mode==require_value && tokenprio[token]>pComma)
				SyntaxError( usingstr( "expected value, found %s",tokenname[token].ToQuoted().CString() ) );
			else
				store_token(rowcol,token);
			mode = require_value;
			continue;

		case tADD:
		case tSUB:
			if(comment) continue;
			if (mode==require_value)
			{
				ucs4char c = a[1];
				if (c=='$' || c=='\'' || c=='%' || ucs4::is_dec_digit(c)) goto number;
				if (token==tADD) continue;			// wirkungsloser prefix-operator "+"
				store_token(rowcol,tNEG);			// prefix operator "-"
				continue;							// mode = require_value;
			}
			store_token(rowcol,token);			// operator "+" or "-"
			mode = require_value;
			continue;

		case tPOSTDECR:	//	++ und -- gehen als pre- und postfix operatoren in beiden op. modi
		case tPOSTINCR:	//	und ändern den op. mode dabei nicht
			if(mode==require_value) token = token==tPOSTINCR ? tPREINCR : tPREDECR;	// prefix ++ and --
			if(!comment) { store_token(rowcol,token); } continue;

		case tHASH:		// "#" kann auch an Stelle eines Values vorkommen: put #2, text
			goto rka;	// danach dann auf jeden Fall ein value

		case tMUL:		// "*" kann auch an Stelle eines values kommen: del *  oder proc(*){}
			 goto rka;	// danach dann auf jeden Fall ein value; auch: ")"

		case tRKauf:	// "(" kann als prioritätssteuernde Klammer an Stelle eines Value stehen
	rka:				//     als Argumentenklammer aber auch an Stelle eine Operators
			if(comment) continue;
			store_token(rowcol,token);
			mode = require_value;
			continue;

		case tRKzu:		// )]} kommen idR an Stelle eines Operators
		case tEKzu:		// "}" als endofproc oder endofprivates kann aber auch nach instructions stehen
		case tGKzu:		// ")" kann aber auch im mode require_value stehen, und zwar bei: proc(){}
						// "]" kann aber auch im mode require_value stehen, und zwar bei: text[1 to]
			if(comment) continue;
			mode = expect_operator;
			store_token(rowcol,token);
			continue;

		case tSTR:		// string literal "text..", «text..», »text..«
		{				// a -> " « or »
			int32 n = a-qa;
			String s = source.StrVal( &n ); if (errno) goto x;
			qp = qa+n;
			if(comment) continue;
			store_token(rowcol,tSTR,add_constant(s));		//xxlog(" %s ",tostr(zp-1));
			mode = expect_operator;
		}	continue;

		case tNUM:		// number literal
number:	{	int32 n = a-qa;
			Double f = source.NumVal( &n ); if (errno) { if(!comment) goto x; errno=ok; continue; }
			assert(n != a-qa);
			qp = qa+n;
			if(comment) continue;
			if(f==0.0) store_token(rowcol,tNUM0); else
			if(f==1.0) store_token(rowcol,tNUM1); else
			store_token(rowcol,tNUM,add_constant(f));
			if( ucs4::is_letter(*qp) || *qp=='_' ) { rowcol += qp-a; errno = separatormissing; goto x; }
			mode = expect_operator;
		}	continue;

		case tMOD:
			if(comment) continue;
			if (mode==require_value) goto number;	// binary number: %0101
			store_token(rowcol,tMOD);				// operator %
			mode = require_value;
			continue;

		case tIDF:				// Identifier: reserved word, variable etc.
			while ( ucs4::is_letter(*qp) || ucs4::is_dec_digit(*qp) || *qp=='_' ) { qp++; }
			if(comment) continue;

		{	NameHandle nh = FindNameHandle(String(a,qp-a));	// unlocked NH

			if( nh && qp-a<=9							// max. token length currently: 8
				&& (zi==0 || zbu[zi-1].token!=tDOT) )	// ignore token names after tDOT  =>  'my_list.name' or 'foo.sin' possible
			{
				int tok = tFROM;		// from = first non-special-char word
				do { if (nh==tokennamehandle[tok]) break; } while (++tok<=tLOOP);

				if (tok<=tLOOP)			// found?
				{
				// check and set operator mode
					switch(tokenprio[tok])
					{
					case procOneArg:
					case procWithArg:
						store_token(rowcol,tok);
						mode = require_value;
						continue;

					case instruction:
						store_token(rowcol,tok);
						if(tok==tELIF) 		// elif expands to tELIF + tNOP + tIF
						{
							store_token(rowcol,tNOP);	// for jump target
							store_token(rowcol,tIF);
						}
						else if(tok==tELSE||tok==tLOOP||tok==tTHEN)
						{
							store_token(rowcol,tNOP);	// for jump target
						}
						mode = require_value;
						continue;

					case valueNoArg:
						mode = expect_operator;
					//	if(tok==tSTDIN)  *zp++ = Token(rowcol,tNUM0); else
					//	if(tok==tSTDOUT) *zp++ = Token(rowcol,tNUM1); else
					//	if(tok==tSTDERR) *zp++ = Token(rowcol,tNUM,add_constant(2.0)); else
						if(tok==tPI) 	 store_token(rowcol,tNUM,add_constant(d_pi)); else
						if(tok==tEC) 	 store_token(rowcol,tNUM,add_constant(d_ec)); else
						if(tok==tNEWLINE)store_token(rowcol,tSTR,add_constant(String('\n'))); else
						if(tok==tTAB) 	 store_token(rowcol,tSTR,add_constant(String('\t'))); else
										 store_token(rowcol,tok);
						continue;

					case pComma:
						store_token(rowcol,tok);
						if (tok==tTO)					// [value] to [value]  		both values optional
						{
							mode = require_value;		// wg: 2. opt. value: modus wird von "]" ignoriert
							continue;
						}
						if (tok==tFROM)					// value from value			both values required
						{
							if (mode==require_value) SyntaxError("expected value, found 'from'");
							mode = require_value;
							continue;
						}
						// error: goto default

					default:
						store_token(rowcol,tok);
						SetError( internalerror,String("internal error. token = ")+tokenname[tok].ToQuoted() ); goto x;
					}
				}
			}

		// unknown identifier; e.g. variable name
			if(nh) LockNameHandle(nh); else nh = NewNameHandle(String(a,qp-a));
			store_token(rowcol,tIDF,nh);
			UnlockNameHandle(nh);
			mode = expect_operator;
		}	continue;

		}	// switch token

	}		// while no error

x:	while (comment--)
	{
		zi--;
		assert(zbu[zi].token==tKANF);
		rowcol = zbu[zi].rowcol;
		errno = errKANF;	/* unterminated block comment */
	}

	store_token(rowcol,tEOF);
	assert(zi<=zlen);
	this->qa = zbu;
	this->qe = zbu+zi;
	zbu = nullptr;
}



/*	set branch target
	branch targets can only be:
		tNOP		inserted after else, elif and loop
		tDO			does nothing, is effectively a nop
		tTHEN		does nothing, is effectively a nop
	these nops are removed after optimization.
	branch targets must not move or vanish during reorder!
	branch targets' IDs -rowcol- must not be duped during reorder!
*/
void Compiler::SetDist ( Token* ip, Token* dest )
{
	assert(dest->token==tNOP);
	assert(dest->argument_type==arg_void || dest->argument_type==arg_destination);
	assert(ip->argument_type==arg_void);
	if(dest->argument_type!=arg_destination)
	{
		dest->argument.destination = lfdnr++;
		dest->argument_type        = arg_destination;
	}
	ip->argument.destination   = dest->argument.destination;
	ip->argument_type          = arg_long;
}


/* ----	set flow controls in matching range -------------------------------
		if ip points to an opening token,
			then run up to matching closing token
			else run until tEOF
		sets errno on error
		noerror: returns pointer behind closing token or to tEOF.
		error:   returns pointer to error position:
				 if matching closing token is not found, then returns position of opening token.
				 if unexpected intermediate or closing token is found, then this position is returned.

		set branch offsets in flow control tokens
		back branches are set immediately
		forward branches are remembered by building a linked list starting in ip1

		do0==nullptr: do0 and do1 are ignored
		else:      do0 and do1 are ip0 and ip1 of an outer do..loop
				   they are used to handle 'unexpected' next, until, while and exit for the outer loop

		brackets are crosslinked.
*/
Token* Compiler::set_offset ( Token* ip, bool indo )
{
	Token* ip0 = ip;				// ptr -> start token; e.g. tIF or tDO
	Token* ip1 = ip;				// ptr -> last forward branch token; e.g. tELSE  or  last tUNTIL/tWHILE/tEXIT
	uchar  c   = ip->token;			// opening token
	indo = indo || c==tDO;
	uchar  d;						// current token
	c = c==tEKauf ? tEKzu			// c := matching closing token
	  : c==tRKauf ? tRKzu
	  : c==tGKauf ? tGKzu
	  : c==tDO    ? tLOOP
	  : c==tIF    ? tTHEN
	  : c==tTRY   ? tTHEN
	  :             tEOF;


	xxlog(" <<[%s -> %s] ",CString(tokenname[ip->token]),CString(tokenname[c]));

	if (c!=tEOF) ip++;				// skip opening token

	for(;;)
	{
		d = ip->token;

		xxlog("%s ",tostr(ip));

		switch(d)
		{
		default:	ip++;
					continue;

		case tEKzu:	xxlog("[tEKzu]"); 	goto a;
		case tGKzu:	xxlog("[tGKzu]"); 	goto a;
		case tRKzu:	xxlog("[tRKzu]"); 	goto a;
				a:	if(c!=d) goto ue;
					xxlog(">> ");
					return ip+1;

		case tEOF:	xxlog("[tEOF]");
					if(c!=d) goto ue;
					xxlog(">> ");
					return ip;

		case tGKauf:xxlog("[tGKauf]"); goto b;
		case tIF:	xxlog("[tIF]"); 	goto b;
		case tTRY:	xxlog("[tTRY]");	goto b;
				b:	ip = set_offset(ip,indo);
					if(errno==ok) continue;
					return ip;

	//	elif is tokenized to 2 tokens: tELIF.dist + tIF.dist
	//	the part starting at tIF can be handeld as plain vanilla if … else … then
	// 	after that ip will point behind tTHEN an we insert the .dist for tELIF
		case tELIF:	xxlog("[tELIF]");
					if (c!=tTHEN) goto ue;			// expected closing token = tTHEN
					if (ip1!=ip0) goto ue;			// tELSE already encountered
					if (ip0->token!=tIF) goto ue;	// start token must be tIF  ((not tTRY))
					SetDist(ip0,ip+1);				// set tIF.dist -> tNOP behind tELIF
					ip1 = ip;						// remember tELIF
					ip = set_offset(ip+2,indo);		// handle subsequent tIF .. tTHEN
					SetDist(ip1,ip-1);				// set tELIF.dist -> tTHEN
					return ip;

		case tELSE:	xxlog("[tELSE]");
					if (c!=tTHEN) goto ue;
					if (ip1!=ip0) goto ue;		// unexpected 2nd tELSE
					if (ip0->token==tTRY) ip->token=tCATCH;	// --> try .. catch .. then
					SetDist(ip0,ip+1);			// ip+1 should be a nop
					ip1 = ip; ip++;
					continue;

		case tTHEN:	xxlog("[tTHEN]");
					if (c!=tTHEN) goto ue;
					if (ip1->token==tTRY) ip->token=tNOCATCH;	// --> try .. nocatch
					xxlog(">> ");
					SetDist(ip1,ip+1);					// set offset in tIF, tTRY or tELSE
					return ip+2;

		case tRKauf:xxlog("[tRKauf]"); goto c;
		case tEKauf:xxlog("[tEKauf]"); goto c;
		case tDO:	xxlog("[tDO]"); 	goto c;
				c:	ip = set_offset(ip,indo);
					if(errno==ok) continue;
					return ip;

		case tNEXT:	xxlog("[tNEXT]");	goto d;
		case tWHILE:xxlog("[tWHILE]");	goto d;
		case tUNTIL:xxlog("[tUNTIL]");	goto d;
		case tEXIT:	xxlog("[tEXIT]");	goto d;
				d:	if (!indo) goto ue;
					ip++;
					continue;

		case tLOOP:	xxlog("[tLOOP]");
					if (d!=c) goto ue;
					xxlog(">> ");
					SetDist(ip0,ip+1);					// set dist in tDO
					return ip+2;

		}	// switch token

		break;

	}	// loop over tokens

ue:	if(c==tEOF)	{ SetError( nestingerror, String("unexpected ") + tokenname[d].ToQuoted('\'') ); return ip;  }
	else 		{ SetError( nestingerror, tokenname[c].ToQuoted('\'') + " missing" );		 	 return ip0; }
}



inline bool Compiler::is_comma ( )
{
	if (qp->token!=tKOMMA) return no;
	qp++; return yes;
}

bool Compiler::expect_comma ( )
{
	if(errno) return no;
	if(is_comma()) return yes;
	SyntaxError("comma expected");
	return no;
}

bool Compiler::expect ( uchar token )
{
	if(errno) return no;
	if(qp->token==token) { qp++; return yes; }
	SyntaxError(tokenname[token].ToQuoted() + " expected");
	return no;
}


/*		<value>					->	<value>
		<name> = <value>		->	<value> tRENAME_NH.namehandle
		@<name> = <value>		->	<value> <name> tRENAME
*/
ResultClass Compiler::argument()
{
	ResultClass rc = rc_unknown;

	if( qp->token==tAT )		// test for: @<name> = <value>
	{
		Token* q0 = qp;
		int    z0 = zi;
		int    v0 = v_now;
		int    r0 = r_now;

		qp++; value(pDot);

		if(qp->token==tGL)
		{
			q0=qp; qp++; 	// q0 -> "="
			rc = value_temp(pComma);
			store_token( q0->rowcol, tSWAPARGS );
			store_token( q0->rowcol, tRENAME );		// <var|temp> <name> tRENAME -- <var|temp>
			rc = rc_temp;
		}
		else // "@" didn't start a computed name => store unnamed value
		{
			qp    = q0;
			zi    = z0;
			v_now = v0;
			r_now = r0;
			rc = value(pComma);
		}
	}
	else if( qp->token==tIDF && qp[1].token==tGL )		//	<idf> = <value>
	{
		Token* qp0 = ++qp; qp++;		// qp0 -> "="
		rc = value_temp(pComma);
		assert(qp0[-1].argument_type==arg_namehandle);		// 2005-07-04 kio:	was: XXXTRAP(qp0->argument_type==arg_namehandle);
		store_token( qp0->rowcol, tRENAME_NH, qp0[-1].argument.namehandle );	//	was: store_token( qp0->rowcol, tRENAME_NH, qp0->argument.namehandle );
		rc = rc_temp;
	}
	else
	{
		rc = value(pComma);
	}

	return rc;
}



/*		tIDF oder tAT		<idf>		-> token_idf.namehandle
							@<text>		-> <text> token_at
*/
void Compiler::idf ( TokenEnum token_idf, TokenEnum token_at )
{
	if(qp->token==tAT)
	{
		Token* qp0 = qp; qp++; value(pDot);
		store_token( qp0->rowcol, token_at );
	}
	else if(qp->token==tIDF)
	{
		store_token( qp->rowcol, token_idf, qp->argument.namehandle ); qp++;
	}
	else SyntaxError("name expected");
}


/*		@<name>		-> <value>    und return: -1
		<idf>		->                return: code
*/
int Compiler::nickname ( int(*conv_proc)(String) )
{
	if(qp->token==tAT)
	{
		qp++; value(pDot);
	}
	else if(qp->token==tIDF)
	{
		String nick = GetNameForHandle(qp->argument.namehandle); qp++;
		return (*conv_proc)(nick);	// setzt ggf errno
	}
	else SyntaxError("nickname expected");
	return -1;
}


/*		new var globals.anton[<value>,<value>].@(<value>)
		parses the path, creates the var and returns the varref on stack
*/
void Compiler::newvar ( bool mustbenew )
{
	if( qp->token==tGLOBALS || 		// gibts immer
		qp->token==tLOCALS || 		// gibts immer
		qp->token==tROOT || 		// gibts immer
		qp->token==tTHIS || 		// [new] [var] this.<idf> darf ggf. bummern
	//	qp->token==tERROR ||		// ***TODO*** to be improved
		qp->token==tENV )			// ***TODO*** to be improved
	{
		store_token(qp++);			// existing starting point for new var path
		if(qp->token!=tDOT && qp->token!=tEKauf)
			SyntaxError("\".<name>\" or \"[<index>]\" expected");
	}
	else
	{
		store_token(qp->rowcol, tLOCALS);
		goto dot;
	}
	while(errno==ok)
	{
		if(qp->token==tDOT)   { qp++; dot: idf(tVARIDF,tVARAT); continue; } else
	//	if(qp->token==tRKauf) { NIMP(); /* TODO */ } else
		if(qp->token==tEKauf) { qp++; value(pComma); store_token(qp->rowcol,tVARIDX); expect(tEKzu); }
		else break;
	};
	if( errno==ok && mustbenew )
	{
		uchar& token = zbu[zi-1].token;
		if(token==tVARIDF) 	token=tNEWIDF; else
		if(token==tVARAT) 	token=tNEWAT;  else
		if(token==tVARIDX) 	token=tNEWIDX; else	IERR();
	}
}


void Compiler::procliteral ( Token* qp0 )
			// proc literal: proc ( <arguments> ) { <statements> }
			//	code:		 tPROC.dist.n.m <n*namehandle> <statements> -- <temp>
			// 				 dist = dist to skip proc body; n = args total; m = args before '*'
			// gespeichert werden erst mal: proc.dest, xx.n, xx.m, n* xx.idf, statements, nop
{
			// in:  qp0 -> tPROC
			//		qp  -> "("

	int n  =  0; 		// argc
	int m  = -1;		// idx of '*'
	int z0 = zi;		// -> tPROC
	store_token(qp0);	// tPROC
	store_token(qp0->rowcol,tRKauf);	// .n		((stack-neutraler opcode!!))
	store_token(qp0->rowcol,tRKzu);		// .m		((stack-neutraler opcode!!))
	expect(tRKauf);
	if(errno) return;

	if(qp->token!=tRKzu)
	{
		do
		{
			if(qp->token==tIDF)
			{
				store_token(qp); v_down(1);
				assert(qp->argument_type==arg_namehandle);
				qp++; n++;
				continue;
			}
			else if(qp->token==tMUL)
			{
				if(m==-1) { qp++; m=n; continue; }
				SyntaxError("multiple '*' in argument list");
				return;
			}
			else
			{
				SyntaxError("name or ')' expected");
				return;
			}
		}
		while( is_comma() );
	}

	zbu[z0+1].argument.arguments = n;	zbu[z0+1].argument_type = arg_long;
	zbu[z0+2].argument.arguments = m;	zbu[z0+2].argument_type = arg_long;

	expect(tRKzu);
	expect(tGKauf);

	int vm = v_max; v_max = 0;
	int vn = v_now; v_now = 0;
	int rm = r_max; r_max = 0;
	int rn = r_now; r_now = 0;
	int z1 = zi;
	store_token(qp[-1].rowcol, tCHECKSTACK, 123);
	execute();							// <statements>
	assert(!(v_now && errno==ok));
	assert(!(r_now && errno==ok));
	zbu[z1].argument.arguments = v_max+r_max;
	xlog(" proc:v=%i,r=%i;",(int)v_max,(int)r_max);
	v_max = vm;
	v_now = vn;
	r_max = rm;
	r_now = rn;

	store_token(qp->rowcol,tRETURN0);	// return
	store_token(qp->rowcol,tNOP);		// nop = target for tPROC.dist
	SetDist(zbu+z0,zbu+zi-1);
	expect(tGKzu);
}


/*	syntax:	[ <tid> = ] spawn <proc> ( <arguments> ) [ prio <value ]
	code:	<proc:value> <n*temp> <prio:value> tSPAWN.n.m <m*opcode> -- <tid:temp>
*/
ResultClass Compiler::spawn()
{
	Token* qp0 = qp-1;
	assert(qp0->token==tSPAWN);

// proc:
	value(pPostfix);		// proc
	store_token(qp->rowcol,tREQPROC);

// arguments:
	expect(tRKauf);
	int n = 0;
	if(qp->token!=tRKzu)
		do
		{ 	ResultClass rc = argument(); n++;
			if(rc!=rc_temp) store_token(qp->rowcol,tREQTEMP);
		}
		while(is_comma());
	expect(tRKzu);
	if(errno) return rc_unknown;

// prio:
	if(qp->token==tPRIO)
	{
		qp++;
		value(pUna);
	}
	else
	{
		store_token(qp0->rowcol,tSELF);
		store_token(qp0->rowcol,tPRIO);		// own prio
	}

// spawn:
	store_token(qp0->rowcol,tSPAWN,n,3);	// tSPAWN.n.m
	v_down(n-1);		// n down aber -1 weil tEXECINSTR nochmal down 1

// code for spawned child:
	store_token(qp0->rowcol,tEXECINSTR,n);
	store_token(qp0->rowcol,tRETURN0);

	return rc_temp;
}


/* ----	compile source -----------------------
		compile up to errno, tEOF or tGKzu.
		return qp -> offending instruction
*/
void Compiler::execute( )
{
	xlogIn("Compiler::execute()");
		assert(v_now==0);
	//	assert(r_now==0);

	while( errno==ok )
	{
		assert(qp<qe);

		uint   z0;
		Token* qp0 = qp++;
		uchar  tok = qp0->token;
		int32 n;
		ResultClass rc;

		xxlog("  | ");

		IFDEBUG( store_token(qp0->rowcol,tCHECKSTACKS); )
		assert(v_now==0);
	//	assert(r_now==0);

		switch(tok)
		{
		default:		// e.g.:	min(a,b) = c
			qp--;
			rc = value(pAssign); if(errno) goto x;				// TODO: <text>[<idx>] = ... usw.
			{	uchar ztok = zbu[zi-1].token;

				if(ztok==tPOSTINCR||ztok==tPOSTDECR)						// Quick add/sub 1:  <idf>++  or  <idf>--
				{
					zbu[zi-1].token = ztok==tPOSTINCR ? tPREINCR : tPREDECR;	// same effect here but faster
					store_token(qp->rowcol,tDROP);
					continue;
				}
				if(ztok==tEXECPROC && tokenprio[qp->token]!=pAssign)		// procedure call:  <idf>(<arguments>)
				{
					zbu[zi-1].token = tEXECINSTR; v_down(1);
					continue;
				}
			}
			// now only assignments left:

			tok = qp->token;

			if(tok==tARROW)
			{
				// dem linksseitigen Ausdruck wird tNODEREF nachgestellt,
				// was tIDF etc. in Execute() testen und dann ggf. nicht dereferenzieren.
				// Das wäre nicht nötig, wenn der linke Ausdruck als 2. ausgewerted würde (right-to-left).
				// Dann könnte man direkt auf tARROW testen.

				if(rc!=rc_var) { errno = varreq; goto x; }
				store_token(qp->rowcol,tNODEREF);
				qp0=qp; qp++;
				rc = value_var(pComma);
				store_token(qp0);
				continue;
			}

			if(tokenprio[tok]!=pAssign)
			{
				// if(rc==rc_var||rc==rc_unknown) SyntaxError("assignment operator expected"); else
				SyntaxError(/*expression yields value*/"instruction missing");		// rc_temp, rc_const & no assignment follows: what the heck.. ?
				goto x;
			}
			if(rc!=rc_var)
			{
				if(rc!=rc_unknown) { errno = varreq; goto x; }
				store_token(qp->rowcol,tREQVAR);
			}

			qp0=qp; qp++;							// qp0 -> operator

			if(tok==tANDANDGL||tok==tORORGL)		// --> pruning
			{
				store_token( qp0->rowcol, tDUP );

				z0 = zi;
				store_token(qp0->rowcol,tok==tORORGL?tOROR:tANDAND);
				value(pComma);
				store_token( qp0->rowcol, tNOP );		// jump target
				SetDist(zbu+z0,zbu+zi-1);

				store_token( qp0->rowcol, tREQTEMP );
				store_token( qp0->rowcol, tBOOL );		// <var|temp> -- <var|temp>

				store_token( qp0->rowcol, tGL );
			}
			else if(tok==tGL||tok==tHASHHASHGL)
			{
				rc = value_temp(pComma);
				store_token(qp0);
			}
			else
			{
				rc = value(pComma);
				store_token(qp0);
			}
			continue;

		case tEOF:
			qp--; goto x;

		case tGKzu:
			qp--; goto x;

		case tSEMIK:	//	;
		case tTHEN:
			continue;

		case tNOP:		//	((jump_target))
			store_token(qp0);
			continue;

		case tRETURN:	//	return [<value>]
		{	z0 = zi;
			int vn = v_now;
			int rn = r_now;
			value(pComma);
			if (errno) { errno=ok; v_now=vn; r_now=rn; qp=qp0+1; zi=z0; store_token(qp0->rowcol,tRETURN0);  }
			else store_token(qp0);
		}	continue;

		case tEND:		//	end [<value>]
		{	z0 = zi;
			int vn = v_now;
			int rn = r_now;
			value(pComma);
			if (errno) { errno=ok; v_now=vn; r_now=rn; qp=qp0+1; zi=z0; store_token(qp0->rowcol,tNUM0);  }
			store_token(qp0);
		}	continue;

		case tDO:		//	do.dist
			r_up(RCountDo);
			goto stq0;

		case tLOOP:		//	loop
			r_down(RCountDo);
			goto stq0;

		case tTRY:		//	try
			r_up(RCountTry);
			goto stq0;

		case tNOCATCH:	//	try .. then
		case tCATCH:	//	try .. catch.dist .. then
			r_down(RCountTry);
			goto stq0;

		case tEXIT:		//	exit
		case tNEXT:		//	next
		case tELSE:		//	else.dist
		case tELIF:		//	elif <value>	splitted in:  tELIF.dist tIF.dist <value>
stq0:		store_token(qp0);
			continue;

		case tGKauf:	//	"{"
			store_token(qp0);
			r_up(RCountGKauf);
			execute();
			r_down(RCountGKauf);
			assert(!(v_now && errno==ok));
		//	XXXTRAP(r_now&&errno==ok);
			store_token(qp);
			expect(tGKzu);
			continue;

		// <value> token --
		case tIF:		//	if.dist <value>
		case tWHILE:	//	while <value>
		case tUNTIL:	//	until <value>
		case tSORT:		//	sort <variable>
		case tRSORT:	//	rsort <variable>
		case tSHUFFLE:	//	shuffle <variable>
		case tCD:		//	cd <value>
		case tWAIT:		//	wait <value>
		case tSHEDULE:	//	shedule <value>
		case tSUSPEND:	//	suspend <value>
		case tRESUME:	//	resume <value>
		case tTERMI:	// 	termi <value>
		case tREQUEST:	//	request <value>
		case tRELEASE:	//	release <value>
		case tTRIGGER:	//	trigger <value>
			value(pComma);
			store_token(qp0);
			continue;

		case tCALL:		//	call <value> [, ...]
			n=-1; do { n++; value(pComma); } while(is_comma());
			if(n>255) SetError("too many arguments");
			store_token(qp0->rowcol,tok,n); v_down(n); r_up(RCountCall); r_down(RCountCall);
			store_token(qp0->rowcol,tDROP);
			break;

		case tEVAL:		//	eval <value> [, ...]		// <value> tEVAL -- <value>
			tok = tEXE;
		case tINCLUDE:	//	include <value> [, ...]		// 	<value> tINCLUDE -- <value>
		case tREQUIRE:	//	require <value> [, ...]
			do
			{	value(pComma);
				store_token(qp0->rowcol,tok);
				store_token(qp0->rowcol,tDROP);
			}
			while( is_comma() );
			r_up(RCountEvalOrInclude); r_down(RCountEvalOrInclude);
			continue;

		//	code:	<value> <n*temp> <value> tSPAWN.n.m <m*opcode> -- <temp>
		//	syntax:	spawn <idf>(<arguments>) [prio <value>]
		case tSPAWN:
			spawn();
			store_token(qp0->rowcol,tDROP);
			continue;

		case tSWAP:		//	swap <variable>, <variable>
			value_var(pComma);
			expect_comma();
			value_var(pComma);
			store_token(qp0);
			continue;

						//	<temp|var> <value> tJOIN  --  <temp|var>
		case tJOIN:		//	join <variable> , <value>
			value_var(pComma);
			expect_comma();
			value(pComma);
			store_token(qp0);
			store_token(qp0->rowcol,tDROP);
			continue;

						//	<temp|var> [<value>] tSPLIT -- <temp|var>
		case tSPLIT:	//	split <variable> [,<value>]
			value_var(pComma); n = 0;
			if(is_comma()) { value(pComma); n = 1; }
			store_token(qp0->rowcol,tSPLIT,n);
			store_token(qp0->rowcol,tDROP);
			v_down(n);
			continue;

						//	<temp|var> <value> <value> tREPLACE -- <temp|var>
		case tREPLACE:	//	replace <variable>, <value>, <value>
			value_var(pComma);
			expect_comma();
			value(pComma);
			expect_comma();
			value(pComma);
			store_token(qp0);
			store_token(qp0->rowcol, tDROP);
			continue;

						//	<temp|var>         tCONVERT_TO.nn  --  <temp|var>
						// 	<temp|var> <value> tCONVERT_TO.ff  --  <temp|var>
		case tCONVERT:	//	convert <variable> to|from <idf> [to|from ...]
			value_var(pComma);
		{
			bool f=qp->token==tFROM;
			bool t=qp->token==tTO;
			if(!(f||t)) { SyntaxError("'from' or 'to' expected"); return; }
			do
			{	qp++;
				n = nickname( GetEncodingFromName );
				store_token( qp0->rowcol, f?tCONVERT_FROM:tCONVERT_TO, n );
				if(n==-1) v_down(1);
				f=qp->token==tFROM;
				t=qp->token==tTO;
			}
			while( errno==ok && (t||f) );
		}
			store_token( qp0->rowcol,tDROP );
			continue;

		case tFREEZE:	//	freeze [var] <variable> [, ...]
						//	freeze file <value> [, ...]
		case tMELT:		//	melt [var] <variable> [, ...]
						//	melt file <value> [, ...]
			do
			{
				if(qp->token==tFILE)
				{	TODO();
					Token* qp1 = qp; qp++;
					value(pComma);
					store_token(qp1);
					store_token(qp0);
				}
				else
				{
					if(qp->token==tVAR) qp++;
					value(pComma);
					store_token(qp0);
				}
			}
			while(is_comma());
			continue;

		case tGETCHAR:	//	getchar [#n,] <variable> [, ...]	-->		<stream> <var>          tGETCHAR -- <stream>
		case tGET:		//	get     [#n,] <variable> [, ...]	-->		<stream> <var> <flag:0> tGET	 -- <stream>
		case tEDIT:		//	edit    [#n,] <variable> [, ...]	-->		<stream> <var> <flag:0> tEDIT	 -- <stream>
		case tREAD:		//	read    [#n,] <variable> [, ...]	-->		<stream> <var> <flag:0> tREAD	 -- <stream>
			if(qp->token!=tHASH)
				store_token( qp0->rowcol,tSTDIN );			// <stream>
			else
			{
				qp++; value(pComma);						// <stream>
				expect_comma();
			}
			do
			{	value(pComma);
				if(tok!=tGETCHAR) store_token( qp0->rowcol,tNUM0 );		// flag: 0 = initial call
				store_token( qp0->rowcol,tok );							// token tGET, tGETCHAR, tREAD, tEDIT
			}
			while(is_comma());
			store_token( qp0->rowcol,tDROP );							// drop stream#
			continue;

						// 	<value> <n*value> tSYS.n -- <temp>
		case tSYS:		// 	sys  [#n,] <value> [, ...]
			if(qp->token!=tHASH)
				store_token( qp0->rowcol,tSTDOUT );
			else
			{
				qp++;
				value(pComma);		// stream for stdout
				expect_comma();
			}
			value(pComma);			// filename
			store_token(qp->rowcol,tREQSTRING);
			for( n=0; is_comma(); n++ )
			{
				value(pComma);		// optional arguments
			}
			store_token( qp0->rowcol,tSYS,n );	// tSYS.n
			v_down(n);
		//	store_token( qp0->rowcol,tWAITOUTPUT );	// wait output free
			store_token( qp0->rowcol,tPUT   );	// print result
			store_token( qp0->rowcol,tDROP  );	// drop stream#
			continue;

						//	<stream> <value> tLOG -- <stream>
						//	<stream> <value> tPUT -- <stream>
						//	<stream> <value> tWRITE -- <stream>
						//	<stream> <value> tPUTBACK -- <stream>
		case tLOG:		// 	log [#n,] <value> [, ...]
		case tWRITE:	//	write [#n,] <value> [, ...]
		case tPUT:		//	put [#n,] <value> [, ...]
		case tPUTBACK:	//	putback [#n,] <value> [, ...]
			if(qp->token!=tHASH)
				store_token( qp0->rowcol,tok==tLOG?tSTDERR:tok==tPUTBACK?tSTDIN:tSTDOUT );
			else
			{
				qp++; value(pComma);
				expect_comma();
			}
			do
			{	value(pComma);
			//	store_token( qp0->rowcol,tWAITOUTPUT );	// wait output free
				store_token( qp0->rowcol,tok==tLOG?tPUT:tok );		// token or tPUT for tLOG
			}
			while(is_comma());
			store_token( qp0->rowcol,tDROP );			// drop stream#
			continue;

		case tCLOSE:	//	close [#n]
			if(qp->token!=tHASH)
				store_token( qp0->rowcol,tSTDOUT );
			else
			{
				qp++;
				value_var(pComma);
			}
			store_token(qp0);
			continue;

						//	<stream:var> <value> tOPEN --
		case tOPENIN:	//	open [#n,] <value>
		case tOPEN:		//	open [#n,] <value>
		case tOPENOUT:	//	open [#n,] <value>
			if(qp->token!=tHASH)
				store_token( qp0->rowcol, tok==tOPENIN?tSTDIN:tSTDOUT );
			else
			{
				qp++;
				value_var(pComma);
				expect_comma();
			}
			value(pComma);
			store_token(qp0);
			continue;

		case tDEL:		//	del [var] <variable> [, ...]
			do			//	del file|link|dir <value>
			{
				Token* qp1=qp; qp++;
				tok = qp1->token;

				if( tok==tFILE || 	// del file <filename>
					tok==tLINK ||
					tok==tSOCK ||
					tok==tPIPE ||
					tok==tDIR )
				{
					value(pComma);
					store_token(qp1);		//	prefix tFILE, tDIR, tLINK
					store_token(qp0);		//	tDEL
				}
				else if(tok==tMUL)	//	del *
				{
					store_token(qp1->rowcol,tLOCALS);
					store_token(qp1->rowcol,tLIST,0);
					store_token(qp0->rowcol,tGL);
				}
				else						//	del <var>
				{							//	del var <var>
					if(tok!=tVAR)  qp--;
					value_var(pComma);
					store_token(qp0);		// tDEL
				}
			}
			while(is_comma());
			continue;


		case tRENAME:	//	rename [var] <var> = <value>
			do			//	rename file|dir|link <filename> = <value>
			{
				Token* qp1=qp; qp++;
				tok = qp1->token;

				if( tok==tFILE || 		// rename file <oldname> = <newname>
					tok==tLINK || 		// rename link <oldname> = <newname>
					tok==tSOCK || 		// rename sock <oldname> = <newname>
					tok==tPIPE || 		// rename pipe <oldname> = <newname>
					tok==tDIR ) 			// rename dir  <oldname> = <newname>
				{
					value(pAssign);				//	old filename
					expect(tGL);
					value(pAssign);				//	new filename
					store_token(qp1);			//	prefix tFILE, tDIR, tLINK
					store_token(qp0);			//	tRENAME
					v_down(1);	// tFILE tRENAME dropt 2, tRENAME nur 1
				}
				else							//	rename <var> = <text>
				{								//	rename var <var> = <text>
					if(tok!=tVAR)  qp--;
					value_var(pAssign);			//	<var>
					expect(tGL);
					value(pComma);				//	<newname>

					if(zbu[zi-1].token==tSTR)	// 	rename <var> = "text"
					{							//	-->  use of tRENAME_NH possible  ((->Optimize()?))
						zi--; v_down(1); xxlog(" <--backup*1<--");
						NameHandle nh = NewNameHandle( (*constants)[zbu[zi].argument.index].Text() );
						store_token( qp0->rowcol, tRENAME_NH, nh );
						UnlockNameHandle(nh);
						// TODO: textkonstante wieder löschen, wenn nachweislich nur hierfür erzeugt
					}
					else
					{
						store_token( qp0 );		// tRENAME
					}
					store_token(qp0->rowcol,tDROP);		// tRENAME[_NH] leaves <var|temp> on stack
				}
			}
			while(is_comma());
			continue;


		case tIOCTL:	//	ioctl [#n,] <idf> = <value> [, ...]
						//	ioctl [#n,] <variable> = <idf> [, ...]
			errno = notyetimplemented; break;

		case tDIR:		//	syntax:	[new] dir <value>
						//	code:	<value> tDIR tNEW|tVAR
	td:		value(pComma);
			store_token(qp0);			// tDIR
			store_token(qp0->rowcol,tok==tNEW?tNEW:tVAR);
			v_down(1);
			continue;

		case tLINK:		//	syntax:	[new] link <value> = <value>
						//	code:	<value> <value> tLINK tNEW|tVAR
	tl:		value(pAssign);
			expect(tGL);
			value(pComma);
			store_token(qp0);			// tLINK
			store_token(qp0->rowcol,tok==tNEW?tNEW:tVAR);
			v_down(2);
			continue;

		case tFILE:		//	syntax:	new file <value> [ = <value>]
						//	code:	<value> <value> tFILE tNEW
						//	syntax:	file <value> = <value>
						//	code:	<value> <value> tFILE tVAR
						//	syntax:	file <value> [ #= <value> ]
						//	code:	<value> <value> tFILE tHASHGL
	tf:		value(pAssign);
			if(qp->token==tGL)
			{
				qp++; value(pComma);
				store_token(qp0);					// tFILE
				store_token(qp0->rowcol,tok==tNEW?tNEW:tVAR);
				v_down(2);
			}
			else if( qp->token==tHASHGL )
			{
				qp++; value(pComma);
				store_token(qp0);					// tFILE
				store_token(qp0->rowcol,tHASHGL);
			}
			else if( tok!=tNEW )
			{
				store_token( qp0->rowcol, tSTR, add_constant(emptyString) );
				store_token(qp0);					// tFILE
				store_token(qp0->rowcol,tHASHGL);
			}
			else
			{
				store_token( qp0->rowcol, tSTR, add_constant(emptyString) );
				store_token(qp0);					// tFILE
				store_token(qp0->rowcol,tNEW);
				v_down(2);
			}
			continue;

		case tSOCK:
	ts:		errno = notyetimplemented;
			return;

		case tPIPE:
	tpi:	errno = notyetimplemented;
			return;

		case tNEW:		//	new [ |var|file|dir|link|proc] <var|name> [= <value>]
			qp0++; qp++;
			if(qp0->token==tDIR)  goto td;
			if(qp0->token==tLINK) goto tl;
			if(qp0->token==tFILE) goto tf;
			if(qp0->token==tSOCK) goto ts;
			if(qp0->token==tPIPE) goto tpi;
			if(qp0->token==tPROC) goto tp;

			if(qp0->token!=tVAR) { qp--; qp0--; }
			// goto tVAR

		case tVAR:		//	[new] [var] <variable> [= <value>] [, ...]
			do
			{	newvar(tok==tNEW); if(errno) break;
				if(qp->token==tGL)
				{
					qp0=qp; qp++;
					rc = value(pComma);
					if(rc!=rc_temp) store_token(qp0->rowcol,tREQTEMP);
					store_token(qp0->rowcol,tGL);
				}
				else if(qp->token==tARROW)
				{
					qp0=qp; qp++;
					rc = value(pComma);
					if(rc!=rc_var)
					{	if(rc==rc_unknown) store_token(qp0->rowcol,tREQVAR);
						else { errno = varreq; goto x; }
					}
					store_token(qp0->rowcol,tARROW);
				}
				else
				{
					store_token( qp0->rowcol,tDROP );
				}
			}
			while(is_comma());
			continue;

		case tPROC:		//	syntax:	[new] proc <variable> ( [<arguments> ) { <statements> }
						//	code:	tPROC.dist.n.m <n*namehandle> <statements> -- <temp>
						// 			dist = dist to skip proc body; n = args total; m = args before '*'
		tp:	newvar(tok==tNEW);					// 	<var>
			procliteral(qp0);					//	<proc>
			store_token(qp0->rowcol,tGL);		// 	<var> = <procvalue>
			continue;

		}	// switch token

	}	// loop

x:	;
}




/* ----	wg. monadic operators -----------------------------
		Es ist die Frage, ob man die Argumente
		mit SingleNumArg(..) oder NumValue(..,pUna)
		auswerten sollte.
		Der Unterschied ist, ob Klammern um das Argument als Funktionsklammern verstanden werden,
		und dann ist nach ")" Schluss, oder ob Klammern als prioritätssteuernde Klammern verstanden
		werden, und dann wird () [] und . nach ")" noch zum Argument gerechnet.
		Beispiele:
		-number			so soll es sein:            (), [], . alle danach syntax error; also egal
		-text			wird -> number gecastet:	[] kann nur zum Argument gehören
		-proc			anwendung auf rückgabewert: () muss zum Argument gehören
		-list			rekursive anwendung: 		[] sollte zum Argument gehören

						+, ~, !, ++, -- entsprechend

		+ - ! ~ ++ --	=> Klammern werden als Prioritätsklammern verstanden.
						=> Verwendung von NumValue(..,pUna)

		a[i]			i wirkt immer auf den inhalt von a (und nicht auf den namen "a")
		a.b[i]			i wirk immer auf a.b (und nicht auf "b")

		a.@"b"[i]		soll i auf "b" wirken, geht das so
		a.@("b"[i])		oder so

		(a.@"b")[1]		soll i auf a.@"b" wirken, geht das so
		a.@("b")[1]		oder, wenn die Klammern proc- und keine prio-Klammern sind
						da dies der üblichen Schreibweise a.b[i] am nächsten kommt,
						ist diese Interpretation erwünscht.

		@				=> Klammern werden als Funktionsklammern verstanden.
						=> Verwendung von SingleStrValue()
*/


/* ----	evaluate expression -----------------------------------------------------
		in:  v -> temp cell
		out: if result=value			 : v-> temp cell && value in temp cell
			 if result=reference		 : v-> variable
			 if pSemicolon && operator= : v=nullptr ((don't print result in shell))
*/
ResultClass Compiler::value_var ( int prio )
{
	IFDEBUG(
	int rn = r_now;
	int vn = v_now;
	)

	ResultClass rc = value(prio);

	IFDEBUG(
	assert(r_now==rn+0||errno);
	assert(v_now==vn+1||errno);
	)

	if(errno==ok&&rc!=rc_var)
	{
		if(rc!=rc_unknown) errno = varreq;
		else store_token(qp->rowcol,tREQVAR);
	}
	return rc_var;
}


ResultClass Compiler::value_temp ( int prio )
{
	IFDEBUG(
	int rn = r_now;
	int vn = v_now;
	)

	ResultClass rc = value(prio);

	IFDEBUG(
	assert(r_now==rn+0||errno);
	assert(v_now==vn+1||errno);
	)

	if(errno==ok&&rc!=rc_temp)
	{
		store_token(qp->rowcol,tREQTEMP);
	}
	return rc_temp;
}


ResultClass Compiler::value ( int prio )
{
	if (errno) return rc_unknown;
	xxlog(" a{%i:",v_now);

// ---- expect value ----

	assert(qp<qe);
	uint   z0;
	Token* qp0 = qp++;
	uchar tok = qp0->token;
	int32 n;
	ResultClass rc = rc_unknown;

	switch(tok)
	{
	default:
		qp--; errno = expectedvalue; xxlog(" error}"); return rc_unknown;

//	case tEC:		//	ec
//	case tPI:		//	pi
//	case tNEWLINE:	//	nl
//	case tTAB:		//	tab
	case tNUM:		// 	<number>
	case tNUM0:		//	0
	case tNUM1:		//	1
	case tSTR:		// 	<text>
	case tSTDIN:	//	stdin
	case tSTDOUT:	//	stdout
	case tSTDERR:	//	stderr
		store_token(qp0);
		rc = rc_const;
		break;

	case tIDF:		// 	<idf>
		store_token(qp0);
		rc = rc_var;
		break;

	case tWORDS:	//	words
	case tNOW:		//	now
		store_token(qp0);
		rc = rc_temp;
		break;

	case tERROR:	//	error
	case tENV:		//	env
	case tGLOBALS:	//	globals
	case tLOCALS:	//	locals
	case tTHIS:		//	this
	case tROOT:		//	root
		store_token(qp0);
		rc = rc_var;
		break;

//	case tADD:		//	+ <value>		Unary operator +	((nop))
//		break;

	case tSOCK:
	case tPIPE:
		errno = notyetimplemented;	// TODO
		xxlog(" error}");
		return rc_unknown;

	case tDIR:		//	dir <value>		->	<value> tDIR tAT -- <value>
	case tFILE:		//	file <value>	->	<value> tFILE tAT -- <value>
	case tLINK:		//	link <value>	->	<value> tLINK tAT -- <value>
		rc = value_temp(pUna);
		store_token(qp0);
		store_token(qp0->rowcol,tAT);
		rc = rc_temp;
		break;

	// <temp> tXXX -- <temp>
	case tNEG:		//	- <value>		Unary operator -
	case tNOT:		// 	! <value>		Unary operator !	((boolean not))
	case tTILDE:	// 	~ <value>		Unary operator ~	((bitwise not))
	case tABS:		// 	abs <value>
	case tSIGN:		// 	sign <value>
	case tROUND:	// 	round <value>
	case tFRACT:	// 	fract <value>
	case tINT:		// 	int <value>
	case tSIN:		//	sin <value>
	case tCOS:		// 	cos <value>
	case tTAN:		// 	tan <value>
	case tASIN:		//	asin <value>
	case tACOS:		//	acos <value>
	case tATAN:		//	atan <value>
	case tSINH:		//	sinh <value>
	case tCOSH:		//	cosh <value>
	case tTANH:		//	tanh <value>
	case tSQUARE:	//	square <value>
	case tSQRT:		//	sqrt <value>
	case tRANDOM:	//	random <value>
	case tTOUPPER:	//	upperstr <value>
	case tTOLOWER:	//	lowerstr <value>
	case tINFO:		//	info <value>
	case tMDATE:	//	mdate <value>
	case tFULLPATH:	//	fullpath <value>
	case tDATESTR:	//	datestr <value>
	case tDATEVAL:	//	dateval <value>
	case tLOGE:		//	loge <value>
	case tLOG10:	//	log10 <value>
	case tLOG2:		//	log2 <value>
	case tEXPE:		//	expe <value>
	case tEXP2:		// 	exp2 <value>
	case tEXP10:	//	exp10 <value>
	case tCHARSTR:	//	charstr <value>
	case tBOOL:		//	bool <value>
		rc = value_temp(pUna);
		store_token(qp0);
		rc = rc_temp;
		break;

	// <var> tXXX -- <var>
	case tPREDECR:	//	-- <value>		Unary operator --	((pre-decrement))
	case tPREINCR:	//	++ <value>		Unary operator ++	((pre-increment))
		rc = value_var(pUna);
		store_token(qp0);
		rc = rc_var;
		break;

	// <value> tXXX -- <value>
	case tINCLUDE:	//	include <value>
	case tEVAL:		// 	eval <value>
		value(pUna);
		store_token(qp0);
		r_up(RCountEvalOrInclude); r_down(RCountEvalOrInclude);
		rc = rc_unknown;
		break;

	case tREQUIRE:	//	require <value>
		r_up(RCountEvalOrInclude); r_down(RCountEvalOrInclude);
		goto ERR_NIMP;

	// <value> tXXX -- <temp>
	case tINDEX:		//	index <variable>
	case tCOUNT:		// 	count <value>
	case tGETNAME:		//	name <variable>
	case tTOTEXT:		//	string <value>
	case tCHARCODE:		//	charcode <value>
	case tISTEXT:		//	istext <value>
	case tISNUMBER:		//	isnumber <value>
	case tISLIST:		//	islist <value>
	case tISPROC:		//	isproc <value>
	case tISFILE:		//	isfile <value>
	case tISDIR:		//	isdir <value>
	case tISLINK:		//	islink <value>
	case tISTRIGGERED:	//	istriggered <irpt:value>
		value(pUna);
		store_token(qp0);
		rc = rc_temp;
		break;

	// <temp> <value> token -- <temp>
	case tLOG:			//	log ( <value>, <value> )
	case tEXPN:			//	exp ( <value>, <value> )
	case tLEFTSTR:		//	leftstr ( <value>, <value> )
	case tRIGHTSTR:		//	rightstr ( <value>, <value> )
	case tJOIN:			//	join ( <value>, <value> )
		expect(tRKauf);
		rc = value_temp(pComma);
		expect(tKOMMA);
		value(pComma);
		expect(tRKzu);
		store_token(qp0);
		rc = rc_temp;
		break;

	// 	<value> <n*value> tSYS.n -- <temp>
	case tSYS:		//	sys ( <value> [, ...] )
		expect(tRKauf);
		value(pComma);			// filename
		store_token(qp->rowcol,tREQSTRING);
		for( n=0; is_comma(); n++ ) { value(pComma); }		// optional arguments
		store_token( qp0->rowcol,tSYS,n );	// tSYS.n
		v_down(n);
		expect(tRKzu);
		rc = rc_temp;
		break;

	// 	<value> <n*value> token.n -- <value>
	case tCALL:		//	call ( <value> [, ...] )
		r_up(RCountCall); r_down(RCountCall);
	case tMIN:		// 	min ( <value> [, ...] )
	case tMAX:		// 	max ( <value> [, ...] )
		expect(tRKauf);
		n=-1; do { n++; value(pComma); } while(is_comma());
		expect(tRKzu);
		if(n>255) SetError("too many arguments");
		store_token(qp0->rowcol,tok,n); v_down(n);
		rc = rc_unknown;
		break;


	//	code:	<value> <n*temp> <value> tSPAWN.n.m <m*opcode> -- <temp>
	//	syntax:	spawn <idf>(<arguments>) [prio <value>]
	case tSPAWN:
		rc = spawn();
		break;


	//	<temp> [<value>] tSPACESTR.n -- <temp>
	case tSPACESTR:	//	spacestr ( <value> [, <value>] )
	case tHEXSTR:	//	hexstr ( <value> [,<value>] )
	case tBINSTR:	//	binstr ( <value> [,<value>] )
	case tSPLIT:	// 	split ( <value> [,<value>] )
		expect(tRKauf);
		value_temp(pComma); n=0;
		if(is_comma()) { value(pComma); n=1; }
		store_token(qp0->rowcol,tok,n); v_down(n);
		expect(tRKzu);
		rc = rc_temp;
		break;

	// 	<temp> <value> [<value>] tMIDSTR -- <temp>
	case tMIDSTR:	// 	midstr ( <value>, <value> [,<value>] )
	case tSUBSTR:	// 	substr ( <value>, <value> [,<value>] )
	case tMATCH:	// 	match ( <value>, <value> [,<value>] )
		expect(tRKauf);
		rc = value_temp(pComma);
		goto tf;

	// <value> <value> [<value>] tFIND -- <temp>
	case tFIND:		// 	find ( <value>, <value> [,<value>] )
	case tRFIND:	// 	rfind ( <value>, <value> [,<value>] )
		expect(tRKauf);
		rc = value(pComma);
tf:		expect(tKOMMA);
		value(pComma); n=0;
		if(is_comma()) { value(pComma); n=1; }
		expect(tRKzu);
		store_token(qp0->rowcol,tok,n);	v_down(n);
		rc = rc_temp;
		break;

	//	<temp|var> <value> <value> tREPLACE -- <temp|var>
	case tREPLACE:	// 	replace ( <value>, <value> <value> )
		expect(tRKauf);
		value_temp(pComma);
		expect(tKOMMA);
		value(pComma);
		expect(tKOMMA);
		value(pComma);
		expect(tRKzu);
		store_token(qp0);
		rc = rc_temp;
		break;

	case tRKauf:	// 	( … )
		rc = value(pComma);
		expect(tRKzu);
		break;

	//	<temp|var> tCONVERT_TO.nh  --  <temp|var>
	case tCONVERT:	//	convert ( <text> from|to <idf> )
		expect(tRKauf);
		rc = value_temp(pComma);
	{
		bool f=qp->token==tFROM;
		bool t=qp->token==tTO;
		if(!(f||t)) { SyntaxError("'from' or 'to' expected"); xxlog(" error}"); return rc_unknown; }
		do
		{	qp++;
			n = nickname( GetEncodingFromName );
			store_token( qp0->rowcol, f?tCONVERT_FROM:tCONVERT_TO, n );
			if(n==-1) v_down(1);
			f=qp->token==tFROM;
			t=qp->token==tTO;
		}
		while( errno==ok && (t||f) );
	}
		expect(tRKzu);
		rc = rc_temp;
		break;

	case tEXISTS:	//	exists <variable>  |  exists file|dir|link <value>
	{
		tok = qp->token;
		bool kf = tok==tRKauf;
		if(kf) { qp++; tok = qp->token; }

		if( tok==tFILE || tok==tDIR || tok==tPIPE || tok==tSOCK || tok==tLINK )
		{
			Token* qp1 = qp; qp++;
			value(pUna);
			store_token(qp1);			//	tFILE etc.
			store_token(qp0);			// 	tEXISTS
			v_down(1);
			if(kf) expect(tRKzu);
			rc = rc_const;
			break;
		}

		Stack<int> z;

		if( tok==tGLOBALS || tok==tLOCALS || tok==tROOT || tok==tTHIS || tok==tENV || tok==tERROR )
		{
			store_token(qp++);			// existing starting point for var path
		}
		else
		{
			idf( tEXISTS,tEXISTSAT );		// <idf> tEXISTS -- <value> <var|zero>
			if(zbu[zi-1].token==tEXISTS) store_token(qp->rowcol,tNOP);
			z.Push(zi-1);
		}

		while(errno==ok)
		{
			if(qp->token==tDOT)
			{
				qp++; idf(tEXIIDF,tEXIAT);
				if(zbu[zi-1].token==tEXIIDF) store_token(qp->rowcol,tNOP);
				z.Push(zi-1);
			}
			else if(qp->token==tEKauf)
			{
				qp++;
				do
				{	value(pComma);
					store_token(qp->rowcol,tEXIIDX);
					z.Push(zi-1);
				}
				while(is_comma());
				expect(tEKzu);
			}
			else break;
		};

		store_token(qp0->rowcol,tDROP);
		store_token(qp0->rowcol,tNUM1);

		store_token(qp0->rowcol,tNOP);
		while(z.IsNotEmpty()) SetDist(zbu+z.Pop(),zbu+zi-1);

		if(kf) expect(tRKzu);
		rc = rc_const;			// zero or eins
		break;
	}



	case tAT:		// @<value>			unary operator @
		value(pDot);
		store_token(qp0);
		rc = rc_var;
		break;

	case tGKauf:	// list literal:	{ … }
	{	bool f = 0;
		n=0;
		if(qp->token!=tGKzu)
		{
			do
			{	if(n==255) 	// internal limit for tLIST.cnt
				{
					v_down(n);
					store_token(qp0->rowcol,tLIST,n);
					if(f) store_token(qp0->rowcol,tHASHHASH);
					f=1; n=0;	// note: '##' required; 2nd list size = 0
				}
				rc = argument(); n++;
				if(rc!=rc_temp) store_token(qp->rowcol,tREQTEMP);
			}
			while(is_comma());
		}
		expect(tGKzu);
		v_down(n);
		store_token(qp0->rowcol,tLIST,n);
		if(f) store_token(qp0->rowcol,tHASHHASH);
		rc = rc_temp;		//	<n*temp> tLIST.n -- <temp>
	}	break;				//	<temp> <temp> tHASHHASH -- <temp>

	case tPROC:	// proc literal: proc ( <arguments> ) { <statements> }
				//	code:		 tPROC.dist.n.m <n*namehandle> <statements> -- <temp>
				// 				 dist = dist to skip proc body; n = args total; m = args before '*'
		procliteral(qp0);
		rc = rc_temp;
		break;

	}	// switch token



// ==== expect operator ===================================================


	while(errno==ok)
	{
		assert(qp<qe);

		tok = qp->token;
		char myprio = tokenprio[tok];
		if (myprio<=prio) break;
		qp0=qp; qp++;

		switch ( tok )
		{
		default: qp--; break;		// break from infinite loop: no error!

	// normal: diadic, left-to-right
	//	<temp> <value> tADD -- <temp>
		case tADD:
		case tSUB:
		case tMUL:
		case tDIV:
		case tMOD:
		case tOR:
		case tXOR:
		case tAND:
		case tLTLT:
		case tGTGT:
		case tHASH:
			if(rc!=rc_temp) store_token(qp0->rowcol,tREQTEMP);
			value(myprio);
			store_token(qp0);
			rc = rc_temp;
			continue;

		//	<value> <value> tEQ -- <const>
		case tEQ:
		case tNE:
		case tGT:
		case tGE:
		case tLT:
		case tLE:
			value(myprio);
			store_token(qp0);
			rc = rc_const;
			continue;

		//	<temp> <temp> tHASHHASH -- <temp>
		case tHASHHASH:
			if(rc!=rc_temp) store_token(qp0->rowcol,tREQTEMP);
			value_temp(myprio);
			store_token(qp0);
			rc = rc_temp;
			continue;

	// postfix
	// <var> tPOSTINCR -- <temp>
		case tPOSTINCR:
		case tPOSTDECR:
			if(rc!=rc_var)
			{
				if(rc!=rc_unknown) goto ERR_VARREQ;
				store_token(qp0->rowcol,tREQVAR);
			}
			store_token(qp0);
			rc = rc_temp;
			continue;

	// pruning:
		case tANDAND:				// <value> -- <preserved>  + bra.if0
		case tOROR:					// <value> -- <preserved>  + bra.if1
		{	ResultClass rc2;
			z0 = zi;
			store_token(qp0);		// operator

			rc2 = value(myprio);
			if(rc2!=rc_temp&&rc==rc_temp) store_token(qp0->rowcol,tREQTEMP);

			store_token( qp0->rowcol, tNOP );		// jump target
			SetDist(zbu+z0,zbu+zi-1);

			if(rc!=rc_temp) store_token(qp0->rowcol,tREQTEMP);
			store_token( qp0->rowcol, tBOOL );		// <var|temp> -- <var|temp>
		}	continue;

		case tQMARK:	//	c ? c?n:n : c?n:n
		{				//	c ? n : c ? n : c ? n ...
			z0 = zi;							// zp0 -> tQMARK
			store_token(qp0); 						// tQMARK ~ tIF
			rc = value(myprio-1);					// allow nested a?b:c in value!!

			store_token(qp);						// tCOLON ~ tELSE
			store_token(qp->rowcol,tNOP);			// tNOP: jump target
			SetDist(zbu+z0,zbu+zi-1);
			z0 = zi-2; 							// zp0 -> tCOLON
			expect(tCOLON);

			ResultClass rc2;
			rc2 = value(myprio-1);					// allow nested a?b:c in value!!
			store_token(qp->rowcol,tNOP);			// tNOP: jump target
			SetDist(zbu+z0,zbu+zi-1);

			if(rc2!=rc) rc = rc_unknown;
		}	continue;

	// special:
		case tDOT:				// 	<value> . <idf>
			idf(tDOT,tDOTAT);	//	<value> . @<name>
		//	rc = rc;
			continue;

						// <value> [ <value> [, ...] ]
						// <value> [ <value> to <value> ]
						// <value> [ <value> to ]
		case tEKauf:	// <value> [ to <value> ]
eka:		if(qp->token==tTO)
			{
				qp0=qp; qp++;
				value(pComma);
				expect(tEKzu);
				store_token( qp0->rowcol, tTO_IDX );	//	<value> <value> tTO_IDX -- <temp>
				rc = rc_temp;
				continue;
			}

			value(pComma);

			if(qp->token==tTO)
			{
				qp0=qp; qp++;
				if(qp->token==tEKzu)
				{
					qp++;
					store_token( qp0->rowcol, tIDX_TO );	//	<value> <value> tIDX_TO -- <temp>
				}
				else
				{
					value(pComma);
					expect(tEKzu);
					store_token( qp0->rowcol, tIDX_TO_IDX );	//	<value> <value> <value> tIDX_TO_IDX -- <temp>
				}
				rc = rc_temp;
				continue;
			}

			if (qp->token==tKOMMA)
			{
				qp0=qp; qp++;
				store_token( qp0->rowcol, tIDX );	//	<value> <value> tIDX -- <value>
				goto eka;
			}

			if (qp->token==tEKzu)
			{
				qp0=qp; qp++;
				store_token( qp0->rowcol, tIDX );	//	<value> <value> tIDX -- <value>
			//	rc = rc;
				continue;
			}

			SyntaxError("'to', ',' or ']' expected"); xxlog(" error}"); return rc_unknown;

		case tRKauf:	//	syntax:	<value> ( <arguments> )
						//	code:	<proc> <n*temp> tEXECPROC.n -- <value>
			n = 0;
			if(qp->token!=tRKzu)
				do
				{ 	rc = argument(); n++;
					if(rc!=rc_temp) store_token(qp->rowcol,tREQTEMP);
				}
				while(is_comma());
			expect(tRKzu);
			store_token(qp0->rowcol,tEXECPROC,n);	//	<value> <n*temp> tEXECPROC.n -- <value>
			r_up(RCountProcOrInstr); r_down(RCountProcOrInstr);
			v_down(n);
			rc = rc_unknown;
			continue;

		}	// switch(*ip)

		xxlog(" :%i}",v_now);
		return rc;

	}	// while(!error)

	xxlog( errno?" error}":" :%i}", v_now);
	return rc;	// rc_unknown;

ERR_NIMP:	errno = notyetimplemented; xxlog(" error}"); return rc_unknown;
ERR_VARREQ:	errno = varreq; xxlog(" error}"); return rc_unknown;
}




/* generate code:

*/
struct xref
{	uptr 	pointer;
	uint32 	ID;
			xref(uptr p, uint32 n)	{ pointer=p; ID=n; }
			xref()					{}	// for Sorter
};


#define GT(A,B)	((A).ID>(B).ID)
#define TYPE xref
static void SortXref(TYPE* a, TYPE* e)
#include "Templates/sorter.h"


void Compiler::Assemble()
{
	assert(errno==ok);
	xlogIn("Compiler::Assemble()");

	assert(qa!=nullptr);
	assert(qa<qe);
	assert(qe[-1].token==tEOF);

	Stack<xref>	source_stack;
	Stack<xref>	target_stack;

	uint destsize = (qe-qa)*4;
	uchar* destbu = new uchar[destsize];
	uptr dp		  = destbu;
	IFDEBUG(uptr destend  = destbu+destsize;)
	uint32* xrefbu = new uint32[destsize]; int xrefi = 0; uint32 xrefx = 0;

	for(qp=qa;qp<qe;qp++)
	{
		xxlog("[%s]",tostr(qp));

		while(xrefi<dp-destbu) xrefbu[xrefi++] = xrefx;
		xrefx = qp->rowcol;

		switch(qp->token)
		{
		case tNOP:
			if(qp->argument_type==arg_destination)		// nach private scope "}" wirds nicht benutzt
			{
				xxlog(" d=%lu==%i.%i ",dp-destbu,Row(qp->rowcol),Col(qp->rowcol));
				target_stack.Push(xref(dp,qp->argument.destination));
			}
			continue;

//		case tEOF:
//			*dp++ = tEOF;
//			qp++; continue;

		default:
			if (qp->argument_type == arg_void)
			{
				*dp++ = qp->token;
				continue;
			}
			SetError(internalerror,usingstr("internal error in %i", int(__LINE__)) );
			rowcol = qp->rowcol;
			return;

	// file prefixes:
		case tLINK:
		case tDIR:
		case tFILE:
		case tPIPE:
		case tSOCK:
			if (qp->argument_type == arg_void) { *dp++ = (*qp++).token; }
			if (qp->argument_type == arg_void) { *dp++ = qp->token; continue;}
			SetError(internalerror,usingstr("internal error in %i", int(__LINE__)) );
			rowcol = qp->rowcol;
			return;

	// pruning:
		case tEXIAT:			//	tEXIAT.dist
		case tEXIIDX:			//	tEXIIDX.dist
		case tEXISTSAT:			//	tEXISTSAT.dist
		case tOROR:
		case tANDAND:
		case tORORGL:
		case tANDANDGL:
		case tQMARK:
		case tCOLON:
	// branch control:
		case tDO:
		case tTRY:
	//	case tLOOP:
	//	case tWHILE:
	//	case tUNTIL:
	//	case tNEXT:
	//	case tEXIT:
		case tIF:
		case tELSE:
		case tCATCH:
		case tELIF:
			*dp++ = qp->token;
			source_stack.Push(xref(dp,qp->argument.destination));
			xxlog(" s=%lu==%i.%i ",dp-destbu,Row(qp->argument.destination),Col(qp->argument.destination));
			assert(qp->argument_type==arg_long);
			dp += sizeof(displacement);
			continue;

		case tEXIIDF:			//	tEXIIDF.namehandle.dist
		case tEXISTS:			// 	tEXISTS.namehandle.dist
			*dp++ = qp->token;

			poke4(dp,qp->argument.namehandle); dp+=4;
			assert(qp->argument_type==arg_namehandle);
			IFDEBUG( GetNameForHandle(qp->argument.namehandle); )
			qp->argument.namehandle = 0;		// lock geht auf tIDF.nh über

			qp++;
			source_stack.Push(xref(dp,qp->argument.destination));
			xxlog(" s=%lu==%i.%i ",dp-destbu,Row(qp->argument.destination),Col(qp->argument.destination));
			assert(qp->argument_type==arg_long);
			dp += sizeof(displacement);
			continue;

		case tSPAWN:
			*dp++ = qp->token;
			assert(qp->argument_type==arg_two_short);
			assert(qp->argument.n<=255);
			assert(qp->argument.m<=255);
			*dp++ = qp->argument.n;
			*dp++ = qp->argument.m;
			continue;

		case tPROC:
		{
			*dp++ = qp->token;
			source_stack.Push(xref(dp,qp->argument.destination));
			xxlog(" s=%lu==%i.%i ",dp-destbu,Row(qp->argument.destination),Col(qp->argument.destination));
			assert(qp->argument_type==arg_long);
			dp += sizeof(displacement);
			int32 n;
			qp++; *dp++ = n = qp->argument.arguments;		assert(qp->argument_type==arg_long);	 // n
			qp++; *dp++ =     qp->argument.arguments;		assert(qp->argument_type==arg_long);	 // m
			while(n--)
			{
				qp++; poke4(dp,qp->argument.namehandle); dp+=4;
				assert(qp->argument_type==arg_namehandle);
				qp->argument.namehandle = 0; 		// lock geht auf core über
			}
		}	continue;

	// immediate arguments:

		case tDOT:
		case tRENAME_NH:
		case tVARIDF:
		case tNEWIDF:
		case tIDF:
			*dp++ = qp->token;
			poke4(dp,qp->argument.namehandle); dp+=4;
			assert(qp->argument_type==arg_namehandle);
			IFDEBUG( GetNameForHandle(qp->argument.namehandle); )
			qp->argument.namehandle = 0;		// lock geht auf tIDF.nh über
			continue;

		case tNUM:
		case tSTR:
			assert(qp->argument_type==arg_long);
			if(qp->argument.index<=255)
			{
				*dp++ = qp->token;
				*dp++ = qp->argument.index;
			} else {
				*dp++ = tNUM2;
				poke2(dp,qp->argument.index); dp+=2;
				assert((qp->argument.index>>16)==0);
			}
			continue;

		case tLIST:
			*dp++ = qp->token;
			*dp++ = qp->argument.arguments;
			assert(qp->argument_type==arg_long);
			assert (qp->argument.arguments<=255);
			continue;

		case tRETURN:	//	return <value>
		case tEND:		//	end <value>
			*dp++ = qp->token;				// the caller will check the stack
			continue;						// to see whether an argument was returned

	//	case tDEL:		//	del var|file|dir|link <name|text>
	//	case tRENAME:	//	del var|file|dir|link <name|text> = <text>
		case tEXECPROC:	//	( <arguments> )
		case tEXECINSTR://	( <arguments> )
		case tMIN:		// 	min ( <value> [, ...] )
		case tMAX:		// 	max ( <value> [, ...] )
		case tCALL:		//	call ( <value> [, ...] )
		case tSYS:		//	sys ( <value> [, ...] )
		case tSPACESTR:	//	spacestr ( <value> [, <value>] )
		case tHEXSTR:	//	hexstr ( <value> [,<value>] )
		case tBINSTR:	//	binstr ( <value> [,<value>] )
		case tSPLIT:	// 	split  ( <value> [,<value>] )
		case tMIDSTR:	// 	midstr ( <value>, <value> [,<value>] )
		case tSUBSTR:	// 	substr ( <value>, <value> [,<value>] )
		case tFIND:		// 	find   ( <value>, <value> [,<value>] )
		case tRFIND:	// 	rfind  ( <value>, <value> [,<value>] )
		case tMATCH:	// 	match  ( <value>, <value> [,<value>] )
		case tCONVERT_TO:
		case tCONVERT_FROM:
		case tCHECKSTACK:
			*dp++ = qp->token;
			*dp++ = qp->argument.arguments;
			assert(qp->argument_type==arg_long);
			assert(qp->argument.arguments<=255);
			continue;

		case tNEW:
			*dp++ = qp->token;
			*dp++ = qp->argument.arguments;
			*dp++ = qp->argument.arguments >> 8;
			assert(qp->argument_type==arg_long);
			continue;
		}
	}

	assert(dp<=destend);
	assert(qp==qe);
	assert(dp[-1]==tEOF);

	while(xrefi<dp-destbu) xrefbu[xrefi++] = xrefx;

	if(source_stack.Count())
	{
		assert(target_stack.Count()>0);
	// sort target stack by ID
	// normale branch controls erzeugen immer einen sortierten stack
	// durch pruning-operatoren entsteht aber eine fehlsortierte reihenfolge?
		SortXref(target_stack.get_start(),target_stack.get_end());

	// store displacements
		while(source_stack.Count()!=0)
		{
			xref& z = source_stack.Pop();
			uptr dp = z.pointer;			// hier eintragen
			uint32 ID = z.ID;				// ziel

			xref* a=target_stack.get_start();
			xref* e=target_stack.get_end();

			xxlog("<");
			while(e-a>1)
			{
				xref* m = a + (e-a)/2;
				if(ID<m->ID) e=m; else a=m;
			}
			xxlog(">");
			assert(a->ID==ID);

			displacement d = displacement(a->pointer-dp);

			if (sizeof(displacement)==4)
			{
				poke4(dp,d);
			}
			else if (sizeof(displacement)==2)
			{
				poke2(dp,d);
				if( displacement(d) == d ) continue;
				errno = branchtoofar;
				rowcol = xrefbu[dp-destbu]; break;	// error & exit
			}
			else IERR();
		}
	}

	if(errno==ok)
	{
		core = String(dp-destbu, csz1);
		memcpy(core.UCS1Text(),destbu,dp-destbu);
		rowcolxref = String(xrefi, csz4);
		memcpy(rowcolxref.UCS4Text(),xrefbu,xrefi*csz4);
	}

	delete[] destbu;
	delete[] xrefbu;
}





void Compiler::CheckLoops( )
{
	assert(errno==ok);
	xlogIn("Compiler::CheckLoops()");

	assert(qa!=nullptr);
	assert(qa<qe);
	assert(qe[-1].token==tEOF);

	qp=qa;
	while(errno==ok&&qp->token!=tEOF) qp = set_offset(qp,no);

	assert(qp<qe);
	rowcol=qp->rowcol;
}


void Compiler::Reorder ( bool exe )
{
	assert(errno==ok);
	xlogIn("Compiler::Reorder()");

	assert(qa!=nullptr);
	assert(qa<qe);
	assert(qe[-1].token==tEOF);

	zlen = (qe-qa)*3/2;
	zbu  = new Token[zlen];
	zi   = 0;
	qp   = qa;

	v_now = v_max = 0;
	r_now = r_max = 0;
	store_token(qp->rowcol, tCHECKSTACK,123);
	if(exe) execute();
	else  { value(pComma); store_token(qp->rowcol,tRETURN); }
	if(errno==ok) { if(qp->token==tEOF) store_token(qp); else SyntaxError("end of file expected"); }
	assert(v_now==0||errno);
	assert(r_now==0||errno);
	zbu[0].argument.arguments = v_max+r_max;
	xlog(" rootlevel:v_max=%i,r_max=%i;",(int)v_max,(int)r_max);

	if (errno)
	{
		assert(qp<qe);
		rowcol=qp->rowcol;
		delete[] zbu; zbu=nullptr;
	}
	else
	{
		rowcol=0;
		delete[] qa;
		qa=zbu; qe=zbu+zi; zbu=nullptr;
	}
}


void Compiler::Optimize()
{
	assert(errno==ok);
	xlogIn("Compiler::Optimize()");

	assert(qa<qe);
	assert(zbu==nullptr);
	assert(qe[-1].token==tEOF);

//	for(qp=qa;qp<qe;qp++) { if(qp->token==tTHEN) qp->token=tNOP; }
}





void Compiler::Compile ( bool exe )
{
	xlogIn( "Compiler::Compile(%i)", (int)exe );
	assert(errno==ok);
	assert(qa==nullptr);
	assert(bundle && bundle->IsList() && bundle->IsLocked());

	constants = new Var( bundle, bundle_constants,  isList, "constants" );
				new Var( bundle, bundle_source, 	source, "source" );

	cstr where = nullptr;
	bool o = exe && sourcefile.Len();

	Tokenize();			if(errno) { where = " (Compile:Tokenize)"; goto x; }	// source ->  qa
	CheckLoops();		if(errno) { where = " (Compile:ChkLoops)"; goto x; }	//    qa  ->  qa
	Reorder(exe);		if(errno) { where = " (Compile:Reorder)" ; goto x; }	//    qa  ->  qa
	if(o) Optimize();	if(errno) { where = " (Compile:Optimize)"; goto x; }	//    qa  ->  qa
	Assemble();			if(errno) { where = " (Compile:Assemble)"; goto x; }	//    qa  -> core, rowcolxref

	new Var( bundle, bundle_core, core, 	  "core" );
	new Var( bundle, bundle_xref, rowcolxref, "rowcolxref" );

	if(sourcefile.Len()) bundle->AppendItem( new Var( sourcefile, "sourcefile" ) );
	if(cachefile.Len())  bundle->AppendItem( new Var( cachefile,  "cachefile" ) );

	bundle->DeepLockData();
	return;	// ok

x:	if(XXLOG) AppendToError(String(where));
}


void Compiler::Compile ( cString& s, bool exe )
{
	xlogIn( "Compiler::Compile(source,%i)", (int)exe );
	errno=ok;
	Reset();
	source = s;
	Compile(exe);
//	if(errno) PrependToError(RowColString(rowcol));
	xlogline("Compile(): %s",errno?"error":"ok");
}


void Compiler::CompileFile( cString& filename, bool exe )
{
	xlogIn("Compiler::CompileFile(%s,%i)",filename.ToQuoted().CString(),(int)exe);

	errno=ok;
	Reset();
	sourcefile = filename;
	if(ReadCacheFile()!=ok)
	{
		ReadFile(filename,source);
		source = source.FromUTF8();
	}
	if(errno)
	{
		xlogline("CompileFile(): file error");
		return;			// error incl. filename set by ReadFile()
	}
	Compile(exe);
//	if(errno) PrependToError( String("in file ") + filename.ToQuoted() + " " + RowColString(rowcol));

	xlogline("CompileFile(): %s",errno?"error":"ok");
}


OSErr Compiler::ReadCacheFile()		// TODO
{
	return notyetimplemented;
}

void Compiler::WriteCacheFile()		// TODO
{
}









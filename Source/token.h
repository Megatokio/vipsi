/*	Copyright  (c)	Günter Woigk   2001 - 2019
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


/*			Attention:

			printable text in this file must be UTF-8 encoded!
*/



#ifdef	TOKEN_PASS			// included from token.cp ?
	#undef	token_h		// then allow multiple inclusion
#else
	#define	TOKEN_PASS 1
#endif


#ifndef token_h
#define token_h


#if TOKEN_PASS==1
	#include "kio/kio.h"
	#include "Var/NameHandles.h"

	/* ----	priorities for VScript::Value(…) ---------------
			priorities are inspected by operators before processing their operation
			priorities are set by operators before processing their right-side operand
			and by functions before processing their argument(s)

			position of non-ops in this list does not indicate their operator priority
			(functions do not have an operator priority!)
			but gives a clue what they set the op. prio to before evaluating their argument(s)
	*/
	enum
	{
	// non-operator classifications:
		instruction,	// never return a result
		valueNoArg,
		procOneArg,
		procWithArg,

	// LOWEST prio
		pSemicolon,		// <term>:			also: accept 'no value' as result
		pComma,/*pAny*/	// <term>:	 		stop at ')' or ',' or <instruction>

		pAssign,		// assignment:		= += -= /= *= %= &&= ||= >>= <<= &= |= ^=
		pSel,			// selector:		? :
		pBoo,			// boolean:			&& ||
		pCmp, 			// comparison:		== != > >= < <=
		pHash,			// concatenation:	# ##
		pAdd, 			// addition:		+ -
		pMul, 			// multiplication:	* / %
		pBit, 			// masking: 		& | ^
		pRot, 			// rot/shift:		<< >>
		pUna,			// prefix:			set by functions with 1 arg
		pPostfix,		// postfix:			++ -- (
		pDot,			// item selector:	. [

		pMax			// (none)			highest: break all
	// HIGHEST prio
	};


	extern	uchar*	SkipToken		( uchar* ip );
	extern	uchar*	SkipBrackets	( uchar* ip );
	extern	uchar*	SkipMatching	( uchar* ip );
	extern	uchar*	Disass			( class Stream* stream, uchar*p, int n );
	extern	String	DisassString	( const class Var& );
	extern	String	TokenName		( uint t );
	extern	String	TokenSyntax		( uint t );
	extern	String	TokenInfo		( uint t );
	extern	String 	RowColString	( uint err_row, uint err_col );
	extern	String 	RowColString	( struct Token* err_tok );
	extern	char 	tokenprio[];
	extern	String 	tokenname[];
	extern	NameHandle tokennamehandle[];

	inline	uchar*	SkipIdf			( uchar* ip ) { return ip +3 +ip[1]*ip[2]; }
//	extern	uchar*	SkipTerm		( uchar* ip, int prio );

// non-aligned write double
	inline void write_double(uchar*p, double f)
	{
	#if _ALIGNMENT_REQUIRED
		XXXTRAP(sizeof(double)!=8*sizeof(uchar));
		p[0] = ((uchar*)&f)[0];
		p[1] = ((uchar*)&f)[1];
		p[2] = ((uchar*)&f)[2];
		p[3] = ((uchar*)&f)[3];
		p[4] = ((uchar*)&f)[4];
		p[5] = ((uchar*)&f)[5];
		p[6] = ((uchar*)&f)[6];
		p[7] = ((uchar*)&f)[7];
	#else
		#if	defined(_POWERPC)						// G3/G4 can access misaligned
			XXXTRAP(sizeof(double)!=2*sizeof(long));// but this takes very long: ~2000 cycles!
			((long*)p)[0] = ((long*)&f)[0];			// <-- this fails for _I386 -O2
			((long*)p)[1] = ((long*)&f)[1];			// <-- ""
		#else
			*(double*)p = f;
		#endif
	#endif
	}

// non-aligned read double
	inline double read_double(uchar*p)
	{
	#if _ALIGNMENT_REQUIRED
		double f;
		((uchar*)&f)[0] = p[0];
		((uchar*)&f)[1] = p[1];
		((uchar*)&f)[2] = p[2];
		((uchar*)&f)[3] = p[3];
		((uchar*)&f)[4] = p[4];
		((uchar*)&f)[5] = p[5];
		((uchar*)&f)[6] = p[6];
		((uchar*)&f)[7] = p[7];
		return f;
	#else
		#if	defined(_POWERPC)						// G3/G4 can access misaligned
			double f;								// but this takes very long: ~2000 cycles!
			((long*)&f)[0] = ((long*)p)[0];			// <-- this fails for _I386 -O2
			((long*)&f)[1] = ((long*)p)[1];			// <-- probably due to optimization
			return f;								// <-- f is not yet written when returning here!
		#else
			return *(double*)p;
		#endif
	#endif
	}


	#define TOKEN(a,e,f,b,c,d)	a
	enum TokenEnum {

#elif TOKEN_PASS==2
	#define TOKEN(a,e,f,b,c,d)	b
	String tokenname[]={

#elif TOKEN_PASS==3
	#define	TOKEN(a,e,f,b,c,d)	c
	cUTF8Str tokensyntax[]={

#elif TOKEN_PASS==4
	#define	TOKEN(a,e,f,b,c,d)	d
	cUTF8Str tokeninfo[]={

#elif TOKEN_PASS==5
	#define	TOKEN(a,e,f,b,c,d)	e
	char tokenprio[]={

#elif TOKEN_PASS==6
	#define TOKEN(a,e,f,b,c,d)	NewNameHandle(b)
	NameHandle tokennamehandle[] = {

#elif TOKEN_PASS==7
	#define TOKEN(a,e,f,b,c,d)	f
	char	vstacksaldo[] = {

#else
	#error Du hast dich verzählt...
#endif


//		i	= identifier/name
//		n	= numeric expression
//		t	= text expression
//		l	= list expression
//		x	= numeric, text or list expression
//		v	= variable reference
//		[]	= optional part
//		|	= separates options
//		[,…]= optional repetition, separated by komma


// ----	tokens with immediate arguments -----------------------------------

// number constant: 8-byte-double follows
TOKEN(tNUM,		valueNoArg,		+1,		"tNUM",			"tNUM.c",
"internal representation of numeric literals." ),

TOKEN(tNUM2,	valueNoArg,		+1,		"tNUM2",		"tNUM.s",
"internal representation of numeric literals." ),

// number constant: 0  ((null))
TOKEN(tNUM0,	valueNoArg,		+1,		"0.0",			"0",
"internal representation of numeric literal '0'." ),

// number constant: 1  ((one))
TOKEN(tNUM1,	valueNoArg,		+1,		"1.0",			"1",
"internal representation of numeric literal '1'." ),

// string constant: dc.b csz, dc.b chars, dc.s text follows
TOKEN(tSTR,		valueNoArg,		+1,		"tSTR",			"\"hello world\" | «hello world»",
"internal representation of text literals." ),

// identifier: dc.l namehandle
TOKEN(tIDF,		valueNoArg,		+1,		"tIDF",			"foo",
"internal representation of user defined names." ),


// ----	separators, brackets, flags ---------------------------------------
TOKEN(tKOMMA,	pComma,		0, ",",		",",
"Separates arguments in argument list for instructions or procedures.\n"
"Separates indexes in item selector of multi-dimensional lists.\n"
"e.g.:  put \"a=\",a;  a=max(b,c); n=mylist[i,j];" ),

TOKEN(tSEMIK,	pComma,		0, ";",		";",
"Separates statements within a script.\n"
"Optional, rarely really required. Use it if you like.\n"
"e.g.: var a=0; put a; a++; put a;" ),

TOKEN(tRKauf,	pPostfix,	0, "(",		"( n [,…] )",
"Round brackets override operator priority in expressions\n"
"or enclose argument list for procedures.\n"
"e.g.: put sin(pi/3); put 2*(3+4);" ),

TOKEN(tRKzu,	pComma,		0, ")",		"( n [,…] )",
"Round brackets override operator priority in expressions\n"
"or enclose argument list for procedures.\n"
"e.g.: put sin(pi/3); put 2*(3+4);" ),

TOKEN(tEKauf,	pDot,		0, "[",		"t|l [ [n1] [to] [n2] [,…] ]",
"Square brackets enclose subrange or item selector for texts or lists.\n"
"indexes start with 1\n"
"e.g.: put \"abcde\"[2 to 3]; put {1,2,3}[1]." ),

TOKEN(tEKzu,	pComma,		0, "]",		"t|l [ [n1] [to] [n2] [,…] ]",
"Square brackets enclose subrange or item selector for texts or lists.\n"
"indexes start with 1\n"
"e.g.: put \"abcde\"[2 to 3]; put {1,2,3}[1]." ),

TOKEN(tGKauf,	valueNoArg,	0, "{",		"{ … }",
"Curly brackets enclose list literals, procedure bodies,\n"
"or blocks of instructions with private variable scope.\n"
"items in lists may be named using the '=' operator\n"
"which also means, that you cannot use the '=' operator here\n"
"e.g.: var a,y; a = { 1, y=2, 3 }; put \"a=\",a,\" y=\",y\n"
"      proc foo() { put \"hello!\",nl }; foo()\n"
"      { var foo=0 }; put exists foo  // no" ),

TOKEN(tGKzu,	pComma,		0, "}",		"{ … }",
"Curly brackets enclose list literals, procedure bodies,\n"
"or blocks of instructions with private variable scope.\n"
"items in lists may be named using the '=' operator\n"
"which also means, that you cannot use the '=' operator here\n"
"e.g.: var a,y; a = { 1, y=2, 3 }; put \"a=\",a,\" y=\",y\n"
"      proc foo() { put \"hello!\",nl }; foo()\n"
"      { var foo=0 }; put exists foo  // no" ),


// ----	monadic operators -----------------------------------------

/*	pPostfix for idf++ and idf--  postfix operators
	if encountered as functions ++idf or --idf, then they set prio to pComma
*/
TOKEN(tPOSTINCR,	pPostfix,	0, "++",		"n = [++v|v++]",
"pre- or post-increment numeric variable.\n"
"e.g.: var i=0; do while i<10; put i++; loop." ),

TOKEN(tPOSTDECR,	pPostfix,	0, "--",		"n = [--v|v--]",
"pre- or post-decrement numeric variable.\n"
"e.g.: var i=10; do while i>0; put --i; loop." ),


// ----	diadic operators - assignments -----------------------------------------

TOKEN(tGL,		pAssign,		-2, "=",		"v = x",
"assign right-side value to left-side variable.\n"
"the assignment operator returns no value.\n"
"e.g.: var a = 47; a = a*2;" ),

TOKEN(tARROW,	pAssign,		-2, "->",		"v1 = v2",
"make variable1 a reference (pointer) to variable2.\n"
"the assignment operator returns no value.\n"
"e.g.: var a -> 47; a -> s[2];" ),

TOKEN(tPLGL,	pAssign,		-2, "+=",		"v += n",
"add value to variable.\n"
"assignment operators are 'right-oriented', that means: if multiple assignments\n"
"are made in an expression, they are evaluated right-to-left.\n"
"e.g.: var a=20; a += 33;" ),

TOKEN(tMIGL,	pAssign,		-2, "-=",		"v -= n",
"subtract value from variable.\n"
"the assignment operator returns no value.\n"
"e.g.: var a=33; a -= 20;" ),

TOKEN(tMUGL,	pAssign,		-2, "*=",		"v *= n",
"multiply variable with value.\n"
"the assignment operator returns no value.\n"
"e.g.: var a=5; a *= 4;" ),

TOKEN(tDVGL,	pAssign,		-2, "/=",		"v /= n",
"divide variable by value.\n"
"the assignment operator returns no value.\n"
"e.g.: var a=20; a /= 5;" ),

TOKEN(tMODGL,	pAssign,		-2, "%=",		"v %= n",
"divide variable by value and store remainder.\n"
"the assignment operator returns no value.\n"
"e.g.: var a=20; a %= 6;" ),

TOKEN(tHASHGL,	pAssign,		-2, "#=",		"v #= t",
"append text to text variable.\n"
"the assignment operator returns no value.\n"
"if either operand is not text, it is converted to text first.\n"
"if one argument is a list then the operation is applied to each item\n"
"if both arguments are lists then the operation is applied to matching items\n"
"e.g.: var a=\"abc\"; a #= \"de\"; put a;    // abcde\n"
"      var a={1,2,3}; a#=\"X\"; put a;     // { \"1X\", \"2X\", \"3X\" }\n"
"      var a=\"X\"; a#={1,2,3}; put a;     // { \"X1\", \"X2\", \"X3\" }\n"
"      var a={1,2,3}; a#={4,5}; put a;   // { \"14\", \"25\", \"3\" }\n" ),


TOKEN(tGTGTGL,	pAssign,		-2, ">>=",		"v >>= n",
"shift value of numeric variable bitwise right." ),


TOKEN(tLTLTGL,	pAssign,		-2, "<<=",		"v <<= n",
"shift value of numeric variable bitwise left." ),


TOKEN(tANDANDGL, pAssign,		-2, "&&=",		"v &&= n",
"calculate result of boolean AND and store result in left-side variable.\n"
"pruning not yet implemented." ),


TOKEN(tORORGL,	pAssign,		-2, "||=",		"v ||= n",
"calculate result of boolean OR and store result in left-side variable.\n"
"pruning not yet implemented." ),


TOKEN(tXORGL,	pAssign,		-2, "^=",		"v ^= n",
"calculate result of bitwise exclusive OR and store result in left-side variable." ),


TOKEN(tANDGL,	pAssign,		-2, "&=",		"v &= n",
"calculate result of bitwise AND and store result in left-side variable." ),


TOKEN(tORGL,	pAssign,		-2, "|=",		"v |= n",
"calculate result of bitwise OR and store result in left-side variable." ),


TOKEN(tHASHHASHGL, pAssign,		-2, "##=",		"v ##= l",
"append item to list, list to item, or list to list.\n"
"the assignment operator returns no value.\n"
"e.g.: var a=1; a #= { 2,3,4,5 }; a#=6; put a;" ),



// ----	diadic operators ------------------------------------------

TOKEN(tADD,		pAdd,		-1,		"+",		"n = n + n",
"(1) operator: add two numerical values.\n"
"    add two numerical values or text values, which are converted to numbers,\n"
"    results in a number. if one operand is a list, the result is a list, \n"
"    where the operation was applied to all items. if both are lists, they \n"
"    should match in size and the operation is applied to all matching items.\n"
"    e.g.: 4+5 = 9;  ({4,5}+3) = {7,8};  ({2,3})+{4,5} = {6,8}.\n"
"    operator priority is below multiplication.\n"
"(2) optional number sign for positive number.\n"
"    note: preceding a text with operator + converts it to a number.\n"
"    e.g. put +123, +$affe, +\"0123\", +\"$affe\";" ),

TOKEN(tSUB,		pAdd,		-1,		"-",		"n = n - n",
"(1) operator: subtract two numerical values.\n"
"    subtract two numerical values or text values, which are converted to numbers,\n"
"    results in a number. if one operand is a list, the result is a list, \n"
"    where the operation was applied to all items. if both are lists, they \n"
"    should match in size and the operation is applied to all matching items.\n"
"    e.g.: 9-5 = 4;  ({4,5})-3 = {1,2};  ({4,5})-{2,3} = {2,2}.\n"
"    operator priority is below multiplication.\n"
"(2) number sign for negative number.\n"
"    note: preceding a text with operator - converts it to a number.\n"
"    e.g. put -123, -$affe, -\"0123\", -\"$affe\";" ),

TOKEN(tMUL,		pMul,		-1,		"*",		"n = n * n",
"(1) operator: multiply two numerical values. result is a numerical value.\n"
"    if any operand is a text values, then it is converted to a number first.\n"
"    if one operand is a list, then the result is a list,\n"
"    where the operation was applied to all items.\n"
"    if both are lists, then the operation is applied to all matching items.\n"
"    if they don't match in size, the missing items are assumed to be 0.\n"
"    e.g.: 4*5 = 20;  ({4,5})*3 = {12,15};  ({4,5})*{2,3} = {8,15}.\n"
"    operator priority is higher than addition and lower than bit masking.\n"
"(2) wildcard in path pattern for dir(path).\n"
"    e.g. var cafélogo = dir(\"./*.gif\");.\n"
"(3) flag for del statement to delete all variables in local scope.\n"
"    e.g.: del *.\n"
"(4) flag in procedure parameter list to accept any number of arguments.\n"
"    e.g.: proc zähle(a,*){ put \"a=\",a,\" argc=\",count locals,nl };" ),

TOKEN(tDIV,		pMul,		-1,		"/",		"n = n / n",
"    operator: divide one numerical value by another.\n"
"    divide two numerical values or text values, which are converted to numbers,\n"
"    results in a number. if one operand is a list, the result is a list, \n"
"    where the operation was applied to all items. if both are lists, they \n"
"    should match in size and the operation is applied to all matching items.\n"
"    e.g.: 20/5 = 4;  ({40,50})/5 = {8,10};  ({40,50})/{8,5} = {5,10}.\n"
"    operator priority is above addition.\n" ),

TOKEN(tMOD,		pMul,		-1,		"%",		"n = n % n",
"(1) operator: remainder of division of two numerical values.\n"
"    divide two numerical values or text values, which are converted to numbers,\n"
"    results in a number. if one operand is a list, the result is a list, \n"
"    where the operation was applied to all items. if both are lists, they \n"
"    must match in size and the operation is applied to all matching items.\n"
"    e.g.: 21.5%2.5 = 1.5;  ({33,21})%5 = {3,1};  ({36,27}%{5,10}) = {1,7}.\n"
"    operator priority is above addition.\n"
"(2) flag for binary number literal.\n"
"    e.g.: -%0101.1 = -5.5." ),

TOKEN(tHASH,	pHash,		-1,		"#",		"t = t # t",
"(1) operator: concatenate two strings. result is a string.\n"
"    if either operand is not text, it is converted to text first.\n"
"    if one argument is a list then the operation is applied to each item\n"
"    if both arguments are lists then the operation is applied to matching items.\n"
"    if they don't match in size, the missing items are assumed to be \"\".\n"
"    put \"ant\" # \"0123.40\"    // ant0123.40\n"
"    put \"anton\" # 0123.40    // anton123.4\n"
"    put 123.456 # 0123.40    // 123.456123.4\n"
"    put {1,2,3} # \"X\";       // { \"1X\", \"2X\", \"3X\" }\n"
"    put \"X\" # {1,2,3};       // { \"X1\", \"X2\", \"X3\" }\n"
"    put {1,2,3} # {4,5};     // { \"14\", \"25\", \"3\" }\n"
"    operator priority is higher than comparision and lower than addition.\n"
"    e.g.: a#b+c  =  a#(b+c)\n"
"          a#b>c  =  (a#b)>c.\n"
"(2) Select stream for i/o instruction:\n"
"    open, openin, openout, close, log, put, get, read, write, ioctl.\n"
"    e.g.: open#4, \"hello.txt\"; put#4,\"hello world\",nl; close#4;" ),

TOKEN(tHASHHASH, pHash,		-1,		"##",		"l = l ## l",
"operator: concatenate two lists. the result is a list.\n"
"if either operand is not a list, it is converted to a list first.\n"
"{1,2,3}##{4,5,6}  =  {1,2,3,4,5,6}\n"
"1##{2}##\"anton\"   =  {1,2,\"anton\"}\n"
"{1,2,3}##\"anton\"  =  {1,2,3,\"anton\"}\n"
"\"anton\"##{1,2,3}  =  {\"anton\",1,2,3}\n"
"    operator priority is higher than comparision and lower than addition.\n"
"e.g.: a##b+c  =  a##(b+c)\n"
"      a##b>c  =  (a##b)>c."),

TOKEN(tGT,		pCmp,		-1,		">",		"n = x > x",
"operator: compare two values.\n"
"returns 1 if left operand is greater than right operand.\n"
"if values of different type are compared then error condition is set.\n"
"lists are compared by comparing all corresponding items. recursively.\n"
"e.g.: if (a>55) put \"a>55\" then; if(b>\"aaa\") put \"b>aaa\" then;" ),

TOKEN(tLT,		pCmp,		-1,		"<",		"n = x < x",
"operator: compare two values.\n"
"returns 1 if left operand is less than right operand.\n"
"if values of different type are compared then error condition is set.\n"
"lists are compared by comparing all corresponding items. recursively.\n"
"e.g.: if (a<55) put \"a<55\" then; if(b<\"aaa\") put \"b<aaa\" then;" ),

TOKEN(tGE,		pCmp,		-1,		">=",		"n = x >= x",
"operator: compare two values.\n"
"returns 1 if left operand is greater than or equal to right operand.\n"
"if values of different type are compared then error condition is set.\n"
"lists are compared by comparing all corresponding items. recursively.\n"
"e.g.: if (a>=55) put \"a≥55\" then; if(b>=\"aaa\") put \"b≥aaa\" then;" ),

TOKEN(tLE,		pCmp,		-1,		"<=",		"n = x <= x",
"operator: compare two values.\n"
"returns 1 if left operand is less than or equal to right operand.\n"
"if values of different type are compared then error condition is set.\n"
"lists are compared by comparing all corresponding items. recursively.\n"
"e.g.: if (a<=55) put \"a≤55\" then; if(b<=\"aaa\") put \"b≤aaa\" then;" ),

TOKEN(tEQ,		pCmp,		-1,		"==",		"n = x == x",
"operator: compare two values.\n"
"returns 1 if left and right operand have the same value.\n"
"values of different data type are considered not equal.\n"
"lists are compared by comparing all corresponding items. recursively.\n"
"e.g.: if (a==55) put \"a equals 55\" then; if(b==\"aaa\") put \"b == aaa\" then;" ),

TOKEN(tNE,		pCmp,		-1,		"!=",		"n = x != x",
"operator: compare two values.\n"
"returns 1 if left and right operand have different values.\n"
"values of different data type are considered not equal.\n"
"lists are compared by comparing all corresponding items. recursively.\n"
"e.g.: if (a!=55) put \"a!=55\" then; if(b!=\"aaa\") put \"b!=aaa\" then;" ),

TOKEN(tLTLT,	pRot,		-1,			"<<",		"n = n << n",
"operator: bitwise shift left, padding with 0.\n"
"e.g.: 8<<2     = 32\n"
"      0.75<<1  = 1.5\n"
"      2.125<<8 = 544" ),

TOKEN(tGTGT,	pRot,		-1,		">>",		"n = n >> n",
"operator: bitwise shift right, padding with 0.\n"
"e.g.: 32>>2  = 8\n"
"      544>>8 = 2.125\n"
"      1.5>>1 = 0.75" ),

TOKEN(tANDAND,	pBoo,		-1,		"&&",		"n = n && n",
"operator: logical AND.\n"
"if number1 is false, then number2 is not evaluated (pruning)\n"
"e.g.: if (a==0&&a!=0) log \"this can not happen\" then." ),

TOKEN(tOROR,	pBoo,		-1,		"||",		"n = n || n",
"operator: logical OR.\n"
"if number1 is true, then number2 is not evaluated (pruning)\n"
"e.g.: if (a==0||a!=0) log \"this is always true\" then." ),

TOKEN(tAND,		pBit,		-1,		"&",		"n = n & n",
"operator: bitwise AND.\n"
"currently this operation is limited to 32 bit integer values.\n"
"e.g.: $FE & $0F = $0E." ),

TOKEN(tOR,		pBit,		-1,		"|",		"n = n | n",
"operator: bitwise OR.\n"
"currently this operation is limited to 32 bit integer values.\n"
"e.g.: $0E | $58 = $5E." ),

TOKEN(tXOR,		pBit,		-1,		"^",		"n = n ^ n",
"operator: bitwise XOR (exclusive or).\n"
"currently this operation is limited to 32 bit integer values.\n"
"e.g.: $0E ^ $5E = $50."	),

TOKEN(tDOT,		pDot,		+0, 	".",		"v = l.i",
"operator: select named item in list.\n"
"e.g.: var a={1,2,3}; var a.b=4; rename a[2]=\"z\"; log a.z,a.b;" ),


// ----	triadic operators -----------------------------------------

TOKEN(tQMARK,	pSel,		-1,		"?",		"x = n1 ? x2 : x3",
"operator: select expression2 or 3 based on number1.\n"
"if number1 is false then expression2 is not evaluated\n"
"else expression3 is skipped. (pruning)\n"
"operator can be extended like: a = f1 ? n1 : f2 ? n2 : f3 ? n3 : n4\n"
"e.g.: return a>b ? a : b;"	),

TOKEN(tCOLON,	pSel,		-1,		":",		"x = n1 ? x2 : x3",
"operator: select expression2 or 3 based on number1.\n"
"if number1 is false then expression2 is not evaluated\n"
"else expression3 is skipped. (pruning)\n"
"operator can be extended like: a = f1 ? n1 : f2 ? n2 : f3 ? n3 : n4\n"
"e.g.: return a>b ? a : b;"	),



// ==== now regular identifier-type words follow ==================


// ----	special words (sort of separator; flags) ------------------

TOKEN(tFROM, 	pComma,		0,		"from",		"from",								// same prio as tKOMMA
"special separator for 'convert' statement." ),

TOKEN(tTO,		pComma,		0,		"to",		"t|l [ [n1] [to] [n] [,…] ]",		// same prio as tKOMMA
"Special separator for 'convert' statement.\n"
"Subrange selector for texts or lists; e.g. my_list[a to e]" ),

TOKEN(tPRIO,	pComma,		0,		"prio",		"spawn proc() prio n",
"Set the run priority for a new thread" ),


// ----	constants -------------------------------------------------

TOKEN(tEC,		valueNoArg,		+1,		"ec",		"ec",
"Euler's constant: e = 2.718281828459045.\n"
"f'(x)=f(x)  <=>  f(x)=exp(e,x)." ),

TOKEN(tPI,		valueNoArg,		+1,		"pi",		"pi",
"natural constant: π = 3.141592653589793.\n"
"circumference of circle = diameter * π.\n"
"e.g.: put \"sin(180°)=\",sin(pi);" ),

TOKEN(tNEWLINE,	valueNoArg,		+1,		"nl",		"nl",
"text constant for line break.\n"
"e.g.: put \"hello!\",nl;" ),

TOKEN(tTAB,		valueNoArg,		+1,		"tab",		"tab",
"text constant for tabulator.\n"
"e.g.: put \"foo =\",tab,foo,nl;" ),

TOKEN(tSTDIN,	valueNoArg,		+1,		"stdin",	"stdin",
"constant for the standard input stream: 0.\n"
"this is the default stream for 'get'.\n"
"printing to stdin on a tty colorizes the text as set in ~/.vipsi/*.prefs\n"
"e.g.: get foo  =  get #stdin,foo." ),

TOKEN(tSTDOUT,	valueNoArg,		+1,		"stdout",	"stdout",
"constant for the standard output stream: 1.\n"
"this is the default stream for 'put'.\n"
"printing to stdout on a tty colorizes the text as set in ~/.vipsi/*.prefs\n"
"e.g.: put foo  =  put #stdout,foo." ),

TOKEN(tSTDERR,	valueNoArg,		+1,		"stderr",	"stderr",
"constant for the standard error stream: 2.\n"
"this is the default stream for 'log'.\n"
"printing to stderr on a tty colorizes the text as set in ~/.vipsi/*.prefs\n"
"e.g.: log foo  =  put #stderr,foo."	),


// ----	functions -------------------------------------------------

TOKEN(tNOT,		procOneArg,		+0,		"!",		"n = !n",
"logical negation.\n"
"e.g.: if (!exists a) put \"a missing\",nl then." ),

TOKEN(tTILDE,	procOneArg,		+0,		"~",		"n = ~n",
"bitwise negation.\n"
"e.g.: a = a & ~$20." ),

TOKEN(tAT,		procOneArg,		+0,		"@",	"i = @t",
"create identifier from text expression.\n"
"argument binding of this prefix operator is the highest of all operators.\n"
"even higher than […] index and (…) procedure call.\n"
"e.g.: var t=\"Anton\"; put @\"t\"; // prints 'Anton'.\n"
"      var n; put\"variable: \"; get n; try put \" = \", @n; else put \"n.ex.\"; then\n"
"      var l={a=3,b=\"qwertz\",c=5}; put l.@\"b\"[3]; // prints 'e'" ),

TOKEN(tFILE,	procOneArg,		+0,		"file",		"t=file t | file t=t | file t#=t | file t",
"1) read file in binary mode. yields a text value.\n"
"2) save text in binary mode to file.\n"
"3) append text in binary mode to file.\n"
"4) device specifier for paths and urls, e.g. var f = file \"path/to/file\"" ),

TOKEN(tDIR,		procOneArg,		+0,		"dir",		"dir | l=dir t | dir t",
"get contents of named directory.\n"
"name may be full or partial or start with ~/ for user's home directory\n"
"and may contain a match pattern with wildcards for files to list.\n"
"if the path is \"\" then info abount mounted devices is listed instead.\n"
"1) shell command: displays beautified folder listing.\n"
"2) function:      returns a 2-dimensional list.\n"
"3) device specifier in path or url, e.g. put dir \"path/to/dir/\"" ),

TOKEN(tLINK,	procOneArg,		+0,		"link",		"t=link t | link t=t | link t",
"get or set contents of named link.\n"
"name may be full or partial or start with ~/ for user's home directory.\n"
"1) function: get or set link.\n"
"2) device specifier in path or url, e.g. var l = link \"path/to/symbolic/link\"" ),

TOKEN(tPIPE,	procOneArg,		+0,		"pipe",		"pipe t",
"device specifier in path or url, e.g. openin#4, pipe \"path/to/named/pipe\"" ),

TOKEN(tSOCK,	procOneArg,		+0,		"sock",		"sock t",
"device specifier in paths and urls, e.g. open#4, sock \"domain:type:protocol\"" ),

TOKEN(tMDATE,	procOneArg,		+0,		"mdate",	"n=mdate t",
"get modification date of file. yields a numeric value."	),

TOKEN(tCOUNT,	procOneArg,		+0,		"count",	"count s|l",
"querry number of elements in text or list"				),

TOKEN(tINDEX,	procOneArg,		+0,		"index",	"index x",
"get position of item in list" ),

TOKEN(tGETNAME,	procOneArg,		+0,		"name",		"name v",
"querry name of variable. e.g.: name a[0]"				),

TOKEN(tRANDOM,	procOneArg,		+0,		"random",	"random n",
"get a random number in range [0…[n (float!)"			),

TOKEN(tABS,		procOneArg,		+0,		"abs",		"abs n",
"unsigned, absolute value of numeric value"				),

TOKEN(tSIGN,	procOneArg,		+0,		"sign",		"sign n",
"sign of numeric value"									),

TOKEN(tINT,		procOneArg,		+0,		"int",		"int n",
"integral part of numeric value"						),

TOKEN(tFRACT,	procOneArg,		+0,		"fract",	"fract n",
"fractional part of numeric value"						),

TOKEN(tROUND,	procOneArg,		+0,		"round",	"round n",
"round to nearest. xx.5 rounds upwards."				),

TOKEN(tMIN,		procWithArg,	+0,		"min",		"v=min(n[,…]) | v=min(l)",
"minimum of values. result can be a variable reference. if a list is passed\n"
"as single argument, then a reference to the smallest item is returned.\n"
"error is set if different data types are compared.\n"
"if two values are equal, the first is returned.\n"
"e.g.: a=min(a,99); put min({1,6,3,4}); min(a,b)=0; put name(min(a,b));"),

TOKEN(tMAX,		procWithArg,	+0,		"max",		"v=max(n[,…]) | v=max(l)",
"maximum of values. result can be a variable reference. if a list is passed\n"
"as single argument, then a reference to the largest item is returned.\n"
"error is set if different data types are compared.\n"
"if two values are equal, the first is returned.\n"
"e.g.: a=max(a,99); put max({1,6,3,4}); max(a,b)=0; put name(max(a,b));"),

TOKEN(tCOS,		procOneArg,		+0,		"cos",		"n=cos n",
"cosinus of angle.\n"
"full circle = 2*pi." ),

TOKEN(tSIN,		procOneArg,		+0,		"sin",		"n=sin n",
"sinus of angle.\n"
"full circle = 2*pi." ),

TOKEN(tTAN,		procOneArg,		+0,		"tan",		"n=tan n",
"tangens of angle.\n"
"full circle = 2*pi." ),

TOKEN(tACOS,	procOneArg,		+0,		"acos",		"n=acos n",
"angle from cosinus.\n"
"result range: 0 … +pi." ),

TOKEN(tASIN,	procOneArg,		+0,		"asin",		"n=asin n",
"angle from sinus.\n"
"result range: -pi/2 … +pi/2." ),

TOKEN(tATAN,	procOneArg,		+0,		"atan",		"n=atan n",
"angle from tangens.\n"
"result range: -pi/2 … +pi/2." ),

TOKEN(tCOSH,	procOneArg,		+0,		"cosh",		"n=cosh n",
"cosinus hyperbolicus." ),

TOKEN(tSINH,	procOneArg,		+0,		"sinh",		"n=sinh n",
"sinus hyperbolicus." ),

TOKEN(tTANH,	procOneArg,		+0,		"tanh",		"n=tanh n",
"tangens hyperbolicus." ),

TOKEN(tLOG,		procWithArg,	-1,		"log",		"n=log(n1,n2) | log x[,…]",
"(1) function: logarithm of number 2 to base number 1.\n"
"    e.g.: log(16,256) = 2.\n"
"(2) instruction: write data to stderr.\n"
"    printing to stderr on a tty colorizes the text as set in ~/.vipsi/*.prefs\n"
"    except for that, instruction 'log' works exactly like 'put'." ),

TOKEN(tLOG2,	procOneArg,		+0,		"log2",		"n=log2 n",
"logarithm of number to base 2.\n"
"e.g.: log2(4) = 2." ),

TOKEN(tLOGE,	procOneArg,		+0,		"loge",		"n=loge n",
"logarithm of number to base e.\n"
"e.g.: loge(ec*ec) = 2." ),

TOKEN(tLOG10,	procOneArg,		+0,		"log10",	"n=log10 n",
"logarithm of number to base 10.\n"
"e.g.: log10(100) = 2." ),

TOKEN(tEXPN,	procWithArg,	-1,		"exp",		"n=exp(n1,n2)",
"number 1 raised to the power of number 2.\n"
"e.g. exp(16,2) = 256." ),

TOKEN(tEXP2,	procOneArg,		+0,		"exp2",		"n=exp2 n",
"2 raised to the power of number.\n"
"e.g. exp2(3) = 8." ),

TOKEN(tEXPE,	procOneArg,		+0,		"expe",		"n=expe n",
"e raised to the power of number.\n"
"e.g.: expe(2) = e*e." ),

TOKEN(tEXP10,	procOneArg,		+0,		"exp10",	"n=exp10 n",
"10 raised to the power of number.\n"
"e.g.: exp10(2) = 100." ),

TOKEN(tSQUARE,	procOneArg,		+0,		"square",	"n=square n",
"number * number.\n"
"e.g.: square 5 = 25." ),

TOKEN(tSQRT,	procOneArg,		+0,		"sqrt",		"n=sqrt n",
"square root of number.\n"
"e.g.: sqrt 25 = 5." ),

TOKEN(tISTEXT,	procOneArg,		+0,		"istext",	"n=istext x",
"test whether expression is a text value.\n"
"e.g.: istext(\"foo\") = 1; istext({}) = 0." ),

TOKEN(tISNUMBER,procOneArg,		+0,		"isnumber",	"n=isnumber x",
"test whether expression is a numeric value.\n"
"e.g.: isnumber(1+2) = 1; isnumber({}) = 0." ),

TOKEN(tISLIST,	procOneArg,		+0,		"islist",	"n=islist x",
"test whether expression is a list.\n"
"e.g.: islist({1,2}) = 1; islist(12) = 0." ),

TOKEN(tISPROC,	procOneArg,		+0,		"isproc",	"n=isproc x",
"test whether expression is a user-defined procedure.\n"
"e.g.: proc foo(){}; isproc(foo) = 1; isproc(\"foo\") = 0." ),

TOKEN(tISFILE,	procOneArg,		+0,		"isfile",	"n=isfile t",
"test whether path refers to a file.\n"
"also returns 1 if path is a symlink which refers to a file."
"e.g.: var a = isfile(\"~/bin/vipsi\");" ),

TOKEN(tISDIR,	procOneArg,		+0,		"isdir",	"n=isdir t",
"test whether path refers to a directory.\n"
"also returns 1 if path is a symlink which refers to a directory."
"e.g.: isdir(\"/\") = 1; isdir(\"\") = 0." ),

TOKEN(tISLINK,	procOneArg,		+0,		"islink",	"n=islink t",
"test whether path refers to a symbolic link.\n"
"e.g.: var a = islink(\"/var/tmp\");" ),

TOKEN(tEXISTS,	procOneArg,		+1,		"exists",	"n=exists v | n=exists file|dir|link t",
"test whether variable, file, directory or link exists.\n"
"(1) var a={1,2,3}; exists a[3] = 1; exists a[5] = 0;\n"
"(2) exists file \"/tmp/v-3123\"; exists dir \"~/bin/\";" ),

TOKEN(tISTRIGGERED,	procOneArg,	+0,		"istriggered",	"n=istriggered v",
"test whether the passed interrupt is triggered." ),

TOKEN(tBOOL,	procOneArg,		+0,		"bool",		"n=bool x",
"convert expression to boolean number 0 or 1" ),

TOKEN(tTOTEXT,	procOneArg,		+0,		"string",	"t=string x",
"convert expression to text representation. \n"
"e.g.: string(47+11) = \"58\"." ),

TOKEN(tTOUPPER,	procOneArg,		+0,		"upperstr",	"t=upperstr t",
"convert text to all uppercase characters conforming to utf-8 unicode.\n"
"e.g. \"HELLO WORLD!\" = upperstr(\"Hello World!\")." ),

TOKEN(tTOLOWER,	procOneArg,		+0,		"lowerstr",	"t=lowerstr t",
"convert text to all lowercase characters conforming to utf-8 unicode.\n"
"e.g. \"hello world!\" = lowerstr(\"Hello World!\")." ),

TOKEN(tSPACESTR, procWithArg,	+0,		"spacestr",	"t=spacestr(n[,t])",
"Return a text filled with <number> spaces.\n"
"Optionally accepts a fill pattern. if this pattern is longer than 1 character,\n"
"then the resulting text will be <number> * count(<text>) characters long.\n"
"If the fill pattern is an empty text, then the result is an empty text too.\n"
"e.g. var spc=spacestr(12), foo=spacestr(99,\"\"), mupl=spacestr(5,\"hello!\");" ),

TOKEN(tCHARSTR,	procOneArg,		+0,		"charstr",	"t=charstr n",
"convert ucs-4 character (number) to utf-8 character (text).\n"
"e.g.: charstr 97 = \"a\"." ),

TOKEN(tCHARCODE, procOneArg,	+0,		"charcode",	"n=charcode t",
"convert first utf-8 character in text to ucs-4 character number.\n"
"e.g.: charcode \"a\" = 97." ),

TOKEN(tHEXSTR, 	procWithArg,	+0,		"hexstr",	"t=hexstr(n1[,n2])",
"convert number 1 to hexadecimal digit string.\n"
"resulting text is sized to fit or\n"
"sized according to number 2, truncating or padding 0's to the left.\n"
"e.g.: hexstr(240) = \"F0\"; hexstr(240,4) = \"00F0\"; hexstr(240,1) = \"0\"." ),

TOKEN(tBINSTR, 	procWithArg,	+0,		"binstr",	"t=binstr(n1[,n2])",
"convert number 1 to binary digit string.\n"
"resulting text is sized to fit or \n"
"sized according to number 2, truncating or padding 0's to the left.\n"
"e.g.: binstr(10,8) = \"00001010\"; binstr(10,2) = \"10\"." ),

TOKEN(tSUBSTR,	procWithArg,	+0,		"substr",	"t=substr(t,n1,n2)",
"substring [number1 to number2] from text\n"
"-> number2-number1+1 characters.\n"
"e.g.: substr(\"123456\",3,4) = \"34\"" ),

TOKEN(tMIDSTR,	procWithArg,	-1,		"midstr",	"t=midstr(t,n1[,n2])",
"get substring [number1 to number1+number2-1] from text\n"
"-> number2 characters.)\n"
"if number2 is omitted, then get substring up to end of text.\n"
"e.g.: midstr(\"123456\",3,2) = \"34\"" ),

TOKEN(tLEFTSTR,	procWithArg,	-1,		"leftstr",	"t=leftstr(t,n)",
"substring: get number bytes from the left side of the text.\n"
"same as text[to number]." ),

TOKEN(tRIGHTSTR,procWithArg,	-1,		"rightstr",	"t=rightstr(t,n)",
"substring: get number bytes from the right side of the text.\n"
"e.g.: rightstr(\"anton\",3) = \"ton\"." ),

TOKEN(tDATESTR,	procOneArg,		+0,		"datestr",	"t=datestr n",
"convert numeric date format to readable text.\n"
"e.g.: datestr 0 = 1970-01-01 01:00:00." ),

TOKEN(tNOW, 	valueNoArg,		+1,		"now", 		"n=now",
"get system time.\n"
"returns local time in seconds incl. a fractional part.\n"
"time base may vary!\n"
"e.g. log\"current date and time = \",datestr(now),nl;\n"
"     var a=0,t0=now; do while ++a<100000; loop; put \"time=\",now-t0;"),

TOKEN(tDATEVAL,	procOneArg,		+0,		"dateval",	"n=dateval t",
"convert readable date text to numeric format.\n"
"e.g.: dateval \"2002-04-01 12:00\" = 1017658800.\n"
"	  dateval \"2002.04\"          = 1017529200." ),

TOKEN(tFIND,	procWithArg,	-1,		"find",		"n=find(t1,t2[,n3])",
"find first occurance of text 2 in text 1.\n"
"optionally starting search at index number 3\n"
"returns index of position found or 0.\n"
"e.g.: find(\"hello world\",\"l\") = 3." ),

TOKEN(tRFIND,	procWithArg,	-1,		"rfind",	"n=rfind(t1,t2[,n3])",
"find last occurance of text 2 in text 1.\n"
"optionally starting search at index number 3\n"
"returns index of position found or 0.\n"
"e.g.: rfind(\"hello world\",\"l\") = 10." ),

TOKEN(tERROR,	valueNoArg,		+1,		"error",	"l=error",
"((work in progress))\n"
"(1) return error description list for last error:\n"
"    <list> = { text=<text>, row=<number>, col=<number>, file=<text> }\n"
"(2) raise error:\n"
"    error <text>" ),

TOKEN(tENV, 	valueNoArg,		+1,		"env", "l=env",
"returns a list of all environment variables.\n"
"procedure:\n"
"    returns a list with one item for each environment variable.\n"
"    each item is a text containing the variable's value.\n"
"    each item is named with the environment variable's name.\n"
"    e.g.: var i=0; do while ++i<=count env put name env[i],\"=\",env[i],nl loop;\n"
"shell command:\n"
"    displays beautified listing of all environment variables." ),

TOKEN(tWORDS,	valueNoArg,		+1,		"words",	"l=words",
"returns a list of all known tokens, procedures and functions.\n"
"shell command:\n"
"    displays a beautified listing of all words.\n"
"function:\n"
"    returns a list containing all names." ),

TOKEN(tINFO,	procOneArg,		+0,		"info",		"t = info t",
"returns usage information on a token, built-in function or instruction.\n"
"shell command:\n"
"    displays beautified information for token or built-in word.\n"
"    the token/word may be appended without quotes.\n"
"function:\n"
"    returns a text value which contains the usage information." ),

TOKEN(tROOT, valueNoArg,		+1,		"root",		"l=root",
"returns the list with all data of the current instance." ),

TOKEN(tGLOBALS, valueNoArg,		+1,		"globals",		"l=globals",
"returns a list of all globally defined variables.\n"
"shell command: \n"
"    display beautified listing of all global variables.\n"
"function:\n"
"    return a reference to the global variables list.\n"
"    you may access or modify variables therein.\n"
"    accessing globals directly is useful to access script arguments\n"
"    e.g.: put count(globals); put globals[1]; var a = globals.a;" ),

TOKEN(tLOCALS, valueNoArg,		+1,		"locals",		"l=locals",
"returns a list of all variables defined inside the current procedure.\n"
"returns a reference to the list. you may access or modify variables therein.\n"
"note: 'locals' does not refer to a local scope eventually define with { … }\n"
"      which may be accessed by it's name 'privates' if required.\n"
"'locals' always refers to the topmost variables list of the current procedure\n"
"or a reference to the global variable list if called outside any procedure.\n"
"e.g.: proc foo(*){ return count(locals); }" ),

TOKEN(tTHIS, 	valueNoArg,		+1,		"this",		"l=this",
"returns a reference to the list which contains the current procedure\n"
"this may be either an 'object' or the local data of an enclosing procedure.\n"
"- objects are lists which contain data items and procedures.\n"
"  a procedure in this list can access the list, it's object, using 'this'.\n"
"- procedures may contain local procedures, which can access the local data\n"
"  of their parent using 'this'.\n"
"in global procedures 'this' will access the 'globals' variables list.\n"
"normally 'this' is not required, because the variable search includes\n"
"the enclosing list, but it can speed up things." ),

TOKEN(tSELF,	valueNoArg,		+1,		"self",		"v=self",
"returns ID of current thread" ),

TOKEN(tMATCH,	procWithArg,	-1,		"match",	"n=match(t1,t2[,n])",
"test whether text 1 matches pattern in text 2.\n"
"returns boolean value: 0 (no) or 1 (yes).\n"
"the optional flag consists of 4 bits:\n"
"    1 = enable backslash escaping\n"
"    2 = directory separators must match directly\n"
"    4 = initial dot in each file or directory must match directly\n"
"    8 = compare case-insensitive" ),

TOKEN(tFULLPATH,procOneArg,		+0,		"fullpath",	"t=fullpath t",
"get fully qualified path for partial path.\n"
"expands \"./\" and \"../\", symbolic links, \"~\" and relative path\n"
"to the absolute file or directory path." ),


// ----	instructions ------------------------------------------------------

// mixed instruction or function:

TOKEN(tEVAL,	procOneArg,		+0,		"eval",		"eval t  |  x=eval t",
"evaluate text as expression, e.g. a = eval(\"2*3\")\n"
"the text is parsed just like a script, so almost every expression\n"
"is possible here, including execution of own procedures.\n"
"also, the text can access all variables from the current context." ),

TOKEN(tCALL,	procWithArg,	+0,		"call",		"call t1[,…]  |  x=call(t1[,…])",
"call another vipsi script like a function with parameters and return a result.\n"
"the called script text1 is very similar to a standard shell script\n"
"except that it can be passed and return variables of any type.\n"
"it shares the preferences of the current context\n"
"but cannot access it's variables and functions.\n"
"e.g.: var u_rec = call(\"utils/fetch_db_entry.vs\",user_id,number)" ),

TOKEN(tSYS,		procWithArg,	+0,		"sys",		"sys [#n,]t1[,l|,…]  |  t=sys(t1[,l|,…])",
"execute a system command or script with parameters and return it's text output.\n"
"the command text1 is executed and passed all the supplied text parameters.\n"
"text parameters may be passed in a single list or as individual arguments.\n"
"wildcards or '~' are not expanded!\n"
"text output to stderr is printed to stderr.\n"
"error condition is set if the command returns non-zero or aborts on a signal.\n"
"(1) instruction: print output to stdout or selected stream number.\n"
"(2) function:    all it's output to stdout is returned as a single text value.\n"
"e.g.: sys \"rm\", \"text1~\", \"text2~\";\n"
"      var textout = sys( \"rm\", \"text1~\", \"text2~\" ); put textout;\n"
"      var filelist = {\"text1~\",\"text2~\"}; sys \"rm\", filelist;" ),

TOKEN(tINCLUDE,	procOneArg,		+0,		"include",	"include t[,…]  |  x=include t",
"load and execute named script file.\n"
"the script is executed in the current context.\n"
"this means it behaves just as if pasted verbatim at the current position.\n"
"all variables and procedures defined in this script still exist after it's end.\n"
"'end' or 'return' exit from the script only, not from the current context.\n"
"also, the script may return a result or error code with 'return'." ),

TOKEN(tREQUIRE,	procOneArg,		+0,		"require",	"require t[,…]  |  x=require t",
"load and execute a library file. ((***TODO***))\n"
"the script is executed in the current context.\n"
"this means it behaves just as if pasted verbatim at the current position.\n"
"all variables and procedures defined in this script still exist after it's end.\n"
"'end' or 'return' exit from the script only, not from the current context.\n"
"also, the script may return a result or error code with 'return'.\n"
"require is similar to include, except that you give a nickname, not a filename\n"
"and vipsi searches in the predefined library directories for that file." ),

TOKEN(tSPAWN,	procWithArg,	-1,		"spawn",	"v = spawn proc(arguments) [prio n]",
"spawn a new thread which executes the defined procedure.\n"
"returns the thread ID of the new thread" ),

TOKEN(tSPLIT,	procWithArg,	+0,			"split",	"split v1[,t2]  |  l=split(t1[,t2])",
"split text1 at any occurance of text2. yields a list of texts.\n"
"1) instruction: the contents of variable1 is replaced by the result list\n"
"2) function: the result is returned and argument text1 is not modified\n"
"the separator text2 is removed, not preserved.\n"
"if text2 is omitted then text1 is split at char 0, 10, 13 and 10+13\n"
"if text2 is a numeric argument then it is taken as a character code\n"
"if text1 ends with text2 then a final empty text is appended to the list.\n"
"if text1 is a list of texts then the result is a list of lists of texts.\n"
"e.g.: var mytext  = file \"myfile\"; split mytext;\n"
"      var mydbase = split(mytext,\"\\t\")"										),

TOKEN(tJOIN,	procWithArg,	-1,			"join",	"join v,t  |  v=join(l,t)",
"join list of texts into a single text, linking the texts with separator t.\n"
"1) instruction: the contents of the variable is replaced by the result text\n"
"2) function: the result is returned and the argument list is not modified\n"
"if a list item is no text then it is converted to text.\n"
"if the variable is no list then it is simply converted to text.\n"
"e.g.: join mytext,nl; file \"myfile\" = mytext;\n"								),

TOKEN(tCONVERT,	procWithArg,	+0,			"convert",	"convert v from|to i  |  t=convert(t from|to i)",
"convert text to or from named encoding.\n"
"(1) instruction: if used as an instruction, the supplied variable is converted.\n"
"(2) function: if used as a function, the converted text is returned as result.\n"
"possible encoding:\n"
"    printable    escape characters breaking vipsi's syntax for text literals.\n"
"    quoted       escape and add '\"'s at both ends\n"
"    html         convert > < & and \" to &gt; etc.; decode named|code entities.\n"
"    url          convert to/from utf-8 and en/decode special chars for URIs.\n"
"    url_all      as 'url' but also encode the directory separator '/'.\n"
"    tabs[2..9]   entab/detab with tab spacing 2 .. 9 characters.\n"
"    mac_roman    convert to mac roman character set\n"
"    iso_latin_1  convert to iso latin 1 character set\n"
"    windows      convert to iso latin 1\n"
"    utf_8        convert to utf-8 unicode (default encoding for put, log, get…)\n"
"    ucs_2        convert to 2-byte unicode\n"
"    ucs_4        convert to 4-byte unicode\n"
"    ascii_us     convert to 7-bit ascii\n"
"    ascii_ger    convert to 7-bit ascii, German variant (Ä…ß instead of […\\)\n"
"    dos          convert to dos latin US character set (codepage 437)\n"
"    atari_st     convert to atari st character set\n"
"e.g.: convert mytext to printable;\n"
"      convert mytext to mac_roman;\n"
"      convert mytext from html;\n"
"      put convert(mytext to quoted);"),

TOKEN(tREPLACE,	procWithArg,	-2,		"replace",	"replace t1,t2,t3  |  t=replace(t1,t2,t3)",
"scan text 1 and replace any occurance of text 2 with text 3.\n"
"text 1 is scanned from left to right and replaced text is not scanned again.\n"
"if the search text 2 is an empty string, then nothing is done.\n"
"(1) if used as instruction, variable text 1 is replaced by the result.\n"
"(2) if used as function, the modified text is returned as result.\n"
"e.g.: put replace(\"Anton\",\"n\",\"N\");  // ANtoN\n"
"      replace mytext,\"boo\",\"booboo\";\n"
"      mynewtext = replace(mytext,\"boo\",\"booboo\");" ),


// from here only pure instructions:

TOKEN(tNEW,		instruction,	+0,		"new",		"new [i[=x]|file t|dir t][,…]",
"create variable, file or directory.\n"
"opt. assign value to variable." ),

TOKEN(tDEL,		instruction,	-1,		"del",		"del [i|file t|dir t][,…]",
"delete and forget variable, file or directory.\n"
"e.g.: del a,b[1],a.wert" ),

TOKEN(tSWAP,	instruction,	-2,		"swap",		"swap [v,v|file t,file t][,…]",
"swap contents of two variables or files.\n"
"e.g.: var z.a=100,z.b=200; swap z[1],z[2]; put z;" ),

TOKEN(tVAR,		instruction,	0,		"var",		"var i[=x][,…]",
"Create variable if it does not already exist.\n"
"optionally assign initial value" ),

TOKEN(tFREEZE,	instruction,	+0,		"freeze",	"freeze [file] t",
"compress text or file. undo compression with melt.\n"
"note: files must fit into ram twice.\n"
"e.g.: var a=\"fuzzy\";a#=a;a#=a;a#=a; freeze a; count a;" ),

TOKEN(tMELT,	instruction,	+0,		"melt",		"melt [file] t",
"uncompress text or file which has been compressed with freeze.\n"
"note: files must fit into ram twice.\n"
"e.g.: var a=\"fuzzy\";a#=a;a#=a;a#=a; freeze a; melt a;" ),


TOKEN(tPUT,		instruction,	-1,		"put",		"put[#n,]x[,…]",
"write data in text format.\n"
"default stream is stdout.\n"
"default text encoding is UTF-8 unicode.\n"
"printing to stdout on a tty colorizes the text as set in ~/.vipsi/*.prefs\n"
"isText:   'as is', character set and end-of-line conversion applied.\n"
"isNumber: numbers are written in readable format; e.g. +123.456e-78.\n"
"isList:   enclosed in curly brackets; e.g.: { 123, \"anton\", {} }.\n"
"isProc:   prints source (if available) or disassembly." ),

TOKEN(tGET,		instruction,	-2,		"get",		"get[#n,]v[,…]",
"read data in text format.\n"
"default stream is stdin.\n"
"default text encoding is UTF-8 unicode.\n"
"input stops at next end of line for each variable.\n"
"reading from a tty (terminal) uses the line editor.\n"
"printing to stdin on a tty colorizes the text as set in ~/.vipsi/*.prefs" ),

TOKEN(tEDIT,	instruction,	-2,		"edit",		"edit[#n,]v[,…]",
"edit (write and reread) data in text format.\n"
"default stream is stdin.\n"
"default text encoding is UTF-8 unicode.\n"
"edit finished with end of line (return) for each variable.\n"
"reading from a tty (terminal) uses the line editor.\n"
"data presentation is the same as for 'put'." ),

TOKEN(tGETCHAR,	instruction,	-1,	"getchar",		"getchar[#n,]v[,…]",
"read single character in text format.\n"
"default stream is stdin.\n"
"default text encoding is UTF-8 unicode.\n"
"-> returns empty string if no input available.\n"
"reads from tty (terminal) without echo." ),

TOKEN(tPUTBACK,	instruction,	-1,	"putback",		"putback[#n,]x[,…]",
"put back text to an input stream.\n"
"default stream is stdin.\n"
"text is put back as text and returned with subsequent 'get's or 'getchar's.\n"
"'read' does not return text from the putback buffer.\n"
"you can put back any text and as many text as you like.\n"
"e.g.: putback \"A\"; putback nl; putback spacestr(n,charstr(8));" ),

TOKEN(tWRITE,	instruction,	-1,	"write",	"write[#n,]v",
"write text in binary mode.\n"
"default stream is stdout.\n"
"no conversions applied. text is written 'as is'." ),

TOKEN(tREAD,	instruction,	-2,	"read",		"read[#n,]v",
"read text in binary mode.\n"
"default stream is stdin.\n"
"reads from tty (terminal) without echo.\n"
"no conversion applied. text is read 'as is'.\n"
"destination text variable is filled with data from stream.\n"
"on timeout, eof, or error, the text is truncated." ),

TOKEN(tIOCTL,	instruction,	+0,	"ioctl",	"ioctl[#n,] [i=x|v=i]",
"Set stream settings or querry stream state\n"
"•••under development•••\n"
"set only:\n"
"    ioctl flush|flushin|flushout\n"
"set/read:\n"
"    ioctl timeout = 0 (off)  or  number (seconds)\n"
"    ioctl text    = \"utf-8\"  or  \"mac-roman\"\n"
"    ioctl pos     = number         (file: read-write-pointer)\n"
"                    {row,col}      (tty:  cursor location)\n"
"read only:\n"
"    ioctl var = type               (\"tty\", \"file\", etc.)\n"
"    ioctl var = inavail            (bytes readable w/o blocking)\n"
"    ioctl var = outfree            (bytes writable w/o blocking)\n"
"    ioctl var = error              (0=ok, 1=eof, 2=timeout, ...)\n"
"    ioctl var = size               (file: number, tty: {rows,cols})\n"
"    ioctl var = name               (filename)\n"
"    ioctl var = isopen/isopenin/isopenout" ),

TOKEN(tWAIT,		instruction,	-1,		"wait",		"wait n",
"wait number seconds.\n"
"e.g.: wait 0.33" ),

TOKEN(tSHEDULE,		instruction,	-1,	"shedule",	"shedule n",
"suspend execution and shedule resume for the given system time" ),

TOKEN(tSUSPEND,		instruction,	-1,	"suspend",		"suspend v",
"suspend execution of a thread" ),

TOKEN(tRESUME,		instruction,	-1,	"resume",		"resume v",
"resume execution of a thread" ),

TOKEN(tTERMI,		instruction,	-1,	"terminate",	"terminate v",
"terminate a thread" ),

TOKEN(tTRIGGER,		instruction,	-1,	"trigger",		"trigger v",
"trigger an interrupt" ),

TOKEN(tREQUEST,		instruction,	-1,	"request",		"request v",
"request a semaphore" ),

TOKEN(tRELEASE,		instruction,	-1,	"release",		"release v",
"release a semaphore" ),




TOKEN(tSORT,	instruction,	-1,		"sort",		"sort l[,…]",
"sort list in ascending order"													),

TOKEN(tRSORT,	instruction,	-1,		"rsort",	"rsort l[,…]",
"sort list in descending order"													),

TOKEN(tSHUFFLE,	instruction,	-1,		"shuffle",	"shuffle l[,…]",
"shuffle list"																	),

TOKEN(tOPEN,	instruction,	-2,		"open",		"open[#n,]filename",
"open file/stream for reading and writing"										),

TOKEN(tOPENIN,	instruction,	-2,		"openin",	"openin[#n,]filename",
"open file/stream for reading"													),

TOKEN(tOPENOUT,	instruction,	-2,		"openout",	"openout[#n,]filename",
"open file/stream for writing, clearing file"									),

TOKEN(tCLOSE,	instruction,	-1,		"close",	"close[#n]",
"close open file/stream"														),

TOKEN(tRENAME,	instruction,	-1,		"rename",	"rename v=t  |  rename [file|link|dir] t=t",
"rename variable. e.g. rename a[0]=\"anton\"\n"
"rename file, dir or link. e.g. rename link \"foo\" = \"bär\";" ),


// ----	not tKOMMA-repeatable instructions ---------------------------------

TOKEN(tCD,		instruction,	-1,		"cd",		"cd t",
"change working directory. the path may be full or partial\n"
"and may start with ~ for your home directory.\n"
"shell command:\n"
"    the path may either be a text expression\n"
"    or may be written unquoted and unescaped.\n"
"instruction:\n"
"    the path may not be that quick&dirty unquoted stuff.\n"
"    e.g.: cd \"../\";" ),

TOKEN(tPROC,	instruction,	+1,		"proc",		"proc i (…) { … }",
"define a procedure/function/sub routine.\n"
"e.g.: proc fak(n) { if (n>1) return n*fak(n-1); then; return 1; };\n"
"      log \"fak(5) = \",fak(5),nl;" ),

TOKEN(tRETURN,	instruction,	-1,		"return",	"return [x]",
"end function optionally returning a result value.\n"
"e.g.: proc geomed(a,b) { return sqrt(a*b); }" ),

TOKEN(tEND,		instruction,	-1,		"end",		"end [x]",
"instruction:\n"
"    end script optionally returning a value.\n"
"shell command:\n"
"    quit vipsi. synonym for \"quit\"." ),


TOKEN(tDO,		instruction,	+0,		"do",		"do … loop",
"repeat statements between do and loop.\n"
"e.g.: var i=1; do; put i; while i<10; i++; loop;" ),

TOKEN(tTHEN,	instruction,	+0,		"then",		"if n … [else …] then",
"execute statements only if value is true.\n"
"optional else branch if value is false.\n"
"e.g.: if (a>b) return a; else return b; then;"	),


// ----	instructions with immediate arguments -----------------------------

TOKEN(tIF,		instruction,	-1,		"if",		"if n … [else …] then",
"execute statements only if value is true.\n"
"optional else branch if value is false.\n"
"e.g.: if (a>b) return a; else return b; then;"	),

TOKEN(tELSE,	instruction,	+0,		"else",		"if n … [else …] then",
"execute statements only if value is true.\n"
"optional else branch if value is false.\n"
"e.g.: if (a>b) return a; else return b; then;"	),

TOKEN(tELIF,	instruction,	+0,		"elif",		"if n1 … elif n2 … [else …] then",
"execute statements only if preceeding tests failed and this value2 is true.\n"
"optional else branch or further elifs are considered if value2 is false.\n"
"e.g.: if (a>b) return -1; elif (a<b) return +1; else return 0; then;"	),

TOKEN(tTRY,		instruction,	+0,		"try",		"try … [else …] then",
"execute statements but don't abort on run-time errors.\n"
"optional 'else' branch is executed if a run-time error occured.\n"
"then variable 'errorstr' is created and contains the (localized) error message.\n"
"e.g.: try openin \"testfile\"; else log \"testfile error: \",errorstr,nl then;" ),

TOKEN(tWHILE,	instruction,	-1,		"while",	"do … while n … loop",
"repeat statements between do and loop while value is true.\n"
"e.g.: var i=1; do; while i<100; i+=i; loop; log i;" ),

TOKEN(tUNTIL,	instruction,	-1,		"until",	"do … until n … loop",
"repeat statements between do and loop until value becomes true.\n"
"e.g.: var i=1; do; until i>100; i+=i; loop; log i;" ),

TOKEN(tEXIT,	instruction,	+0,		"exit",		"do … exit … loop",
"instruction:\n"
"    exit from do…loop.\n"
"    e.g.: var i=1; do; if i>100 exit then; i+=i; loop; log i;\n"
"shell command:\n"
"    quit vipsi. synonym for \"quit\"." ),

TOKEN(tNEXT,	instruction,	+0,		"next",		"do … next … loop",
"immediately jump to start of do…loop for another go.\n"
"e.g.: var i=1; do; i+=i; if i<100 next then; exit; log \"never seen\" loop;" ),

TOKEN(tLOOP,	instruction,	+0,		"loop",		"do … loop",
"repeat statements between do and loop.\n"
"e.g.: var i=1; do; put i; while i<10; i++; loop;" ),

TOKEN(tEOF,		instruction,	+0,		"tEOF",			"(end of file)",
"Internally used to represent the end of a script file." ),


// ----	special codes for temporary use ----------------------------------

TOKEN(tKOMM,		pComma,		0,		"//",		"// … \\n",
"line comment: text up to end of line is a comment." ),

TOKEN(tKANF,		pComma,		0,		"/*",		"/* … */",
"block comment: text between /* and */ is a comment." ),

TOKEN(tKEND,		pComma,		0,		"*/",		"/* … */",
"block comment: text between /* and */ is a comment." ),


/* pwd:	als Funktion überflüssig -> fullpath(".")
		als shell command benötigt es kein Token
		=> Bezeichner ist frei
		=> info pwd funktioniert nicht ohne spez. Glue dort
	könnte als Token aber auch hinter die Kommentare einsortiert werden
		=> Tokenisierung sollte damit verhindert sein
		=> info pwd würde funktionieren?
TOKEN(tPWD,		valueNoArg,		+0,		"pwd",		"pwd",
"shell command: display the path of the current working directory.\n"
"similar to put fullpath(\".\")" ),
*/


TOKEN(tREQSTRING,	pPostfix,		0,	"tREQSTRING",	"", "" ),
TOKEN(tREQPROC,		pPostfix,		0,	"tREQPROC",		"", "" ),
TOKEN(tREQVAR,		pPostfix,		0,	"tREQVAR",		"", "" ),
TOKEN(tREQTEMP,		pPostfix,		0,	"tREQTEMP",		"", "" ),
TOKEN(tTOTEMP,		pPostfix,		0,	"tTOTEMP",		"", "" ),

TOKEN(tDROP,		instruction,	-1,	"tDROP",		"", "" ),
TOKEN(tDUP,			instruction,	+1,	"tDUP",			"", "" ),
TOKEN(tSWAPARGS,	instruction,	+0,	"tSWAPARGS",	"", "" ),

TOKEN(tNEG,			pUna,			+0,	"tNEG",			"", "" ),
TOKEN(tCONVERT_TO,	pPostfix,		+0,	"tCONVERT_TO",	"", "" ),
TOKEN(tCONVERT_FROM,pPostfix,		+0,	"tCONVERT_FROM","", "" ),
TOKEN(tPREDECR,		pUna,			+0,	"tPREDECR",		"", "" ),
TOKEN(tPREINCR,		pUna,			+0,	"tPREINCR",		"", "" ),
TOKEN(tRENAME_NH,	pPostfix,		+0,	"tRENAME_NH",	"", "" ),
TOKEN(tLIST,		valueNoArg,		+1,	"tLIST",		"", "" ),
TOKEN(tNOP,			pPostfix,		+0,	"tNOP",			"", "" ),
TOKEN(tNODEREF,		pPostfix,		+0,	"tNODEREF",		"", "" ),	// after tIDF etc. before tARROW
TOKEN(tIDX,			pPostfix,		-1,	"tIDX",			"", "" ),
TOKEN(tTO_IDX,		pPostfix,		-1,	"tTO_IDX",		"", "" ),
TOKEN(tIDX_TO,		pPostfix,		-1,	"tIDX_TO",		"", "" ),
TOKEN(tIDX_TO_IDX,	pPostfix,		-2,	"tIDX_TO_IDX",	"", "" ),
TOKEN(tEXECPROC,	instruction,	+0,	"tEXECPROC",	"", "" ),
TOKEN(tEXECINSTR,	instruction,	-1,	"tEXECINSTR",	"", "" ),
TOKEN(tVARIDX,		pPostfix,		-1,	"tVARIDX",		"", "" ),
TOKEN(tNEWIDX,		pPostfix,		-1,	"tNEWIDX",		"", "" ),

TOKEN(tCATCH,		instruction,	+0,	"catch",	"", "" ),
TOKEN(tVARIDF,		instruction,	+0,	"tVARIDF",	"", "" ),
TOKEN(tVARAT,		instruction,	-1,	"tVARAT",	"", "" ),
TOKEN(tNEWIDF,		instruction,	+0,	"tNEWIDF",	"", "" ),
TOKEN(tNEWAT,		instruction,	-1,	"tNEWAT",	"", "" ),
TOKEN(tDOTAT,		instruction,	-1,	"tDOTAT",	"", "" ),
TOKEN(tRETURN0,		instruction,	+0,	"return0",	"", "" ),
TOKEN(tNOCATCH,		instruction,	+0,	"nocatch",	"", "" ),

TOKEN(tSETPRIO,		instruction,	-2,	"setprio",		"setprio v,n",	"change run priority of a thread" ),
TOKEN(tCLEARSEMA,	instruction,	-1,	"clearsema",	"clearsema v",	"clear all pending requests to the passed semaphore" ),
TOKEN(tCLEARIRPT,	instruction,	-1,	"clearirpt",	"clearirpt v",	"clear all pending interrupts for the supplied interrupt cell" ),
TOKEN(tWAITIRPT,	instruction,	-1,	"waitirpt",		"", "" ),
TOKEN(tWAITIRPT_WTO,instruction,	-2,	"waitirpt_wto",	"", "" ),
TOKEN(tREQUEST_WTO,	instruction,	-2,	"request_wto",	"", "" ),

TOKEN(tCHECKSTACKS,	instruction,	+0,	"tCHECKSTACKS","", "" ),
TOKEN(tCHECKSTACK,	instruction,	+0,	"tCHECKSTACK","", "" ),
TOKEN(tEXE,			instruction,	+0,	"tEXE",		"", "" ),
TOKEN(tEXISTSAT,	procOneArg,		+0,	"tEXISTSAT","", "" ),
TOKEN(tEXIIDF,		procOneArg,		+0,	"tEXIIDF",	"", "" ),
TOKEN(tEXIAT,		procOneArg,		-1,	"tEXIAT",	"", "" ),
TOKEN(tEXIIDX,		procOneArg,		-1,	"tEXIIDX",	"", "" ),
};




#if TOKEN_PASS==1
const uint tokens =	tEXIIDX+1;
#endif


#undef TOKEN_PASS
#undef TOKEN

#endif










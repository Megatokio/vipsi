/*	Copyright  (c)	Günter Woigk   2003-2015
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


#ifndef compiler_h
#define compiler_h

#include	"token.h"
#include	"Var/Var.h"

typedef struct Token Token;

// "at [row 123,] col 456: "
extern cstr 	RowColStr		( uint row, uint col );
extern String 	RowColString 	( uint row, uint col );
extern String 	RowColString 	( ulong rowcol );
extern int	 	GetEncodingFromName ( String conv_name );
inline uint  	Row 			( ulong rowcol ) 		{ return rowcol>>12; }
inline uint  	Col 			( ulong rowcol ) 		{ return rowcol&RMASK(12); }
inline ulong 	RowCol 			( uint row, uint col ) 	{ return (row<<12) + col; }

enum ResultClass
{
	rc_unknown=0,
	rc_temp,
	rc_const,
	rc_var,
};

class Compiler
{
	String		sourcefile;
	String		cachefile;
	String		source;
	String		core;
	String		rowcolxref;
	Var*		constants;
	Var*		bundle;		// bundle = { core, constants, rowcolxref [,sourcefile] [,cachefile] }

	Token*		qa;			// source token array
	Token*		qp;			// 	current pointer
	Token*		qe;			// 	end of array
	Token*		zbu;		// destination token array
	uint		zi;			//	current pointer
	uint		zlen;		//	end of array
	ulong		rowcol;
	uint		lfdnr;

	int			v_now;			// stack depth
	int			v_max;
	void		v_up		( int n )		{ if ((v_now+=n)>v_max) v_max=v_now; }
	void		v_down		( int n )		{ v_now-=n; assert(v_now>=0); }
	void		v_delta		( int n )		{ if(n && errno==ok){if(n>0) v_up(n); else v_down(-n);} }
	void		v_clear		( )				{ v_max = v_now = 0; }
	int			r_now;
	int			r_max;
	void		r_up		( int n )		{ if ((r_now+=n)>r_max) r_max=r_now; }
	void		r_down		( int n )		{ r_now-=n; assert(r_now>=0); }
	void		r_clear		( )				{ r_max = r_now = 0; }


	void		kill		( );
	void		init		( );

	ResultClass	spawn		( );
	void 		procliteral	( Token* qp0 );
	int			nickname 	( int(*conv_proc)(String) );
	void		SetDist		( Token*, Token* );
	void		resize_zbu	( );
	int 		add_constant(cString& s);
	int 		add_constant(double f);
	bool 		is_comma 	( );
	bool 		expect_comma( );
	bool 		expect		( uchar  );
	void 		store_token	( Token const* );
	void 		store_token	( Token const& );
	void		store_token	( ulong, uchar );
	void		store_token	( ulong, uchar, long );
	void		store_token	( ulong, uchar, int );
	void		store_token	( ulong, uchar, short,short );
	void		store_token	( ulong, uchar, NameHandle );
	Token* 		set_offset 	( Token*, bool );
	void		convert		( );
	void		execute		( );
	ResultClass	value		( int );
	ResultClass	value_var	( int );
	ResultClass	value_temp	( int );
	void		newvar		( bool );
	void		idf			( TokenEnum, TokenEnum );
	ResultClass	argument	( );
	cstr		tostr		( Token* );

	OSErr		ReadCacheFile	( );
	void		WriteCacheFile	( );

	void		Tokenize 	( );
	void 		CheckLoops	( );
	void 		Reorder 	( bool exe );
	void		Optimize	( );
	void		Assemble	( );

	void		Compile		( bool exe );

public:
				Compiler	( )						{ init(); }
				~Compiler	( )						{ kill(); }
	uint		ErrorRow	( )						{ return rowcol>>12;	   }	// zero-based
	uint		ErrorCol	( )						{ return rowcol&RMASK(12); }	// zero-based

	void		Compile		( cString& Source,   bool Exe=1 );
	void		CompileFile	( cString& Filename, bool Exe=1 );
	void		CompileExe	( cString& Source )		{ Compile(Source,1); }		// <statements>
	void		CompileEval	( cString& Source )		{ Compile(Source,0); }		// <value>

	Var*		GetBundle	( )						{ return bundle; }
	Var*		GetProc		( )						{ return new Var(Proc(bundle,core.UCS1Text())); }

	cString&	GetSourcefile ( )					{ return sourcefile; }

	void		Reset		( );
};



// die folgenden Konstanten werden in Execute.cp zur Verifikation erneut definiert
#define	RCountDo			3
#define	RCountTry			3
#define	RCountGKauf			2
#define	RCountEvalOrInclude	4
#define	RCountCall			6
#define RCountProcOrInstr	5


#endif





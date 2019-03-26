/*	Copyright  (c)	Günter Woigk 1995 - 2019
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

	c-string library

	• this library provides string manipulation for c-style strings.
	• results are typically stored in thread-safe tempMem buffers
	  and are valid until the current TempMemPool is deallocated.
	• they are intended for immediate use,
	  e.g. for printing, intermediate values in string expressions
	  and for returning result strings.
*/

;
#ifndef cstrings_h
#define cstrings_h

#include "kio/kio.h"
template <class T> class Array;
ptr findStr(cstr,cstr);


EXT	str 	emptystr;				// non-const version of ""


INL	bool	is_space(char c)		{ return uchar(c)<=' ' && c!=0; }
INL	bool	is_uppercase(char c)	{ return uchar(c-'A')<='Z'-'A'; }
INL	char	to_lower(char c)		{ return uchar(c-'A')<='Z'-'A' ? c|0x20 : c; }
INL	bool	is_lowercase(char c)	{ return uchar(c-'a')<='z'-'a'; }
INL	char	to_upper(char c)		{ return uchar(c-'a')<='z'-'a' ? c&~0x20 : c; }
INL	bool	is_letter(char c)		{ return uchar((c|0x20)-'a')<='z'-'a'; }

INL	bool	is_bin_digit(char c)  	{ return uchar(c-'0')<='1'-'0'; }	// { return (c|1)=='1'; }
INL	bool	is_oct_digit(char c)  	{ return uchar(c-'0')<='7'-'0'; }	// { return (c|7)=='7'; }
INL	bool	is_dec_digit(char c)  	{ return uchar(c-'0')<='9'-'0'; }
INL	bool	is_hex_digit(char c) 	{ return uchar(c-'0')<='9'-'0' || uchar((c|0x20)-'a') <= 'f'-'a'; }

INL	bool	no_bin_digit(char c)  	{ return uchar(c-'0')>'1'-'0'; }	// { return (c|1)=='1'; }
INL	bool	no_oct_digit(char c)  	{ return uchar(c-'0')>'7'-'0'; }	// { return (c|7)=='7'; }
INL	bool	no_dec_digit(char c)  	{ return uchar(c-'0')>'9'-'0'; }
INL	bool	no_hex_digit(char c) 	{ return uchar(c-'0')>'9'-'0' && uchar((c|0x20)-'a') > 'f'-'a'; }

INL	int		digit_val(char c)       { return uchar(c-'0'); }										 // char -> digit value: [0..9] ---> [0..9]; non-digits ≥ 10
INL	int		digit_value(char c)     { return c<='9' ? uchar(c-'0') : int(uchar((c&~0x20)-'A'))+10; } // char -> digit value: [0..9,A..Z,a..z] -> [0...35]; non-digits ≥ 36

INL	char	hexchar(int n)			{ n &= 15; return char((n>=10 ? 'A'-10 : '0') + n); }		// digit value -> char


// ----	allocate with new[] ----
EXT	str 	newstr		(int n) noexcept(false);		// allocate memory with new[]
EXT	str		newcopy		(cstr)  noexcept(false);		// allocate memory with new[] and copy text


// ---- querries ----
INL int		strLen		(cstr s)			{ return s ? int(strlen(s)) : 0; }		// c-string
EXT	bool	lt			(cstr,cstr);
EXT	bool	gt			(cstr,cstr);
EXT	bool	gt_tolower	(cstr,cstr);
EXT	bool	eq			(cstr,cstr);
EXT	bool	ne			(cstr,cstr);
EXT	bool	startswith	(cstr,cstr);
EXT	bool	endswith	(cstr,cstr);
INL	bool	contains	(cstr z, cstr s)	{ return findStr(z,s); }
EXT	bool	isupperstr	(cstr);
EXT	bool	islowerstr	(cstr);


// ---- allocate in tempMem pool ----
EXT	str		tempstr		(int n) noexcept(false);		// tempmem.h
EXT	str		xtempstr	(int n) noexcept(false);		// tempmem.h
EXT	str		spacestr	(int n, char c=' ');
EXT	str		whitestr	(cstr, char c=' ');
EXT	str		dupstr		(cstr);
EXT	str		xdupstr     (cstr);

EXT	str		substr		(cptr a, cptr e);
INL str		substr		(cuptr a, cuptr e)			{ return substr(cptr(a),cptr(e)); }		// convenience method
EXT	str 	mulstr 		(cstr, int n);
EXT	str 	catstr 		(cstr, cstr);
EXT	str 	catstr 		(cstr, cstr, cstr, cstr=nullptr, cstr=nullptr, cstr=nullptr);
EXT	str 	midstr 		(cstr, int a, int n);
EXT	str 	midstr 		(cstr, int a);
EXT	str 	leftstr 	(cstr, int n);
EXT	str 	rightstr 	(cstr, int n);
INL char	lastchar	(cstr s)					{ return s&&*s ? s[strlen(s)-1] : 0; }

EXT	str		hexstr 		(uint32 n, int len);
EXT	str		hexstr 		(uint64 n, int len);

template<class T> str hexstr(T n, int len)			{ return sizeof(T)>sizeof(uint32) ? hexstr(uint64(n),len) : hexstr(uint32(n),len); }

EXT	str		binstr		(int n, cstr b0="00000000", cstr b1="11111111");
EXT	str		numstr 		(float32);
EXT	str		numstr 		(double);
EXT	str		numstr 		(float128);
EXT	str		numstr 		(int64);
EXT	str		numstr 		(uint64);
EXT	str		numstr 		(int32);
EXT	str		numstr 		(uint32);
EXT	str		charstr		(char);

INL void	toupper		(str s)						{ if(s) for( ;*s;s++ ) *s = to_upper(*s); }
INL void	tolower		(str s)						{ if(s) for( ;*s;s++ ) *s = to_lower(*s); }
EXT	str		upperstr	(cstr);
EXT	str		lowerstr	(cstr);
EXT	str		replacedstr	(cstr,char oldchar, char newchar);	// 2010-12-28: added 'd' to name
EXT	str		quotedstr	(cstr);
EXT	str		unquotedstr	(cstr);
EXT	str		escapedstr	(cstr);
EXT	str		unescapedstr(cstr);
EXT	str		tohtmlstr	(cstr);
EXT	str		fromhtmlstr	(cstr);
EXT	str		toutf8str	(cstr);
EXT	str		fromutf8str	(cstr);						// UCS1 / Latin-1 only. may set errno.
EXT	str 	hexstr		(cstr);
EXT	str 	unhexstr	(cstr);
//str 		uuencodedstr(cstr);
//str 		uudecodedstr(cstr);
EXT	str		base64str	(cstr);
EXT	str		unbase64str	(cstr);
EXT	str 	croppedstr	(str);						// reuses and evtl. modifies argument string!
EXT	str 	croppedstr	(cstr);						// allocated in tempmem

EXT	str		usingstr	(cstr format, va_list arg);
EXT	str		usingstr	(cstr format, ...);

EXT	ptr		findStr		(cstr target, cstr search);
EXT	ptr		rFindStr	(cstr target, cstr search);

EXT	str		datestr		(time_t secs);
EXT	str		timestr		(time_t secs);
EXT	str		datetimestr	(time_t secs);
EXT	time_t	dateVal		(cstr);
EXT	str		durationstr	(time_t secs);
EXT	str		durationstr (double secs);
EXT	str		speakingNumberStr (double);

EXT	void 	split		(Array<str>& array, ptr a, ptr e);			// split at line breaks
INL void	split		(Array<str>& array, str s)					{ split(array,s,strchr(s,0)); }

EXT	void 	split		(Array<str>& array, ptr a, ptr e, char c);	// split at char
INL void	split		(Array<str>& array, str s, char c)			{ split(array,s,strchr(s,0),c); }


EXT	uint	strcpy		(ptr z, cptr q, uint buffersize);
EXT	uint	strcat		(ptr z, cptr q, uint buffersize);


// _________________________________________________________________________
//

#define sameStr			bitte sameStr() durch eq() ersetzen

#ifdef INCLUDE_DEPRECATED

	#define	CompileDate()	(__DATE__" at "__TIME__)

	enum strconvtype { str_noconv,str_html,str_escaped,str_printable=str_escaped,str_quoted };

	str		ConvertedTo	( cstr, strconvtype );
	str		ConvertedFrom(cstr, strconvtype );

//	str 	NewReadStr 	( FILE* ) throw(bad_alloc);		// moved to unix/FILE.cpp
	char	NextChar	( cptr& p );

#endif

#endif










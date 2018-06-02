/*	Copyright  (c)	Günter Woigk 1999 - 2015
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


#ifndef _errors_h_
#define _errors_h_

#include "kio.h"


//	–––––––––––––––––––––––––––––––––––––––––––––––––––
//	Abort & Panic:
//	defined in log.cpp
//	–––––––––––––––––––––––––––––––––––––––––––––––––––

//	abort application with exit(2)
//	used in macros ABORT, INDEX, IERR, TODO etc.
//	must not be called from a function registered with atexit()!
//
extern void abort( cstr format, va_list )	__attribute__((__noreturn__));
extern void abort( cstr formatstring, ... )	__attribute__((__noreturn__));
extern void abort( int error_number )		__attribute__((__noreturn__));


//	abort application with _exit(3)
//
extern void panic( cstr format, va_list )	__attribute__((__noreturn__));
extern void panic( cstr formatstring, ... )	__attribute__((__noreturn__));
extern void panic( int error_number )		__attribute__((__noreturn__));



//	–––––––––––––––––––––––––––––––––––––––––––––––––––
//	Test Macros:
//	must not be called from a function registered with atexit()
//	–––––––––––––––––––––––––––––––––––––––––––––––––––

#define	ABORT(MSG)	abort("in %s line %u: %s", __FILE__, __LINE__, MSG)

// in development version abort(..) on error
// => put a breakpoint in abort(..) !
#ifndef NDEBUG

#define	IERR()		ABORT("INTERNAL ERROR")
#define	TODO()		ABORT("TODO")
#define	ASSERT(X)	do{ if(!(X)) ABORT("TEST FAILED: " #X); } while(0)
#define	INDEX(I,M)	do{ if((size_t)(ssize_t)(I) >= (size_t)(ssize_t)(M)) \
					ABORT(usingstr("INDEX OORANGE: %i ≥ %i", (int)(I), (int)(M))); } while(0)
#define	LIMIT(I,M)	do{ if((size_t)(ssize_t)(I) >  (size_t)(ssize_t)(M)) \
					ABORT(usingstr("SIZE OORANGE: %u > %u",  (int)(I), (int)(M))); } while(0)


// in final version throw internal_error
// which should be caught and displayed:
#else

#define	IERR()		throw internal_error(__FILE__, __LINE__,internalerror)
#define	TODO()		throw internal_error(__FILE__,__LINE__,notyetimplemented)

#define	ASSERT(X)	do{ if(!(X)) throw internal_error(__FILE__,__LINE__,"TEST FAILED: " #X); } while(0)
#define	INDEX(I,M)	do{ if((size_t)(ssize_t)(I) >= (size_t)(ssize_t)(M)) \
					throw index_error(__FILE__,__LINE__,I,M); } while(0)
#define	LIMIT(I,M)	do{ if((size_t)(ssize_t)(I) >  (size_t)(ssize_t)(M)) \
					throw limit_error(__FILE__,__LINE__,I,M); } while(0)
#endif



//	–––––––––––––––––––––––––––––––––––––––––––––––––––
//	Conditional Versions:
//	–––––––––––––––––––––––––––––––––––––––––––––––––––

#define	XSAFE		(SAFE>=1)
#define	XXSAFE		(SAFE>=2)
#define	XXXSAFE		(SAFE>=3)
#define	XXXXSAFE	(SAFE>=4)

#define	XASSERT(X)		do{ if(SAFE>=1)	ASSERT(X); } while(0)
#define	XXASSERT(X)		do{ if(SAFE>=2)	ASSERT(X); } while(0)
#define	XXXASSERT(X)	do{ if(SAFE>=3)	ASSERT(X); } while(0)
#define	XXXXASSERT(X)	do{ if(SAFE>=4)	ASSERT(X); } while(0)

#define	XINDEX(I,M)		do{ if(SAFE>=1)	INDEX(I,M); } while(0)
#define	XXINDEX(I,M)	do{ if(SAFE>=2)	INDEX(I,M); } while(0)
#define	XXXINDEX(I,M)	do{ if(SAFE>=3)	INDEX(I,M); } while(0)
#define	XXXXINDEX(I,M)	do{ if(SAFE>=4)	INDEX(I,M); } while(0)

#define	XLIMIT(I,M)		do{ if(SAFE>=1)	LIMIT(I,M); } while(0)
#define	XXLIMIT(I,M)	do{ if(SAFE>=2)	LIMIT(I,M); } while(0)
#define	XXXLIMIT(I,M)	do{ if(SAFE>=3)	LIMIT(I,M); } while(0)
#define	XXXXLIMIT(I,M)	do{ if(SAFE>=4)	LIMIT(I,M); } while(0)



//	–––––––––––––––––––––––––––––––––––––––––––––––––––
//	set / get error / message:
//	–––––––––––––––––––––––––––––––––––––––––––––––––––


EXT	cstr	errorstr	(int err);
INL	cstr	errorstr	()							{ return errorstr(errno); }


//	–––––––––––––––––––––––––––––––––––––––––––––––––––
//	deprecated old Error stuff:
//	–––––––––––––––––––––––––––––––––––––––––––––––––––


extern int custom_error;


// methods:
EXT	cstr	errorstr	( int err );

INL	int		GetError	( )			{ return errno; }
INL	bool	NoError		( )			{ return errno==ok; }

EXT	cstr	ErrorStr	( int,bool=0 );
INL	cstr	ErrorStr	( )			{ return ErrorStr(errno,1); }
INL	void	ClearError	( )			{ custom_error=0; errno = ok; }
INL	void	SetError	( int e )	{ if(!errno) errno = e; }

EXT	void	SetError	( int, cstr );
INL	void	SetError	( cstr s )	{ SetError(customerror,s); }
INL	void	ForceError	( int e )	{ errno = e; }
EXT	void	ForceError	( int, cstr );
INL	void	ForceError	( cstr s )	{ ForceError(customerror,s); }




#endif









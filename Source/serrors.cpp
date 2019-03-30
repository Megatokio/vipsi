/*	Copyright  (c)	Günter Woigk 1999 - 2019
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


	error handling

note:
	include errors.cp:  support for cstrings only, does not require class String
	_or_    serrors.cp: support for cstrings and class String strings


	2003-12-06 kio	derived this file from errors.cp
*/

#define	SAFE	2
#define	LOG		1

#include	<string.h>
#include	"kio/kio.h"
#include 	"VString/String.h"
#include	<sys/stat.h>


cstr ETEXT[] = {
#define		EMAC(a,b)	b
#include	"kio/error_emacs.h"
};


int 	custom_error = 0;
String 	custom_errmsg;




/* ----	get default error text for error ------------------------------------------------
*/
cstr ErrorStr ( int/*OSErr*/ err, bool custom )
{
	if (err==0)  return "no error";
	if (err==-1) return "unknown error (-1)";
	if (custom && err==custom_error)	return CString(custom_errmsg);
	if (err>=EBAS && err<EBAS+(int)NELEM(ETEXT)) return ETEXT[err-EBAS];
	return usingstr("%s (%i)",strerror(err),(int)err);
}


/* ----	get error text for current error ------------------------------------------------
*/
String ErrorString ( )
{
	return custom_error ? custom_errmsg : errorstr(errno);
}



/* ----	set error code & text ------------------------------------------------
*/
void ForceError ( int err, cString& msg )
{
	errno			= err;
	custom_errmsg	= msg;
	custom_error	= 1;
}

void ForceError ( int err, cstr msg )
{
	errno			= err;
	custom_errmsg	= msg;
	custom_error	= 1;
}

void SetError ( int err, cstr msg )
{
	if(!errno) ForceError( err, msg );
}



// ------------------------------------


void AppendToError ( cString& msg )
{
	custom_errmsg	= ErrorString() + msg;
	custom_error	= 1;
}

void PrependToError ( cString& msg )
{
	custom_errmsg	= msg + ErrorString();
	custom_error	= 1;
}


#if 0
/*	get error string for system or custom error number:
*/
cstr errorstr( int err )
{
	// custom error texts:
	static const cstr ETEXT[] =
	{
	#define  EMAC(A,B)	B
	#include "kio/error_emacs.h"
	};

	if(err==0)						return "no error";
//	if(err==-1)						return "unknown error (-1)";	strerror: "Unknown error: -1"
	if(uint(err-EBAS)<NELEM(ETEXT)) return ETEXT[err-EBAS];
	else							return strerror(err);
}
#endif













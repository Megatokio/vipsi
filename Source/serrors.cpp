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

#include "kio/kio.h"
#include "serrors.h"
#include "VString/String.h"
#include <string.h>
#include <sys/stat.h>

static cstr ETEXT[] = {
#define  EMAC(a,b)	b
#include "kio/errors.h"
};


/*	custom error message <=> errno = -errno
 *
 * 	ATTN: there is one errno per thread
 *	      but only one custom_errmsg!
 */
static String custom_errmsg;


String ErrorString (int err)
{
	// get error text for error
	// err<0  -> custom_errmsg

	if (err == 0) return "no error";
	if (err < 0) return CString(custom_errmsg);
	if (err >= EBAS && err < EBAS+int(NELEM(ETEXT))) return ETEXT[err-EBAS];
	return usingstr("%s (%i)", strerror(err), err);
}

void ForceError (int err, cString& msg)
{
	// set error code & custom text

	assert(err>0);

	errno			= -err;
	custom_errmsg	= msg;
}

void ForceError (int err, cstr msg)
{
	assert(err>0);

	errno			= -err;
	custom_errmsg	= msg;
}

void AppendToError (cString& msg)
{
	assert(errno != 0);

	custom_errmsg = ErrorString() + msg;
	if (errno > 0) errno = -errno;
}

void PrependToError (cString& msg)
{
	assert(errno != 0);

	custom_errmsg = msg + ErrorString();
	if (errno > 0) errno = -errno;
}
















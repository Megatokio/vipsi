/*	Copyright  (c)	Günter Woigk 1999 - 2015
                    mailto:kio@little-bat.de

    This file is free software

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    Permission to use, copy, modify, distribute, and sell this software and
    its documentation for any purpose is hereby granted without fee, provided
    that the above copyright notice appear in all copies and that both that
    copyright notice and this permission notice appear in supporting
    documentation, and that the name of the copyright holder not be used
    in advertising or publicity pertaining to distribution of the software
    without specific, written prior permission. The copyright holder makes no
    representations about the suitability of this software for any purpose.
    It is provided "as is" without express or implied warranty.

    THE COPYRIGHT HOLDER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
    INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
    EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR
    CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
    DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
    TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
    PERFORMANCE OF THIS SOFTWARE.

    error handling

note:
    include errors.cpp:  support for cstrings only, does not require class String
    _or_    serrors.cpp: support for cstrings and class String strings


    1999-01-01	first work on this file
    2001-02-17	added possibility to #include custom error file
    2001-11-09	added support for class String
    2011-01-17	modified to allow simple clearing of errno in most cases
*/


#define	SAFE	1
#define	LOG		1

#include "config.h"
#include "kio/kio.h"
#include "unix/s_type.h"



#if 0
char custom_errmsg[256];
int	 custom_error = 0;


cstr ETEXT[] =
{
#define		EMAC(a,b)	b
#include	"error_emacs.h"
};


/* ===================================================
        Get Error Text:
        does not allocate memory
   =================================================== */

cstr ErrorStr(int err, bool custom)
{
    if(err==0)						return "no error";
    if(err==-1)						return "unknown error (-1)";
    if(custom && err==custom_error)	return custom_errmsg;
    if(uint(err-EBAS)<NELEM(ETEXT)) return ETEXT[err-EBAS];
    else							return strerror(err);
}


/* ===================================================
        Set Error:
   =================================================== */

// unconditional:
//
void ForceError( int err, cstr msg )
{
    strncpy(custom_errmsg,msg,255);
    errno = custom_error = err;
}

// conditional:
//
void SetError( int err, cstr msg )
{
    if(!errno) ForceError(err,msg);
}
#endif



/* ===================================================
        Exception Messages:
   =================================================== */

#include "../unix/FD.h"
file_error::file_error(class FD& fd, int error) throw() : any_error(error),filepath(fd.filepath()),fd(fd.file_id()) {}
//file_error::file_error(class FD& fd, cstr msg) throw()	: any_error(errno?errno:customerror,msg),filepath(fd.filepath()),fd(fd.file_id()){}
file_error::file_error(class FD& fd, int error, cstr msg) throw() : any_error(error,msg),filepath(fd.filepath()),fd(fd.file_id()){}

// helper
static
cstr filename(cstr file) throw()
{
    cstr p = strrchr(file,'/');
    return p ? p+1 : file;
}

//virtual
cstr internal_error::what() const throw()
{
    return usingstr( "%s line %u: %s",
        filename(file), line, text ? text : errorstr(error) );
}

//virtual
cstr limit_error::what() const throw()
{
    return usingstr( "%s line %u: size %u exceeds maximum of %u",
        filename(file), line, sz, max );
}

//virtual
cstr index_error::what() const throw()
{
    return usingstr( "%s line %u: index %u exceeds array size of %u",
        filename(file), line, idx, max );
}

//virtual
//	error 	-> errorstr
//	text	-> text
// 	beides	-> errorstr (text)
//
// 			wenn error und text angegeben sind, wird angenommen,
//			dass der text zusätzlicher ein hinweis ist
//
cstr any_error::what() const throw()
{
    return text
        ? error==customerror || error==-1
            ? text
            : usingstr("%s (%s)",errorstr(error),text)
        : errorstr(error);
}

//virtual
// text==0	-> errorstr: file="filepath"
// text!=0	-> errorstr: file="filepath" (text)
//
// 			wenn auch ein text angegeben ist, wird angenommen,
//			dass dieser zusätzlicher ein hinweis ist
//
cstr file_error::what() const throw()
{
    return usingstr( text ? "%s: file = \"%s\" (%s)"
                          : "%s: file = \"%s\"",
        errorstr(error), filepath, text );
}

#if 0
cstr file_error::what() const throw()
{
	static cstr typelist[16] = {0/*UNKN*/,"Pipe","Ser",0,"Dir",0,"Blk",0,"File",0,"Link",0,"Socket",0,"Erased file",0};

	struct stat data;
	cstr typestr = NULL;
	if(_fd!=-1 && fstat(_fd, &data)==0) typestr = typelist[data.st_mode>>12];

	cstr msg = any_error::what();

	if(typestr)
		 return file ? usingstr("%s %s: %s",typestr,file,msg) : usingstr("%s %i: %s",typestr,_fd,msg);
	else return file ? usingstr("File %s: %s",file,msg) : _fd!=-1 ? usingstr("File %i: %s",_fd,msg) : msg;
}
#endif










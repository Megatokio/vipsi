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

    1995-06-29 kio	started work on pascal string lib
    2000-01-28 kio	started work on c string lib
    2002-01-19 kio	started unix port
    2002-01-26 kio	simplified usingstr(): now using vsnprintf()
    2002-01-26 kio	added xpool[] for unlimited string length support
    2004-12-16 kio	split into functions and qstring part for multi threading safe version for Cocoa
    2006-10-31 kio	renamed QString() to QuickStr() wg. naming collission with Qt.
    2008-05-14 kio	renamed QuickStr() to tempStr()
    2008-05-15 kio	unified thread-safe temporary memory allocation using the new tempMem library
    2010-10-17 kio	skip whitespace after \-escaped linebreak in ConvertedFrom(escaped)
*/


#define	SAFE	3
#define	LOG		1

#include    "config.h"
#include	<math.h>
#include	<time.h>
#include	"../unix/tempmem.h"
#include	"cstrings.h"
#include	"../Templates/Array.h"


// hint for decimal POINT:
//
// #include <clocale>
// std::setlocale(LC_ALL, "en_US");




#ifdef HAVE_NAN_H
#include	<nan.h>
#endif
#ifndef NAN
const double NAN = 0.0/0.0;
#endif
static char null = 0;
str emptystr = &null;



/* ----	allocate char[] -----------------------------------------
        deallocate with delete[]
        presets terminating 0
        2007-07-10: added the 'bad_alloc'
*/
str newstr ( int n ) throw(bad_alloc)
{
    str c = new char[n+1];
    c[n] = 0;
    return c;
}


/* ----	allocate char[] -----------------------------------------
        deallocate with delete[]
        includes OOMEM check
        returns nullptr if source string is nullptr			2001-01-29
*/
str newcopy ( cstr s ) throw(bad_alloc)
{
    str c = nullptr;
    if (s)
    {
        c = newstr(strLen(s));
        strcpy ( c, s );
    }
    return c;
}


/* ----------------------------------------------------------
            METHODS
---------------------------------------------------------- */


/* ----	Create duplicate of string ---------------------------------
        Put a string in the pool
*/
str dupstr ( cstr s )
{
    if (!s||!*s) return emptystr;							// 2000-12-28
    int n = strLen(s);
    str c = tempstr(n);
    memcpy ( c,s,n );
    return c;
}


/* ----	Create duplicate of string --------------------------------- 2009-06-19
        Put a string in the outer pool
*/
str xdupstr ( cstr s )
{
    if (!s||!*s) return emptystr;
    int n = strLen(s);
    str c = xtempstr(n);
    memcpy ( c,s,n );
    return c;
}


/* ----	search sub string ------------------------------------ kio 2001-08-11 ----
                                                        fixed  kio 2013-03-21
*/
char* findStr ( cstr target, cstr search )
{
    if (!search||!*search) return (char*)target;

    int t_len = strLen(target);
    int s_len = strLen(search);

    for ( int i=0; i<=t_len-s_len; i++ )
    {
        const char* s = search;
        const char* t = target+i;
        while(*s && *s==*t){s++;t++;}
        if (*s==0) return (char*)target+i;
    }
    return nullptr;	// not found
}


/* ----	search sub string ------------------------------------ kio 2001-08-22 ----
*/
char* rFindStr ( cstr target, cstr search )
{
    int t_len = strLen(target);
    int s_len = strLen(search);

    if (!search||!*search) return (char*)target+t_len;
    const char* se = search+s_len;

    for ( int i=t_len-s_len; i>=0; i-- )
    {
        const char* s = search;
        const char* t = target+i;
        do { if (s==se) return (char*)target+i; } while (*s++==*t++);
    }
    return nullptr;	// not found
}


/* ----	Create and clear a string ---------------------------
*/
str spacestr ( int n, char c )
{
    if (n<=0) return emptystr;			// kio 26.nov.2000
    str s = tempstr(n);
    memset ( s, c, n );
    return s;
}


/* ----	create blanked-out copy of string ------------------------------------------
        new: 2002-01-27 kio
*/
str whitestr ( cstr q, char c )
{
    str s = dupstr(q);
    for( uchar*p=(uchar*)s; *p; p++ ) if(*p>' ') *p=c;
    return s;
}


/* ----	create string from char ----------------------------------------------------
*/
str charstr ( char c )
{
    str s = tempstr(1);
    *s = c;
    return s;
}


// ----	Replace any occurance of a character by another ----------------------------
str replacedstr ( cstr s, char old, char nju )
{
    str t = dupstr(s);
    for ( char* p=t; *p; p++ ) if (*p==old) *p=nju;
    return t;
}


// ----	Convert a string to all lower case -----------------------------------------
str lowerstr ( cstr s )
{
    str t = dupstr(s);
    tolower(t);
    return t;
}


// ----	Convert a string to all upper case -----------------------------------------
str upperstr ( cstr s )
{
    str t = dupstr(s);
    toupper(t);
    return t;
}


// ----	Test if string is all upper case --------------------------- 2013-03-23 ----
bool isupperstr	( cstr s )
{
//	assert( isupper(0xC4));	// Ä		tuts nicht: isupper() is ASCII only
//	assert(!isupper(0xE4));	// ä
    if(s) while(*s) { if(toupper(*s)!=*s) return no; else s++; }
    return yes;
}

bool islowerstr	( cstr s )
{
//	assert(!islower(0xC4));	// Ä		tuts nicht
//	assert( islower(0xE4));	// ä
    if(s) while(*s) { if(tolower(*s)!=*s) return no; else s++; }
    return yes;
}





/* ----	Repeate string n times ------------------------------------ 22.aug.2001 ----
*/
char* mulstr ( cstr q, int n )
{
    if (q==nullptr || *q==0 || n<=0) return emptystr;

    int len = strLen(q);
    str s   = tempstr(len*n);
    char* z = s;

//	while (n--) z = (char*)memcpy(z,q,len);			should work acc. to man page but doesn't
    while (n--) { memcpy(z,q,len); z+=len; }

    return s;
}


/* ----	Concatenate 2 strings ------------------------------------------------------
*/
char* catstr ( cstr s1, cstr s2 )
{
    str s = tempstr(strLen(s1)+strLen(s2));
    s[0]=0;
    if (s1) strcpy ( s, s1 );
    if (s2) strcat ( s, s2 );
    return s;
}


/* ----	Concatenate up to 6 strings -------------------------------------------------
*/
char* catstr ( cstr s1, cstr s2, cstr s3, cstr s4, cstr s5, cstr s6 )
{
    str s = tempstr( strLen(s1)+strLen(s2)+strLen(s3)+strLen(s4)+strLen(s5)+strLen(s6) );
    s[0]=0;
    if (s1) strcpy ( s, s1 );
    if (s2) strcat ( s, s2 );
    if (s3) strcat ( s, s3 );
    if (s4) strcat ( s, s4 );
    if (s5) strcat ( s, s5 );
    if (s6) strcat ( s, s6 );
    return s;
}


/* ----	Convert number to decimal string ----------
        the string is sized to fit (max. 12 char if sizeof(int32)=4)
*/
char* numstr ( int32 n )
{
    char* s = tempstr(1+sizeof(int32)*5/2);		// 2010-10-17
    sprintf( s, "%i", n );
    return s;
}

char* numstr ( uint32 n )						// 2010-10-17
{
    char* s = tempstr(sizeof(uint32)*5/2);
    sprintf( s, "%u", n );
    return s;
}

char* numstr ( int64 n )						// 2010-12-04
{
    char* s = tempstr(1+sizeof(int64)*5/2);
#if _sizeof_long * _bits_per_byte == 64         // Weiche added: kio 2014-10-03
    sprintf( s, "%li", (long)n );               // cast required: int64 ist unter 64bit-OSX long long, und unter 64bit-Linux long
#else
    sprintf( s, "%lli", (long long)n );
#endif
    return s;
}

char* numstr ( uint64 n )						// 2010-12-04
{
    char* s = tempstr(sizeof(uint64)*5/2);
#if _sizeof_long * _bits_per_byte == 64         // Weiche added: kio 2014-10-03
    sprintf( s, "%lu", (unsigned long)n );      // cast required: int64 ist unter 64bit-OSX long long, und unter 64bit-Linux long
#else
    sprintf( s, "%llu", (unsigned long long)n );
#endif
    return s;
}


/* ----	Convert floating point number to decimal string ---------- kio 15.aug.2001 ----
        the string is sized to fit
*/
char* numstr ( double d )
{
    char* s = tempstr(19);
    sprintf ( s, "%.14g", d );
    return s;
}
char* numstr ( float32 d )
{
    char* s = tempstr(15);
    sprintf ( s, "%.10g", (double)d );
    return s;
}
char* numstr ( float128 d )
{
    char* s = tempstr(27);
    sprintf ( s, "%.22Lg", d );
    return s;
}


/* ----	Convert number to hexadecimal string ----------
*/
char* hexstr ( uint32 n, int digits )
{
    static char hex[] = "0123456789ABCDEF";
    char* c = tempstr(digits);

    while(digits)
    {
        c[--digits] = hex[n&0x0f];
        n >>= 4;
    }
    return c;
}

char* hexstr ( uint64 n, int digits )
{
    static char hex[] = "0123456789ABCDEF";
    char* c = tempstr(digits);

    while(digits)
    {
        c[--digits] = hex[n&0x0f];
        n >>= 4;
    }
    return c;
}


/* ----	Convert number to binary string ---------- 2009-06-07
*/
str binstr( int value, cstr b0, cstr b1 )
{
    int n = strLen(b0); XXXASSERT( n==strLen(b1) );
    str s = tempstr(n); memcpy(s,b0,n);
    while(n--) { if(value&1) s[n]=b1[n]; value=value>>1; }
    return s;
}


/* ----	create string [a ... [e ----------------------  1.apr.00 kio
        does not check for 0-characters between a and e
*/
char* substr ( cstr a, cstr e )
{
    if (e<=a) return emptystr;
    char* c = tempstr((int)(e-a));
    memcpy ( c, a, e-a );
    return c;
}


/* ----	create left substring ------------------------
        splits n bytes from the left
        if source string is shorter, the result is shorter too
*/
char* leftstr ( cstr s, int n )
{
//	int32 m = strLen(s); if (m<n) n=m;		<-- dangerous!
    const char*a=s; 		// kio 2.aug.2001
    const char*e=s+n;		// ""
    while (a<e&&*a) a++;	// ""
    n = (int)(a-s);			// ""

    if (n<=0) return emptystr;
    char* c = tempstr(n);
    memcpy ( c, s, n );
    return c;
}


/* ----	create right substring ------------------------
        splits n bytes from the right
        if source string is shorter, the result is shorter too
*/
char* rightstr ( cstr s, int n )
{
    int32 m = strLen(s); if (m<n) n=m;
    if (n<=0) return emptystr;
    char* c = tempstr(n);
    memcpy ( c, s+m-n, n );
    return c;
}


/* ----	create mid substring ------------------------
        skip a bytes from the left then split n bytes
        if source string is shorter, the result is shorter too
*/
char* midstr ( cstr s, int a, int n )
{
    if (a<0) { n+=a; a=0; }			// korr. 6.aug.2001 kio
    n = min(n,strLen(s)-a);
    if (n<=0) return emptystr;
//	return leftstr(s+a,n);
    return substr(s+a,s+a+n);		// kio 2002-03-29
}


/* ----	create mid-to-end substring -------------------
        kio 2002-03-29
*/
char* midstr ( cstr s, int a )
{
    if(!s) return emptystr;
    if (a<0) a=0;
    return substr(s+a,strchr(s,0));
}


/* ----	create formatted string ----------------------------------
        2002-01-26 kio	now using vsnprintf()
        2003-11-28 kio	now calling usingstr() which calls vsnprintf()
*/
str usingstr ( cstr format, ... )
{
    va_list va;
    va_start(va,format);
    str s = usingstr(format,va);
    va_end(va);
    return s;
}


/* ----	usingstr() -----------------------
        2003-06-26 kio: hopefully fixed "va_list z = va;" problem
        2003-11-28 kio: hopefully fixed "va_copy" problem
        note: caller must call va_start() prior and va_end() afterwards.
*/
str usingstr ( cstr format, va_list va )
{
/*	from the GNU autoconf manual:
    va_copy: The ISO C99 standard provides va_copy for copying va_list variables.
    It may be available in older environments too, though possibly as __va_copy
    (e.g., gcc in strict C89 mode). These can be tested with #ifdef.
    A fallback to memcpy (&dst, &src, sizeof(va_list)) will give maximum portability.

    va_list is not necessarily just a pointer.
    It can be a struct (e.g., gcc on Alpha), which means nullptr is not portable.
    Or it can be an array (e.g., gcc in some PowerPC configurations),
    which means as a function parameter it can be effectively call-by-reference
    and library routines might modify the value back in the caller
    (e.g., vsnprintf in the GNU C Library 2.1).
*/
    int err = errno;							// save errno

    va_list va2;								// duplicate va_list
    va_copy(va2,va);
//	memcpy( &va2, &va, sizeof(va_list) );

    char bu[1];
    int n = vsnprintf( bu, 0, format, va2 );	// calc. req. size

    str z = tempstr(n);
    vsnprintf( z, n+1, format, va );			// create formatted string

    errno = err;								// restore errno
    return z;
}


/* ----	create beautified text for number ---------------------------
        from project vipsi
*/
str speakingNumberStr ( double n )
{
    if (n>=1000000000) return usingstr( "%.3lf GB", n/1000000000 );
    if (n>=1000000   ) return usingstr( "%.3lf MB", n/1000000 );
    if (n>=1000      ) return usingstr( "%.3lf kB", n/1000 );
                       return usingstr( "%.0lf bytes", n );
}


/* ----	convert string ------------------------------
*/
static const char cc[] = "\\\"\e\a\b\f\n\r\t\v";	// control codes
static const char ec[] = "\\\"eabfnrtv";			// escape characters

static const char* StrComp(cstr a, cstr b)
{	char c,d;
    while((c=*a)&&(d=*b)&&c==d) {a++;b++; }
    return a;
}

str	tohtmlstr( cstr s0 )
{
    str  s;
    char c;
    int  i;
    char *z;

    if (!s0||!*s0) return emptystr;
    s = (str)s0;

    for ( i=0; (c=s[i]); i++ )
    {
        switch(c)
        {
        default: 	continue;
        case '<':	z = (str)"&lt;";   break;
        case '>':	z = (str)"&gt;";   break;
        case '&':	z = (str)"&amp;";  break;
        case '"':	z = (str)"&quot;"; break;		// 2002-07-30 kio: added
        case '\n':	z = (str)"<br>";   break;		// 2004-07-08 kio: added
        }
        s = catstr( leftstr(s,i), z, s+i+1 ); i+=3;
    }

    return s!=s0 ? s : dupstr(s0);
}

str	escapedstr( cstr s0 )
{
    str  s;
    char const *q;
    char *z;
    uchar uc;

    if (!s0||!*s0) return emptystr;
    z = s = (str)s0;

    while ( (uc=*z) )
    {
        if( uc>=' ' && uc!=0x7F && uc!='\\' && uc!='\"' ) { z++; continue; }
//		if( uc>=' ' && uc<0x7F && uc!='\\' && uc!='\"' ) { z++; continue; }
//		if( uc>=0x80&&UTF8CharValid(z)) { z=UTF8StrNextChar(z); continue; }

        if( (q=strchr(cc,uc)) )
        {
            uc = ec[q-cc];													// 2011-01-02 kio korr.
            q=s; s=catstr( substr(s,z+1), z ); z+=s-q; *z++='\\'; *z++=uc;	// 2011-01-02 kio
            continue;
        }
        char oc[]="\\000";
        oc[1]='0'+(uc>>6); uc&=(1<<6)-1;
        oc[2]='0'+(uc>>3); uc&=(1<<3)-1;
        oc[3]='0'+(uc>>0);
        q=s; s=catstr(substr(s,z),oc,z+1); z+=4+(s-q); continue;
    }

    return s!=s0 ? s : dupstr(s0);
}

str quotedstr( cstr s )
{
    return catstr( "\"", escapedstr(s), "\"" );
}

str	unescapedstr( cstr s0 )		// 2012-06-13 reworked for full support of '\'
{
    char c, *q, *z;

    if(!s0||!*s0) return emptystr;
    str s = dupstr(s0);

    q = z = strchr(s,'\\');

    if(q) for(;;)
    {
        while((c=*q++)!='\\')
        {
            *z++ = c;
            if(!c) return s;
        }

        c = *q++;					// c = next char after '\'

    // \ooo => octal value for next byte. 9th bit discarded
    // \oo     allowed but not recommended.
    // \o	   allowed but not recommended.
        if (is_oct_digit(c))
        {
            uint8 d;
            c = digit_val(c);
            d = digit_val(*q); if (d<8) { q++; c=(c<<3)+d; }
            d = digit_val(*q); if (d<8) { q++; c=(c<<3)+d; }
            // c &= 0xFF;
        }

    // \xHH => hex coded value for next byte
    // \xH     allowed but not recommended.
    // \x      'x' masked for unknown reason => stores 'x'
        else if (c=='x')
        {
            uint8 d;
            d = digit_value(*q); if (d<16) { q++; c=d;        }
            d = digit_value(*q); if (d<16) { q++; c=(c<<4)+d; }
        }

    // \cX => control code value for next byte
    // \c     (at eof) => stores 'c'
        else if (c=='c')
        {
            if (*q) c = *q++ & 0x1f;
        }

    // '\' at end of string => store '\':
        else if(!c)
        {
            c = '\\';
            q--;
        }

    // '\n' => store '\n' and skip following white space
        else if(c=='\n'||c=='\r')
        {
            while( *q && *q<=' ' ) q++;
            continue;	// don't store the '\n'
        }

    // "well known abreviations" or '"' or '\' or s.th. escaped for unknown reason:
        else
        {
            static const char ec[] = "eabfnrtv";			// escape character
            static const char cc[] = "\e\a\b\f\n\r\t\v";	// control code
            char const* p = strchr(ec,c);
            if(p) c = cc[p-ec]; 			// control code found
                                            // else self-escaped char
        }

        *z++ = c;
    }
    return s;
}

str	unquotedstr( cstr s0 )
{
    char c;
    str  s;
    uint n;

    if(!s0||!*s0) return emptystr;

    s = dupstr(s0);

    n=(uint)strlen(s);
    c=s[0];
    if( n>=2 && (c=='"'||c=='\'') && s[n-1]==c )
    {
		// wir ignorieren, dass das schließende Zeichen fehlerhafterweise escaped sein könnte.
		// unescapedstr() wird dann das letzte '\' erhalten.
		s[n-1] = 0;
		s++;
    }

    return unescapedstr(s);
}

str	fromhtmlstr( cstr s0 )
{
    char c;
    char *q,*z;
    str  s;

    if(!s0||!*s0) return emptystr;

    s = dupstr(s0);

    q = z = strchr(s,'&');

    if(q) for(;;)
    {
        while((c=*q++)!='&') { *z++=c; if(!c) return s; }
        if (*StrComp("gt;",  q)==0) { *z++ = '>'; q+=3; continue; }
        if (*StrComp("lt;",  q)==0) { *z++ = '<'; q+=3; continue; }
        if (*StrComp("amp;", q)==0) { *z++ = '&'; q+=4; continue; }
        if (*StrComp("quot;",q)==0) { *z++ = '"'; q+=5; continue; }	// 2002-07-30 kio: added;  2008-05-12 kio: korr
        *z++='&';	// unquoted '&'
    }
    return s;
}


inline bool utf8_is_7bit( signed char c )	{ return c>=0; }
inline bool	utf8_no_7bit( signed char c )	{ return c<0;  }
inline bool utf8_is_fup	( signed char c )	{ return c<(signed char)0xc0; }
inline bool utf8_no_fup	( signed char c )	{ return c>=(signed char)0xc0; }


str fromutf8str( cstr qstr )		// 2011-03-16
{
    if(qstr==nullptr||*qstr==0) return nullptr;

    if(*qstr==(char)0xef && *(qstr+1)==(char)0xbb && *(qstr+2)==(char)0xbf) qstr += 3;	// skip BOM

    int	zlen = 0;					// golden rule: every non_fup makes a character!
    for(cptr q=qstr;*q;) zlen += utf8_no_fup(*q++);

    str	zstr = tempstr(zlen);
    cptr   q = qstr;
    ptr    z = zstr;
    uint8 c1,c2;

    while(*q)
    {
        if( utf8_is_7bit(c1=*q++) )					{ *z++ = c1; continue; }
        if( utf8_is_fup(c1) )						{ errno=unexpectedfup; continue; }
        if( *q==0||utf8_no_fup(c2=*q)||c1>=0xc4 )	{ *z++ = '?'/*c1*/; errno=EOVERFLOW; continue; }
        q++; *z++ = (c1<<6) + (c2&0x3f);
    }
    return zstr;
}



str toutf8str( cstr qstr )		// 2011-03-16
{
    if(qstr==nullptr||*qstr==0) return nullptr;

    int	zlen = 0;
    cptr q=qstr; while(*q) zlen += utf8_no_7bit(*q++); zlen += q-qstr;
    str	zstr = tempstr(zlen);
    ptr z = zstr;
    q = qstr;

    while(*q)
    {
        uint c = *q++;
        if(utf8_is_7bit(c)) { *z++ = c; continue; }	// 7-bit ascii
        *z++ = 0xC0+(c>>6);		// 2-char utf8 code
        *z++ = 0x80+(c&0x3f);
    }
    return zstr;
}


/* ====	date&time =============================================
        note: time_t is always in UTC  (seconds since 1970-1-1 0:00 GMT)
*/
static uint next_number ( cuptr& c, cuptr e )
{
    uint  n = 0;
    while ( c<e && no_dec_digit(*c) ) { c++; }
    while ( c<e && is_dec_digit(*c) ) n = n*10 + *c++ -'0';
    return n;
}

time_t dateVal ( cstr date )
{	tm d;
    const uchar* c = (cuptr)date;
    const uchar* e = (cuptr)strchr(date,0);

    uint n = next_number(c,e);
    if (n<70) n+=2000; else if (n<100) n+=1900;
    d.tm_year = n-1900;

    d.tm_mon  = next_number(c,e) - 1;
    d.tm_mday = next_number(c,e);
    d.tm_hour = next_number(c,e);
    d.tm_min  = next_number(c,e);
    d.tm_sec  = next_number(c,e);

//	d.tm_wday   = 0;	output field
//	d.tm_yday   = 0;	""
    d.tm_isdst  = 0;
#if !defined(_SOLARIS)
    d.tm_gmtoff = 0;
    d.tm_zone   = nullptr;
#endif

    return mktime(&d);	// local time
//	return timegm(&d);	// UTC
}


str datetimestr ( time_t secs )
{
    tm* dt = localtime(&secs);
    if(dt)
        return usingstr( "%04u-%02u-%02u %02u:%02u:%02u",
                dt->tm_year+1900, dt->tm_mon+1, dt->tm_mday, dt->tm_hour, dt->tm_min, dt->tm_sec );
    else
        return catstr("localtime: ",errorstr());


//	return dt ? usingstr( "%04u-%02u-%02u %02u:%02u:%02u",
//			    dt->tm_year+1900, dt->tm_mon+1, dt->tm_mday, dt->tm_hour, dt->tm_min, dt->tm_sec )
//			  : catstr("localtime: ",errorstr());
}

str datestr ( time_t secs )
{
    tm dt; localtime_r(&secs,&dt);
    return usingstr( "%04u-%02u-%02u", dt.tm_year+1900, dt.tm_mon+1, dt.tm_mday );
}

/*
str datestr ( time_t secs, cstr format )
{
    if(!format) format = "D.M.YYYY";

    tm* dt = localtime(&secs);
    const int N = 32;
    char s[N];

    cptr q = format;
    ptr z = s;
    char c;

    while((c=*q++) && z<s+(N-4))
    {
        if(c=='D')
        {
            if(*q==c || dt->tm_mday>9) { *z++ = '0' + dt->tm_mday/10; while(*q==c) q++; }
            *z++ = dt->tm_mday%10;
            continue;
        }
        else if(c=='M')
        {
            if(*q==c || (dt->tm_mon+1) > 9) { *z++ = '0' + (dt->tm_mon+1)/10; while(*q==c) q++; }
            *z++ = (dt->tm_mon+1)%10;
            continue;
        }
        else if(c=='Y')
        {
            uint y = dt->tm_year+1900;
            if(*q==c) q++;
            if(*q==c) { *z++ = '0' + y/1000; } y %= 1000;
            if(*q==c) { *z++ = '0' + y/100; }  y %= 100;
            *z++ = '0' + y/10; y %= 10;
            *z++ = '0' + y;
            while(*q==c) q++;
            continue;
        }
        else
        {
            *z++ = c;
        }
    }

    *z = 0;
    return dupstr(s);
}
*/

str timestr ( time_t secs )
{
    tm dt;
    localtime_r(&secs,&dt);
    return usingstr( "%02u:%02u:%02u", dt.tm_hour, dt.tm_min, dt.tm_sec );
}

str durationstr ( time_t secs )
{	uint s,m,h,d;

    s = (uint)secs;		if (s<=300)	return usingstr("%u sec.",          s);
    m = s/60; s = s%60; if (m<60)	return usingstr("%um:%us",        m,s);
    h = m/60; m = m%60;	if (h<24)	return usingstr("%uh:%um:%us",  h,m,s);
    d = h/24; h = h%24;				return usingstr("%ud:%uh:%um",d,h,m  );
}


str durationstr ( double secs )		// 2010-10-16
{
    return secs>=60 ? durationstr( (time_t)secs ) : usingstr( "%.3f sec.",secs );
}





#ifdef INCLUDE_DEPRECATED

char NextChar ( char const *& p )
{	char c;
    c = *p++; if (c!='\\') return c;	// plain char
    c = *p++;
    if( (c-'0')&~7 )					// no octal digit
    {
        cptr q = strchr(ec,c);
        if (c==0) p--;					// dont advance text pointer if "\" at end of text
        if (q) return cc[q-ec]; 		// control code
        return c;						// unkn. escape char / char escaped for unknown reason
    }
    else								// octal digit
    {
        uchar n = c-'0';
        if (((c=*p++-'0')&~7)==0) { n=n*8+c; if (((c=*p++-'0')&~7)==0) return n*8+c; }
        p--; return n;
    }
}


str ConvertedTo ( cstr s, strconvtype ctype )
{
    switch(ctype)
    {
    case str_noconv:	return newCopy(s);
    case str_html:		return ToHtml(s);
    case str_escaped:	return Escaped(s);
    case str_quoted:	return quotedstr(s);
    default:			IERR();
    }
}

str ConvertedFrom ( cstr s, strconvtype ctype )
{
    switch(ctype)
    {
    case str_noconv:	return newCopy(s);
    case str_html:		return FromHtml(s);
    case str_escaped:	return Unescaped(s);
    case str_quoted:	return Unquoted(s);
    default:			IERR();
    }
}

#endif


bool lt( cptr a, cptr b )
{
    while(*a==*b && *a) { a++; b++; }
    return *a < *b;
}


bool gt( cptr a, cptr b )
{
    while(*a==*b && *a) { a++; b++; }
    return *a > *b;
}

bool gt_tolower( cptr a, cptr b )
{
    while(*a && to_lower(*a)==to_lower(*b)) { a++; b++; }
    return to_lower(*a) > to_lower(*b);
}


/* ---- compare strings -------------------------------------
        nullptr pointers are assumed ""
*/
bool eq( cptr s, cptr t )
{
    if (s&&t)										// kio 2000-12-28
    {
        while (*s) if (*s++!=*t++) return false;
        return *t==0;								// kio 2000-04-01
    }
    else
    {
        return ((!s)||(!*s)) && ((!t)||(!*t));		// kio 2002-01-20
    }
}

bool ne( cptr s, cptr t )
{
    if (s&&t)										// kio 2000-12-28
    {
        while (*s) if (*s++!=*t++) return true;
        return *t!=0;								// kio 2000-04-01
    }
    else
    {
        return (s && *s) || (t && *t);				// kio 2002-01-20 korr 2014-03-09
    }
}



str hexstr(cstr s)
{
    if(!s) return nullptr;

    uint n = 2 * (uint)strlen(s);
    str z = tempstr(n);
    char c;
    while((c=*s++)) { *z++ = hexchar(c>>4); *z++ = hexchar(c); }
    return z-n;
}

/*	returns nullptr on any error
*/
str unhexstr(cstr s)
{
    if(!s) return nullptr;

    uint n = (uint)strlen(s);
    if(n&1) return nullptr;
    n = n/2;
    str z = tempstr(n);
    while(*s)
    {
        uint c1 = digit_value(*(uptr)s++);
        uint c2 = digit_value(*(uptr)s++);
        if((c1|c2)>>4) return nullptr;
        *z++ = (c1<<4)+c2;
    }
    return z-n;
}


static uint8 base64[65]    = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
//static uint8 base64url[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
#define x 0xff
static uint8 unbase64[128] =
{ 	x,x,x,x,x,x,x,x,        x,x,x,x,x,x,x,x,         x,x,x,x,x,x,x,x,         x,x,x,x,x,x,x,x,
    x,x,x,x,x,x,x,x,        x,x,x,62,x,62,x,63,      52,53,54,55,56,57,58,59, 60,61,x,x,x,x,x,x,
    x,0,1,2,3,4,5,6,        7,8,9,10,11,12,13,14,    15,16,17,18,19,20,21,22, 23,24,25,x,x,x,x,63,
    x,26,27,28,29,30,31,32, 33,34,35,36,37,38,39,40, 41,42,43,44,45,46,47,48, 49,50,51,x,x,x,x,x };
#undef x




/*	base64 encode a string
    base64 encoding uses only 3 special characters: '+'  and  '/'  and  '='
    note: the original encoding can add line breaks
*/
str base64str(cstr s)
{
    if(!s) return nullptr;

    uint slen = (uint)strlen(s);
    uint zlen = (slen+2)/3*4;

    str  z = tempstr(zlen);
    cptr e = s + slen-slen%3;

    while(s<e)
    {
        uint32 c = *(uptr)s++;
               c = (c<<8) + *(uptr)s++;
               c = (c<<8) + *(uptr)s++;

        *z++ = base64[ c>>18    ];
        *z++ = base64[(c>>12)&63];
        *z++ = base64[(c>>6 )&63];
        *z++ = base64[ c     &63];
    }
    if(*s)
    {
        uint32 c = *(uptr)s++;
        bool   f = *s;
        c=c<<8; if(f) c += *(uptr)s++;
        c=c<<8;

        *z++ =     base64[ c>>18    ];
        *z++ =     base64[(c>>12)&63];
        *z++ = f ? base64[(c>>6 )&63] : '=';		// indicate padding
        *z++ =                          '=';		// indicate padding
    }

    return z-zlen;
}



/*	returns nullptr on any error
*/
str unbase64str(cstr s)
{
    if(!s) return nullptr;

    uint slen = (uint)strlen(s);
    if(slen%4) return nullptr;

    uint padd = s[slen-1]!='=' ? 0 : s[slen-2]!='=' ? 1 : 2;
    uint zlen = slen/4*3-padd;

    str  z = tempstr(zlen);
    cptr e = z + zlen-zlen%3;

    while(z<e)
    {
        uint c1 = *(uptr)s++;
        uint c2 = *(uptr)s++;
        uint c3 = *(uptr)s++;
        uint c4 = *(uptr)s++;	if((c1|c2|c3|c4)&0x80) return nullptr;
        c1 = unbase64[c1];
        c2 = unbase64[c2];
        c3 = unbase64[c3];
        c4 = unbase64[c4];		if((c1|c2|c3|c4)&0x80) return nullptr;
        uint32 n = (c1<<18) + (c2<<12) + (c3<<6) + c4;
        *z++ = n>>16;
        *z++ = n>>8;
        *z++ = n;
    }

    XXXASSERT((*s==0)==(padd==0));

    if(padd)
    {
        uint c1 = *(uptr)s++;
        uint c2 = *(uptr)s++;
        uint c3 = *(uptr)s++;
        if((c1|c2|c3)&0x80) return nullptr;

        c1 = unbase64[c1];
        c2 = unbase64[c2];
        c3 = padd==2 ? 0 : unbase64[c3];
        if((c1|c2|c3)&0x80) return nullptr;

        *z++ = (c1<<2) + (c2>>4);
        if(padd==2)
        *z++ = (c2<<4) + (c3>>2);
    }

    return z-zlen;
}


bool startswith( cstr a, cstr b )
{
    if(!b) return yes;		// leerer suchstring
    if(!a) return !*b;		// leeres target? => nur ok wenn leerer suchstring

    while(*b)
    {
        if(*a++ != *b++) return no;
    }
    return yes;
}

bool endswith( cstr a, cstr b )
{
    if(!b) return yes;		// leerer suchstring
    if(!a) return !*b;		// leeres target? => nur ok wenn leerer suchstring

    cstr b0 = b;
    a = strchr(a,0);
    b = strchr(b,0);

    while(b>b0)
    {
        if(*--a != *--b) return no;
    }
    return yes;
}


/*	split string at "well known" line separators
    the data of the string is reused for the array strings!
    => 	if string is in temp mem
        then the returned strings are in temp mem too
    NOTE: char *e is overwritten with 0
*/
void split(Array<str>& array, ptr a, ptr e)
{
    XXASSERT(a!=nullptr);

    #define line_separators 0b0011010000010001

    while(a<e)
    {
        // append not-yet-0-delimited string:
        array.append(a);

        // search for line end:
        uint8 c = 0;
        for(;a<e;a++)
        {
            c = *a; if(c>13) continue;
            if((1<<c)&line_separators) break;
        }

        // poke cstring delimiter at string end:
        *a++ = 0;
        if((c==10||c==13) && a<e && *a+c==23) a++;
    }
}


/*	split string at separator
    the data of the string is reused for the array strings!
    => 	if string is in temp mem
        then the returned strings are in temp mem too
    NOTE: char *e is overwritten with 0
*/
void split(Array<str>& array, ptr a, ptr e, char c)
{
    XXASSERT(a!=nullptr);

    while(a<e)
    {
        // append not-yet-0-delimited string:
        array.append(a);

        // search for line end:
        while(a<e && *a!=c) { a++; }

        // poke cstring delimiter at string end:
        *a++ = 0;
    }
}


/*	remove white spaces at start and end of string
    reuses the argument sting!
*/
str croppedstr(str s)
{
    while(*s && *(uptr)s<=' ') s++;
    ptr p = strchr(s,0);
    while(--p>=s) if(*(uptr)p<=' ') *p=0; else break;
    return s;
}


str croppedstr(cstr s)
{
    return croppedstr(dupstr(s));
}


/*  strncpy variant which always terminates the destination string
    Returns the resulting string size or the buffer size, if the string was truncated.
    The string is always delimited with a 0 character unless sz = 0.
*/
uint strcpy( ptr z, cptr q, uint sz )
{
    ptr za = z, ze = za+sz;
    while(z<ze) { if((*z++=*q++)==0) return (uint)(--z-za); }
    if(sz) *--z = 0;
    return sz;
}

/*  strncat variant which always terminates the destination string
    Returns the resulting string size or the buffer size, if the string was truncated.
    The string is always delimited with a 0 character
        unless sz = 0 or if no '\0' is found in z within sz characters,
        then strcat returns sz and the returned string was and is not terminated!
    Note: you can blindly concatenate multiple strings with strcat, it will not overflow!
*/
uint strcat( ptr z, cptr q, uint sz )
{
    ptr za = z, ze = za+sz;
    while(z<ze && *z) z++;
    return (uint)(z-za + strcpy(z,q,(uint)(ze-z)));
}














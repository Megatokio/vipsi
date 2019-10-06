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

	2003-08-15 kio	fixed buggy call to ioctl() in SetBlocking() and SetAsync()
	2003-10-09 kio	FullPath(): enhanced to match broken utf-8 filenames
	2004-05-07 kio	FullPath(): added buffer overflow check for initial rpath assembly
	2004-05-08 kio	FullPath(): returned path now has a trailing "/" for a deref'ed, existing directory
	2004-06-11 kio	FullPath(): Bug: always overwrote n.ex. path components with latin1
	2007-02-14 kio	NewTempFile(), DupFile(), filecopy(), lots of new file i/o functions.
*/

#define SAFE 3
#define LOG 1
#include "kio/kio.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <new>
#include "file_utilities.h"
#include "errors.h"
DEBUG_INIT_MSG


typedef struct std::nothrow_t nothrow_t;		// <new>:	std::nothrow_t
nothrow_t nothrow;


/* ---- set/clear blocking io -----------
		=> errno EAGAIN wenn input empty/output full
*/
void SetBlocking ( int fd, bool onoff )
{
	int arg = fcntl(fd,F_GETFL,&arg);
	if (arg==-1) return; // errno set
	if (onoff) arg &= ~O_NONBLOCK; else arg |= O_NONBLOCK;
	fcntl(fd,F_SETFL,arg);
}

/* ----	set/clear async io mode ----------
		=> signal SIGIO wenn input available/output possible
		***???*** lt. dt. man page geht das mit fcntl() nicht!
		***!!!*** lt. engl. man page geht es.
*/
void SetAsync ( int fd, bool onoff )
{
	int arg = fcntl(fd,F_GETFL,&arg);
	if (arg==-1) return; // errno set
	if (onoff) arg |= O_ASYNC; else arg &= ~O_ASYNC;
	fcntl(fd,F_SETFL,arg);
}


void SetBlocking ( FILE* file, bool onoff )	{ SetBlocking(fileno(file), onoff); }
void SetAsync    ( FILE* file, bool onoff )	{ SetAsync   (fileno(file), onoff); }


/* ---- stat() file --------------------------------
*/
static int stat ( cstr path, struct stat* buf, bool follow_symlink )
{
	return follow_symlink ? stat(path,buf) : lstat(path,buf);
}


/* ---- modification date of file ------------------
*/
time_t FileMTime ( cstr path, bool follow_symlink )
{
	struct stat fs;
	stat(path,&fs,follow_symlink);
	return M_TIME(fs);
}


/* ---- last access date of file ------------------
*/
time_t FileATime ( cstr path, bool follow_symlink )
{
	struct stat fs;
	stat(path,&fs,follow_symlink);
	return A_TIME(fs);
}


/* ---- last status change time of file ------------------
*/
time_t FileCTime ( cstr path, bool follow_symlink )
{
	struct stat fs;
	stat(path,&fs,follow_symlink);
	return C_TIME(fs);
}



s_type ClassifyFile ( mode_t mode )
{
	if (S_ISREG (mode)) return s_file;  else	// File
	if (S_ISFIFO(mode)) return s_pipe;  else	// Pipe
	if (S_ISDIR (mode)) return s_dir;   else	// Directory
	if (S_ISBLK (mode)) return s_block; else	// Block device
	if (S_ISSOCK(mode)) return s_sock;  else	// Socket
	if (S_ISLNK (mode)) return s_link;  else	// Symbolic link
#if !defined(_POSIX_SOURCE) && !defined(_SOLARIS)
	if (S_ISWHT (mode)) return s_null;  else	// White out
#endif
	if (S_ISCHR (mode)) return s_tty;   else	// Character special
	return s_unkn;
}


s_type ClassifyFile ( cstr path, bool follow_symlink )
{
	if (!path) path="";		// --> ENOENT
	struct stat fs;
	if( stat(path,&fs,follow_symlink) ) return s_none;
	else return ClassifyFile(fs.st_mode);
}


s_type ClassifyFile ( int fd )
{
//	if (fd<0) return s_none;	--> EBADF
	struct stat data;
	if (fstat(fd, &data)) return s_none;
	else return ClassifyFile(data.st_mode);
}


s_type ClassifyFile( FILE* file )
{
	if (file) return ClassifyFile(fileno(file));
	else { SetError(EBADF); return s_none; }
}


off_t FileLength ( cstr path, bool follow_symlink )
{	struct stat fs;
	if (!path||!*path) return -1;
	if (stat(path,&fs,follow_symlink)) return -1;	// error
	return fs.st_size;
}

off_t FileLength ( int fd )
{	struct stat fs;
	if (fd<0) return -1;
	if (fstat(fd,&fs)) return -1;			// error
	return fs.st_size;
}

off_t FileLength ( FILE* file )
{	struct stat fs;
	if (!file) return -1;
	if (fstat(fileno(file),&fs)) return -1;	// error
	return fs.st_size;
}



/* ----	separate filename from path ---------------------------------------
		returned string points into passed path argument  ((guaranteed!))
		returned string is empty if path ended with '/'
*/
cstr FileNameFromPath ( cstr path )
{
	assert(path);
	cstr p = strrchr(path,'/');
	return p ? p+1 : path;
}


/* ----	separate filename or last directory from path ---------------------
		returned string points into passed path argument  ((guaranteed!))
*/
cstr LastComponentFromPath ( cstr path )
{
	assert(path);
	cstr p = strchr(path,0) -2;
	while( p>=path && *p!='/' ) { p--; }
	return p>=path ? p+1 : path;
}


/* ----	separate filename extension from path -----------------------------
		returned string points into passed path argument  ((guaranteed!))
		returned strings includes '.'
		if path has no valid extension, then the returned string points to chr(0) at end of path
*/
cstr ExtensionFromPath ( cstr path )
{
	path = FileNameFromPath(path);
	cstr dot = strrchr(path,'.');
	cstr spc = strrchr(path,' ');
	if( dot>spc ) return dot; else return strchr(path,0);
}


/* ----	separate filename basename from path ------------------------------
		returns string in cstring pool
*/
cstr BasenameFromPath ( cstr path )
{
	path = FileNameFromPath(path);
	return substr( path, ExtensionFromPath(path) );
}


/* ----	separate directory path from path ------------------------------
		returns string in cstring pool
		returned string includes final '/'
		returns "./" instead of "" if passed path contains no '/'
*/
cstr DirectoryPathFromPath ( cstr path )
{
	path = substr( path, FileNameFromPath(path) );
	return *path ? path : "./";
}


/* ----	convert filename to latin-1 ------------------------- [rewritten: kio 2004-05-10]

		normally when searching a directory for a given filename, we expect both,
		the filename to search and the filenames in the directory, to have the same encoding.

		if both use utf-8 or both use the same 8-bit character set everything is fine.
		even if both use _different_ 8-bit character sets, the application can at least
		access all files, even if the filenames displayed may look weird.

		but if our application internally uses UCS2 or UCS4 encoded strings (unicode)
		then it will convert filenames to utf-8 encoded c-style strings to access the files.

		valid utf-8 strings can not express all arbitrary 8-bit character sequences.
		thus this way some 8-bit character set filenames not only display strange
		but also become completely unreachable for the application.

		Latin1() implements a quirk to find iso-latin-1 encoded filenames in a directory
		which correspond to a utf-8 encoded filename.

	howto:
		1. match your filename against all filenames in the directory of interest without quirks.
		2. if you don't find a match, try to match against Latin1(filename) as a second shot.

	returns:
		returns iso-latin-1 encoded filename or
		returns nullptr if conversion did not result in a different filename
*/
static cstr Latin1 ( cstr filename )
{
// does filename contain non-Ascii letters?
// if not, then there is no difference between utf-8 and latin-1 --> quick exit
	cptr q = filename;
	for(;;)
	{
		char c = *q++;
		if(c>0)  continue;
		if(c==0) return nullptr;				// no non-ascii letters found
		else	 break;						// non-acii letter found
	}

// convert filename to iso-latin-1:
// if filename contains utf-8 character codes >= 256 --> give up
// if filename contains broken utf-8 itself --> give up
// if filename contains illegal overlong encoding --> give up
	while(*q) q++;							// find string end
	str file = tempstr(q-filename);			// temp string
	ptr z = file;
	q = filename;
	for(;;)
	{
		char c = *q++;
		if (c>0)  { *z++ = c; continue; }
		if (c==0) { *z=0; break; }			// conversion utf-8 -> latin-1 complete
	// 2 byte codes:	110xxxxx 10xxxxxx
	// ill. overlong:	1100000x 10xxxxxx
	// latin-1 range:	1100001x 10xxxxxx
		if ((c&0xFE)!=0xC2) return nullptr;	// char code too high, ill. overlong encoding or broken utf-8
		uchar c2 = *q++ - 0x80;
		if (c2>=0x3F) return nullptr;			// no fup -> broken utf-8
		*z++ = (c<<6) + c2;					// combine c1 + fup
	}

// the resulting latin-1 string may also be a valid utf-8 string again!
// if so, we must not return it:
//
// if we used it, and it actually is an ordinary utf-8 filename,
//		then we access and manipulate wrong files:
//		e.g. we try to delete all files "XY" and unexpectedly also delete all files "Z".
// if we don't use it, though it really is a latin-1 filename,
//		then it is still accessible by the application
//		though it probaly will have a weird filename
//		which even may contain exotic characters which your display font does not provide
//		therefore showing up as the 'character replacement char', frequently a square box.
//
// so using it is dangerous, not using it is just ugly.
// therefore we assert that the converted filename is invalid utf-8 and discard it otherwise.
//
// routine adopted from:
// 		static UCS4Char* ucs4_from_utf8 ( UCS4Char*z, cUTF8CharPtr a, cUTF8CharPtr e, bool latin_1 )
// 		in lib-kio/ustring/sstring.cp
// 		re-implemented to avoid dependency on class String.
	z = file;
	for (;;)
	{
		uint32 c0 = uchar(*z++);
		if ((char)c0>0) continue;					// ascii
		if (c0==0) return nullptr;						// end of filename reached without invalid utf-8
		if ((char)c0 < (char)0xc0) return file;		// unexpected fups
		if (c0>=0xfe) return file;					// ill. codes

	// parse multi-byte character:
		uint32 n = 0;		// UCS-4 char code akku
		uint  i = 0;		// UTF-8 character size

		while( c0 & (0x80u>>(++i)) )
		{
			uint32 c1 = uchar(*z++);
			if ((char)c1 >= (char)0xc0) return file;// no fup: truncated char (end of text or non-fup)
			n = (n<<6) + (c1&0x3F);
		}

	// now: i = total number of digits
	//      n = UCS4 char code without c0 bits
		n += (c0&(0x7f>>i)) << (6*i-6);				// n := UCS4 char code

	// test for ill. overlong encoding:
		if ( n < (1u<<(i*5-4)) ) return file;		// ill. overlong encoding
	}
}



/* ---- real path ----------------------------------
		resolve partial paths
		resolve stuff like "//" and "./" and "../"
		resolve home directory
		resolve symbolic links
		resolve broken utf-8 filenames  ((option))
		no error for not present final path component
		=> assume directory/file will be created
		returned path has a trailing "/" for an existing, deref'ed directory
		errno: clears or sets errno, prepends offending subpath to errormsg

		returned path is temp only, allocated in quick string pool (--> cstrings.cp)

		2004-05-07 kio	added buffer overflow check for initial rpath assembly
*/
cstr FullPath( cstr path, bool follow_symlink )
{
	const int bu_size = MAXPATHLEN+255;
	char bu[bu_size+1];

	int lc = 33;				// link counter -> detect recursion

	ptr k1, lpath, l_end, rpath, r_end;
	lpath = l_end = bu;
	rpath = r_end = bu +bu_size;

	// sanity check:
	if (!path||!*path) path = ".";

	// copy path to right side of buffer:
	rpath -= strlen(path);
	if ( rpath <= l_end ) { ForceError(ENAMETOOLONG); goto x; }
	memcpy ( rpath, path, r_end-rpath );

	// path starts with '~'  -->  prepend home directory
	if( rpath[0]=='~' && (r_end-rpath==1||rpath[1]=='/') && (k1=getenv("HOME")) && k1[0]=='/' )
	{
		rpath++;				// discard "~".
								// after that a "/" or str_end is assured by above tests
		int n = strlen(k1);
		rpath -= n;
		if ( rpath <= l_end ) { ForceError(ENAMETOOLONG); goto x; }
		memcpy ( rpath, k1, n );
	}

	// not an absolute path  -->  prepend local directory
	else if( rpath[0]!='/' )
	{
		k1 = getcwd(bu,bu_size);
		assert(k1==bu);
		assert(k1[0]=='/');

		memcpy ( rpath, path, r_end-rpath );	// <-- copy again: MacOSX/Darwin garbages whole buffer!
		*--rpath = '/';
		int n = strlen(k1);
		rpath -= n;
		if ( rpath <= l_end ) { ForceError(ENAMETOOLONG); goto x; }
		memmove( rpath, k1, n );
	}


// *** doit ***

/*	note:

	bu: [==================================================================]
		[lpath........l_end        ..unused..         rpath...........r_end]

		lpath  is already checked & deref'ed
		rpath  is not yet checked & deref'ed
*/

	for(;;)
	{
		errno=ok;

	// left side path _is_ a checked & deref'ed directory path:
		if( l_end==lpath || l_end[-1]!='/' ) *l_end++ = '/';

	// finished ?
		if( rpath>=r_end )  break;

	// split k1 from rpath
		k1 = l_end;					// move current component w/o trailing '/' from rpath to lpath
		while(rpath<r_end&&*rpath!='/') { *l_end++ = *rpath++; }
		if(rpath<r_end) rpath++; 	// rpath = "remaining/rpath"

	// handle k1:

	// discard "//"
		if (k1+0==l_end) continue;

	// discard "./"
		if (k1+1==l_end&&k1[0]=='.') { l_end = k1; continue; }

	// "../" -> backup one directory
		if( k1+2==l_end && k1[0]=='.' && k1[1]=='.' )
		{
			for( l_end=k1-1; l_end>lpath&&l_end[-1]!='/'; ) { l_end--; }
			continue;
		}

	// does path component exist?
		struct stat statdata;
		*l_end=0;
		lstat(lpath,&statdata);
		if(errno)
		{
			if ( errno==ENOENT )
			{
				cstr k2 = Latin1(k1);	// try iso-latin-1
				if (k2)
				{
					assert(strlen(k2) < size_t(l_end-k1)); 	// strlen(latin1) < strlen(utf8)
					str latin1path = new char[l_end-lpath];
					memcpy(latin1path,lpath,k1-lpath);
					strcpy(latin1path+(k1-lpath),k2);
					errno = noerror;
					lstat(latin1path,&statdata);
					if (errno == noerror)	// success! => latin1 übernehmen
					{				// copy text & update l_end
						for( l_end=k1; *k2; ) { *l_end++ = *k2++; }
					}
					delete[] latin1path;
				}
			}
			if(errno)
			{
				if( errno==ENOENT && rpath==r_end )
				{
					errno=ok;		// no error if final path component not found
				}
				break;
			}
		}

	// handle directory:
		if( S_ISDIR(statdata.st_mode) )
		{
			continue;
		}

	// handle link:
		if( S_ISLNK(statdata.st_mode) && (rpath<r_end || follow_symlink) )
		{
			if (--lc==0) { ForceError(ELOOP); break; }
			int n = readlink( lpath, l_end, rpath-l_end-1 );
			if (n<0) { ForceError(ENAMETOOLONG); break; }
			if (n==0) { l_end = k1; continue; }
			if (rpath<r_end) *--rpath='/';
			rpath -= n;
			assert(rpath >= l_end);
			memmove( rpath, l_end, n );
			l_end = *rpath=='/' ? lpath : k1;
			continue;
		}

	// handle regular file ((or pipe et.al.))
		if( rpath<r_end ) ForceError(ENOTDIR);
		break;
	}


	assert( l_end != lpath );
	path = substr(lpath,l_end);
x:	if (errno) ForceError( errno, usingstr("%s: %s",quotedstr(path),errorstr()) );
	return path;
}


/* ----	provide a temp file name --------------
		temp file must be renamable to file.
*/
cstr TempFileName ( cstr file, bool follow_symlink )
{
	file = FullPath(file,follow_symlink);
	return catstr(file,".temp");
}


/*	create temp file in $TMPDIR or "/tmp/"
	opens file for r/w by the calling user.
	unlinks file so that it will vanish after close().
	returns fd on success.
	returns -1 if creating file fails. errno set.
*/
int NewTempFile ( )
{
	cstr tmpdir = getenv("TMPDIR");
  //if (!tmpdir) tmpdir = P_tmpdir;		strongly discouraged! - uses setting from compiling system - not the one running on!

	for(;;)
	{
		cstr path = catstr( tmpdir?tmpdir:"/tmp/", hexstr(random(),8) );
		int  fd   = open( path, O_RDWR|O_CREAT|O_EXCL, 0600 );
		if (fd>=0) { unlink(path); return fd; }		// opened ok
		if (errno==EEXIST) continue;				// exists => retry
		if (tmpdir) { tmpdir = nullptr; continue; }	// maybe $TMPDIR is broken
		return -1;									// can't create temp file!
	}
}


// ioctl -> tty control
// tcgetattr/tcsetattr -> tty control

int TerminalSize ( int tty, int& rows, int& cols )
{
	struct winsize data;
	data.ws_row = (int16)-1;
	data.ws_col = (int16)-1;
	int r = ioctl ( tty, TIOCGWINSZ, &data );
	rows = (int16)data.ws_row;
	cols = (int16)data.ws_col;
	return r;
}

int TerminalRows ( int tty )
{
	struct winsize data;
	data.ws_row = (int16)-1;
	ioctl ( tty, TIOCGWINSZ, &data );
	return (int16)data.ws_row;
}

int TerminalCols ( int tty )
{
	struct winsize data;
	data.ws_col = (int16)-1;
	ioctl ( tty, TIOCGWINSZ, &data );
	return (int16)data.ws_col;
}

int TerminalWidth ( int tty )
{
	struct winsize data;
	data.ws_xpixel = (int16)-1;
	ioctl ( tty, TIOCGWINSZ, &data );
	return (int16)data.ws_xpixel;
}

int TerminalHeight ( int tty )
{
	struct winsize data;
	data.ws_ypixel = (int16)-1;
	ioctl ( tty, TIOCGWINSZ, &data );
	return (int16)data.ws_ypixel;
}


/* note: does not work.
*/
int SetTerminalSize(int tty, int rows, int cols )
{
	struct winsize data;
	int r = ioctl ( tty, TIOCGWINSZ, &data );
	if(r!=-1)
	{
		data.ws_xpixel = data.ws_xpixel/data.ws_col*cols;		// superfluxous
		data.ws_ypixel = data.ws_ypixel/data.ws_row*rows;		// superfluxous
		data.ws_row    = rows;
		data.ws_col    = cols;
		r = ioctl ( tty, TIOCSWINSZ, &data );
	}
	return r;
}



// ################################################################
//		unified file i/o  --  nothrow_t functions
//
//		the nothrow_t functions always clear or set errno
//		and return the amount of bytes actually written or read.
// ################################################################


/*	open file:
	• absolute path
	• relative path
	• home directory
*/
int open_file ( nothrow_t, cstr path, int flags, int mode ) throw()
{
	if (!path) path = "";							// security only. will throw.

	if( path[0]=='~' && path[1]=='/' )				// test: add user's home directory path?
	{
		cstr home = getenv("HOME");
		if ( home && home[0]=='/' )					// home dir known?
		{
			int f = home[ strlen(home)-1 ]=='/';	// ends with '/' ?
			path = catstr( home, path+1+f );		// prepend home dir to path
		}
	}
	return open( path, flags, mode );
}

//int open_file_r ( nothrow_t, cstr path )			throw()	{ return open_file(nothrow, path,O_RDONLY); }
//int open_file_rw( nothrow_t, cstr path )			throw()	{ return open_file(nothrow, path,O_RDWR); }
//int open_file_w ( nothrow_t, cstr path, int mode )	throw()	{ return open_file(nothrow, path,O_WRONLY|O_CREAT|O_TRUNC,mode); }
//int open_file_a ( nothrow_t, cstr path, int mode )	throw()	{ return open_file(nothrow, path,O_WRONLY|O_CREAT|O_APPEND,mode); }
//int open_file_n ( nothrow_t, cstr path, int mode )	throw()	{ return open_file(nothrow, path,O_WRONLY|O_CREAT|O_EXCL,mode); }
//

uint32 read_bytes ( nothrow_t, int fd, ptr p, uint32 bytes ) throw()
{
	errno=ok;							// clear errno and custom_error
	uint32 m = bytes;
r:	uint32 n = read( fd, p, m );
	if (n==m)	{ errno=noerror;   return bytes;   }
	if (n==0)	{ errno=endoffile; return bytes-m; }
	if (n<m)	{ p+=n; m-=n; goto r; }		// n<m  <=>  n!=-1
	if (errno==EINTR) goto r;				// slow device only
//	if (errno==EAGAIN) goto r;				// non-blocking dev only
	return bytes-m;
}

off_t write_bytes ( nothrow_t, int fd, cptr p, uint32 bytes ) throw()
{
	errno=ok;							// clear errno and custom_error
	uint32 m = bytes;
w:	uint32 n = write( fd, p, m );
	if (n==m)	{ errno=noerror; return bytes; }
	if (n<m)	{ p+=n; m-=n; goto w; }		// n<m  <=>  n!=-1
	if (errno==EINTR) goto w;				// slow device only
//	if (errno==EAGAIN) goto w;				// non-blocking dev only
	return bytes-m;							// bytes written until failure. errno set.
}


/*	Truncate file at current fpos and return current fpos == file size
	error: return -1
	note: should only fail if the fd is unsuited (invalid, r/o or not a file).
*/
off_t clip_file ( nothrow_t, int fd ) throw()
{
	errno=ok;							// clear errno and custom_error
	off_t fpos = lseek( fd, 0, SEEK_CUR );
	if( fpos>=0 && ftruncate( fd, fpos )==0 ) { errno=noerror; return fpos; }
	return -1;								// error
}


/*	Close file
	return: errno
	error:  the file is not closed
	note:   should only fail, if the fd is invalid.
*/
int/*err*/ close_file ( nothrow_t, int fd ) throw()
{
	errno=ok;							// clear errno and custom_error
	do
	{ if( close(fd)==0 ) { errno=noerror; return noerror; }
	} while (errno==EINTR);					// slow device only
	return errno;
}


/*	Truncate and close file at current fpos
	return: errno
	error:  the file is not closed
*/
//int/*err*/ clip_and_close ( nothrow_t, int fd ) throw()
//{
//	if( clip_file(nothrow,fd)==-1 ) return errno;
//	else return close_file(nothrow,fd);
//}




// ################################################################
//		unified file i/o  --  throw(..) functions
//
//		the throw(..) functions only return if no error occured.
//		else they throw a file_error exception.
//
//		ok:		function returns, all data was read or written, errno = ok.
//		error:	throws exception, bytes read or written unknown, errno set.
//				note: premature end of file is handled as an error.
// ################################################################


/*	open file:
	• absolute path
	• relative path
	• home directory
*/
int open_file ( cstr path, int flags, int mode ) noexcept(false)
{
	int fd = open_file( nothrow, path, flags, mode );
	if(fd!=-1) return fd; else throw(file_error(path,errno));
}

int open_file_r ( cstr path )				noexcept(false)	{ return open_file(path,O_RDONLY); }
int open_file_rw( cstr path )				noexcept(false)	{ return open_file(path,O_RDWR); }
int open_file_w ( cstr path, int mode )		noexcept(false)	{ return open_file(path,O_WRONLY|O_CREAT|O_TRUNC,mode); }
int open_file_a ( cstr path, int mode )		noexcept(false)	{ return open_file(path,O_WRONLY|O_CREAT|O_APPEND,mode); }
int open_file_n ( cstr path, int mode )		noexcept(false)	{ return open_file(path,O_WRONLY|O_CREAT|O_EXCL,mode); }


/*	Truncate file at current fpos and return current fpos == file size
	note: should only fail if the fd is unsuited (invalid, r/o or not a file).
*/
off_t clip_file ( int fd ) noexcept(false)
{
	errno=ok;							// clear errno and custom_error
	off_t fpos = lseek( fd, 0, SEEK_CUR );
	if( fpos<0 || ftruncate( fd, fpos )!=0 ) throw file_error("(unkn.)",errno);
	errno = ok; return fpos;
}


/*	Close file
	error: the file is not closed.
	note:  should only fail, if the fd is invalid.
*/
void close_file ( int fd ) noexcept(false)
{
	errno=ok;							// clear errno and custom_error
	do
	{ if( close(fd)==0 ) { errno=noerror; return; }
	} while (errno==EINTR);					// slow device only

	throw file_error("(unkn.)",errno);
}


/*	Truncate and close file at current fpos
	error: the file is not closed.
*/
void clip_and_close ( int fd ) noexcept(false)
{
	clip_file(fd);
	close_file(fd);
}


off_t seek_fpos( int fd, off_t fpos, int whence ) noexcept(false)
{
	fpos = lseek( fd, fpos, whence );
	if( fpos==-1 ) throw file_error("(unkn.)",errno);
	return fpos;
}


uint32 read_bytes ( int fd, ptr p, uint32 bytes ) noexcept(false)
{
	errno=ok;							// clear errno and custom_error
	uint32 m = bytes;
r:	uint32 n = read( fd, p, m );
	if (n==m)	{ errno=noerror; return bytes; }
	if (n==0)	{ errno=endoffile; } else
	if (n<m)	{ p+=n; m-=n; goto r; }		// n<m  <=>  n!=-1
	if (errno==EINTR) goto r;				// slow device only
//	if (errno==EAGAIN) goto r;				// non-blocking dev only
	throw file_error("(unkn.)",errno);
}


uint32 write_bytes ( int fd, cptr p, uint32 bytes ) noexcept(false)
{
	errno=ok;							// clear errno and custom_error
	uint32 m = bytes;
w:	uint32 n = write( fd, p, m );
	if (n==m)	{ errno=noerror; return bytes; }
	if (n<m)	{ p+=n; m-=n; goto w; }		// n<m  <=>  n!=-1
	if (errno==EINTR) goto w;				// slow device only
//	if (errno==EAGAIN) goto w;				// non-blocking dev only
	throw file_error("(unkn.)",errno);
}


#if defined(_BIG_ENDIAN)

/*	read array of short ints in Intel/Z80 byte order
	note: for little endian cpus (intel/z80) this is straight forward
		  and defined inline as read_data(fd,int16*,cnt)
*/
uint32 read_short_data_z( int fd, int16* bu, uint32 items ) noexcept(false)
{
	read_data( fd, bu, items );				// throw file_error
	for( uint32 i=0; i<items; i++ ) { bu[i] = Peek2Z(bu+i); }
	return items;
}

/*	write array of short ints in Intel/Z80 byte order
	note: for little endian cpus (intel/z80) this is straight forward
		  and defined inline as write_data(fd,int16*,cnt)
*/
uint32 write_short_data_z( int fd, int16 const* bu, uint32 items ) noexcept(false)
{
	if( items<=4*1024 ) try
	{
		int16 zbu[items];				// throw bad_alloc
		for( uint32 i=0; i<items; i++ ) { Poke2Z( zbu+i, bu[i]); }
		write_data( fd, zbu, items );	// throw file_error
		return items;
	}
	catch(bad_alloc){}

	write_short_data_z( fd, bu,         items/2 );
	write_short_data_z( fd, bu+items/2, items-items/2 );
	return items;
}
#endif


#if defined(_LITTLE_ENDIAN)

/*	read array of short ints in internet/ppc/68k byte order
	note: for big endian cpus (ppc/68k) this is straight forward
		  and defined inline as read_data(fd,int16*,cnt)
*/
uint32 read_short_data_x( int fd, int16* bu, uint32 items ) noexcept(false)
{
	read_data( fd, bu, items );				// throw file_error
	for( uint32 i=0; i<items; i++ ) { bu[i] = peek2X(bu+i); }
	return items;
}

/*	write array of short ints in internet/ppc/68k byte order
	note: for big endian cpus (ppc/68k) this is straight forward
		  and defined inline as write_data(fd,int16*,cnt)
*/
uint32 write_short_data_x( int fd, int16 const* bu, uint32 items ) noexcept(false)
{
	if( items<=4*1024 ) try
	{
		int16 zbu[items];				// throw bad_alloc
		for( uint32 i=0; i<items; i++ ) { poke2X( zbu+i, bu[i]); }
		write_data( fd, zbu, items );	// throw file_error
		return items;
	}
	catch(bad_alloc&){}

	write_short_data_x( fd, bu,         items/2 );
	write_short_data_x( fd, bu+items/2, items-items/2 );
	return items;
}
#endif


int8 read_char ( int fd ) noexcept(false)
{
	int8 c;
	read_bytes(fd,&c,1);
	return c;
}

void write_char ( int fd, int8 c ) noexcept(false)
{
	write_bytes(fd,&c,1);
}

int16 read_short ( int fd ) noexcept(false)
{
	int16 z;
	read_bytes(fd,(ptr)&z,2);
	return z;
}

int16 read_short_x ( int fd ) noexcept(false)
{
	int8 bu[2];
	read_bytes(fd,bu,2);
	return peek2X(bu);
}

int16 read_short_z ( int fd ) noexcept(false)
{
	int8 bu[2];
	read_bytes(fd,bu,2);
	return peek2Z(bu);
}

void write_short ( int fd, int16 n ) noexcept(false)
{
	write_bytes(fd,(cptr)&n,2);
}

void write_short_x ( int fd, int16 n ) noexcept(false)
{
	int8 bu[2];
	poke2X(bu,n);
	write_bytes(fd,bu,2);
}

void write_short_z ( int fd, int16 n ) noexcept(false)
{
	int8 bu[2];
	poke2Z(bu,n);
	write_bytes(fd,bu,2);
}


int32 read_long ( int fd ) noexcept(false)
{
	int32 z;
	read_bytes(fd,(ptr)&z,4);
	return z;
}

int32 read_long_x ( int fd ) noexcept(false)
{
	int8 bu[4];
	read_bytes(fd,bu,4);
	return peek4X(bu);
}

int32 read_long_z ( int fd ) noexcept(false)
{
	int8 bu[4];
	read_bytes(fd,bu,4);
	return peek4Z(bu);
}

void write_long ( int fd, int32 n ) noexcept(false)
{
	write_bytes(fd,(cptr)&n,4);
}

void write_long_x ( int fd, int32 n ) noexcept(false)
{
	int8 bu[4];
	poke4X(bu,n);
	write_bytes(fd,bu,4);
}

void write_long_z ( int fd, int32 n ) noexcept(false)
{
	int8 bu[4];
	poke4Z(bu,n);
	write_bytes(fd,bu,4);
}

uint32 read_u3byte_x ( int fd ) noexcept(false)
{
	int8 bu[4]={0,0,0,0};
	read_bytes(fd,bu+1,3);
	return peek4X(bu);
}

int32 read_3byte_x ( int fd ) noexcept(false)
{
	int8 bu[4]={0,0,0,0};
	read_bytes(fd,bu+1,3);
	if(bu[1]<0) bu[0]=0xff;
	return peek4X(bu);
}

uint32 read_u3byte_z ( int fd ) noexcept(false)
{
	int8 bu[4]={0,0,0,0};
	read_bytes(fd,bu,3);
	return peek4Z(bu);
}

int32 read_3byte_z ( int fd ) noexcept(false)
{
	int8 bu[4]={0,0,0,0};
	read_bytes(fd,bu,3);
	if(bu[2]<0) bu[3]=0xff;
	return peek4Z(bu);
}

//void write_nstr ( int fd, cstr s ) throw(file_error,limit_error)
//{
//	size_t len = s ? strlen(s) : 0;
//	if(len>0xff) throw limit_error("write_nstr: string too long",len,0xff);
//	write_uchar(fd,len);
//	write_data(fd,s,len);
//}

str read_nstr ( int fd ) noexcept(false)
{
	str s = nullptr; uint len = read_uchar(fd);
	if(len) { s = tempstr(len); read_data(fd,s,len); } return s;
}

str read_new_nstr ( int fd ) noexcept(false)
{
	str s = nullptr; uint len = read_uchar(fd);
	if(len) { s = newstr(len); read_data(fd,s,len); } return s;
}

off_t file_size ( int fd ) noexcept(false)
{
	struct stat fs;
	//if( fd<0 ) throw(file_error(EBADF));
	if( fstat(fd,&fs) ) throw(file_error("(unkn.)",errno));
	return fs.st_size;
}

/*	duplicate source file to destination file
	copy file contents from src to dest file.
	truncate dest file to src_sz bytes.
	src_sz=-1:	copy entire file.
	src_sz>=0:	copy entire file or src_sz bytes, whatever is less.
	ok:			errno cleared.	returns bytes copied.
	error:		errno set.		returns -1.
*/
//off_t DupFile ( int dest_fd, int src_fd, off_t src_sz )
//{	off_t n,m,dest_sz = 0;
//
//	if ( src_sz==-1 ) src_sz = FileLength(src_fd);
//	if ( src_sz==-1 ) return -1;						// failed
//	if ( lseek(dest_fd,0,SEEK_SET) < 0 ) return -1;		// failed
//	if ( lseek(src_fd,0,SEEK_SET) < 0 )  return -1;		// failed
//
//	n  = min( off_t(50 MB), src_sz );
//a:	ptr bu = new(nothrow) char[n]; if (!bu) { n=(n+1)/2; goto a; }
//
//	while( dest_sz<src_sz )
//	{
//		n = Min( n, src_sz-dest_sz );
//		m = read_bytes( nothrow, src_fd, bu, n );
//		if ( m && write_bytes( nothrow, dest_fd, bu, m ) == m ) { dest_sz += m; continue; }
//		else	{ if( errno==endoffile ) break; }		// src_file shorter than src_sz
//		delete[] bu; return -1;							// failed
//	}
//
//	delete[] bu;
//	if( ftruncate(dest_fd,dest_sz) == 0 ) errno=ok;
//	return dest_sz;
//}


/*	copy 'len' bytes at current positions from src file to dest file.
	copies in reverse order if dest > src fpos, because src and dest file may be same.
	ok:		errno cleared.	returns bytes copied.
	error:	errno set.		returns -1.			dest file in unknown state.
*/
off_t filecopy ( int dest_fd, int src_fd, off_t len )
{	off_t m,n,rem=len;

	if(len<=0) { errno=ok; return 0; }			// nothing to do.
	off_t dest_fpos = lseek(dest_fd,0,SEEK_CUR);	if (dest_fpos==-1) return -1;	// failed
	off_t src_fpos  = lseek(src_fd,0,SEEK_CUR);		if (src_fpos==-1)  return -1;	// failed

	n = min( off_t(50 MB), len );
a:	ptr bu = new(nothrow) char[n]; if(!bu) { n=n/2; goto a; }

	if (dest_fpos <= src_fpos )
	{
		while (rem)
		{
			n = min( n, rem );
			m = read_bytes( nothrow, src_fd, bu, n );
			if ( m && write_bytes( nothrow, dest_fd, bu, m ) == m ) { rem -= m; continue; }
			goto x;							// failed
		}
	}
	else
	{
		src_fpos  += len;
		dest_fpos += len;
		while (rem)
		{
			n = min( n, rem );
			src_fpos  = lseek( src_fd, src_fpos-n, SEEK_SET );	 if (src_fpos==-1)  goto x;
			dest_fpos = lseek( dest_fd, dest_fpos-n, SEEK_SET ); if (dest_fpos==-1) goto x;
			m = read_bytes( nothrow, src_fd, bu, n );
			if ( m && write_bytes( nothrow, dest_fd, bu, m ) == m ) { rem -= m; continue; }
	x:		delete[] bu; return -1;							// failed
		}
	}

	errno=ok;
	delete[] bu;
	return len;
}


/*	copy 'len' bytes at current positions from src file to dest file.
	copies in reverse order if dest > src fpos, because src and dest file may be same.
	ok:		errno cleared.	returns bytes copied.
	error:	errno set.		returns -1.			dest file in unknown state.
*/
off_t filecopy ( int dest_fd, off_t dest_pos, int src_fd, off_t src_pos, off_t len )
{
	if ( lseek( dest_fd,dest_pos,SEEK_SET ) < 0 ) return -1;		// failed
	if ( lseek( src_fd,src_pos,SEEK_SET ) < 0 )	  return -1;		// failed
	return filecopy( dest_fd, src_fd, len );
}











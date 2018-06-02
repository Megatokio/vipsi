/*	Copyright  (c)	Günter Woigk 2001 - 2015
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


    Wrapper class for the Unix file descriptor
    • based on file "fd_throw.cpp"
    • stores fd and filename
    • does not support copy c'tor or asignment: pass FD by reference only!
*/


#define SAFE 3
#define LOG 1
#include "config.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <termios.h>
#include "Templates/Array.h"
#include "tempmem.h"
#include "FD.h"
#include "Templates/ADA.h"
#include <unistd.h>
#include <sys/select.h>


#if !defined(_POSIX_SOURCE) && !defined(_SOLARIS)
    #define M_TIME(TS) ((TS).st_mtimespec.tv_sec)
    #define A_TIME(TS) ((TS).st_atimespec.tv_sec)
    #define C_TIME(TS) ((TS).st_ctimespec.tv_sec)
#else
    #define M_TIME(TS) ((TS).st_mtime /* +(TS).st_mtimesec/1e9 */)
    #define A_TIME(TS) ((TS).st_atime /* +(TS).st_atimesec/1e9 */)
    #define C_TIME(TS) ((TS).st_ctime /* +(TS).st_ctimesec/1e9 */)
#endif


// --- DATA ---------------------------

FD FD::stdin (0,"STDIN" );
FD FD::stdout(1,"STDOUT");
FD FD::stderr(2,"STDERR");


// --- HELPER ---------------------------

#define THROW_FILE_ERROR(WHERE) 	throw file_error(fpath, fd, errno, WHERE)



// --- CREATE & DESTUCT ---------------------------

// Create file descriptor from path
// using mode for opening mode and perm for file permissions
//
//	file path:
//	• absolute
//	• relative to pwd
//	• relative to home directory
//	open mode:
//	• unix open mode
//	perm:
//	• unix file permissions
//	error:
//	• throws file_error
//
int FD::open_file( cstr path, int mode, int perm ) THF
{
    XXASSERT(path!=NULL);
    XXASSERT(fd==-1);

    if(path[0]=='~' && path[1]=='/')			// test: add user's home directory path?
    {
        cstr home = getenv("HOME");
        if(home && home[0]=='/')				// home dir known?
        {
            int f = home[strlen(home)-1]=='/';	// ends with '/' ?
            path = catstr(home, path+1+f);		// prepend home dir to path
        }
    }

    if(mode=='r') mode = O_RDONLY;					else
    if(mode=='n') mode = O_WRONLY|O_CREAT|O_EXCL;   else
    if(mode=='a') mode = O_WRONLY|O_CREAT|O_APPEND; else
    if(mode=='w') mode = O_WRONLY|O_CREAT|O_TRUNC;  // else mode = mode;

    delete[] fpath;
    fpath = newcopy(path);
a:	fd = open(path, mode, perm);
    if(fd>=0) return ok;
    if(errno==EINTR) goto a;					// WebDAV
    THROW_FILE_ERROR("fd108");
}

//void FD::open_file_r ( cstr path )			THF	{ open_file(path, 'r', 0664); }
//void FD::open_file_rw( cstr path )			THF	{ open_file(path, O_RDWR, 0664); }
//void FD::open_file_w ( cstr path, int mode )	THF	{ open_file(path, 'w', mode); }
//void FD::open_file_a ( cstr path, int mode )	THF	{ open_file(path, 'a', mode); }
//void FD::open_file_n ( cstr path, int mode )	THF	{ open_file(path, 'n', mode); }


// create temp file in $TMPDIR or "/tmp/"
// opens file for r/w by the calling user.
// unlinks file so that it will vanish after close().
//
void FD::open_tempfile() THF
{
    XXASSERT(fd==-1);

    cstr tmpdir = getenv("TMPDIR");
    if(tmpdir && lastchar(tmpdir)!='/') tmpdir = catstr(tmpdir,"/");

    for(;;)
    {
        delete[] fpath;
        fpath = newcopy(catstr(tmpdir ? tmpdir : "/tmp/", hexstr((uint32)random(),8)));

        fd = open(fpath,O_RDWR|O_CREAT|O_EXCL,0600);
        if(fd>=0) { unlink(fpath); return; }		// opened ok

        if(errno==EEXIST) continue;					// exists => retry
        if(tmpdir) { tmpdir = NULL; continue; }		// maybe $TMPDIR is broken
        THROW_FILE_ERROR("fd145");					// can't create temp file!
    }
}


// Assignment operator
// *MOVE* fd in most cases: 		set q.fd = -1
// *COPY* stdin, stdout, stderr		q.fd untouched
//
void FD::operator= ( FD& q ) NTH
{
    XXASSERT(fd<=2);
    XXASSERT(this!=&q);

    delete[] fpath;
    fpath = newcopy(q.fpath);
    fd    = q.fd;

    if(q.fd>2) q.fd = -1;
}


// set fd and fpath
// try to avoid this
//
void FD::set_file_id(int fd, cstr fpath) NTH
{
    XXASSERT(this->fd==-1);

    this->fd = fd;
    delete[] this->fpath;
    this->fpath = newcopy(fpath);
}


// Destructor
// closes file and dispose fpath
// does not throw
// logs error if closing fails
//
FD::~FD() NTH
{
    if(close_file(0)) LogLine("file \"%s\" failed to close: %s", fpath, strerror(errno) );
    delete[] fpath;
}


//	Close file
//	fd is set to -1
//	does not dispose fpath
//	if file could not be closed properly:
//	thf=1: throws
//	thf=0: returns errno
//	note: should only fail if the fd is invalid
//	fd = -1, stdin, stdout and stderr are silently ignored
//
int FD::close_file( bool thf ) THF
{
    int f=fd; fd=-1;
    if(f<=2) return ok;
a:	if(close(f)==0) return ok;
    if(errno==EINTR) goto a;		// slow device only
    if(thf) THROW_FILE_ERROR("fd209");
    return errno;
}


// set/clear blocking io
//	=> errno EAGAIN wenn input empty/output full
//
int FD::set_blocking( bool f ) NTH
{
    int arg = fcntl(fd,F_GETFL,&arg);
    if(arg==-1) return -1;  // errno set
    if(f) arg &= ~O_NONBLOCK; else arg |= O_NONBLOCK;
    return fcntl(fd,F_SETFL,arg);
}

// set/clear async io mode ----------
//	=> signal SIGIO wenn input available/output possible
//	***???*** lt. dt. man page geht das mit fcntl() nicht!
//	***!!!*** lt. engl. man page geht es.
//
int FD::set_async( bool f ) NTH
{
    int arg = fcntl(fd,F_GETFL,&arg);
    if (arg==-1) return -1; // errno set
    if (f) arg |= O_ASYNC; else arg &= ~O_ASYNC;
    return fcntl(fd,F_SETFL,arg);
}



// --- QUERIES -------------------------------


// file modification time
time_t FD::file_mtime() const NTH
{
    struct stat fs;
    fstat(fd,&fs);
    return M_TIME(fs);
}

// last file access time
time_t FD::file_atime() const NTH
{
    struct stat fs;
    fstat(fd,&fs);
    return A_TIME(fs);
}

// last file status change time
time_t FD::file_ctime() const NTH
{
    struct stat fs;
    fstat(fd,&fs);
    return C_TIME(fs);
}

// get file size
// returns -1 on error
off_t FD::file_size() const NTH
{
    struct stat fs;
    if (fstat(fd,&fs)) return -1;			// error
    return fs.st_size;
}

// helper
inline
bool is_writable( mode_t mode, gid_t gid, uid_t uid) NTH
{
    // File mode
    // Read, write, execute/search by owner
    // #define	S_IRWXU		0000700		/* [XSI] RWX mask for owner */
    // #define	S_IRUSR		0000400		/* [XSI] R for owner */
    // #define	S_IWUSR		0000200		/* [XSI] W for owner */
    // #define	S_IXUSR		0000100		/* [XSI] X for owner */
    // Read, write, execute/search by group
    // #define	S_IRWXG		0000070		/* [XSI] RWX mask for group */
    // #define	S_IRGRP		0000040		/* [XSI] R for group */
    // #define	S_IWGRP		0000020		/* [XSI] W for group */
    // #define	S_IXGRP		0000010		/* [XSI] X for group */
    // Read, write, execute/search by others
    // #define	S_IRWXO		0000007		/* [XSI] RWX mask for other */
    // #define	S_IROTH		0000004		/* [XSI] R for other */
    // #define	S_IWOTH		0000002		/* [XSI] W for other */
    // #define	S_IXOTH		0000001		/* [XSI] X for other */

    return (mode & S_IWOTH) || ((gid==getegid())&&(mode & S_IWGRP)) || ((uid==geteuid())&&(mode & S_IWUSR));
}

// helper
inline
s_type classify_file(mode_t mode) NTH
{
    return s_type(mode>>12);
}

// test whether file is writeable for process
// based on unix file permissions
// probably very imperfect solution
// does not detect wprot file system etc.
// returns 'no' on error
//
bool FD::is_writable() const NTH
{
    struct stat data;
    if(fstat(fd, &data)) return no;		// error
    else return ::is_writable(data.st_mode,data.st_gid,data.st_uid);
}

// classify file
// returns a type id as defined in "s_type.h"
//
s_type FD::classify_file() const NTH
{
    struct stat data;
    if (fstat(fd, &data)) return s_none;
    else return ::classify_file(data.st_mode);
}

// set permissions for owner|group|others
// sets permissions for 'who' to 'perm' 							who.perm := perm
// DOES NOT set 'perm' for 'who' and reset 'perm' for all others! 	who.perm |= perm; others.perm &= ~perm
// returns errno
// who:
// • owner = 0700
// • group = 0070
// • other = 0007
// perm:
// • readable   = 0444
// • writable   = 0222
// • executable = 0111
//
int FD::set_permissions( mode_t who, mode_t perm ) NTH
{
    struct stat fs;
    int err = fstat(fd,&fs);
    if(err) return errno;

    perm = (fs.st_mode & ~who) | (perm & who);

    if(perm==fs.st_mode) return ok;
    return fchmod(fd,perm) ? errno : ok;
}

// set permissions for file
// returns errno
//
int FD::set_permissions( mode_t perm ) NTH
{
    return fchmod(fd,perm) ? errno : ok;
}


// --- READ AND WRITE FILE ------------------------------


//	Truncate or grow file to new file size 'size';
//
off_t FD::resize_file( off_t size ) THF
{
    while(ftruncate(fd,size)!=0)
    {
        if(errno==EINTR) continue;
        THROW_FILE_ERROR("fd374");
    }
    return size;	// for command chaining
}


off_t FD::seek_fpos( off_t fpos, int whence ) THF
{
    fpos = lseek(fd, fpos, whence);
    if(fpos==-1) THROW_FILE_ERROR("fd383");
    return fpos;
}

uint32 FD::write_fmt( cstr format, ... ) THF
{
    va_list va;
    va_start(va,format);

    int err = errno;
    va_list va2;								// duplicate va_list
    va_copy(va2,va);
    char bu[1];
    int n = vsnprintf( bu, 0, format, va2 );	// calc. req. size
    char s[n+1];
    vsnprintf( s, n+1, format, va );			// create formatted string
    errno = err;

    va_end(va);
    return write_bytes(s,n);
}

//	read line-break separated string from file or stream
//	line separators: 0, 4, 10, 12 or 13
//	file:    also handles \n\r or \r\n
//	stream:  slow!
//	returns: tempstr
//	returns: NULL on endoffile			2014-02-06
//	throws on error except eof
//
str FD::read_str() THF
{
    #define line_separators 0b0011010000010001
    str s;

    if(is_file())		// regular file
    {
        s = NULL;		// rval
        uint8 bu[100+1];
a:		uint32 n = read_bytes((ptr)bu,100,no), i;
        uint8 c = 0;

        if(n==0) return s;		// n==0  =>  endoffile: s may be NULL

        for(i=0;i<n;i++)		// search for eol
        {
            c = bu[i];
            if(c<=13 && (line_separators & (1<<c))) break;
        }
        bu[i] = 0;
        s = s ? catstr(s,(str)bu) : substr(bu,bu+i);
        if(i==n) goto a;			// no eol found

        // found eol at bu[i]:

        skip_bytes(int32(i+1-n));	// reposition file behind eol

        if(c==10||c==13)			// test for \n\r or \r\n
        {
            if(i+1<n)				// next char already in bu[] ?
            {
                if(c+bu[i+1]==23) skip_bytes(+1);		// skip if \n\r or \r\n
                return s;
            }
            else					// next char not in bu[] => need to read it
            {
                n = read_bytes((ptr)bu,1,no);			// read next char
                if(n && c+bu[0]!=23) skip_bytes(-1);	// undo if not \n\r or \r\n
                return s;
            }
        }

    }

    else // serial device
    {
        int n = 0;
        int sz = 100;				// initial max. size
        s  = tempstr(sz);

    // read character-by-character and break on linebreak
        for(;;)
        {
            uint8 c;
            ssize_t m = ::read(fd,&c,1);

            if(m>0)
            {
                if(c<=13 && (line_separators & (1<<c))) break;

                if(n==sz)			// s[] full => need more!
                {
                    str z = s;
                    sz += sz/2;
                    s = tempstr(sz);
                    memcpy(s,z,n);
                }

                s[n++] = c;
            }
            if(n==0) throw file_error(fpath,fd,endoffile,"fd483");	// cannot happen
            if(errno==EINTR) continue;								// read from slow device interrupted TODO: wait
            if(errno==EAGAIN) { usleep(5000); continue; }			// non-blocking dev only
            THROW_FILE_ERROR("fd924");
        }
        s[n] = 0;		// string delimiter
    }

    return s;		// tempstr
}


void FD::write_nstr( cstr s ) THF
{
    if(s)
    {
        uint32 len = (uint32)strlen(s);

        if(len>=253)
            if(len>>16)	{ write_uint8(255); write_uint32_z(len); }
            else		{ write_uint8(254); write_uint16_z(len); }
        else			{					write_uint8(len);    }

        write_data(s,len);
    }
    else write_uint8(253);			// 253 => NULL !
}

str FD::read_nstr() THF
{
    uint32 len = read_uint8();
    if(len>=253)
    {
        if(len==253) return NULL;
        else len = len==255 ? read_uint32_z() : read_uint16_z();
    }
    str s = tempstr(len);
    read_data(s,len);
    return s;
}

str FD::read_new_nstr() THF
{
    uint32 len = read_uint8();
    if(len>=253)
    {
        if(len==253) return NULL;
        else len = len==255 ? read_uint32_z() : read_uint16_z();
    }
    str s = new char[len+1]; s[len]=0;
    read_data(s,len);
    return s;
}





/*	read file into Array of str
    the strings in the array are in temp mem
*/
void FD::read_file(Array<str>& a, uint32 maxsize) THF
{
    off_t sz = file_remaining();
    if(sz>maxsize) throw file_error(fpath,fd,limiterror,"fd547");
    uint32 n = (uint32)sz;
    str s = tempstr(n);
    read_bytes(s,n);
    split(a,s,s+n);
}

/*	read file into StrArray
    the strings in the array are allocated with new()
*/
void FD::read_file(StrArray& a, uint32 maxsize) THF
{
    TempMemPool tmp;
    try
    {
        Array<str> z;
        read_file(z,maxsize);
        for(uint i=0;i<z.count();i++) a.append(z[i]);
    }
    catch(file_error& e)
    {
        e.text = xdupstr(e.text);
        throw e;
    }
}


/*	write StrArray to file
    the lines are separated with '\n'
    NULL strings are not written
*/
void FD::write_file(Array<str>& a) THF
{
    for(uint i=0;i<a.count();i++)
    {
        if(*a[i]==0) continue;
        write_bytes(a[i],(uint32)strlen(a[i]));
        write_uint8('\n');
    }
}


// ----------------------------------------
// 		read/write native byte order:
// ----------------------------------------

/*	read n bytes or up to eof or throw
    does not throw on eof
    may suspend thread while waiting for slow devices
    sets or clears errno:
    • noerror
    • endoffile
    throws on other error
    returns number of bytes actually read
    if noerror   returned value = requeste value
    if endoffile returned value < requested value
*/
uint32 FD::read_bytes( void* p, uint32 bytes, int ) THF
{
    uint32 m = bytes;
r:	uint32 n = (uint32)::read(fd,p,m);
    if(n==m) { errno=noerror; return bytes; }		// most common case
    if(n==0) { errno=endoffile; return bytes - m; }	// end of file
    if(n<m)  { p=(ptr)p+n; m-=n; goto r; }			// n>0 => some bytes read before interrupted
    if(errno==EINTR)  { goto r; }					// interrupted. slow device only
    if(errno==EAGAIN) { usleep(5000); goto r; }		// not ready. non-blocking dev only
    THROW_FILE_ERROR("fd516");						// anything else
}

/*	read n bytes or throw
    may suspend thread while waiting for slow devices
    returned value is always the requested value
    errno is always cleared
    note: this function could be easily merged with read_bytes(void*,uint32,int)
    but is supplied separately in case 2 arguments are faster than 3
    because this variant is used heavily for reading ints
*/
uint32 FD::read_bytes( void* p, uint32 bytes ) THF
{
    uint32 m = bytes;
r:	uint32 n = (uint32)::read(fd,p,m);
    if(n==m) { errno=noerror; return bytes; }		// most common case
    if(n==0) { errno=endoffile; goto x; }			// eof: throw
    if(n<m)  { p=(ptr)p+n; m-=n; goto r; }			// n<m => some bytes read before interrupted
    if(errno==EINTR)  { goto r; }					// interrupted. slow device only
    if(errno==EAGAIN) { usleep(5000); goto r; }		// not ready. non-blocking dev only
x:	THROW_FILE_ERROR("fd536");						// anything else
}

/*	test whether at least 1 byte is available for input
    does not throw
    return yes:	at least 1 byte available
    return no:  no data available
                errno = endoffile		files: end of file
                errno = EAGAIN			non-blocking devices: no data available yet
*/
bool FD::data_available() const NTH
{
    fd_set fdset;
    FD_ZERO(&fdset);
    FD_SET(fd,&fdset);
    struct timeval timeout = {0,0};

r:	int n = select(fd+1/*nfds*/, &fdset/*readfds*/, NULL/*writefds*/, NULL/*exceptfds*/, &timeout);
    if(n>=0) { errno=noerror; return n; }
    if(errno==EINTR) goto r;
    else return no;	// error
}


/*	write n bytes or throw
    may suspend thread while waiting for slow devices
    returned value is always the requested value
*/
uint32 FD::write_bytes( const void* p, uint32 bytes ) THF
{
    uint32 m = bytes;
w:	uint32 n = (uint32)::write(fd,p,m);
    if(n==m) { errno=noerror; return bytes; }
    if(n<m)	 { p=(ptr)p+n; m-=n; goto w; }			// n<m  <=>  n!=-1  => some bytes written
    if (errno==EINTR) { goto w; }					// slow device only
    if(errno==EAGAIN) { usleep(5000); goto w; }		// non-blocking dev only
    THROW_FILE_ERROR("fd551");
}


// ------------------------------------------------------
// read/write internet byte order (msb first, big endian)
// ------------------------------------------------------

int16 FD::read_int16_x() THF
{
    int8 bu[2];
    read_bytes(bu,2);
    return peek2X(bu);
}

uint32 FD::write_int16_x( int16 n ) THF
{
    int8 bu[2];
    poke2X(bu,n);
    return write_bytes(bu,2);
}

uint32 FD::read_uint24_x() THF
{
    int8 bu[4]={0,0,0,0};
    read_bytes(bu+1,3);
    return peek4X(bu);
}

int32 FD::read_int24_x() THF
{
    int8 bu[4]={0,0,0,0};
    read_bytes(bu+1,3);
    if(bu[1]<0) bu[0]=0xff;
    return peek4X(bu);
}

uint32 FD::write_int24_x( int32 n ) THF
{
    int8 bu[4];
    poke4X(bu,n);
    return write_bytes(bu+1,3);
}

int32 FD::read_int32_x() THF
{
    int8 bu[4];
    read_bytes(bu,4);
    return peek4X(bu);
}

uint32 FD::write_int32_x( int32 n ) THF
{
    int8 bu[4];
    poke4X(bu,n);
    return write_bytes(bu,4);
}

int64 FD::read_int64_x() THF
{
    int8 bu[8];
    read_bytes(bu,8);
    return peek8X(bu);
}

uint32 FD::write_int64_x( int64 n ) THF
{
    int8 bu[8];
    poke8X(bu,n);
    return write_bytes(bu,8);
}


// ------------------------------------------------------
// read/write intel byte order (lsb first, little endian)
// ------------------------------------------------------

int16 FD::read_int16_z() THF
{
    int8 bu[2];
    read_bytes(bu,2);
    return peek2Z(bu);
}

uint32 FD::write_int16_z( int16 n ) THF
{
    int8 bu[2];
    poke2Z(bu,n);
    return write_bytes(bu,2);
}


uint32 FD::read_uint24_z() THF
{
    int8 bu[4]={0,0,0,0};
    read_bytes(bu,3);
    return peek4Z(bu);
}

int32 FD::read_int24_z() THF
{
    int8 bu[4]={0,0,0,0};
    read_bytes(bu,3);
    if(bu[2]<0) bu[3]=0xff;
    return peek4Z(bu);
}

uint32 FD::write_int24_z( int32 n ) THF
{
    int8 bu[4];
    poke4Z(bu,n);
    return write_bytes(bu,3);
}

int32 FD::read_int32_z() THF
{
    int8 bu[4];
    read_bytes(bu,4);
    return peek4Z(bu);
}

uint32 FD::write_int32_z( int32 n ) THF
{
    int8 bu[4];
    poke4Z(bu,n);
    return write_bytes(bu,4);
}

int64 FD::read_int64_z() THF
{
    int8 bu[8];
    read_bytes(bu,8);
    return peek8Z(bu);
}

uint32 FD::write_int64_z( int64 n ) THF
{
    int8 bu[8];
    poke8Z(bu,n);
    return write_bytes(bu,8);
}


// ---------------------------------------------------
//						TTY
// ---------------------------------------------------

// ioctl -> tty control
// tcgetattr/tcsetattr -> tty control
// NOTE: uses ioctl() functions which MAY BE NON-PORTABLE!


// get terminal width and height
// returns errno
// sets dims to -1,-1 on error
// used in vipsi
//
int FD::get_terminal_size( int& rows, int& cols ) NTH
{
    struct winsize data;
    int r = ::ioctl(fd, TIOCGWINSZ, &data);
    if(r)
    {
        rows = cols = -1;
        return errno;
    }
    else
    {
        rows = data.ws_row;
        cols = data.ws_col;
        return errno=noerror;
    }
}

// get terminal width (character columns)
// sets|clears errno
// returns -1 on error
// used in vipsi
//
int FD::terminal_cols() NTH
{
    struct winsize data;
    if(ioctl(fd, TIOCGWINSZ, &data)) return -1;
    errno = noerror;
    return (int) (short) data.ws_col;
}

// get terminal height (character rows)
// sets|clears errno
// returns -1 on error
//
int FD::terminal_rows() NTH
{
    struct winsize data;
    if(ioctl(fd, TIOCGWINSZ, &data)) return -1;
    errno = noerror;
    return (int) (short) data.ws_row;
}

// get terminal width in pixels
// sets|clears errno
// returns -1 on error
//
int FD::terminal_width() NTH
{
    struct winsize data;
    if(ioctl(fd, TIOCGWINSZ, &data)) return -1;
    errno = noerror;
    return (int)(short)data.ws_xpixel;
}

// get terminal height in pixels
// sets|clears errno
// returns -1 on error
//
int FD::terminal_height() NTH
{
    struct winsize data;
    if(ioctl(fd, TIOCGWINSZ, &data)) return -1;
    errno = noerror;
    return (int)(short)data.ws_ypixel;
}

// note: does not work on OSX (as of ~ 2010)
// maybe Linux-only
// sets|clears errno
// returns errno
//
int FD::set_terminal_size( int rows, int cols ) NTH
{
    struct winsize data;
    if(ioctl(fd, TIOCGWINSZ, &data)) return errno;

    data.ws_xpixel = data.ws_xpixel/data.ws_col*cols;		// superfluxous
    data.ws_ypixel = data.ws_ypixel/data.ws_row*rows;		// superfluxous
    data.ws_row    = rows;
    data.ws_col    = cols;
    if(ioctl(fd, TIOCSWINSZ, &data)) return errno;

    return errno=noerror;
}






#if 0 && defined(_BIG_ENDIAN)

/*	read array of int16 ints in Intel/Z80 byte order
    note: for little endian cpus (intel/z80) this is straight forward
          and defined inline as read_data(fd,int16*,cnt)
*/
uint32 read_int16_data_z( int fd, int16* bu, uint32 items ) THF
{
    read_data( fd, bu, items );				// throw file_error
    for( uint32 i=0; i<items; i++ ) { bu[i] = peek2Z(bu+i); }
    return items;
}

/*	write array of int16 ints in Intel/Z80 byte order
    note: for little endian cpus (intel/z80) this is straight forward
          and defined inline as write_data(fd,int16*,cnt)
*/
uint32 write_int16_data_z( int fd, int16 const* bu, uint32 items ) THF
{
    if( items<=4*1024 )
    {
        int16 zbu[items];				// throw bad_alloc
        for( uint32 i=0; i<items; i++ ) { poke2Z( zbu+i, bu[i]); }
        write_data( fd, zbu, items );	// throw file_error
        return items;
    }

    write_int16_data_z( fd, bu,         items/2 );
    write_int16_data_z( fd, bu+items/2, items-items/2 );
    return items;
}
#endif


#if 0 && defined(_LITTLE_ENDIAN)

/*	read array of int16 ints in internet/ppc/68k byte order
    note: for big endian cpus (ppc/68k) this is straight forward
          and defined inline as read_data(fd,int16*,cnt)
*/
uint32 read_int16_data_x( int fd, int16* bu, uint32 items ) THF
{
    read_data( fd, bu, items );				// throw file_error
    for( uint32 i=0; i<items; i++ ) { bu[i] = peek2X(bu+i); }
    return items;
}

/*	write array of int16 ints in internet/ppc/68k byte order
    note: for big endian cpus (ppc/68k) this is straight forward
          and defined inline as write_data(fd,int16*,cnt)
*/
uint32 write_int16_data_x( int fd, int16 const* bu, uint32 items ) throw(file_error)
{
    if( items<=4*1024 )
    {
        int16 zbu[items];				// throw bad_alloc
        for( uint32 i=0; i<items; i++ ) { poke2X( zbu+i, bu[i]); }
        write_data( fd, zbu, items );	// throw file_error
        return items;
    }

    write_int16_data_x( fd, bu,         items/2 );
    write_int16_data_x( fd, bu+items/2, items-items/2 );
    return items;
}
#endif


// copy block at current file position from one file to another
// file positions increment accordingly
// may also copy inside a single file:
// overlapping blocks are copied non-destructively!
//
void copy( FD& q, FD& z, off_t count ) THF
{
    const int32 max_bu_size = 128*1024*1024;	// 128 MB

    // copy 1 block?
    if(count<=max_bu_size)
    {
        try
        {
            ADA<int8> bu((uint32)count);
            q.read_bytes (&bu[0],(uint32)count);
            z.write_bytes(&bu[0],(uint32)count);
            return;
        }
        catch(bad_alloc&){}
    }

    // more than one copy cycle required:
    // different handling for 2 files or single file required:

    if(&q!=&z)	// 2 files:
    {
        // copy block in 2 chunks:

a:		copy(q,z,count/2);
        copy(q,z,count-count/2);
        return;
    }

    // same file!

    off_t qpos = q.file_position();
    off_t zpos = z.file_position();
    if(zpos <= qpos || zpos >= qpos+count) goto a;	// no overlap or qpos>=zpos => copy upward

    // same file, blocks overlap and qpos<zpos
    // => copy 2nd block first!

    off_t n1 = count/2;			// calc block sizes
    off_t n2 = count - n1;

    q.seek_fpos(qpos+n1);		// copy 2nd block
    z.seek_fpos(zpos+n1);
    copy(q,z,n2);

    q.seek_fpos(qpos);			// copy 1st block
    z.seek_fpos(zpos);
    copy(q,z,n1);

    q.seek_fpos(qpos+count);	// adjust final file positions
    z.seek_fpos(zpos+count);
}



#if defined(_BIG_ENDIAN)

/*	read array of int16 ints in Intel/Z80 byte order
    note: for little endian cpus (intel/z80) this is straight forward
          and defined inline as read_data(fd,int16*,cnt)
*/
uint32 FD::read_int16_data_z( int16* bu, uint32 items ) THF
{
    read_data(bu, items);				// throw file_error
    for(uint32 i=0; i<items; i++) { bu[i] = peek2Z(bu+i); }
    return items;
}

/*	write array of int16 ints in Intel/Z80 byte order
    note: for little endian cpus (intel/z80) this is straight forward
          and defined inline as write_data(fd,int16*,cnt)
*/
uint32 FD::write_int16_data_z( int16 const* bu, uint32 items ) throw(file_error)
{
    if(items<=4*1024)
    {
        int16 zbu[items];				// throw bad_alloc
        for(uint32 i=0; i<items; i++) { poke2Z(zbu+i, bu[i]); }
        write_data(zbu, items);			// throw file_error
        return items;
    }

    write_int16_data_z(bu,         items/2);
    write_int16_data_z(bu+items/2, items-items/2);
    return items;
}
#endif


#if defined(_LITTLE_ENDIAN)

/*	read array of int16 ints in internet/ppc/68k byte order
    note: for big endian cpus (ppc/68k) this is straight forward
          and defined inline as read_data(fd,int16*,cnt)
*/
uint32 FD::read_int16_data_x( int16* bu, uint32 items ) THF
{
    read_data(bu, items);				// throw file_error
    for(uint32 i=0; i<items; i++) { bu[i] = peek2X(bu+i); }
    return items;
}

/*	write array of int16 ints in internet/ppc/68k byte order
    note: for big endian cpus (ppc/68k) this is straight forward
          and defined inline as write_data(fd,int16*,cnt)
*/
uint32 FD::write_int16_data_x( int16 const* bu, uint32 items ) throw(file_error)
{
    if(items<=4*1024)
    {
        int16 zbu[items];				// throw bad_alloc
        for(uint32 i=0; i<items; i++) { poke2X(zbu+i, bu[i]); }
        write_data(zbu, items);			// throw file_error
        return items;
    }

    write_int16_data_x(bu,         items/2);
    write_int16_data_x(bu+items/2, items-items/2);
    return items;
}
#endif















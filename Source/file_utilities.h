/*	Copyright  (c)	Günter Woigk 2001 - 2020
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

#ifndef file_utilities_h
#define	file_utilities_h

#include "kio/kio.h"
#include "unix/s_type.h"

#if !defined(_POSIX_SOURCE) && !defined(_SOLARIS)
	#define M_TIME(TS) ((TS).st_mtimespec.tv_sec)
	#define A_TIME(TS) ((TS).st_atimespec.tv_sec)
	#define C_TIME(TS) ((TS).st_ctimespec.tv_sec)
#else
	#define M_TIME(TS) ((TS).st_mtime)
	#define A_TIME(TS) ((TS).st_atime)
	#define C_TIME(TS) ((TS).st_ctime)
#endif

typedef struct std::nothrow_t nothrow_t;		// <new>:	std::nothrow_t

/*typedef enum
{
	s_none,
	s_file,
	s_dir,
	s_tty,
	s_pipe,
	s_sock,
	s_block,
	s_link,
	s_null,
	s_unkn,
	s_any		// used for selecting files by type
}
s_type;*/


extern	s_type	ClassifyFile	( mode_t mode );
extern	s_type	ClassifyFile	( int    fd   );
extern	s_type	ClassifyFile	( FILE*  file );
extern	s_type	ClassifyFile	( cstr path, bool follow_symlink=1 );
inline	bool	IsFile			( cstr path, bool follow_symlink=1 )	{ return ClassifyFile(path,follow_symlink)==s_file; }
inline	bool	IsDir			( cstr path, bool follow_symlink=1 )	{ return ClassifyFile(path,follow_symlink)==s_dir; }
inline	bool	IsLink			( cstr path )							{ return ClassifyFile(path,no)==s_link; }
inline	bool	IsFile			( FILE* file )							{ return ClassifyFile(file)==s_file; }
inline	bool	IsDir			( FILE* file )							{ return ClassifyFile(file)==s_dir; }
inline	bool	IsTTY			( FILE* file )							{ return ClassifyFile(file)==s_tty; }


extern	void	SetBlocking		( int fd, bool onoff );
extern	void	SetAsync		( int fd, bool onoff );
extern	void	SetBlocking		( FILE*,  bool onoff );
extern	void	SetAsync		( FILE*,  bool onoff );


extern	time_t	FileMTime		( cstr path, bool follow_symlink=1 );	// modification time
extern	time_t	FileATime		( cstr path, bool follow_symlink=1 );	// last access time
extern	time_t	FileCTime		( cstr path, bool follow_symlink=1 );	// last status change time



extern	off_t	FileLength		( cstr  path, bool follow_symlink=1 );	// returns -1 and sets errno on error
extern	off_t	FileLength		( int   fd   );							// returns -1 and sets errno on error
extern	off_t	FileLength		( FILE* file );							// returns -1 and sets errno on error


extern	cstr	FileNameFromPath		( cstr path );
extern	cstr	ExtensionFromPath		( cstr path );
inline	str		ExtensionFromPath		( str path )			{ return str(ExtensionFromPath(cstr(path))); }
extern	cstr	BasenameFromPath		( cstr path );
extern	cstr	DirectoryPathFromPath	( cstr path );

extern	cstr	FullPath		( cstr path, bool follow_symlink   );	// returns cstr in strPool
extern	cstr	TempFileName	( cstr file, bool follow_symlink=1 );
extern	int		NewTempFile		();										// create & open temp file in /tmp/
extern	off_t	DupFile			( int dest_fd, int src_fd, off_t max_size=-1 );
extern	off_t	filecopy		( int dest_fd, off_t dest, int src_fd, off_t src, off_t bytes );
extern	off_t	filecopy		( int dest_fd, int src_fd, off_t bytes );	// at current positions

extern	int		TerminalSize	( int tty, int& rows, int& cols );
extern	int		TerminalRows	( int tty );
extern	int		TerminalCols	( int tty );
extern	int		TerminalWidth	( int tty );		// pixel
extern	int		TerminalHeight	( int tty );		// pixel
extern	int		SetTerminalSize	( int tty, int rows, int cols );		// does not work !?



// ################################################################
//		unified file i/o  --  nothrow_t functions
//
//		the nothrow_t functions always clear or set errno
//		and return the amount of bytes actually written or read.
// ################################################################

// auto-closing fd:
class FD
{	int fd;
	FD()		: fd(-1) {}
	FD(int fd)	: fd(fd) {}
	~FD()		{ if(fd!=-1) close(fd); }
};

// struct nothrow_t { };				"errors.h"
// extern const nothrow_t nothrow;		"errors.h"
#define TPL template<class T>
#define SOT sizeof(T)
#define EXT extern
#define INL inline

EXT int		open_file	( nothrow_t, cstr path, int flags, int mode=0664 ) noexcept;
//EXT int		open_file_r	( nothrow_t, cstr path )						throw();	// must exist.
//EXT int		open_file_rw( nothrow_t, cstr path )						throw();	// must exist.
//EXT int		open_file_w	( nothrow_t, cstr path, int mode=0664 )			throw();	// truncates file !!
//EXT int		open_file_a	( nothrow_t, cstr path, int mode=0664 )			throw();	// append mode.
//EXT int		open_file_n	( nothrow_t, cstr path, int mode=0664 )			throw();	// must not exist.
//
//EXT uint32	read_bytes	( nothrow_t, int fd, ptr p, uint32 bytes )		throw();
//TPL	uint32	read_bytes	( nothrow_t, int fd, T*  p, uint32 bytes )		throw()	{ return read_bytes( nothrow,fd,(ptr)p,bytes ); }
//TPL uint32	read_data	( nothrow_t, int fd, T*  p, uint32 items )		throw()	{ return read_bytes( nothrow,fd,(ptr)p,items*SOT )/SOT;}
//
//EXT off_t	write_bytes	( nothrow_t, int fd, cptr p, uint32 bytes )		throw();
//TPL off_t	write_bytes	( nothrow_t, int fd, T const* p, uint32 bytes )	throw() { return write_bytes(nothrow,fd,(cptr)p,bytes);}
//TPL uint32	write_data	( nothrow_t, int fd, T const* p, uint32 items )	throw() { return write_bytes(nothrow,fd,(cptr)p,items*SOT)/SOT;}
//INL uint32	write_str	( nothrow_t, int fd, cstr p )					throw()	{ return p&&*p ? write_bytes(nothrow,fd,p,strlen(p)) : 0; }
//
//EXT	off_t		clip_file		( nothrow_t, int fd )					throw();
//EXT	int/*err*/	close_file		( nothrow_t, int fd )					throw();
//EXT	int/*err*/	clip_and_close	( nothrow_t, int fd )					throw();


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

EXT int		open_file			( cstr path, int flags, int mode=0664 )	noexcept(false);
EXT int		open_file_r			( cstr path )							noexcept(false);	// must exist.
EXT int		open_file_rw		( cstr path )							noexcept(false);	// must exist.
EXT int		open_file_w			( cstr path, int mode=0664 )			noexcept(false);	// truncates file!
EXT int		open_file_a			( cstr path, int mode=0664 )			noexcept(false);	// append mode.
EXT int		open_file_n			( cstr path, int mode=0664 )			noexcept(false);	// new: must not exist.

EXT uint32	read_bytes			( int fd, ptr p, uint32 bytes )			noexcept(false);
TPL uint32	read_bytes			( int fd, T*  p, uint32 bytes )			noexcept(false)	{ read_bytes(fd,ptr(p),bytes); return bytes; }
TPL uint32	read_data			( int fd, T*  p, uint32 items )			noexcept(false)	{ read_bytes(fd,ptr(p),items*SOT); return items; }
TPL uint32	read_data			( int fd, T*  p )						noexcept(false)	{ read_bytes(fd,ptr(p),SOT); return 1; }

EXT uint32	write_bytes			( int fd, cptr p, uint32 bytes )		noexcept(false);
TPL uint32	write_bytes			( int fd, T const* p, uint32 bytes )	noexcept(false)	{ write_bytes(fd,cptr(p),bytes); return bytes; }
TPL uint32	write_data			( int fd, T const* p, uint32 items )	noexcept(false)	{ write_bytes(fd,cptr(p),items*SOT); return items; }
TPL uint32	write_data			( int fd, T const* p )					noexcept(false)	{ write_bytes(fd,cptr(p),SOT); return 1; }
INL uint32	write_str			( int fd, cstr p )						noexcept(false)	{ return p&&*p ? write_bytes(fd,p,uint(strlen(p))) : 0; }

EXT	off_t	clip_file			( int fd )								noexcept(false);
EXT	void	close_file			( int fd )								noexcept(false);
EXT	void	clip_and_close		( int fd )								noexcept(false);

EXT off_t	seek_fpos			( int fd, off_t fpos, int f=SEEK_SET )	noexcept(false);
INL off_t	seek_endoffile		( int fd )								noexcept(false)	{ return seek_fpos(fd,0,SEEK_END); }
INL off_t	skip_bytes			( int fd, off_t signed_offset )			noexcept(false)	{ return seek_fpos(fd,signed_offset,SEEK_CUR); }
INL off_t	rewind_file			( int fd )								noexcept(false)	{ return seek_fpos(fd,0,SEEK_SET); }

INL off_t	file_position		( int fd )								noexcept(false)	{ return seek_fpos(fd,0,SEEK_CUR); }
EXT off_t	file_size			( int fd )								noexcept(false);
INL off_t	file_remaining		( int fd )								noexcept(false)	{ return file_size(fd) - file_position(fd); }
INL	bool	is_at_eof			( int fd )								noexcept(false)	{ return file_position(fd) >= file_size(fd); }
INL	bool	is_near_eof			( int fd, off_t proximity )				noexcept(false)	{ return file_remaining(fd) <= proximity; }

#if defined(_BIG_ENDIAN)
INL	uint32	read_short_data_x	( int fd, int16*p,		 uint32 items )	noexcept(false)	{ return read_data(fd,p,items); }
EXT	uint32	read_short_data_z	( int fd, int16*p,		 uint32 items )	noexcept(false);
INL	uint32	write_short_data_x	( int fd, int16 const*p, uint32 items )	noexcept(false)	{ return write_data(fd,p,items); }
EXT	uint32	write_short_data_z	( int fd, int16 const*p, uint32 items )	noexcept(false);
#elif defined(_LITTLE_ENDIAN)
EXT	uint32	read_short_data_x	( int fd, int16*p,		 uint32 items )	noexcept(false);
INL	uint32	read_short_data_z	( int fd, int16*p,		 uint32 items )	noexcept(false)	{ return read_data(fd,p,items); }
EXT	uint32	write_short_data_x	( int fd, int16 const*p, uint32 items )	noexcept(false);
INL	uint32	write_short_data_z	( int fd, int16 const*p, uint32 items )	noexcept(false)	{ return write_data(fd,p,items); }
#endif


EXT int16	read_short		( int fd )				noexcept(false);
EXT int16	read_short_x	( int fd )				noexcept(false);		// x: internet byte order, 68k, ppc
EXT int16	read_short_z	( int fd )				noexcept(false);		// z: i386, z80
INL uint16	read_ushort		( int fd )				noexcept(false)		{ return uint16(read_short(fd));   }
INL uint16	read_ushort_x	( int fd )				noexcept(false)		{ return uint16(read_short_x(fd)); }
INL uint16	read_ushort_z	( int fd )				noexcept(false)		{ return uint16(read_short_z(fd)); }

EXT void	write_short		( int fd, int16 )		noexcept(false);
EXT void	write_short_x	( int fd, int16 )		noexcept(false);
EXT void	write_short_z	( int fd, int16 )		noexcept(false);
INL void	write_ushort	( int fd, uint16 n )	noexcept(false)		{ write_short(fd,int16(n));   }
INL void	write_ushort_x	( int fd, uint16 n )	noexcept(false)		{ write_short_x(fd,int16(n)); }
INL void	write_ushort_z	( int fd, uint16 n )	noexcept(false)		{ write_short_z(fd,int16(n)); }

EXT int32	read_long		( int fd )				noexcept(false);
EXT int32	read_long_x		( int fd )				noexcept(false);
EXT int32	read_long_z		( int fd )				noexcept(false);
INL uint32	read_ulong		( int fd )				noexcept(false)		{ return uint32(read_long(fd));   }
INL uint32	read_ulong_x	( int fd )				noexcept(false)		{ return uint32(read_long_x(fd)); }
INL uint32	read_ulong_z	( int fd )				noexcept(false)		{ return uint32(read_long_z(fd)); }

EXT void	write_long		( int fd, int32 )		noexcept(false);
EXT void	write_long_x	( int fd, int32 )		noexcept(false);
EXT void	write_long_z	( int fd, int32 )		noexcept(false);
INL void	write_ulong		( int fd, uint32 n )	noexcept(false)		{ write_long(fd,int32(n));   }
INL void	write_ulong_x	( int fd, uint32 n )	noexcept(false)		{ write_long_x(fd,int32(n)); }
INL void	write_ulong_z	( int fd, uint32 n )	noexcept(false)		{ write_long_z(fd,int32(n)); }

EXT int8	read_char		( int fd )				noexcept(false);
INL uint8	read_uchar		( int fd )				noexcept(false)		{ return uint8(read_char(fd)); }
EXT void	write_char		( int fd, int8 )		noexcept(false);
INL void	write_uchar		( int fd, uint8 c )		noexcept(false)		{ write_char(fd,int8(c)); }

EXT int32	read_3byte_x	( int fd )				noexcept(false);
EXT int32	read_3byte_z	( int fd )				noexcept(false);
EXT uint32	read_u3byte_x	( int fd )				noexcept(false);
EXT uint32	read_u3byte_z	( int fd )				noexcept(false);

//EXT	void	write_nstr		( int fd, cstr s )		throw(file_error,limit_error);	// prefixes data with uint8 len
EXT str		read_nstr		( int fd )				noexcept(false);		// read into tempstr
EXT str		read_new_nstr	( int fd )				noexcept(false);		// read into newstr


#undef TPL
#undef SOT



#endif



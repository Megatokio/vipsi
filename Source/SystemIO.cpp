/*	Copyright  (c)	Günter Woigk   2002 - 2019
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

	2003-07... kio	changes to use UCS4String library
	2003-08-09 kio	ReadDirectory(): mounted devices for all (?) Unixes
*/

#define SAFE 3
#define LOG 1
#include "kio/kio.h"
#include <sys/param.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <dirent.h>
#if defined(_LINUX)
#include <sys/vfs.h>
#include "unix/os_utilities.h"
#endif
#include "SystemIO.h"
#include "Var/Var.h"



INIT_MSG

// serrors.cpp:
inline	String	ErrorString	( int/*OSErr*/ e )				{ return errorstr(e); }
extern	String	ErrorString	( );
extern	void	ForceError	( int/*OSErr*/e, cString& msg );
inline	void	SetError	( int/*OSErr*/e, cString& msg )	{ if(errno==ok) ForceError(e,msg); }
inline	void	ForceError	( cString& msg )				{ ForceError(-1,msg); }
inline	void	SetError	( cString& msg )				{ if(errno==ok) ForceError(-1,msg); }
extern	void	AppendToError	( cString& msg );
extern	void	PrependToError	( cString& msg );


const String failed(" failed: ");



/* ==== files&folders ==========================================
*/

/*
static str ModeStr ( mode_t mode )
{
	char r[]="-r";
	char w[]="-w";
	char x[]="-x";
	char s[]="rwxrwxrwx";
	s[0] = r[(mode&S_IRUSR)!=0];
	s[1] = w[(mode&S_IWUSR)!=0];
	s[2] = x[(mode&S_IXUSR)!=0];
	s[3] = r[(mode&S_IRGRP)!=0];
	s[4] = w[(mode&S_IWGRP)!=0];
	s[5] = x[(mode&S_IXGRP)!=0];
	s[6] = r[(mode&S_IROTH)!=0];
	s[7] = w[(mode&S_IWOTH)!=0];
	s[8] = x[(mode&S_IXOTH)!=0];
	cstr z = mode&S_ISUID ? "u" : mode&S_ISGID ? "g" : "-";
	return CatStr(z,s);
}
*/

/*
static str EffModeStr ( mode_t mode, gid_t file_grp, uid_t file_usr )
{
	gid_t g = getegid();
	uid_t u = geteuid();
	cstr  z = mode&S_ISUID ? "u" : mode&S_ISGID ? "g" : "-";

// mix bits to 'other' bits:
	if (g==file_grp) mode |= (mode>>3);
	if (u==file_usr) mode |= (mode>>6);
	if (u==0)        mode |= (mode>>6);		// prüfen: ist root==0?

	char r[]="-r";
	char w[]="-w";
	char x[]="-x";
	char s[]="rwx";
	s[0] = r[(mode&S_IROTH)!=0];
	s[1] = w[(mode&S_IWOTH)!=0];
	s[2] = x[(mode&S_IXOTH)!=0];
	return CatStr(z,s);
}
*/

/*
uint32 VolumeFlags ( cstr path )
{
	bool wprot   = 1;	// 1
	bool ejectbl = 0;	// 2
	bool mounted = 0;	// 4

#if !defined(_SOLARIS)
	struct statfs fs;
	int n = statfs(FullPath(path,1),&fs);
	if (n) return 1+0+0;	// errno set
#endif

#if defined(_LINUX) || defined(_SOLARIS)
	wprot  =0;
	ejectbl=0;
	mounted=1;
	xlog("\n*** SystemIO.cp:\n*** VolumeFlags(): not yet implemented for your platform.\n");
#else
	int16 f = fs.f_flags;
	wprot   = f&MNT_RDONLY;
    ejectbl = (f&MNT_NOSUID) && (f&MNT_NODEV);
	mounted = fs.f_blocks>0;
#endif

	return (wprot<<0)+(ejectbl<<1)+(mounted<<2);
}
*/

/*
double VolumeFree ( cstr path )
{
#if defined(_SOLARIS)
	xlog("\n*** SystemIO.cp:\n*** VolumeFree(): not yet implemented for your platform.\n");
	return 0.0;
#else
	struct statfs fs;
	int n = statfs(FullPath(path,1),&fs);
	if (n) return 0;		// errno set
	return (double)fs.f_bsize * (double)fs.f_bavail;
#endif
}
*/


/* ----	get absolute, deref'ed path -------------------------------
		resolve things like partial path, ~, ./ and ../
	#ifdef MATCH_INVALID_UTF8_PATHS
		resolve broken, non-utf-8 path components  ((option))
		these cannot be expressed as String !!
	#endif
*/
cstr FullPath ( cString& path, bool follow_symlink )
{
	return errno ? "" : FullPath(CString(path),follow_symlink);
}



/* ----	working directory --------------------------------------
*/
void ChangeWorkingDir ( cString& path )
{
	cstr cpath = FullPath(path,yes);
	if(errno) return;

	chdir ( cpath );
	if(errno) PrependToError( String("chdir ")+ToQuoted(path)+failed );
}

String WorkingDirString ( )
{
	for ( uint32 n=0x80; n<0x10000; n*=2 )
	{
		str s = getcwd ( tempstr(n), n );
		if (s) return s;
	}
	//SetError(erange); <- set by getcwd()
	return emptyString;
}



/* ----	check file type -----------------------------------------
		no error if named file does not exist
		error only if path portion is bogus.
*/
s_type ClassifyFile ( cString& path, bool follow_symlink )
{
	cstr cpath = FullPath(path,follow_symlink);
	if (errno) return s_none;
	s_type r = ClassifyFile(cpath,no);
	errno=ok;		// if s_none
	return r;
}



/* ----	create regular file ---------------------------------------
		in: mode = "new", "write" or "append"
			does nothing if errno is already set
*/
void CreateFile ( cString& path, cString& data, cstr mode )
{
	if (errno) return;

	if( data.Csz()!=csz1 )
	{
		if( XLOG && data.ReqCsz()!=csz1 ) logline("warning: CreateFile() truncated non-8bit data");
		String s(data);
		s.ResizeCsz(csz1);
		CreateFile( path, s, mode );
		return;
	}

	assert(mode && (*mode=='n'||*mode=='a'||*mode=='w'));

	cstr cpath = FullPath(path,yes);
	if (errno) goto x;	// note: FullPath() returns no error if only last path component does not exist!
	{	s_type r = ClassifyFile(cpath,no);
		if (r==s_none) errno=ok;
		else if (*mode=='n'||r!=s_file) { errno = EEXIST; goto x; }

		const int W_FLAGS = O_WRONLY|O_CREAT/*|O_NONBLOCK*/;
		int flags = *mode=='n' ? W_FLAGS|O_EXCL : *mode=='a' ? W_FLAGS|O_APPEND : W_FLAGS|O_TRUNC;
		int fd = open(cpath,flags,0666);
		if (fd>=0)
		{
			write(fd,data.Text(),data.Len());
			close(fd);
		}
	}

	if (errno) { x: PrependToError( String("create ")+ToQuoted(path)+failed ); }
}


/* ----	create directory -------------------------------------
		in:	mode = "new" or "write"
			does nothing if errno already set
*/
void CreateDir ( cString& path, cstr mode )
{
	if (errno) return;

	assert(mode && (*mode=='n' || *mode=='w'));

	cstr cpath = FullPath(path,yes);
	if (errno) goto x;	// note: FullPath() returns no error if only last path component does not exist!

	{	s_type r = ClassifyFile(cpath,no);
		if (r==s_none) errno=ok;
		else if (*mode=='n'||r!=s_dir) { errno = EEXIST; goto x; }

		mode_t z = umask(0);
		mkdir ( cpath, 0777 );
		umask(z);
		if (errno==EEXIST) errno=ok;
	}

	if (errno) { x: PrependToError( String("mkdir ")+ToQuoted(path)+failed ); }
}


/* ---- create symbolic link ------------------------------------------
		create a symbolic link to 'dest' at position 'path'
		in:	mode = "new" or "write"
			does nothing if errno already set
*/
void CreateLink( cString& path, cString& dest, cstr mode )
{
	if (errno) return;

	assert(mode && (*mode=='n' || *mode=='w'));

	cstr cpath = FullPath(path,no);
	if (errno) goto x;	// note: FullPath() returns no error if only last path component does not exist!

	{
		s_type r = ClassifyFile(cpath,no);

		if (r==s_none)	// <path> is free
		{
			errno=ok;
		}
		else			// <path> exists
		{
			if (*mode=='n'||r!=s_link)
			{
				errno = EEXIST; goto x;
			}
			else
			{	// symlink() can't replace an existing link => remove old link
				if ( unlink(cpath) ) goto x;
			}
		}

		cstr cdest = CString(dest);		// contents of the symbolic link
		symlink(cdest,cpath);
	}

	if (errno) { x: PrependToError( String("mklink ")+ToQuoted(path)+failed ); }
}



/* ----	read binary file into string variable -------------------
		in: does nothing if errno already set
			except clearing the result string
*/
void ReadFile ( cString& path, String& data )
{
	if (errno) return;

	data.Clear();
	cstr cpath = FullPath(path,yes);
	if(errno) goto x;

	{
		int fd = open( cpath,O_RDONLY );
		assert( (errno==ok) == (fd>=0) );

		if(errno==ok)
		{
			off_t len = file_size(fd);
			if( len>1 GB /* || ( sizeof(off_t)!=sizeof(size_t) && size_t(len)!=len ) */ )
				SetError("file too long to load into ram");

			if(errno==ok)
			{
				data = String(len,csz1);
				off_t n = 0;
				while ( errno==ok && n<len )
				{
					ssize_t r = read ( fd, data.Text()+n, len-n );
					if (r==-1)
					{
						if (errno==EINTR || errno==EAGAIN) errno=ok;
					}
					else
					{
						n += (size_t)r;
					}
				}
				if (n<len) data.Resize(n);
			}

			close(fd);
		}
	}

	if(errno) { x: PrependToError( String("read file ")+ToQuoted(path)+failed ); }
}


/* ----	get link target ----------------------------------------
		in: does nothing if errno already set
			except clearing the result string
*/
void ReadLink ( cString& path, String& data )
{
	if (errno) return;

	cstr cpath = FullPath(path,no);

	if(errno==ok)
	{
		if (IsLink(cpath))
		{
			char bu[MAXPATHLEN];
			int n = readlink( cpath, bu, MAXPATHLEN );
			if (n>=0) data = String(cptr(bu),n);
		}
		else
		{
			errno = EINVAL;
		}
	}

	if(errno)
	{
	 	data.Clear();
		PrependToError( String("read link ")+ToQuoted(path)+failed );
	}
}


#if !defined(_POSIX_SOURCE) && !defined(_SOLARIS)
	#define M_TIME(TS) ((TS).st_mtimespec.tv_sec)
	#define A_TIME(TS) ((TS).st_atimespec.tv_sec)
	#define C_TIME(TS) ((TS).st_ctimespec.tv_sec)
#else
	#define M_TIME(TS) ((TS).st_mtime)
	#define A_TIME(TS) ((TS).st_atime)
	#define C_TIME(TS) ((TS).st_ctime)
#endif


static str effmodestr( mode_t mode, gid_t file_grp, uid_t file_usr )
{
	gid_t g = getegid();
	uid_t u = geteuid();

// mix bits to 'other' bits:
	if (g==file_grp) mode |= (mode>>3);
	if (u==file_usr) mode |= (mode>>6);
	if (u==0)        mode |= (mode>>6);		// prüfen: ist root==0?

	str s = binstr(mode," ---"," rwx");
	s[0] = mode&S_ISUID ? 'u' : mode&S_ISGID ? 'g' : '-';
	return s;
}


/* ----	read directory into vektor variable -------------------
		in:	does nothing, if errno is already set
   			except: v is always made a vektor
*/
void ReadDir ( cString& path, Var& v )
{
	assert( &v );
	v.SetType(isList);
	if (errno) return;

	Var tmpl(isList);
	tmpl.ResizeList(6);
	tmpl[0].SetName("fname");		// "name" is a token...
	tmpl[1].SetName("flags");
	tmpl[2].SetName("length");
	tmpl[3].SetName("mtime");		// last modification
	tmpl[4].SetName("atime");		// last access
	tmpl[5].SetName("ctime");		// last status change

// 	=>	file or directory
//	=>	v.name     = "file_or_subdirectory_name"
//		v.flags    = "drwx" ... "----"
//		v.length   = file length or files in sub directory
//		v.atime	   = last access t_time
//		v.mtime	   = last modification t_time
//		v.ctime	   = last modification t_time to meta data

	cstr dirpath = FullPath(path,yes);
	if(errno) return;

	String patternstring;
	if (lastchar(dirpath)!='/')
	{
		assert(!IsDir(dirpath));							// note: sets sporadic errors
		patternstring = FileNameFromPath(dirpath);
		dirpath = DirectoryPathFromPath(dirpath);
	}
	DIR* dir = opendir(dirpath);  						// note: sets sporadic errors
	if (!dir) return; 				// error
	errno=ok;										// clear sporadic errors

	struct dirent* direntry;
	dirpath = newcopy(dirpath);		// make durable

	while ( (direntry=readdir(dir)) && errno==ok )
	{
		cstr filename = direntry->d_name;
		if ( filename[0]=='.' && ( filename[1]==0 || (filename[1]=='.' && filename[2]==0) ) )
		{	// skip "." and ".."
			// note: doing this stupid testing here,
			//		 because obviously they are not always direntry[0] and direntry[1]
			continue;
		}
		String filenamestring = String(filename,ISO_LATIN_1);	// => also works with broken filenames
		if (patternstring.Len()&&!FNMatch(filenamestring,patternstring,fnm_basic)) continue;
		cstr filepath = catstr(dirpath,filename);
		struct stat filestat;
		if( lstat(filepath,&filestat) ) break;			// don't deref links; break on error

		Var* w = new Var(tmpl); if (!w) { errno = outofmemory; break; }
		v.AppendItem(w);

		bool is_link = S_ISLNK (filestat.st_mode);
		if( is_link && stat(filepath,&filestat) ) 	// deref links
		{	// error: dead link
			errno=ok;
			(*w)[0] = filenamestring;
			(*w)[1] = "l---";
			(*w)[2] = 0.0;							// file length / items in dir
			(*w)[3] = 0.0;							// M_TIME(filestat);
			(*w)[4] = 0.0;							// A_TIME(filestat);
			(*w)[5] = 0.0;							// C_TIME(filestat);
			continue;
		}

		// not a link or link deref'ed ok:

		bool is_dir = S_ISDIR(filestat.st_mode);
		int32 n = filestat.st_size;
		if(is_dir)
		{
			n = 0;
			assert(filenamestring.LastChar()!=ucs1char('/'));
			filenamestring+='/';
			DIR* D = opendir(filepath);
			if (D)
			{ 	for ( n=0-2; readdir(D); n++ ) {}	// count items in subdir
				closedir(D);
			}
			errno=ok;
		}

		char* flags = effmodestr(filestat.st_mode,filestat.st_gid,filestat.st_uid);
		if(is_dir)  flags[0]='d';
		if(is_link) flags[0]='l';

		(*w)[0] = filenamestring;
		(*w)[1] = flags;
		(*w)[2] = n;					// file length or items in dir
		(*w)[3] = M_TIME(filestat);
		(*w)[4] = A_TIME(filestat);
		(*w)[5] = C_TIME(filestat);
	}

	delete[] dirpath;

	closedir(dir);
}


void ReadNode ( cString& path, Var& v, bool follow_symlink )
{
	cstr cpath = FullPath(path,follow_symlink);
	if (errno) return;

	s_type r = ClassifyFile(cpath,no);
	switch(r)
	{
	case s_file:	v.SetString(); ReadFile(path,v.Text()); break;
	case s_dir:		ReadDir (path,v); break;
	case s_link:	v.SetString(); ReadLink(path,v.Text()); break;
	default:		errno = unsuitedfiletype;
					PrependToError( String("read ")+ToQuoted(path)+failed );
					break;
	}
}



/* ----	rename file or folder -----------------------------------------
		oldname must be of type ntyp; except if ntyp==s_any
		newname must not exist; or -if overwrite allowed- must be of same type as oldname
*/
void RenameNode ( cString& oldname, cString& newname, bool overwrite, bool follow_symlink, s_type ntyp )
{	s_type otyp;
	cstr nstr,ostr;

	ostr = FullPath(oldname,follow_symlink); if(errno) goto x;
	otyp = ClassifyFile(ostr,no);
	if (otyp==s_none) { goto x; }
	if (ntyp!=s_any && otyp!=ntyp) { errno = wrongfiletype; goto x; }

	nstr = FullPath(newname,follow_symlink); if(errno) goto x;
	ntyp = ClassifyFile(nstr,no);
	if (ntyp==s_none) errno=ok;
	else if (!overwrite || ntyp!=otyp) { errno = EEXIST; goto x; }

	rename(ostr,nstr);		// renames files, dirs and symlinks!

	if(errno) { x: PrependToError( String("rename ")+ToQuoted(oldname)+" to "+ToQuoted(newname)+failed ); }
}



/* ----	delete file or folder -----------------------------------------
		in: does nothing if errno already set
*/
void DeleteNode ( cString& path, bool follow_symlink, s_type typ )
{	s_type t;

	cstr cpath = FullPath(path,follow_symlink);
	if(errno) return;				// note: FullPath() includes path in error msg

	t = ClassifyFile(cpath,no);
	if (t==s_none) { goto x; }
	if (typ!=s_any && t!=typ) { errno = wrongfiletype; goto x; }

//	remove(cpath);		// removes files, dirs and links
						// sets error for dirs even on success! (Linux FC1)
	if (t==s_dir) rmdir(cpath); else unlink(cpath);

	if(errno) { x: PrependToError( String("remove ")+ToQuoted(path)+failed ); }
}



/* ----	swap two files ------------------------------------------
*/
void SwapFiles ( cString& path1, cString& path2 )
{
	cstr v = FullPath(path1,1);
	cstr w = FullPath(path2,1);
	if(errno) return;

	cstr z = TempFileName(w); assert(errno==ok);	// must work
	rename(v,z);	if(errno) { PrependToError( String("rename (swap) ")+ToQuoted(path1)+failed ); return; }
	rename(w,v);	if(errno) { PrependToError( String("rename (swap) ")+ToQuoted(path2)+failed ); return; }
	rename(z,w);    assert(errno==ok);				// must work
}




/* ----	read volume info into vektor variable -------------------
		v is made a vektor even if load failed
		v is shrinked to fit
*/
void GetVolumeInfo ( Var& v )
{
	assert( &v );
	v.SetType(isList);
	if (errno) return;

	Var tmpl(isList);
	tmpl.ResizeList(22);
	tmpl[0].SetName("fname");			// mount point
	tmpl[1].SetName("flags");			// drwx
	tmpl[2].SetName("length");			// items in root dir
	tmpl[3].SetName("mtime");			// last modification
	tmpl[4].SetName("atime");			// last access
	tmpl[5].SetName("ctime");			// last status change
	tmpl[6].SetName("fs_type_id");		// number: statfs.f_type			((magic numbers for misc. fs types))
	tmpl[7].SetName("mountflags");		// number: BSD: statfs.f_flags; LINUX: /bin/mount
	tmpl[8].SetName("blocksize");		// number: statfs.f_bsize			((block size; best tfu))
	tmpl[9].SetName("iosize");			// number: BSD: statfs.f_iosize		((best tfu))
	tmpl[10].SetName("blocks");			// number: statfs.f_blocks			((blocks total))
	tmpl[11].SetName("blocksfree");		// number: statfs.f_bavail			((blocks free user))
	tmpl[12].SetName("blocksfree_su");	// number: statfs.f_bfree			((blocks free su))
	tmpl[13].SetName("files");			// number: statfs.f_files			((inodes total))
	tmpl[14].SetName("filesfree");		// number: statfs.f_ffree			((inodes free))
	tmpl[15].SetName("owner");			// ?       BSD: statfs.f_owner
	tmpl[16].SetName("fstype");			// text:   BSD: statfs.f_fstypename; LINUX: /bin/mount
	tmpl[17].SetName("mountpoint");		// text:   BSD: statfs.f_mntonname;  LINUX: /bin/mount
	tmpl[18].SetName("device");			// text:   BSD: statfs.f_mntfromname;LINUX: /bin/mount
	tmpl[19].SetName("bytes");			// number: blocksize * blocks
	tmpl[20].SetName("bytesfree");		// number: blocksize * blocksfree
	tmpl[21].SetName("bytesfree_su");	// number: blocksize * blocksfree_su

#if defined(_BSD)
	struct statfs* info;
	int n = getmntinfo(&info, 1?MNT_WAIT:MNT_NOWAIT);
#else
	str bu = ExecCmd("/bin/mount",nullptr);		TRAP(!bu);
	Var mi = Var(String(bu)); delete[]bu;		// assumes UTF-8
	mi.SplitLines();							// split into lines[]
	mi.DeleteLastItem();						// delete last empty line
	mi.SplitLines(' ');							// split into fields[][]
	// mi[i] = { device, "on", mountpoint, "type", fs_type, (flags) }
	int n = mi.ListSize();
#endif

	for (int i=0;i<n;i++)
	{
	#if defined(_BSD)
		struct statfs& fs = info[i];						// file system info
		String mpath = fs.f_mntonname;						// utf-8
		String mdev  = fs.f_mntfromname;					// utf-8
		if ( fs.f_files+fs.f_ffree==0 ) continue;			// special: volfs, free automount slots
		if ( mpath=="/dev" ) continue;						// devfs, fdesc
	#else
		String mpath = mi[i][2].Text();
		String mdev  = mi[i][0].Text();
		#if !defined(_SOLARIS)
			struct statfs fs; if(statfs(mpath.CString(),&fs)) return;	// file system info
		#endif
	#endif

		if ( mdev.Find('/')==-1 ) continue;					// "none", "usbdevfs", etc.

		struct stat d;										// mount dir info
		if (stat(mpath.CString(), &d)) return;				// error
		assert(S_ISDIR(d.st_mode));

		DIR* D = opendir(mpath.CString()); if (!D) return;	// error
		int len; for(len=-2;readdir(D);len++) {}			// used inodes: count files&dirs
		closedir(D);

		char flags[] = "drwx";
	#if defined(_BSD)
		int16 f = fs.f_flags;
		if (f&MNT_RDONLY) flags[2]='-';
	#else
		// stat.st_mode		***TODO***  check for read-only device
	#endif

		Var* w = new Var(tmpl); if (!w) { errno = outofmemory; return; }
		v.AppendItem(w);
		(*w)[0] = mpath;
		(*w)[1] = flags;
		(*w)[2] = len;						// files&dirs in mount dir

		(*w)[3] = M_TIME(d);
		(*w)[4] = A_TIME(d);
		(*w)[5] = C_TIME(d);

	#if defined(_SOLARIS)
		(*w)[6] = "unknown (not yet implemented for Solaris)";		// ***TODO***
	#else
		(*w)[6] = fs.f_type;
	#endif

	#if defined(_BSD)
		(*w)[7] = fs.f_flags;
	#else
		(*w)[7] = "((not implemented))";
		//	***TODO*** ToNumber( mi[i][5] )
	#endif

	#if defined(_SOLARIS)
		(*w)[8] = "unknown (not yet implemented for Solaris)";		// ***TODO***
	#else
		(*w)[8] = fs.f_bsize;
	#endif

	#if defined(_BSD)
		(*w)[9] = fs.f_iosize;
	#else
		(*w)[9] = (*w)[8];
	#endif

	#if defined(_SOLARIS)
		(*w)[10] =		// ***TODO***
		(*w)[11] =		// ***TODO***
		(*w)[12] =		// ***TODO***
		(*w)[13] =		// ***TODO***
		(*w)[14] = "unknown (not yet implemented for Solaris)";		// ***TODO***
	#else
		(*w)[10] = fs.f_blocks;
		(*w)[11] = fs.f_bavail;
		(*w)[12] = fs.f_bfree;
		(*w)[13] = fs.f_files;
		(*w)[14] = fs.f_ffree;
	#endif

	#if defined(_BSD)
		(*w)[15] = fs.f_owner;
	#else
		(*w)[15] = "((unknown))";		// ***TODO*** /bin/ls
	#endif

	#if defined(_BSD)
		(*w)[16] = fs.f_fstypename;
	#else
		(*w)[16] = mi[i][4];
	#endif

		(*w)[17] = mpath;
		(*w)[18] = mdev;
	#if defined(_SOLARIS)
		(*w)[19] = 0;		// ***TODO***
		(*w)[20] = 0;		// ***TODO***
		(*w)[21] = 0;		// ***TODO***
	#else
		(*w)[19] = (double)fs.f_bsize * fs.f_blocks;
		(*w)[20] = (double)fs.f_bsize * fs.f_bavail;
		(*w)[21] = (double)fs.f_bsize * fs.f_bfree;
	#endif
	}
}







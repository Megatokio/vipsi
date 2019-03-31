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
*/

#ifndef systemio_h
#define	systemio_h

#include "kio/kio.h"
#include "VString/String.h"
#include "unix/os_utilities.h"
#include "Stream.h"



extern uint 	VolumeFlags		( cstr path );
extern double	VolumeFree		( cstr path );
extern void		GetVolumeInfo	( Var& v );

extern cstr		FullPath		( cString& path, bool follow_symlink );

extern void		ChangeWorkingDir( cString& path );
extern String	WorkingDirString( );

extern s_type	ClassifyFile	( cString& path, bool follow_symlink=1 );
inline bool		NodeExists		( cString& path, bool follow_symlink=1 )	{ return ClassifyFile(path,follow_symlink)!=s_none; }
inline bool		IsFile			( cString& path, bool follow_symlink=1 )	{ return ClassifyFile(path,follow_symlink)==s_file; }
inline bool		IsDir			( cString& path, bool follow_symlink=1 )	{ return ClassifyFile(path,follow_symlink)==s_dir;  }
inline bool		IsLink			( cString& path )							{ return ClassifyFile(path,no            )==s_link; }

extern void		CreateFile		( cString& path, cString& data, cstr mode /*n/w/a*/ );
inline void		NewFile			( cString& path, cString& data )			{ CreateFile(path,data,"new"); }
inline void		WriteFile		( cString& path, cString& data )			{ CreateFile(path,data,"write"); }
inline void		AppendFile		( cString& path, cString& data )			{ CreateFile(path,data,"append"); }

extern void		CreateDir		( cString& path, cstr mode /*n/w*/ );
inline void		NewDir			( cString& path )							{ CreateDir(path,"new"); }
inline void		WriteDir		( cString& path )							{ CreateDir(path,"write"); }

extern void		CreateLink		( cString& path, cString& dest, cstr mode /*n/w*/ );
inline void		NewLink			( cString& path, cString& dest )			{ CreateLink(path,dest,"new"); }
inline void		WriteLink		( cString& path, cString& dest )			{ CreateLink(path,dest,"write"); }

extern void		ReadNode		( cString& path, Var& v, bool follow_symlink );
extern void		ReadFile		( cString& path, String& data );
extern void		ReadDir			( cString& path, Var& v );
extern void		ReadLink		( cString& path, String& data );

extern void		RenameNode		( cString& oldname, cString& newname, bool overwrite, bool follow_symlink, s_type typ=s_any );
inline void		RenameFile		( cString& oldname, cString& newname, bool overwrite=no );
inline void		RenameDir		( cString& oldname, cString& newname, bool overwrite=no );
inline void		RenameLink		( cString& oldname, cString& newname, bool overwrite=no );

extern void		DeleteNode		( cString& path, bool follow_symlink, s_type typ=s_any );	// file, dir, link, pipe, sock, etc.
inline void		DeleteFile		( cString& path );
inline void		DeleteDir		( cString& path );
inline void		DeleteLink		( cString& path );

extern void		SwapFiles		( cString& path, cString& path2 );




inline void RenameFile	( cString& oldname, cString& newname, bool overwrite )	{ RenameNode( oldname, newname, overwrite, yes, s_file ); }
inline void RenameDir	( cString& oldname, cString& newname, bool overwrite )	{ RenameNode( oldname, newname, overwrite, yes, s_dir );  }
inline void RenameLink	( cString& oldname, cString& newname, bool overwrite )	{ RenameNode( oldname, newname, overwrite, no, s_link );  }

inline void DeleteFile	( cString& path )	{ DeleteNode( path, yes, s_file ); }
inline void DeleteDir	( cString& path )	{ DeleteNode( path, yes, s_dir );  }
inline void DeleteLink	( cString& path )	{ DeleteNode( path, no, s_link );  }


#endif







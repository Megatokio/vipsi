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

//	***!*** man console_codes


#ifndef Stream_h
#define Stream_h

#include "config.h"
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include "file_utilities.h"
#include "VString/String.h"
#include "Irpt.h"
#include "Thread.h"
//#include "unix/s_type.h"
//#include "unix/fd_throw.h"


/*	------------------------------------------
	color enumerations for tty attributes
	used in VT_FGColor() et.al.
*/
typedef int VT_Color;
enum
{		VT_normal = 0,
		VT_black,	VT_red,			VT_green,	VT_yellow,
		VT_blue,	VT_magenta,		VT_cyan,	VT_white,
		VT_dark=0,	VT_bright=16,
		VT_bold=32,	VT_inverse=64,	VT_underline=128
};


/*	------------------------------------------
	vt100 escape sequences
*/
typedef char escseq[8];
extern const escseq vt100_clearattr;
extern const escseq vt100_clearfgcolor;
extern const escseq vt100_clearbgcolor;
extern const escseq vt100_bold[2];						// off/on
extern const escseq vt100_inverse[2];					// ""
extern const escseq vt100_underline[2];					// ""
extern cstr			vt100_setfgcolor	( VT_Color );					// internal use only!
extern cstr			vt100_setbgcolor	( VT_Color );					// internal use only!
extern cstr			vt100_setattr		( VT_Color fg, VT_Color bg );	// internal use only!

#define	vt100_request_position			"\033[6n"						// --> Terminal responds
#define	vt100_set_position(row,col)		usingstr("\033[%u;%uH",(uint)row,(uint)col)	// top,left = 1,1
#define	vt100_save_position				"\033""7"						// esc+'7'
#define	vt100_restore_position			"\033""8"						// esc+'8'
#define vt100_locate(row,col)			vt100_set_position(row,col)		// top,left = 1,1
#define	vt100_hlocate(col)				usingstr("\033[%uG",(uint)col)		// top,left = 1,1
#define	vt100_vlocate(row)				usingstr("\033[%ud",(uint)row)		// top,left = 1,1
#define	vt100_cursor_left_1				"\033[D"
#define	vt100_cursor_right_1			"\033[C"
#define vt100_cursor_up_1				"\033[A"						// no scroll
#define vt100_cursor_down_1				"\033[B"						// no scroll
#define	vt100_cursor_left(n)			usingstr("\033[%uD",(uint)n)
#define	vt100_cursor_right(n)			usingstr("\033[%uC",(uint)n)
#define vt100_cursor_up(n)				usingstr("\033[%uA",(uint)n)		// no scroll
#define vt100_cursor_down(n)			usingstr("\033[%uB",(uint)n)		// no scroll

#define	vt100_bell						"\007"
#define vt100_bs						"\010"
#define vt100_tab						"\011"
#define vt100_crnl						"\012\015"
#define	vt100_clear_right				"\033[0K"						// incl. crsr loc.
#define vt100_clear_left				"\033[1K"						// incl. crsr loc.
#define vt100_clear_down				"\033[0J"						// incl. crsr loc.
#define vt100_clear_up					"\033[1J"						// incl. crsr loc.
#define vt100_clear_line				"\033[2K"
#define vt100_clear_screen				"\033[2J"
#define vt100_cls						"\033[H\033[J"					// clear scrn & home crsr



/*	------------------------------------------
	wrap string with vt100 attribute on/off escape sequences
*/
extern String		VT_Bold				( cString& );
extern String		VT_Inverse			( cString& );
extern String		VT_Underline		( cString& );
extern String		VT_FGColor			( cString& s, VT_Color c );
extern String		VT_BGColor			( cString& s, VT_Color c );
extern String		VT_Attr				( cString& s, VT_Color fg, VT_Color bg=VT_normal );



/*	structs for input and output settings and state
*/
struct output_data
{
	OSErr			state;				// stream state, error code from signal handler
	int32			io_cnt;				// idx-- remaining io count
	ptr				io_ptr;				// ptr++ io buffer ptr
	String			string;				// persistent io buffer
	Irpt			irpt;				// interrupt triggered when async io finished
	double			last_io;			// system timestamp of last io
	double			timeout;			// setting: timeout	(provided for caller)
	CharEncoding	encoding;			// setting: character encoding for Get() and Put()
};

struct input_data : output_data
{
	uint32			io_stopctls;		// stop ctls in use for async io
	String			io_buffer;			// binary putback buffer for overread bytes after stopctl for serial devices

	union { char bu[6]; UCS1Char c1; UCS2Char c2; UCS4Char c4; }
					getchar;			// persistent input buffer for GetChar()

	uint32			getstring_stopctls;	// GetString():		setting: mask of ctl codes which stop input
	int32			getstring_maxsize;	// GetString():		setting: max. input size when no stopctl found

	int				edid;				// GetString(tty):	editor resume id
	int				cols;				// GetString(tty):	terminal width
	int				idx, idx_soll;		// GetString(tty):	real and requested cursor position (idx in target string)
	UCS4Char		c1,c2,c3;			// GetString(tty):	escape sequence in progress
	#define			last_ctl	c1		// GetString():		last stopctl: for DOS line break folding. also used by GetChar()
};


/*	frequently used values for stopctls:
*/
#define stopctls_none	0x00000000
#define	stopctls_crlf	((1<<0)|(1<<3)|(1<<4)|(1<<10)|(1<<12)|(1<<13))		// cr,lf,ff,eof etc.
#define stopctls_all	0xFFFFFFFF


/* Values for 'resume':
*/
enum ResumeCode
{
	resume_io,			// resuming call after start_io
	start_io,			// first call, caller will resume with resume_io after EAGAIN
	test_io,			// first call, get immediately availabe data only, don't shedule async i/o
	edit_io,			// first call, print and edit target string (GetString & tty only)
};


/*	==========================================
					class Stream
	==========================================
*/
class Stream : private BPObj
{
friend class Var;
friend class VScript;

	Stream*			next;
	Stream*			prev;

	int				fd;					// unix file descriptor
	String			filepath;			// open() path
	s_type			filetype;			// device type

	input_data		input;
	output_data		output;

	String			putback_string;
	VT_Color		tty_attr;			// for automatic text coloring on tty

// UPs:
	void			init				( );
	void			kill				( );
	void			poll_output			( );
	void			poll_input			( );
	cptr			find_stop_ctl		( cptr, cptr, uint32 );
	void			putback_or_lseek	( cString& );
	void			putback_or_lseek	( cptr, int32 );

// prohibit:
					Stream				( const Stream& );				// prohibit
	void			operator=			( const Stream& );				// prohibit


// ==== PUBLIC ===============================================================
public:

// static functions:
#ifdef SIGPOLL
	static void		PollPendingInput	( );
	static void		PollPendingOutput	( );
#endif
#ifdef SIGIO
	static void		PollPendingIO		( );
#endif

// constructor, destructor:
					Stream				( int fd, cstr path );			// stdin/out/err
					Stream				( cString& path, int flags );	// open
					~Stream				( )				{ kill(); }		// close

// set/get data members:
	int				FD					( )	const		{ return fd; }
	cString&		FilePath			( )	const		{ return filepath; }

	s_type			FileType			( )	const		{ return filetype; }
	bool			IsFile				( )	const		{ return filetype == s_file; }
	bool			IsFileOrBlockDevice	( )	const		{ return filetype == s_file || filetype == s_block; }
	bool			NoFileOrBlockDevice	( )	const		{ return filetype != s_file && filetype != s_block; }
	bool			IsTTY				( )	const		{ return filetype == s_tty; }
	bool			NoTTY				( )	const		{ return filetype != s_tty; }
	void			SetTTYAttr			( VT_Color c )	{ tty_attr = c; }

// i/o state:
	OSErr			InputState			( )	const		{ return input.state; }
	OSErr			InputError			( )	const		{ return input.state==EAGAIN ? ok : input.state; }
	bool			IsOpenIn			( )	const		{ return input.state!=inputnotpossible; }
	bool			InputInProgress		( ) const		{ return input.state==EAGAIN; }
	bool			InputFinished		( )	const		{ return input.state!=EAGAIN; }
	void			CancelInput			( )				{ if( input.state==EAGAIN ) input.state=ok; }
	void			ClearInputError		( )				{ if( input.state!=EAGAIN && input.state!=inputnotpossible )  input.state=ok; }

	OSErr			OutputState			( )	const		{ return output.state; }
	OSErr			OutputError			( )	const		{ return output.state==EAGAIN ? ok : output.state; }
	bool			IsOpenOut			( )	const		{ return output.state!=outputnotpossible; }
	bool			OutputInProgress	( ) const		{ return output.state==EAGAIN; }
	bool			OutputFinished		( )	const		{ return output.state!=EAGAIN; }
	void			CancelOutput		( )				{ if( OutputInProgress() ) output.state=ok; }
	void			ClearOutputError	( )				{ if( output.state!=EAGAIN && output.state!=outputnotpossible ) output.state=ok; }

// async i/o data:
	int32			InputBytesRemaining	( )	const		{ return input.io_cnt; }
	int32			OutputBytesRemaining( )	const		{ return output.io_cnt; }
	int32			InputBytes			( )	const		{ return input.string.Len() - input.io_cnt; }
	int32			OutputBytes			( )	const		{ return output.string.Len() - output.io_cnt; }
	ptr&			InputPtr			( )				{ return input.io_ptr; }
	ptr&			OutputPtr			( )				{ return output.io_ptr; }
	double&			LastInput			( )				{ return input.last_io; }
	double&			LastOutput			( )				{ return output.last_io; }
	Irpt&			InputInterrupt		( )				{ return input.irpt;  }
	Irpt&			OutputInterrupt		( )				{ return output.irpt; }
	String&			InputBuffer			( )				{ return input.string; }
	String&			OutputBuffer		( )				{ return output.string; }
	double&			InputTimeout		( )				{ return input.timeout; }
	double&			OutputTimeout		( )				{ return output.timeout; }

// binary read/write:
// note: read/write must not be called while async i/o pending.
	int32			Write				( cptr bu, int32 cnt );					// EAGAIN -> lock buffer & wait for async i/o finished!
	int32			Read				( ptr bu, int32 cnt, uint32 stopctls=0 );	// EAGAIN -> lock buffer & wait for async i/o finished!

	inline void		Write				( cString& );							// EAGAIN -> wait for async i/o finished!
	void			Write				( cstr );								// EAGAIN -> wait for async i/o finished!
	inline void		Read				( String& s, ResumeCode );				// EAGAIN -> wait for async i/o finished!

// converted get/put
// note: read/write must not be called while async i/o pending.
	void			PutChar				( UCS4Char c )			{ PutString( String(c) ); }
	void			PutString			( cString& s );
	bool			GetChar				( UCS4Char&, ResumeCode );
	inline void		GetChar				( String& s, ResumeCode );
	void			GetString			( String& s, ResumeCode );
	bool			GetCharAvail		( );

	CharEncoding&	InputEncoding		( )						{ return input.encoding; }
	CharEncoding&	OutputEncoding		( )						{ return output.encoding; }

	int32&			GetStringMaxSize	( )						{ return input.getstring_maxsize; }
	uint32&			GetStringStopCtls	( )						{ return input.getstring_stopctls; }

// putback buffer:
	void			ClearPutbackBuffer	( )						{ putback_string.Clear(); }
	void			PutbackString		( cString& s )			{ if(IsOpenIn()) putback_string = s + putback_string; else errno=inputnotpossible; }
	void			PutbackChar			( UCS4Char& c )			{ PutbackString(String(c)); }

// file and block devices:
	off_t			GetFileLength		( ) const				{ return file_size(fd);			}				// returns -1 on error
	off_t			GetFilePosition		( ) const				{ return lseek(fd,0,SEEK_CUR);		}				// ""
	off_t			SetFilePosition		( off_t d )				{ return lseek( fd, d, SEEK_SET );	}				// ""
	bool			SetFileLength		( off_t d )				{ return ftruncate( fd, d ); /*0=ok,1=error*/	}	// must be openout
	bool			TruncateFile		( )						{ off_t n = lseek(fd,0,SEEK_CUR); return n==-1 || ftruncate(fd,n); } // at current FP

// tty:
	void			TTYGetWindowSize	( int& rows, int& cols ){ TerminalSize(fd,rows,cols);		}
	int				TTYGetWindowCols	( )						{ return TerminalCols(fd);			}
	int				TTYGetWindowRows	( )						{ return TerminalRows(fd);			}
	void			TTYSetWindowSize	( int rows, int cols )	{ SetTerminalSize(fd,rows,cols);	}

	void			TTYRequestPosition	( )						{ Write("\e[6n",4);					}	// --> Terminal responds
	void			TTYReceivePosition	( int& row, int& col, ResumeCode );								// top,left = 1,1
	void			TTYSetPosition		( int row, int col )	{ Write(vt100_locate(row,col));		}	// top,left = 1,1
	void 			TTYSavePosition		( )						{ Write("\e7",2);		/*esc+'7'*/	}
	void 			TTYRestorePosition	( )						{ Write("\e8",2);		/*esc+'8'*/	}

	void 			TTYLocate 			( int row, int col )	{ TTYSetPosition(row,col);			}	// top,left = 1,1
	void 			TTYHLocate			( int col  )			{ Write(vt100_hlocate(col));		}	// top,left = 1,1
	void 			TTYVLocate			( int row  )			{ Write(vt100_vlocate(row));		}	// top,left = 1,1
	void 			TTYCursorLeft 		( )						{ Write("\e[D",3);					}
	void 			TTYCursorRight		( )						{ Write("\e[C",3);					}
	void 			TTYCursorUp 		( )						{ Write("\e[A",3);					}	// no scroll
	void 			TTYCursorDown 		( )						{ Write("\e[B",3);					}	// no scroll
	void 			TTYCursorLeft 		( int n )				{ Write(vt100_cursor_left(n));		}
	void 			TTYCursorRight		( int n )				{ Write(vt100_cursor_right(n));		}
	void 			TTYCursorUp 		( int n )				{ Write(vt100_cursor_up(n));		}	// no scroll
	void 			TTYCursorDown 		( int n )				{ Write(vt100_cursor_down(n));		}	// no scroll
	void 			TTYBell 			( )						{ Write("\007",1);					}
	void 			TTYTab				( )						{ Write("\t",1);					}
	void 			TTYNL 				( )						{ Write("\n\r",2);					}
	void			TTYClearRight		( )						{ Write("\e[0K",4);					}	// incl. crsr loc.
	void			TTYClearLeft		( )						{ Write("\e[1K",4);					}	// incl. crsr loc.
	void			TTYClearLine		( )						{ Write("\e[2K",4);					}
	void			TTYClearDown		( )						{ Write("\e[0J",4);					}	// incl. crsr loc.
	void			TTYClearUp			( )						{ Write("\e[1J",4);					}	// incl. crsr loc.
	void			TTYClearScreen		( )						{ Write("\e[2J",4);					}
	void 			TTYCls				( )						{ Write("\e[H\033[J",6);			}	// clear scrn & home crsr

	void			TTYClearAttr		( )						{ Write( vt100_clearattr,4 );		}
	void			TTYSetBold			( bool f = on )			{ Write( vt100_bold[f],5-f );		}
	void			TTYSetInverse		( bool f = on )			{ Write( vt100_inverse[f],5-f );	}
	void			TTYSetUnderline		( bool f = on )			{ Write( vt100_underline[f],5-f );	}
	void			TTYSetFGColor		( VT_Color c )			{ Write( vt100_setfgcolor(c) );		}
	void			TTYSetBGColor		( VT_Color c )			{ Write( vt100_setbgcolor(c) );		}
	void			TTYSetAttr			( VT_Color fg, VT_Color bg=VT_normal ) { Write( vt100_setattr(fg,bg) ); }
};


inline void Stream::Write ( cString& s )
{
	if( s.Csz()!=csz1 )							// then we must resize to csz1 but we must not modify the source string!
	{
		String z = s; z.ResizeCsz(csz1);
		Write( z.Text(), z.Len() );
		if( errno==EAGAIN ) output.string = z;	// lock buffer
	}
	else
	{
		Write( s.Text(), s.Len() );
		if( errno==EAGAIN ) output.string = s;	// lock buffer
	}
}

inline void Stream::Read ( String& s, ResumeCode r )
{
	assert( r==start_io || r==resume_io );

	if( r==start_io )
	{
		if( s.NotWritable() || s.Csz()!=csz1 ) s = String( s.Len(), csz1 );	// uncleared
		int32 n = Read( s.Text(), s.Len(), stopctls_none );
		assert( n==s.Len() || errno!=ok );
		if( errno==ok ) return;
		if( errno==EAGAIN ) input.string = s;		// EAGAIN -> input still in progress -> lock buffer
		s.Truncate(n);
	}
	else // resume_io
	{
		errno = input.state;
		s = input.string;							// get input buffer
		s.Crop( 0, input.io_cnt );					// monitor progress
	}
}

inline void Stream::GetChar ( String& s, ResumeCode r )
{
	UCS4Char z;
	if( GetChar(z,r) ) s = String(z); else if(errno!=EAGAIN) s.Clear();
}




#endif











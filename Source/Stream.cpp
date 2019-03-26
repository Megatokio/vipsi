/*	Copyright  (c)	Günter Woigk 2002 - 2019
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

	2002-04-28 kio	moved open and close of file into creator/destructor
	2003-07	   kio	changes to use UCS4String library
	2004	   kio	started mod for vipsi-new
	2005-07-05 kio	mod for vipsi-new finished
*/

/*	Stream class with asynchronous i/o for Interpreters

overview:

	Read() and Write():									for binary i/o
	PutChar(), GetChar(), GetString() and PutString():	for text i/o with character encoding

asynchronous:

	Writing tries to write immediately but buffers all not immediately written data for async output.
	Reading tries to read immediately but shedules all not yet written data for async input.

	The async i/o pending state is notified by errno==EAGAIN on return.

	Though this implementation is called  "asynchronous", it is so only when viewed from the script.
	It is "synchronous" when viewed from the interpreter.

	When i/o becomes possible again, the SIGIO (SIGPOLL) signal is raised and the main program must
	catch it. It then must synchronously from the main event loop call the PollPendingIO() handler.

	It is not possible to call PollPendingIO() from the signal handler directly. This would require
	signal blocking during some critical code and lots of volatile declarations for multi-processor safety.
	Also at some points String instances are created and destroyed which is, at least, not thread safe.

async i/o pseudo-code example:

	assumption:
		script:			get(my_string);
		virtual code:	PUSH_ADDRESS(my_string), GET_STRING, WAIT_ASYNC_INPUT

	in interpreter's opcode dispatcher loop:

	  case GET_STRING:
		STDIN->GetString( TOP_STACK_STRING,false );
		goto NEXT;

	  case WAIT_ASYNC_INPUT:
		if( STDIN->InputFinished() ) NEXT;
		if( SystemTime() > STDIN->LastInput()+STDIN->InputTimeout() ) goto TIMEOUT_ERROR;
		STDIN->GetString( TOP_STACK_STRING,true );
		if( errno!=EAGAIN ) goto NEXT;
		INSTR_PTR--;
		STDIN->InputInterrupt().Shedule( STDIN->LastInput()+STDIN->InputTimeout() );
		goto DISP;
*/

#define SAFE 3
#define LOG 1
#include "Stream.h"

INIT_MSG


static VT_Color last_tty_attr = VT_normal;			// for automatic text coloring on tty



/* ----	UP: suche in Buffer nach stopctls -----------
		suche nach Controlcodes für die das Bit in der stopctls-Maske gesetzt ist
		return:		nullptr:	no stopctl found
					ptr:	ptr zeigt hinter den ersten stopctl im Buffer

		note:
		-!-	Immer nur im Zielpuffer nach stopctls suchen, nie im Quellpuffer, z.B. nie im input.io_buffer!
			find_stop_ctl() erwartet ggf. alignierte Adressen und liest bis zu 3 Bytes vor Startpointer a.

		-1-	Stream::find_stop_ctl() beachtet für die Zeichengröße die aktuelle Textkodierung.
			Es wird angenommen, dass nur *Text* mit stopctls gelesen wird.
			Dann besagt ucs2/ucs4, dass der Quelltext eine fixe Zeichenbreite von 2/4 Bytes hat.
			Ein Controlcode muss für ein *Zeichen* gelten, bei ucs2/ucs4 also für 2 oder 4 Byte.
			Bei Ein-Byte-Zeichensätzen und auch bei UTF-8 ergibt sich das Problem nicht.

			Sollen binäre Daten mit stopctls gelesen werden, muss die Textkodierung ggf. vorher
			auf ucs1/utf8 gestellt werden; wobei utf-8 sowieso default für Streams ist.

		-2-	Bei ucs2 und ucs4 wird außerdem angenommen, dass der Text auf 2- bzw. 4-Byte-Adressen
			aligniert ist, was sinnvollerweise immer so sein sollte und bei Strings auch immer so ist.

			Wenn a und e nicht aligniert sind, so wird angenommen, dass sie 'in' den Buchstaben zeigen.
			Für a wird dann angenommen, dass a auf den Anfang zurückgestellt werden kann, weil
			diese Bytes sehr wohl schon im Buffer stehen,
			und für e wird angenommen, dass die fehlenden Bytes noch nicht gelesen wurden.
*/
inline bool is_stopctl( UCS4Char c, uint32 stopctls )
{
	return c<' ' && (stopctls&(1<<c));
}
inline cUCS1Char* find_stop_ctl ( cUCS1Char* a, cUCS1Char* e, uint32 stopctls )
{
	while( a<e ) { UCS1Char c = *a++; if( c<' ' && (stopctls&(1<<c)) ) return a; }
	return nullptr;
}
inline cUCS2Char* find_stop_ctl ( cUCS2Char* a, cUCS2Char* e, uint32 stopctls )
{
	while( a<e ) { UCS2Char c = *a++; if( c<' ' && (stopctls&(1<<c)) ) return a; }
	return nullptr;
}
inline cUCS4Char* find_stop_ctl ( cUCS4Char* a, cUCS4Char* e, uint32 stopctls )
{
	while( a<e ) { UCS4Char c = *a++; if( c<' ' && (stopctls&(1<<c)) ) return a; }
	return nullptr;
}

cptr Stream::find_stop_ctl ( cptr a, cptr e, uint32 stopctls )
{
	switch( input.encoding )
	{
	default:	return (ptr) ::find_stop_ctl( (UCS1Char*)        a,      (UCS1Char*)        e,      stopctls );
	case ucs2:	return (ptr) ::find_stop_ctl( (UCS2Char*)(size_t(a)&~1), (UCS2Char*)(size_t(e)&~1), stopctls );
	case ucs4:	return (ptr) ::find_stop_ctl( (UCS4Char*)(size_t(a)&~3), (UCS4Char*)(size_t(e)&~3), stopctls );
	}
}


/* ----	count characters in UTF-8 string ----------------------------
*/
inline int32 utf8_charcount ( cptr a, cptr e )
{
	int32 n = 0; while (a<e) n += utf8_no_fup(*a++); return n;
}


/* ----	put back data -----------------------------------------------
		stelle den Lesezeiger des Streams zurück
		block devices:	lseek
		serial devices:	put back to input.io_buffer
*/
inline void Stream::putback_or_lseek ( cString& s )
{
	if( s.Len() ) if( NoFileOrBlockDevice() || lseek(fd,-s.Len(),SEEK_CUR)==-1 )
	{
		input.io_buffer = s + input.io_buffer;
	}
}

inline void Stream::putback_or_lseek ( cptr p, int32 n )
{
	if( n ) if( NoFileOrBlockDevice() || lseek(fd,-n,SEEK_CUR)==-1 )
	{
		input.io_buffer = String( (uptr)p, n ) + input.io_buffer;
	}
}

#include <sys/time.h>
#include <sys/select.h>

bool Stream::GetCharAvail()
{
	if(putback_string.Len()) return true;
	fd_set fd = { {1<<0}/*stdin*/ };  	// bitmask of fd numbers to test
	timeval t={0,0};					// tv_sec, tv_usec
	int rc = select(1/*num_fds*/, &fd/*fd4in?*/, nullptr/*fd4out?*/, nullptr/*fd4error?*/, &t/*timeout*/ );
	//if(rc==-1) state=errno;
	return rc >= 1;	// num_fd bits set on return
}


// ============================================================================
// ====	    TTY / VT100     ===================================================
// ============================================================================


const escseq vt100_clearattr    =   "\033[0m";					// reset all attr
const escseq vt100_clearfgcolor =   "\033[39m";					// foreground := default color
const escseq vt100_clearbgcolor =   "\033[49m";					// background := default color
const escseq vt100_bold[2]      = { "\033[22m", "\033[1m" };	// bold off/on
const escseq vt100_inverse[2]   = { "\033[27m", "\033[7m" };	// inverse off/on
const escseq vt100_underline[2] = { "\033[24m", "\033[4m" };	// underline off/on

// UP:	create esc sequence to set/clear fg/bg color
//		note: return modified static cstr -> for immediate use only!
cstr vt100_setfgcolor ( VT_Color c )
{
	bool bright = c&VT_bright; c &= 15; if (c==VT_normal) return vt100_clearfgcolor;
	static escseq e = "\033[39m"; if (bright) e[2] = '9';
	e[3] = '0' -VT_black +c; return e;
}

cstr vt100_setbgcolor ( VT_Color c )
{
	bool bright = c&VT_bright; c &= 15; if (c==VT_normal) return vt100_clearbgcolor;
	static escseq bu[2] = { "\033[49m", "\033[100m" };
	ptr e = bu[bright]; e[3+bright] = '0' - VT_black +c; return e;
}

cstr vt100_setattr ( VT_Color fg, VT_Color bg )
{
	static char s[40]; ptr p = s;
							{ memcpy( p, vt100_clearattr, 4 );    p+=4; }		// clear attr; löscht auch fg/bg color
	if (fg&VT_bold)         { memcpy( p, vt100_bold[1], 4 );      p+=4; }
	if (fg&VT_underline)    { memcpy( p, vt100_underline[1], 4 ); p+=4; }
	if (fg&VT_inverse)      { memcpy( p, vt100_inverse[1], 4 );   p+=4; }
	if ((fg&15)!=VT_normal)	{ strcpy( p, vt100_setfgcolor(fg) );  p=strchr(p,0); }
	if ((bg&15)!=VT_normal)	{ strcpy( p, vt100_setbgcolor(bg) );  p=strchr(p,0); }
	*p=0; return s;
}


/* ---------------------------------------------------------------
		set'n'clear attributes functions for printing strings
		the following functions frame the passed string with
		control sequences which set the desired attributes
		and clear those attributes after printing.
		note:
			there is no possibility to test whether some of the
			desired attributes were already set before printing
		1) 	so if they are set they are set before and always reset after printing
		2)	but if they are not set they are not reset before nor after printing
*/

String VT_Bold      ( cString& s )	{ return String(vt100_bold[1])      + s + vt100_bold[0];      }
String VT_Inverse   ( cString& s )	{ return String(vt100_inverse[1])   + s + vt100_inverse[0];   }
String VT_Underline ( cString& s )	{ return String(vt100_underline[1]) + s + vt100_underline[0]; }
String VT_FGColor	( cString& s, VT_Color c ) { return (c&15)==VT_normal ? s : String(vt100_setfgcolor(c)) + s + vt100_clearfgcolor; }
String VT_BGColor	( cString& s, VT_Color c ) { return (c&15)==VT_normal ? s : String(vt100_setbgcolor(c)) + s + vt100_clearbgcolor; }

String VT_Attr ( cString& s, VT_Color fg, VT_Color bg )
{
	String z = s;
	if (fg&VT_bold)			z = VT_Bold(z);
	if (fg&VT_inverse)		z = VT_Inverse(z);
	if (fg&VT_underline)	z = VT_Underline(z);
	if ((fg&15)!=VT_normal)	z = VT_FGColor(z,fg);	// color == VT_normal -> fg color is not set nor reset
	if ((bg&15)!=VT_normal)	z = VT_BGColor(z,bg);	// color == VT_normal -> bg color is not set nor reset
	return z;
}



// ====================================================================================
//				Stream:	Creator / Destructor
// ====================================================================================


/*	reverse lookup table fd -> Stream
	required for interrupt handler SignalHandler(…)
*/
static Stream*	stream0 = nullptr;


/* ----	UP: initialize stream --------
*/
void Stream::init ( )
{
	fd				= -1;			// -1 <=> open failed
	//filepath.Clear();
	filetype		= s_none;

	output.state	= outputnotpossible;
	input.state		= inputnotpossible;
	//output.io_cnt	= 0;
	//input.io_cnt	= 0;
	//output.io_ptr	= nullptr;
	//input.io_ptr	= nullptr;
	//output.lastio	= 0;
	//input.lastio	= 0;
	//output.irpt.Clear();
	//input.irpt.Clear();
	//output.string	= "";
	//input.string  = "";
	output.timeout	= 10.0;
	input.timeout	= 10.0;
	output.encoding	= utf8;
	input.encoding	= utf8;

	//input.io_stopctls		 = 0x00000000;
	//input.io_buffer.Clear();
	input.getstring_stopctls = stopctls_crlf;
	input.getstring_maxsize	 = 1<<14;

	//putback_string.Clear();
	tty_attr = VT_normal;

	if(stream0) { next=stream0; prev=next->prev; next->prev=this; prev->next=this; }
	else		{ stream0 = next = prev = this; }
}


/* ----	UP: close file, cleanup --------
		note: Flush or Cancel pending i/o befor closing
*/
void Stream::kill ( )
{
	if( (uint)fd<=2 ) return;							// never close 0/1/2 = stdin/out/err

	assert( this != stream0 );							// stream0 zeigt auf stdin, out oder err
	assert( !InputInProgress() && !OutputInProgress() );	// Flush or Cancel pending i/o befor closing

	next->prev = prev; prev->next = next;
	if(fd>0) close(fd);									// TODO: may set errno
}



/* ----	creator for stdin/out/err ------------------------
		create a stream for stdin, stdout or stderr
		out: errno set/cleared
		errno:	ok		stream for stdio created
				other	error. (currently: none)
*/
Stream::Stream ( int f, cstr fname )
:	filepath(fname)
{
	assert( f>=0 && f<=2 );
	assert( fname && *fname );

	xlogIn( "Stream(%i,%s)\n", f, quotedstr(fname) );

	init();
	fd  	 = f;
	filetype = ClassifyFile(f);
	if( f==0					) input.state  = ok;	// input possible
	if( f>=1 || filetype==s_tty ) output.state = ok;	// output possible
	if( IsTTY() ) { input.timeout = 60*60;/*1std*/ output.timeout = 60;/*1min*/ }
	errno = ok;
}


/* ----	creator for stream --------------------------------
		create stream for file, pipe, etc.
		out: errno set/cleared
		errno:	ok		stream for stdio created
				other	error.
*/
Stream::Stream ( cString& fname, int flags )
:	filepath(fname)
{
	xlogIn( "Stream(%s,%4x)\n", quotedstr(CString(fname)), flags );
	assert( fname.Len()>0 );

	init();
	cstr cpath = FullPath(CString(fname),1);			// also finds bogus utf-8 filenames. sets errno
	if (errno) goto x;

	flags |= O_NONBLOCK;
	fd = open(cpath,flags,6*(1+8+64));

	if (fd<0)	// open failed
	{
x:		xlogline("failed");
	}
	else		// open ok
	{
		if( flags & (O_RDWR|O_WRONLY|O_APPEND) ) output.state = ok;		// output possible
		if( output.state || (flags&O_RDWR) )	 input.state  = ok;		// input pssible. note: O_RDONLY is supposedly == 0x0000
		filetype = ClassifyFile(fd);
		if( IsTTY() ) { input.timeout = 60*60;/*1std*/ output.timeout = 60;/*1min*/ }
		errno = ok;
	}
}



// ===============================================================
//				Interrupt (Signal) Handler
// ===============================================================

/* ----	write data from output buffer to device --------
		out:	 output.state:	 ok, EAGAIN or real error
				 output.last_io: updated whenever data was actually written
				 output.irpt:	 triggered on error or when all bytes written (-> EAGAIN no longer active)
*/
void Stream::poll_output()
{
	assert( output.state == EAGAIN );
	assert( output.io_cnt >= 0 );

	int32 n	= output.io_cnt;
	ptr  p	= output.io_ptr;
	int32 n0	= n;
	int	 state;

a:	int32 m = write( fd, p, n );
	if (m==-1) { state=errno; if(state==EINTR) goto a; goto x; /*EAGAIN or real error*/ }
	assert(m!=0 || n==0);
	n-=m; p+=m; if(n>0) goto a;
	state = ok;

x:	output.state = state;
	if( n!=n0 ) { output.io_cnt = n; output.io_ptr = p; output.last_io = SystemTime(); }
	if( state!=EAGAIN ) output.irpt.Trigger();		// !EAGAIN => ok:fertig oder fehler
}



/* ----	read data from device into input buffer --------
		out:	 input.state:	ok, EAGAIN or real error
				 input.last_io:	updated whenever data was actually received
				 input.irpt:	triggered on error or when input completed (-> EAGAIN no longer active)
		input_stopctls:
			-> input vorzeitig beendet wenn stopctl gefunden -> io_cnt>0 && io_ptr<io_end && io_ptr[-1]==stopctl
			-> überzählige Daten werden in den input.io_buffer zurückgestellt
*/
void Stream::poll_input()
{
	assert( input.state == EAGAIN );
	assert( input.io_cnt >= 0 );

	ptr  p	= input.io_ptr;
	int32 n	= input.io_cnt;
	int32 n0	= n;
	int  state;
	cptr e;

a:	int32 m = read( fd, p, n );
	if (m==-1) { state=errno; if(state==EINTR) goto a; else goto x; /*EAGAIN or real error*/ }
	assert(m!=0 || n==0);							// <-- nur files/block devs dürfen 0 at endoffile liefern
	if( input.io_stopctls && (e = find_stop_ctl(p,p+m,input.io_stopctls)) )
	{
		assert( input.io_buffer.Len() == 0 );
		input.io_buffer = String( (UCS1Char*)e, m-(e-p) );
		n -= e-p; p = const_cast<ptr>(e);
	}
	else
	{
		n -= m; p += m; if(n>0) goto a;
	}
	state = ok;

x:	input.state = state;
	if( n!=n0 ) { input.io_cnt = n; input.io_ptr = p; input.last_io = SystemTime(); }
	if( state!=EAGAIN ) input.irpt.Trigger();		// !EAGAIN => ok:fertig oder fehler
}



#ifdef SIGIO
void Stream::PollPendingIO ( )
{
	Stream* s = stream0;
	if(s) do
	{
		if( s->output.state==EAGAIN ) s->poll_output();
		if( s->input.state==EAGAIN  ) s->poll_input();
	}
	while( (s=s->next)!=stream0 );
	errno=ok;
}
#endif



#ifdef SIGPOLL
void Stream::PollPendingInput ( )
{
	Stream* s = stream0;
	if(s) do { if( s->input.state==EAGAIN ) s->poll_input(); } while( (s=s->next)!=stream0 );
	errno=ok;
}

void Stream::PollPendingOutput ( )
{
	Stream* s = stream0;
	if(s) do { if( s->output.state==EAGAIN ) s->poll_output(); } while( (s=s->next)!=stream0 );
	errno=ok;
}
#endif



// ===============================================================
//					Binary Read and Write
// ===============================================================


/* ----	write data to stream --------

		in:		buffer with data to write.
				ignores errno on entry
				checks output.state: async_io_pending, error, !openout and fd!=-1.
				writes directly to fd if possible and queues the rest for async i/o.

		out:	returns: number of bytes immediately written.
		errno:	errno = output.state (except asynciopending)
				ok:		all data written immediately. no data buffered for async i/o.
				EAGAIN:	partial data written. remainder is sheduled for async i/o.
						output.irpt is cleared and output.last_io is set to now for convenience.
						caller MUST LOCK BUFFER until async i/o completes.
				asynciopending: Write() was called while async i/o in progress.
						nothing was written. wait for interrupt & try again.
				other:	error. partial data may be written.
*/
int32 Stream::Write ( cptr p, int32 n )
{
	int32 m, io = 0;
	OSErr state = output.state;

	if( state!=ok )	goto oe;	// fp==-1, !openout, EAGAIN (async i/o pending) oder real error
	if( n<=0 )		goto ok;	// ok

// write directly to device

a:	m = write( fd, p, n );
	if (m==-1) { state=errno; if(state==EINTR) goto a; if( state==EAGAIN) goto e; else goto x; }
	io+=m; if(m==n) goto ok; p+=m; n-=m; goto a;

// EAGAIN: output full => signal handler called when output possible again

e:	assert(NoFileOrBlockDevice());

	output.irpt.Clear();
	output.last_io = SystemTime();		// set even if no i/o yet done: caller will add timeout to timestamp
	output.io_ptr  = const_cast<ptr>(p);
	output.io_cnt  = n;
x:	output.state   = state;				// state=EAGAIN -> activate async i/o
	return io;							// errno=EAGAIN -> caller must lock buffer

oe:	errno = state==EAGAIN ? asynciopending : state;
	return io;							// state = errno = error

ok:	errno = ok;							// errno = ok
	return io;							// state = ok
}



/* ----	write cstring ---------------------------
		similar to Write(ptr,cnt); but for cstrings

		out:
		errno:	EAGAIN:	caller must wait for async i/o to complete.
				remaining data is buffered. (no need to lock cstr s)

		note:	intended for dynamic cstrings (locals and qstring pooled).
				prefer Write(ptr,cnt) for static cstings.
*/
void Stream::Write ( cstr s )
{
	int32 n = strlen(s);
	int32 m = Write(s,n);
	if( errno==EAGAIN )									// async i/o pending
	{
		output.string = String((UCS1Char*)s+m,n-m);		// make a durable copy of the data (assuming cstr in qstring pool)
		output.io_ptr = output.string.Text();			// and adjust io_ptr for the durable data
	}
}



/* ----	write String -----------------------------
		colorize output for stdin/out/err on tty
		convert text acc. to encoding
		and write string
*/
void Stream::PutString ( cString& s )
{
	if( IsTTY() && fd<=2 && tty_attr!=last_tty_attr )
		 Write( String(vt100_setattr(last_tty_attr=tty_attr,VT_normal)) + s.ConvertedTo(output.encoding) );
	else Write( s.ConvertedTo(output.encoding) );
}



/* ----	read data from stream -------------------

		in:		buffer for data to read
				stopctls: bitmask of control codes which prematurely stop input
				ignores errno on entry
				checks input.state: async_io_pending, error, !openin and fd!=-1.
				reads directly from fd if possible and queues the rest for async i/o.

		out:	returns: bytes immediately read.
		errno:	errno = input.state	(except asynciopending)
				ok:		all data read. no async i/o pending.
						if stopctls!=0 then bytes read can be less than requested.
				EAGAIN:	partial data read. remainder is sheduled for async i/o.
						input.irpt is cleared and input.last_io is set to now for convenience.
						caller MUST LOCK BUFFER until async i/o completes.
				asynciopending: Read() was called while async i/o in progress: wait & try again.
				other:	error, e.g. endoffile. partial data up to error position read.
*/
int32 Stream::Read ( ptr p, int32 n, uint32 stopctls )
{
	int32 m, io = 0;
	cptr q,e;
	int32 state = input.state;

	if( state!=ok )	goto ie;			// fp==-1, !openin, EAGAIN (async i/o pending) oder i/o error
	if( n<=0 )		goto ok;			// ok

// get data from binary putback buffer:

	m = input.io_buffer.Len();
	if(m)
	{
		if(m>n) m = n;
		q = input.io_buffer.Text();		// note: Bytes erst kopieren und dann im Zielstring nach stopctls suchen,
		memcpy( p, q, m );				//		 weil der input.putback idR. nicht für ucs2/ucs4 ausgerichtet ist.
		if( stopctls && (e = find_stop_ctl( p, p+m, stopctls )) ) { m = n = e-p; }
		input.io_buffer.Crop(m,0);
		io+=m; if(n==m) goto ok; p+=m; n-=m;
	}

// read directly from device:

a:	m = read( fd, p, stopctls ? min(n,128) : n );
	if (m==-1) { state=errno; if(state==EINTR) goto a; if(state==EAGAIN) goto ea; else goto x; }
	if (m==0)  { assert(IsFileOrBlockDevice()); errno = state = endoffile; goto x; }
	if( stopctls && (e = find_stop_ctl( p, p+m, stopctls )) )
	{
		io += e-p;
		putback_or_lseek( e, m-(e-p) );
ok:		errno = ok;						// errno = ok
		return io;						// state = ok
	}
	io += m; if(m==n) goto ok; p+=m; n-=m; goto a;

// EAGAIN: input empty => signal handler called when input possible again

ea:	assert(NoFileOrBlockDevice());

	input.io_stopctls = stopctls;
	input.irpt.Clear();
	input.last_io = SystemTime();		// also set when no i/o yet done: caller will add timeout to timestamp
	input.io_ptr  = p;
	input.io_cnt  = n;
x:	input.state   = state;				// state=EAGAIN -> enable async i/o
	return io;							// errno=EAGAIN -> caller must lock buffer

ie:	errno = state==EAGAIN ? asynciopending : state;
	return io;							// errno = state = error
}



// ============================================================================
//					Text Input
// ============================================================================


/* ----	read a single character ----------------------------------------------------
		in:		first call:			resume = 0 = start_io
				subsequent calls:	resume = 1 = resume_io
				or single call:		resume = 2 = test_io

		read a single character with text conversion.
		checks putback buffer.
		no echo.

		to read a character call GetChar() with resume=start_io.
		while GetChar() returns errno=EAGAIN, wait for input.irpt and call GetChar() with resume=resume_io.

		return:	true:		one character read.
				false:		no character read: error or resume=test_io and no complete character available.

		errno:	ok:			reading finished. start_io or resume_io: one character returned.
											  test_io: zero or one character returned if immediately available.
				EAGAIN:		caller must wait for input.irpt and call GetChar(z,1) until completed
							bytes read so far are buffered internally. (no need to lock UCS4Char& z)
				asynciopending: Bummer: GetChar(c,0) was called with async i/o still in progress -> wait & try again.
				other:		error, e.g. endoffile. no character returned.
							=> caller may resume trying to read after clearing the error, e.g. for reading a growing file.

		notes:	incomplete multi-byte characters are always put back before return.
				if conversion is maliciously changed before GetChar(z,1), then some bytes may be lost.
*/
bool Stream::GetChar ( UCS4Char& z, ResumeCode resume )
{
	int32 io,mx;
	ptr const p  = input.getchar.bu;		// <-- union input.getchar is used as persistent buffer

	if( resume==resume_io )
	{
		io = input.io_ptr-p;	assert( uint32(io) <= 6 );			// <-- GetChar(c,resume_io) probably called without GetChar(c,start_io)
		mx = io+input.io_cnt;	assert( uint32(io) <= uint32(mx) );	// <-- GetChar(c,resume_io) probably called without GetChar(c,start_io)
		errno = input.state;
	}
	else	// first call: start_io or test_io
	{
		if( input.state!=ok )
		{
			errno = input.state==EAGAIN  ? asynciopending : input.state;
			return 0;									// return error
		}

		if( putback_string.Len() ) { z = putback_string[0]; putback_string.Crop(1,0); errno /* = input.state */ = ok; return 1; }

		mx = input.encoding==ucs4 ? 4 : input.encoding==ucs2 ? 2 : 1;
		io = Read( p, mx, stopctls_none );
	}

	assert( errno==input.state );
	assert ( (io==mx) == (errno==ok) );

// handle fixed-size characters:

	if( input.encoding != utf8 )
	{
		if( io<mx )										// input not finished ?
		{
x:			if( input.state==EAGAIN )					// input still in progress -> EAGAIN
			{
				if( resume!=test_io ) return 0;			// return EAGAIN
				errno = input.state = ok;				// test_io -> finish input with 'no char'
			}
			putback_or_lseek(input.getchar.bu,io);		// error: put back bytes already read (if any)
			return 0;									// return error
		}

	// convert data and return result:

		switch( input.encoding )
		{
		case ucs4:	z = input.getchar.c4; break;
		case ucs2:	z = input.getchar.c2; break;
		case ucs1:	z = input.getchar.c1; break;
		default:	z = String(input.getchar.c1).ConvertedFrom(input.encoding)[0]; break;
		}
		return 1;
	}

// handle utf8:

a:	if( mx==1 )			// -> at most 1 byte read: utf8 character start byte
	{
		if( io==0 ) goto x;								// io<mx -> error or EAGAIN still pending
		assert( io==mx );								// io==1 && mx==1 -> first byte read

		if( utf8_is_7bit(*p) )							// quick exit for 7-bit ascii
		{
			z = input.getchar.c1;
			return 1;
		}

		if( utf8_is_fup(*p) ) io = 0;					// forget bogus fup, get new starter
		else mx = UTF8CharNominalSize(*p);				// get fups
		io += Read( p+io, mx-io, stopctls_none );
		goto a;
	}
	else				// at least 1 byte read (utf8 starter), all fups requested
	{
		assert( mx==UTF8CharNominalSize(*p));

		cptr e = UTF8NextChar(p,p+io);					// e -> starter des nächsten Zeichens
		if( e < p+io )									// truncated char
		{
			putback_or_lseek( e, p+io-e );
			errno = input.state = ok;					// clear EAGAIN, endoffile or other error, if any
			z = UCS4ReplacementChar;
			return 1;
		}

		if( io<mx ) goto x;								// EAGAIN -> exit; other error -> put back incomplete char & exit

		z = UCS4CharFromUTF8(p);						// OK: return converted char
		return 1;
	}
}



/* ----	read string up to stopctl ----------------------------------------------------
		in:		resume = start_io	first call
				resume = resume_io	subsequent call
				resume = test_io	single call: read only what is immediately available
				resume = edit_io	edit target string. TTY only.

		read string with text conversion.
		checks putback buffer.
		echo on tty if possible.

		to read a string call GetString() with resume=start_io.
		while GetString() returns errno=EAGAIN, wait for input.irpt and call GetString() with resume=resume_io.

		out:	returns:	-
		errno:	errno = input.state (except for asynciopending)
				ok:			reading finished: reading stopped at stopctl, which is stored in input.last_ctl.
							if input was not stopped by a stopctl, e.g. for error, getstring_maxsize or test_io,
							then input.last_ctl contains an arbitrary non-ctl.
				EAGAIN:		caller must wait for input.irpt and call GetString(s,true) until completed.
							input.irpt is cleared and input.last_io is set to now for convenience.
							bytes read so far are buffered internally. (no need to lock String s)
				asynciopending: GetString(s,0) was called with async i/o still in progress -> wait & try again.
				other:		error, e.g. endoffile. partial data up to error position read.
							=> caller may resume trying to read after clearing the error, e.g. for reading a growing file.

		notes:	incomplete multi-byte characters are always put back before return.
				if conversion is maliciously changed before GetString(s,1), then some bytes may get lost.
*/
void Stream::GetString ( String& s, ResumeCode resume )
{
	if( NoTTY() || output.state==outputnotpossible || resume==test_io || input.getstring_stopctls==0 )
	{
		int32 io,mx; ptr p;
		String& z = input.string;	// buffer

		if( resume==resume_io )
		{
			errno = input.state;
			p  = z.Text();
			mx = z.Len();
			io = input.io_ptr-p;
		}
		else	// first call: start_io or test_io
		{
			if( input.state!=ok )
			{
				if( input.state==EAGAIN ) { errno=asynciopending; return; }
				errno = input.state; s.Clear(); return;
			}

		// get data from user putback buffer:

			int32 m = putback_string.Len();
			if(m)
			{
				int32 sz = putback_string.Csz();
				cptr q = putback_string.Text();
				cptr e = find_stop_ctl( q, q+m*sz, input.getstring_stopctls );
				if(e)
				{
					m = (e-q)/sz;
					s = putback_string.LeftString(m-1);
					input.last_ctl=putback_string[m];
					putback_string.Crop(m,0);
					return;
				}
				// else kein stopctl im putback_buffer: putback_buffer wird erst ganz zum Schluss vor s geklemmt.
			}

			mx = input.getstring_maxsize;
			if(z.NotWritable()||z.Csz()!=csz1||z.Len()!=mx) z = String( mx, csz1 );
			p  = z.Text();
			io = Read( p, mx, input.getstring_stopctls );
		}

		assert( errno == input.state );
		assert ( uint32(io) <= uint32(mx) );			// <-- GetChar(c,true) probably called without GetChar(c,false)
		assert( z.Csz() == csz1 );

		if( input.state==EAGAIN )					// async i/o still pending
		{
			if( resume!=test_io ) return;			// wait for more
			errno = input.state = ok;				// return what we have
		}

	// putback incomplete last char
	// convert data to text

		switch( input.encoding )
		{
		default:
			s = z.LeftString(io).ConvertedFrom(input.encoding);
			break;

		case ucs4:
			putback_or_lseek( p+(io&~3), io&3 );
			s = z.LeftString(io).FromUCS4();
			break;

		case ucs2:
			putback_or_lseek( p+(io&~1), io&1 );
			s = z.LeftString(io).FromUCS2();
			break;

		case ucs1:
			s = z.LeftString(io);
			break;

		case utf8:
			cptr e = UTF8LastChar(p,p+io);					// e -> starter des letzten Zeichens
			if( e>=p && e+UTF8CharNominalSize(e)>p+io )		// incomplete?
			{
				putback_or_lseek( e, io-(e-p) );			// put back incomlete last char
				io = e-p;
			}
			s = z.LeftString(io).FromUTF8();
			break;
		}

	// prepend putback text, if any

		if( putback_string.Len() ) { s = putback_string + s; putback_string.Clear(); }
		if( s.Len() && is_stopctl(s[s.Len()-1],input.getstring_stopctls) ) { input.last_ctl=s.LastChar(); s.Crop(0,1); }
		else input.last_ctl='X';
		return;
	}


// ----	input from TTY:	----------------------------

	else
	{
		UCS4Char &c1=input.c1, &c2=input.c2, &c3=input.c3, c4;
		String& z = input.string;

		if( resume == resume_io )
		{
			if( (errno=input.state)!=ok )  return;
			if( (errno=output.state)!=ok ) return;
			// errno = ok;

			switch( input.edid )
			{
			case 0: return;		// we are finished!
			case 3:	goto e3;
			case 4:	goto e4;
			case 5:	goto e5;
			case 6:	goto e6;
			case 7:	goto e7;
			case 8:	goto e8;
			case 9:	goto e9;
			default: IERR();
			}
		}
		else	// start_io or edit_io
		{
			if( input.state!=ok  ) { errno = input.state==EAGAIN  ? asynciopending : input.state;  return; }
			if( output.state!=ok ) { errno = output.state==EAGAIN ? asynciopending : output.state; return; }

		// query window size and start column for input
			input.cols = TTYGetWindowCols();
			if(errno||input.cols==0) { input.cols=80; errno=ok; }

		// colorize text on tty, print s and locate crsr
			if( resume==edit_io ) z = s.ToEscaped();
			else				  z.Clear();
			PutString(z);			// also colorizes text
			input.idx_soll = input.idx = z.Len();
			goto e3;
		}

	// do it
		for(;;)
		{

	e3:		input.edid = 3;
			if( errno ) return;

		// repositioning of cursor pending?
			input.idx_soll = minmax(0,input.idx_soll,(int)z.Len());
			if( input.idx > input.idx_soll ) { Write( SpaceString( input.idx - input.idx_soll, UCS4Char(8) ) ); input.idx = input.idx_soll; continue; }
			if( input.idx < input.idx_soll ) { PutString( z.SubString( input.idx, input.idx_soll ) ); input.idx = input.idx_soll; continue; }

		// get next character:
			GetChar(c1,start_io); while(errno) { input.edid=4; return; e4: GetChar(c1,resume_io); }

		// printable character?
			if ( UCS4CharIsPrintable(c1) )
			{
				z = z.LeftString(input.idx) + String(c1) + z.MidString(input.idx);
				input.idx_soll++;
				PutString( z.MidString(input.idx) );
				input.idx = z.Len();
				continue;
			}

		// stopctl:
	cr:		if( is_stopctl(c1,input.getstring_stopctls) )
			{
				PutString ( z.MidString(input.idx) );		// crsr ans Textende stellen
				s = z.FromEscaped();						// unescape ctls (if any)
			//	input.last_ctl = c1;						// provide stopctl		((note: input.last_ctl === c1))
				input.edid = 0;
				return;
			}

		// 9=TAB
			if (c1==9)
			{
				int n = (input.idx+3)%4 +1;
				z = z.LeftString(input.idx) + SpaceString(n) + z.MidString(input.idx);
				input.idx_soll += n;
				PutString( z.MidString(input.idx) );
				input.idx = z.Len();
				continue;
			}

		// 8=DEL or 127=RUBOUT
			if ( c1==8 || c1==0x7f )
			{
				if(input.idx==0) continue;
				input.idx_soll--; input.idx--;
				Write(vt100_bs,1);
				if(errno) { input.edid=5; return; } e5:

	d8:			z = z.LeftString(input.idx) + z.MidString(input.idx+1);
				PutString ( z.MidString(input.idx) + String(' ') );
				input.idx = z.Len()+1;
				continue;
			}

		// Anything else we know are ESC sequences:
			if (c1!=27) goto pb_c1;

	// ---- escape sequence

			GetChar(c2,start_io); while(errno==EAGAIN) { input.edid=6; return; e6: GetChar(c2,resume_io); }
			if(errno) goto pb_c1;

			if (c2=='O')	// Oh (not null): test for "cursor keys in application mode"
			{
				GetChar(c3,start_io); while(errno==EAGAIN) { input.edid=7; return; e7: GetChar(c3,resume_io); }
				if( errno ) goto pb_c12;
				switch(c3)
				{
					case 'A': /* crsr up   */	input.idx_soll -= input.cols; continue;
					case 'B': /* crsr down */	input.idx_soll += input.cols; continue;
					case 'C': /* crsr right*/	input.idx_soll += 1;			continue;
					case 'D': /* crsr left */	input.idx_soll -= 1;			continue;
					case 'M': /* shift return*/	c1 = 13; goto cr;			// <shift><return> => <return>
				}
				goto pb_c123;
			}

			if (c2!='[') goto pb_c12;		// unknown esc. seq.

	// ---- esc [ sequence

			GetChar(c3,start_io); while(errno==EAGAIN) { input.edid=8; return; e8: GetChar(c3,resume_io); }
			if(errno) goto pb_c12;

			switch(c3)
			{
			case 'C':	input.idx_soll += 1; continue;				// cursor right
			case 'B':	input.idx_soll += input.cols; continue;		// cursor down
			case 'F':	input.idx_soll  = z.Len(); continue;		// Ende
			case 'D':	input.idx_soll -= 1; continue;				// cursor left
			case 'A':	input.idx_soll -= input.cols; continue;		// cursor up
			case 'H':	input.idx_soll  = 0; continue;				// Pos1

			case '2':	// esc [ 2 ~	Einfügen
			case '3':	// esc [ 3 ~	Entfernen
			case '5':	// esc [ 5 ~	Pg Up
			case '6':	// esc [ 6 ~	Pg Down
				// expect '~'
				GetChar(c4,start_io); while(errno==EAGAIN) { input.edid=9; return; e9: GetChar(c4,resume_io); }
				if(errno) goto pb_c123;
				if(c4=='~')
				{
					if(c3=='2') { }					// "Einfügen": keine Bedeuting
					if(c3=='3')	{ if(input.idx<z.Len()) goto d8; continue; }	// "Entfernen"
					if(c3=='5') { }					// Pg Up: Back in history
					if(c3=='6') { }					// Pg Down: Forward in history
				}
				PutbackChar(c4); goto pb_c123;
			}

pb_c123:	PutbackChar  ( c3 );
pb_c12:		PutbackChar  ( c2 );
pb_c1:		PutbackString( String(c1).ToEscaped() );
			continue;
		}
	}
}


/* ----	receive cursor position ------------------------------------
		receive position after a VT100 cursor position request
		top,left = 1,1

		call TTYRequestPosition() before calling TTYReceivePosition()

		start with  resume = start_io
		resume with resume = resume_io until errno!=EAGAIN

		out:	errno = input.state (except asynciopending or ttynoresponse)
		errno:	ok:			position received and stored in row and col.
				EAGAIN:		wait and resume.
				asynciopending: Bummer: TTYReceivePosition(r,c,0) called with input or output still in progress.
				ttynoresponse:	Problem: received lots of data (preserved!) but no get_position response.

		note:	uses input.io_buffer
				the caller should ignore input.timeout and timeout after a few seconds only
*/
void Stream::TTYReceivePosition ( int& row, int& col, ResumeCode resume )
{
	assert(resume!=test_io);
	//XXTRAP(NoTTY());		<-- will just fail

	uint io;				// bytes received so far
	ptr a,e,pe,p;			// received data start, end, ptr to char(esc), work ptr
	char c;

	if( resume==start_io )
	{
		if( input.state!=ok ) { errno = input.state==EAGAIN ? asynciopending : input.state; return; }
		if( input.string.Len()<80||input.string.IsWritable()||input.string.Csz()!=csz1 ) input.string = String(80,ucs1);
		a  = input.string.Text();
		io = Read( a, 80, stopctls_none );		// -> error, EAGAIN or ok
	}
	else
	{
		errno = input.state;
		a  = input.string.Text();
		io = input.io_ptr - a;

		assert(input.string.Len()>=80&&input.string.NotWritable()&&input.string.Csz()==csz1);
		assert(io<=80);
	}
	e = a + io;
	p = a;

a:	row=0; col=0;

	if(p==e) goto x; if(*p!=27)  goto a; else pe = p-1;
	if(p==e) goto x; if(*p!='[') goto a;

b:	if(p==e) goto x; c = *p++; if(c==';') goto c;
	if(no_dec_digit(c)) goto a;
	row = row*10 + digit_val(c); goto b;

c:	if(p==e) goto x; c = *p++; if(c=='R') goto d;
	if(no_dec_digit(c)) goto a;
	col = col*10 + digit_val(c); goto c;

d:	if( p<e  ) input.io_buffer = String((UCS1Char*)p,e-p)  + input.io_buffer;
	if( pe>a ) input.io_buffer = String((UCS1Char*)a,pe-a) + input.io_buffer;
	if(errno==EAGAIN) input.state=ok; errno=ok;
	return;

// didn't finish:
x:	if(errno==ok)
	{
		errno = ttynoresponse;
		input.io_buffer = input.string.LeftString(io) + input.io_buffer;
	}
}





























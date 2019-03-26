/*	Copyright  (c)	Günter Woigk 2001 - 2017
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

	2003-08-23 kio	changes for eliminated Var type 'isVoid'
	2005-07-07 kio	rework Streams finished
	2005-07-10 kio	fixed tLIST
*/

#define SAFE 3
#define LOG 1
#include "config.h"
#include <math.h>
#include <signal.h>
#include <sys/time.h>
#include "unix/os_utilities.h"
#include "kio/peekpoke.h"
#include "freeze.h"
#include "token.h"
#include "SystemIO.h"
#include "VScript.h"


INIT_MSG


#ifdef OPCODE_PROFILING
ulong	opcodeProfile[256];
ulong	opcodeTupels[256][256];
#endif

#if defined(SIGIO) && defined(SIGPOLL)
	#error hmm hmm ...
#endif

const 	Double d_ln10 = log(10.0);
const 	Double d_ln2  = log( 2.0);
//const	Double d_ln16 = log(16.0);

sigset_t allsigs;	/* filled */
sigset_t nosigs;	/* cleared */

int		ignore_sigsegv = 0;		// ignore counter for signal SIGSEGV


/* ----	start TimeShed timer --------------------------
		the realtime timer is used to count down for the
		next sheduled thread in the t_timeshed list
*/
void StartRTimer ( double d )
{
	if(d>0.000010)	// 10 µs mind. wg. Timer-Granularity
	{
		xxlog("StartRTimer(%f)",d);
		if(d>9999.0) d=9999.0;

		struct itimerval itv;
		itv.it_interval.tv_sec	= 0;	// don't repeat
		itv.it_interval.tv_usec	= 0;
		itv.it_value.tv_sec		= (long)d;
		itv.it_value.tv_usec	= (long)(((d-(long)d)*1000000));
		if( setitimer(ITIMER_REAL, &itv, NULL) == 0 ) return;
		Log("StartRTimer() failed.");
		errno=ok;
	}

	disp_flag |= timeshed;
}


/* ----	i/o signal handler --------
		TODO: signal handler: get fd in non-Linux
		TODO: signal handler: handle POLL_MSG, POLL_ERR, POLL_PRI, POLL_HUP if necc.
		http://www.linux.se/showMan.php?TITLE=sigaction&SECTION=2
		note:  signal cause available in siginfo->si_code
		Linux: fd available in siginfo->si_fd
*/
#ifdef SIGPOLL
	static SIGPOLL_Handler ( int/*sig*/, siginfo_t* siginfo, void*/*user_context*/ )
	{
		switch( siginfo->si_code )
		{
		case POLL_MSG:	// input msg avail
		case POLL_ERR:	// i/o err
		case POLL_PRI:	// high prio input avail
		case POLL_HUP:	// device disconnected
		default:	{	OSErr e = errno; log("SIGPOLL_Handler(): not POLL_IN/POLL_OUT"); errno = e; } break;
		case POLL_OUT:	disp_flag |= signal_output; break;
		case POLL_IN:	disp_flag |= signal_input;  break;
		}
	}
#endif


static void CatchSignals(int signo)
{
	switch(signo)
	{
	case SIGALRM:	//	exit:		ITIMER_REAL alarm timer: SIGALRM_Handler()
		xxlog("{SIGALRM}");
		disp_flag |= timeshed;
		return;

#ifdef SIGIO
	case SIGIO:		//	exit:		i/o available:		handled in SIGIO_Handler()
		xxlog("{SIGIO}");
		disp_flag |= signal_io;
		return;
#endif

#ifdef SIGPOLL		// note: we do not get a siginfo_t here!
//	case SIGPOLL:	//	exit:		Pollable event:		handled in SIGPOLL_Handler()
//		xxlog("{SIGPOLL}");
//		disp_flag |= signal_io;
//		break;
#endif

	case SIGINT:	//	exit:		Terminal interrupt signal ^C.
		xxlog("{SIGINT}");
		disp_flag |= control_c;
		break;

	case SIGSEGV:	//	exit&dump:	Invalid memory reference.
		if (ignore_sigsegv) { ignore_sigsegv--; return; }
		xlog("{SIGSEGV}");
		exit(3); 	//	ignore flag not set or segv repeated

	case SIGSYS:	//	exit&dump: 	Bad system call.
	case SIGFPE:	//	exit&dump:	Erroneous arithmetic operation.
	case SIGILL:	//	exit&dump:	Illegal instruction.
	case SIGBUS:	//	exit&dump 	Access to an undefined portion of a memory object.
	case SIGABRT:	//	exit&dump:	Process abort signal.
	case SIGQUIT:	//	exit&dump:	Terminal quit signal.
	case SIGTRAP: 	//	exit&dump: 	Trace/breakpoint trap.
	case SIGXCPU: 	// 	exit&dump:	CPU time limit exceeded.
	case SIGXFSZ: 	// 	exit&dump:	File size limit exceeded.
		xlog("{signal(3) %i}",signo);
		exit(3);

	case SIGHUP: 	//	exit:		Hangup.
	case SIGPIPE:	//	exit:	 	Write on a pipe with no one to read it.
	case SIGTERM:	//	exit:	 	Termination signal.
	case SIGUSR1:	//	exit:	 	User-defined signal 1.
	case SIGUSR2:	//	exit:	 	User-defined signal 2.
	case SIGPROF: 	//	exit:	 	Profiling timer expired.
	case SIGVTALRM: //	exit:	 	Virtual timer expired.
	case SIGKILL:	//	exit:	 	Kill (cannot be caught or ignored).		kill -9 <pid>
		xlog("{signal(2) %i}",signo);
		exit(2);

	case SIGURG: 	//	ignore: 	High bandwidth data is available at a socket.
	case SIGCHLD:	//	ignore:		Child process terminated or stopped.
	case SIGCONT:	//	resume:	 	Continue executing, if stopped.
		break;

	case SIGSTOP:	//	suspend: 	Stop executing (cannot be caught or ignored).
	case SIGTSTP:	//	suspend: 	Terminal stop signal.
		xxlog("{SIGTSTP}",signo);
		break;

	case SIGTTIN: 	//	suspend: 	Background process attempting read.
	case SIGTTOU:	// 	suspend:	Background process attempting write.
		break;

	default:
		xlog("{unknown signal %i}",signo);
		exit(2);
	}
}


/* ----	start signal handling ---------------------------
*/
static void StartSignals ( )
{
	sigfillset ( &allsigs );
	sigemptyset( &nosigs  );

	struct sigaction sigact;
// additionally blocked signals during signal processing: none
	sigemptyset ( &sigact.sa_mask );

// signals ignored:
//	sigact.sa_handler = SIG_IGN;
//	sigaction(SIGXCPU,  &sigact, NULL);

// signals default action:
//	sigact.sa_handler = SIG_DFL;
//	sigaction(SIGXCPU,  &sigact, NULL);

// signals handled via sa_handler:		-> void(*)(int)
	sigact.sa_handler = CatchSignals;
	sigaction(SIGSEGV,  &sigact, NULL);
//	sigaction(SIGINT,   &sigact, NULL);
	sigaction(SIGALRM,  &sigact, NULL);
#ifdef SIGIO
	sigaction(SIGIO,	&sigact, NULL);
#endif

// signals handled via sa_sigaction:	-> 	void(*)(int,siginfo_t*,void*)
	sigact.sa_flags |= SA_SIGINFO;
#ifdef SIGPOLL
	sigact.sa_sigaction = SIGPOLLHandler;
	sigaction(SIGPOLL, &sigact, NULL);
#endif

// allow all signals now:
	sigprocmask(SIG_UNBLOCK, &allsigs, NULL);	// or: pthread_sigmask()
	errno=ok;								// might already been interrupted
}


/* ----	terminate signal handling -----------------------
		quick & dirty
*/
#if 0
static void StopSignals ( )
{
	sigprocmask(SIG_BLOCK, &allsigs, NULL);

	struct sigaction sigact;
	sigemptyset ( &sigact.sa_mask );
	sigact.sa_handler = SIG_DFL;
	for(int i=1;i;i<<=1) { sigaction(i, &sigact, NULL); }

	errno=ok;
}
#endif




static Var* FindVar ( NameHandle nh, Var* v )
{
	for(;;)
	{
		Var* r = v->FindItem(nh); if(r) return r;
		r = v; v = v->Parent(); if (v) continue;
		return nh == r->GetNameHandle() ? r : NULL;
	}
}




/* ----	compress string ----------------------------------------
		using the freeze algorithm
	note: freeze is character based and after compression you cannot tell
		from the compressed data whether it was csz1, csz2 or csz4.
		also it seems to be no good idea to extend the algorithm to remember the
		uncompressed character size, since it typically works on arbitrary binary data.
		it is up to the caller to know how to interpret the data.
		as a solution we simply keep the character size of the string
	problem: csz2|4 strings provide a buffer size multiple of 2|4
		additional bytes must be handled and must not be passed to melt()
	solution: compressed buffer cannot start with reference-to-template but
		does always start with verbatim-data which starts with byte %111xxxxx
		so we start with padding 0 .. 3 bytes before the compressed data
		for security reason we use %11001111 which would start a reference-to-template
		with offset of approx. -2 MB (in case the "initial data header" idea is realized)
*/
void FreezeString ( String& v )
{
	CharSize csz = v.Csz();

	if( csz>csz1 )
	{
		csz = v.ReqCsz();
		v.ResizeCsz(csz);
	}

	size_t qlen = v.Len() * csz;
	if (qlen<=4) return;							// compressed == uncompressed
	size_t zlen = (1+qlen/(1<<12)) * ((1<<12)+1);	// this _should_ be large enough for the worst case

	if (csz>csz1)
	{
		String s(v.Len(),csz);
		ptr z = s.Text();

		for( int i=0; i<csz; i++ )
		{
			for ( ptr q=v.Text()+i, e=z+s.Len(); z<e; ) { *z++=*q; q+=csz; }
		}

		v = s;
	}

	uptr qptr = (uptr)v.Text();
	uptr zptr = new uchar[zlen]; if(!zptr) { errno = outofmemory; return; }

	zlen = Freeze ( qptr, qlen, zptr, zlen );		// may set error "out of memory" for tables

	if(errno==ok)
	{
		assert( (*zptr|0x1F)==0xFF );	// compressed data does not start with verbatim-date !!!
		assert (  *zptr      !=0xCF );	// compressed data starts with reference-to-template with offset -2 MB !!
		int d = (-zlen) & (csz-1);
		assert( (zlen+csz-1)/csz*csz == (zlen+d) );
		v = emptyString;
		v = String((zlen+d)/csz,csz);
		assert(v.Csz() == csz);
		*(long*)(v.Text()) = 0xCFCFCFCF;			// 4x %11001111
		memcpy(v.Text()+d,zptr,zlen);
	}

	delete[] zptr;
}


void MeltString ( String& v )
{
	CharSize csz = v.Csz();
	if (csz>csz1)
	{
		csz = v.ReqCsz();
		if (csz<v.Csz())
		{
			Log("MeltString(): warning: csz seems increased");
			v.ResizeCsz(csz);
		}
	}

	size_t qlen = v.Len() * csz;
	if (qlen<=4) return;							// compressed == uncompressed
	uptr qptr = (uptr)v.Text();
	while( qlen && *qptr==0xCF ) { qptr++; qlen--; }
	size_t zlen = MeltSize(qptr,qlen); if (errno) return;
	if (zlen%csz) { errno = corrupteddata; return; }

	String z(zlen/csz,csz);
	uptr zptr = (uptr)z.Text();

	zlen = Melt ( qptr, qlen, zptr, zlen );			// may set error "out of memory" for tables

	if(errno==ok)
	{
		assert(zlen == MeltSize(qptr,qlen));

		if (csz==csz1)
		{
			v = z;
		}
		else
		{
			v = emptyString;
			v = String(zlen/csz,csz);

			ptr q = z.Text();

			for( int i=0; i<csz; i++ )
			{
				for ( ptr z=v.Text()+i, e=q+v.Len(); q<e; ) { *z=*q++; z+=csz; }
			}
		}
	}
}


int GetEncodingFromName ( String conv_name )
{
	String s = conv_name.ToLower();
	for (int i=0;(i=s.Find('_',i))>=0;) s=s.LeftString(i)+s.MidString(i+1);

	//				xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx xxxxxxx
	int n = String("quoted  escaped printab html    utf8    upperca lowerca macroma us      asciius "
				   "german  asciige atari   atarist rtos    dos     cp437   amiga   windows archime "
				   "ucs1    ucs2    ucs4    upper   lower   url     urlall  mime").Find(s.LeftString(7));
	if (n>=0)
	{
		const CharEncoding e[] =
				{ quoted, escaped, escaped, html, utf8, uppercase, lowercase, mac_roman, ascii_us, ascii_us,
				  ascii_ger, ascii_ger, atari_st,atari_st, rtos, cp_437, cp_437, iso_latin_1, iso_latin_1, iso_latin_1,
				  ucs1, ucs2, ucs4, uppercase, lowercase, url, url_all, mime };
		return e[n/8];
	}

	if (LeftString(s,3)=="iso"||LeftString(s,5)=="latin")
	{
		if (no_dec_digit(s[s.Len()-2]))
			switch(s[s.Len()-1])
			{
			case '1':	return iso_latin_1;
			}
	}

	if( (s.Len()==4&&LeftString(s,3)=="tab") || (s.Len()==5&&LeftString(s,4)=="tabs") )
	{
		UCS4Char c = s[s.Len()-1];
		if (c>='1'&&c<='9') return CharEncoding( c -'1' +tab1 );
	}

	SetError( String("unknown conversion: ") + conv_name );
	return utf8;
}



cstr VScript::tostr(uchar*tok,Var**constants)
{
	switch(*tok)
	{
//	case tNUM0:	return "0";
//	case tNUM1: return "1";
	case tNUM:  return numstr(constants[peek1(tok+1)]->Value());
	case tSTR:  return CString(constants[peek1(tok+1)]->Text().ToQuoted());
	case tIDF:	return catstr("idf:",CString(GetNameForHandle(NameHandle(peek4(tok+1)))));
 	default:	return CString(tokenname[*tok]);
	}
}





#define PRIOSHIFT	12
#define	PRIOMULT	(1<<PRIOSHIFT)


/*	Execute:

	resumes current state
		which must be a task switching intermediate state

	and runs until
		a signal is encountered
			then the current instruction is completed
			and the state of the current thread is saved as for a task switch
			return value: 	NULL
			errno:		  	EINTR

		an error in the main thread	is not catched:
			the state of the current thread is saved as for a task switch
			a return to tTERMI is pushed on the rstack
			return value:	NULL
			errno:			error code
			errstr:			error msg
			errobj:			object or file name

		tEOF is executed
			return value: 	vstack empty:	NULL
							else:			vstack.top()
			errno:		  	OK

		tEND is executed
			return value:	tEND.0:			NULL
							tEND.1:			vstack.top()
			errno:			OK

		tRETURN on ground level
			return value:	tRETURN.0		NULL
							tRETURN.1		vstack.top()
			errno:			OK
*/

#define TopPtr				(vp[0])
#define Top					(*TopPtr)
#define Arg2Ptr				(vp[-1])
#define Arg2				(*Arg2Ptr)
#define Arg3Ptr				(vp[-2])
#define Arg3				(*Arg3Ptr)
#define ArgNPtr(N)			(vp[1-(N)])
#define ArgN(N)				(*ArgNPtr(N))

#define	Push(VPTR)			{ vp++; /*XXXTRAP(vp>=(Var**)rp);*/ TopPtr=VPTR; Top.lock(); }
#define	Dup()				{ vp++; TopPtr=Arg2Ptr; Top.lock(); }
#define Drop()				{ Top.unlock(); /*TopPtr=NULL;*/ vp--; }
#define	Nip()				{ Arg2.unlock(); Arg2Ptr=TopPtr; /*TopPtr=NULL;*/ vp--; }
#define Drop2()				{ Drop(); Drop(); }
#define DropPush(VPTR)		{ Var*VP=VPTR; VP->lock(); Top.unlock(); TopPtr=VP; }			// note: Reihenfolge stellt sicher, dass VPTR gelockt bleibt,
#define Drop2Push(VPTR)		{ Var*VP=VPTR; VP->lock(); Drop(); Top.unlock(); TopPtr=VP; }	//		 auch wenn VPTR ein item in Top.List() oder Arg2.List() ist.
//#define Deref(VPTR)			{ if(VPTR->IsVarRef()&&*ip!=tNODEREF) VPTR=VPTR->GetVarRef(); }
#define Deref(VPTR)			( (VPTR)->IsVarRef()&&*ip!=tNODEREF ? (VPTR)->GetVarRef() : VPTR )

/* ----	Klassifizierung von Variablen auf dem vstack -------------------
	variablen:				additionally locked by parent, 			data_unlocked
	konstanten:				additionally locked by constants.list, 	data_locked
	temp:					only on vstack => no double lock; 		data & type unlocked
	in use:					locked by parent, locked by s/o else, locked by Top
*/
#define	TopTypeIsLocked()	(Top.type_is_locked())
#define	Arg2TypeIsLocked()	(Arg2.type_is_locked())

#define	TopIsConst()		(Top.data_is_locked())
#define	TopIsNoConst()		(Top.data_is_unlocked())
#define	Arg2IsConst()		(Arg2.data_is_locked())
#define	Arg3IsConst()		(Arg3.data_is_locked())

#define	TopIsTemp()			(Top.is_not_double_locked())
#define	TopIsNoTemp()		(Top.is_double_locked())
#define	Arg2IsNoTemp()		(Arg2.is_double_locked())
#define	Arg2IsTemp()		(Arg2.is_not_double_locked())
#define	Arg3IsNoTemp()		(Arg3.is_double_locked())
#define	ArgNIsNoTemp(N)		(ArgN(N).is_double_locked())

#define	TopIsVar()			(TopIsNoTemp() && TopIsNoConst())
#define	TopIsNoVar()		(TopIsTemp()   || TopIsConst())
#define	Arg2IsNoVar()		(Arg2IsTemp()  || Arg2IsConst())

// Asserts:

#ifndef NDEBUG
  #define Assert1Arg()			if(vp<va)		goto ASSERT_VSTACK;
  #define Assert2Args()			if(vp-1<va)		goto ASSERT_VSTACK;
  #define Assert3Args()			if(vp-2<va)		goto ASSERT_VSTACK;
  #define AssertNArgs(N)		if(vp-(N)+1<va)	goto ASSERT_VSTACK;

  #define TopAssertVar()		if(TopIsNoVar())	goto ASSERT_ISVAR;
  #define Arg2AssertVar()		if(Arg2IsNoVar())	goto ASSERT_ISVAR;

  #define TopAssertNonConst() 	if(TopIsConst())	goto ASSERT_ISNONCONST
  #define Arg2AssertNonConst() 	if(Arg2IsConst())	goto ASSERT_ISNONCONST
  #define Arg3AssertNonConst() 	if(Arg3IsConst())	goto ASSERT_ISNONCONST

  #define TopAssertTemp()		if(TopIsNoTemp())	goto ASSERT_ISTEMP
  #define Arg2AssertTemp()		if(Arg2IsNoTemp())	goto ASSERT_ISTEMP
  #define Arg3AssertTemp()		if(Arg3IsNoTemp())	goto ASSERT_ISTEMP
  #define ArgNAssertTemp(N)		if(ArgNIsNoTemp(N)) goto ASSERT_ISTEMP

  #define TopAssertString()		if(Top.IsNoText())	  goto ASSERT_ISSTRING
  #define Arg1AssertString()	if(Arg1.IsNoText())	  goto ASSERT_ISSTRING
  #define Arg2AssertStream()	if(Arg2.IsNoStream()) goto ASSERT_ISSTREAM
  #define Arg3AssertStream()	if(Arg3.IsNoStream()) goto ASSERT_ISSTREAM
  #define Arg2AssertList()		if(Arg2.IsNoList())	  goto ASSERT_ISLIST
  #define TopAssertNumber()		if(Top.IsNoNumber())  goto ASSERT_ISNUMBER
#else
  #define Assert1Arg()			if(0) goto ASSERT_VSTACK;
  #define Assert2Args()			if(0) goto ASSERT_VSTACK;
  #define Assert3Args()			if(0) goto ASSERT_VSTACK;
  #define AssertNArgs(N)		if(0) goto ASSERT_VSTACK;

  #define TopAssertVar()		if(0) goto ASSERT_ISVAR;
  #define Arg2AssertVar()		if(0) goto ASSERT_ISVAR;

  #define TopAssertNonConst() 	if(0) goto ASSERT_ISNONCONST
  #define Arg2AssertNonConst() 	if(0) goto ASSERT_ISNONCONST
  #define Arg3AssertNonConst() 	if(0) goto ASSERT_ISNONCONST

  #define TopAssertTemp()		if(0) goto ASSERT_ISTEMP
  #define Arg2AssertTemp()		if(0) goto ASSERT_ISTEMP
  #define Arg3AssertTemp()		if(0) goto ASSERT_ISTEMP
  #define ArgNAssertTemp(N)		if(0) goto ASSERT_ISTEMP

  #define TopAssertString()		if(0) goto ASSERT_ISSTRING
  #define Arg1AssertString()	if(0) goto ASSERT_ISSTRING
  #define Arg2AssertStream()	if(0) goto ASSERT_ISSTREAM
  #define Arg3AssertStream()	if(0) goto ASSERT_ISSTREAM
  #define Arg2AssertList()		if(0) goto ASSERT_ISLIST
  #define TopAssertNumber()		if(0) goto ASSERT_ISNUMBER
#endif

// Runtime type checks:

#define TopReqNonConst()		if(TopIsConst()) goto ERR_NONCONSTREQ
#define Arg2ReqNonConst()		if(Arg2IsConst()) goto ERR_NONCONSTREQ
#define Arg3ReqNonConst()		if(Arg3IsConst()) goto ERR_NONCONSTREQ

#define	TopReqTypeUnlocked()	if(TopTypeIsLocked()) goto ERR_VTYPELOCKED
#define	Arg2ReqTypeUnlocked()	if(Arg2TypeIsLocked()) goto ERR_VTYPELOCKED

#define	TopReqVar()				if(TopIsNoVar()) goto ERR_VARREQ;
#define	Arg2ReqVar()			if(Arg2IsNoVar()) goto ERR_VARREQ;

#define	TopReqTemp()			if(TopIsNoTemp())  { Top.decr_lock();  TopPtr=new Var(Top);   Top.incr_lock();  }
#define	Arg2ReqTemp()			if(Arg2IsNoTemp()) { Arg2.decr_lock(); Arg2Ptr=new Var(Arg2); Arg2.incr_lock(); }
#define	Arg3ReqTemp()			if(Arg3IsNoTemp()) { Arg3.decr_lock(); Arg3Ptr=new Var(Arg3); Arg3.incr_lock(); }

#define	TopReqNotInUse()		if(TopIsInUse()) goto ERR_VARINUSE
#define	Arg2ReqNotInUse()		if(Arg2IsInUse()) goto ERR_VARINUSE
#define TopReqNumber()			if(Top.IsNoNumber()) goto ERR_NUMREQ
#define Arg2ReqNumber()			if(Arg2.IsNoNumber()) goto ERR_NUMREQ
#define TopReqString()			if(Top.IsNoText()) goto ERR_STRREQ
#define Arg2ReqString()			if(Arg2.IsNoText()) goto ERR_STRREQ
#define Arg3ReqString()			if(Arg3.IsNoText()) goto ERR_STRREQ
#define TopReqStringList()		if(Top.IsNoText()&&Top.IsNoList()) goto ERR_STRORLISTREQ
#define Arg2ReqStringList()		if(Arg2.IsNoText()&&Arg2.IsNoList()) goto ERR_STRORLISTREQ
#define TopReqTid()				if(Top.IsNoThread()) goto ERR_TIDREQ
#define Arg2ReqTid()			if(Arg2.IsNoThread()) goto ERR_TIDREQ
#define TopReqIrpt()			if(Top.IsNoIrpt()) goto ERR_IRPTREQ
#define Arg2ReqIrpt()			if(Arg2.IsNoIrpt()) goto ERR_IRPTREQ
#define TopReqSema()			if(Top.IsNoSema()) goto ERR_SEMAREQ
#define Arg2ReqSema()			if(Arg2.IsNoSema()) goto ERR_SEMAREQ
#define TopReqProc()			if(Top.IsNoProc()) goto ERR_PROCREQ
#define ArgNReqProc(N)			if(ArgN(N).IsNoProc()) goto ERR_PROCREQ
#define ArgNReqString(N)		if(ArgN(N).IsNoText()) goto ERR_STRREQ
#define TopReqList()			if(Top.IsNoList()) goto ERR_LISTREQ
#define TopReqStream()			if(Top.IsNoStream()) goto ERR_STREAMREQ
#define Arg2ReqStream()			if(Arg2.IsNoStream()) goto ERR_STREAMREQ
#define Arg3ReqStream()			if(Arg3.IsNoStream()) goto ERR_STREAMREQ
#define Arg2ReqList()			if(Arg2.IsNoList()) goto ERR_LISTREQ


static NameHandle	nh_error;
static NameHandle	nh_error_number;
static NameHandle	nh_error_text;
static NameHandle	nh_error_stack;
static NameHandle	nh_error_row;
static NameHandle	nh_error_col;
static NameHandle	nh_error_file;
static NameHandle	nh_error_info;
static NameHandle	nh_privates;
static NameHandle	nh_locals;




Var* VScript::Execute ( )
{
	xlogIn("VScript::Execute()");

	StartSignals();

	nh_privates 	= NewNameHandle("privates");
	nh_locals   	= NewNameHandle("locals");
	nh_error		= NewNameHandle("error");
	nh_error_number	= NewNameHandle("number");
	nh_error_text	= NewNameHandle("message");
	nh_error_stack	= NewNameHandle("stack");
	nh_error_row	= NewNameHandle("row");
	nh_error_col	= NewNameHandle("col");
	nh_error_file	= NewNameHandle("file");
	nh_error_info	= NewNameHandle("info");

/* cache frequently used variables
*/
	uchar*		ip;			// instruction pointer
	Var**		vp;			// vstack pointer
	StackData*	rp;			// rstack pointer
	#define va	((Var**)(thread->stack))
	#define	ve	((Var**)rp)
	#define	ra	((StackData*)vp+1)
	#define re	(thread->stack_end)
	assert(sizeof(Var*)==sizeof(StackData));

	Thread*	thread;			// the thread which the registers are loaded for
	Var**	constants;		// thread.proc.bundle.constants.list.array

#ifdef OPCODE_PROFILING
	uchar	last_opcode = 255;
#endif

#define	SaveRegisters()									\
	thread->ip = ip;		/* instruction pointer	*/	\
	thread->vp = vp;		/* vstack pointer		*/	\
	thread->rp = rp;		/* rstack pointer		*/	\

#define	LoadRegisters()									\
	ip = thread->ip;		/* instruction pointer	*/	\
	vp = thread->vp;		/* vstack pointer		*/	\
	rp = thread->rp;		/* rstack pointer		*/	\
	constants = thread->proc->ProcConstants();

//	start vip
	thread = t_running; if(thread) goto load_regs;
	thread = t_root;	assert(t_root);
	LoadRegisters();
	goto test_disp;






#define LOOP	goto exe2
#define	DISP	goto test_disp
#define	SWITCH	goto test_switch
#define	DROP	goto exe4
#define	DROP2	goto exe5
#define	QUICK	goto exe6
#define	ERROR	goto error_exit


// no thread is willing to run  =>  sleep
sleep:
	{
		sigprocmask(SIG_BLOCK,&allsigs,NULL);

		if(!disp_flag)
		{
			xxlog("<<sleep>>");
			#if defined(SIGPOLL) || 1
				sigsuspend(&nosigs);			// signal handler werden aufgerufen => kein Problem mit SIGPOLL
				errno=ok;
			#else
				int sig;
				sigwait(&allsigs,&sig);			// signal handler werden nicht aufgerufen => kein siginfo_t für SIGPOLL!
				CatchSignals(sig);
			#endif
			xxlog("<<wakeup>>");
		}

		sigprocmask(SIG_UNBLOCK,&allsigs,NULL);
	}


// check dispatcher flag
test_disp:
	if(disp_flag)
	{
		long d = disp_flag; disp_flag &= ~d;

	// countdown timer expired
		if(d&timeshed)
		{
			double systemtime = SystemTime();
			while( t_timeshed )
			{
				if( t_timeshed->time <= systemtime )
				{
					t_timeshed->link_running();
				}
				else
				{
					StartRTimer(t_timeshed->time-systemtime);
					break;
				}
			}
		}

	// asynchronous interrupt
		#if defined(WITH_ASYNC_IRPT) || 0
			if(d&irptshed)
			{
				for(uint i=0;i<NELEM(asyncCnt);i++)
				{
					int n = asyncCnt[i];
					if (n)
					{
						asyncIrpt[i]->Trigger(n);
						asyncCnt[i] -= n;
					}
				}
			}
		#endif

	// async i/o
		#ifdef SIGPOLL
			if(d&signal_input)  Stream::PollPendingInput();
			if(d&signal_output) Stream::PollPendingOutput();
		#else
			if(d&signal_io)		Stream::PollPendingIO();
		#endif
	}


// switch registers on thread switch:
test_switch:
	if (thread==t_running) QUICK;
	if (!t_running) goto sleep;

	xxlog("<<switch_regs>>");

	SaveRegisters();
	if(t_zombie) { assert(t_zombie!=t_running); delete t_zombie; t_zombie=NULL; }
	thread = t_running;

load_regs:
	assert(thread);
	LoadRegisters();
	assert(thread->globals->type_is_locked());
	assert(thread->locals->type_is_locked());
	assert(root->data_is_locked());
	QUICK;


/* ==== main entry point for virtual instruction interpreter loop ====
*/
exe5:	Drop();
exe4:	Drop();

exe2:
	if (errno)
		ERROR;

exe6:

	IFDEBUG( if(errno)
	{
		PrependToError("IERR: error after QUICK: ");
		ERROR;
	} )

	xxlog("<%s>",tostr(ip,constants));

	#ifdef OPCODE_PROFILING
	if(verbose)
	{
		opcodeProfile[uchar(*ip)]++;
		opcodeTupels[last_opcode][uchar(*ip)]++;
		last_opcode = *ip;
	}
	#endif

	#ifndef NDEBUG
	{
		if(vp+1<va)		goto ERR_VSTACKUNDERFLOW;
		if(rp>=re)		goto ERR_RSTACKUNDERFLOW;
	}
	{
		if(rp<ra)		goto ERR_STACKOVERFLOW;
	}
	#endif

	assert(root->ListSize() >= 5);

	if(XXLOG)
	{
		Log("{%s}",tokenname[*ip].CString());
	}

	switch((TokenEnum)*ip++)
	{
	case tDUP:
		Assert1Arg();
		Dup();
	case tNODEREF:			// before tARROW
		QUICK;

	case tDROP:
		Assert1Arg();
		Drop();
		QUICK;

	case tSWAPARGS:
		Assert2Args();
		kio::swap(TopPtr,Arg2Ptr);
		QUICK;

	case tREQPROC:			//	assertion
		Assert1Arg();
		if(Top.IsProc()) QUICK;
		goto ERR_PROCREQ;

	case tREQSTRING:		//	assertion
		Assert1Arg();
		if(Top.IsText()) QUICK;
		goto ERR_STRREQ;

	case tREQVAR:			//	assertion
		Assert1Arg();
		if(TopIsVar()) QUICK;
		goto ERR_VARREQ;

	case tREQTEMP:			// 	cond. conversion
		Assert1Arg();
		if(TopIsTemp()) QUICK;
	case tTOTEMP:			// 	uncond. conversion
		Assert1Arg();
		assert(TopIsNoTemp());
		Top.decr_lock();
		TopPtr = new Var(Top);
		Top.incr_lock();
		QUICK;

	case tNUM0:				// 	tNUM0 -- <const>
		Push(zero);
		QUICK;

	case tNUM1:				// 	tNUM1 -- <const>
		Push(eins);
		QUICK;

	case tSTR:				// 	tSTR.n -- <const>
	case tNUM:				// 	tNUM.n -- <const>
		Push(constants[*ip++]);
		QUICK;

	case tNUM2:			// 	tNUM2.nn -- <const>
		Push(constants[(ushort)peek2(ip)]); ip+=2;
		QUICK;

	case tPOSTINCR:		// <var> tPOSTINCR -- <temp>
		Assert1Arg();
		TopReqVar();
	{	Var* a = new Var(Top);
		Top += 1.0; Top.decr_lock();
		TopPtr = a; a->incr_lock();
	}	QUICK;

	case tPOSTDECR:		// <var> tPOSTDECR -- <temp>
		Assert1Arg();
		TopReqVar();
	{	Var* a = new Var(Top);
		Top -= 1.0; Top.decr_lock();
		TopPtr = a; a->incr_lock();
	}	QUICK;

	case tPREINCR:		// <var> tPREINCR -- <var>
		Assert1Arg();
		TopAssertVar();
		Top += 1.0;
		QUICK;

	case tPREDECR:		// <var> tPREINCR -- <var>
		Assert1Arg();
		TopAssertVar();
		Top -= 1.0;
		QUICK;

	case tGL:			// 	<var> <temp> tGL --
		Assert2Args();
		TopAssertTemp();
		Arg2AssertNonConst();
		assert(Arg2Ptr!=root);
		assert(TopPtr!=root);
		assert(t_root!=NULL);
	#if 0
		Arg2.MoveData(Top);			// <-- tests source for type.locked -->  proc(){ … return locals} fails!
		XXXTRAP(t_root==NULL);
		DROP2;
	#else
		assert( Top.parent == NULL );
		assert (!(Top.IsList() && Top.Contains(Arg2)));		// <-- TODO: kann das vorkommen?

		if( Arg2.type_and_data_unlocked() || (Arg2.data_is_unlocked() && Top.get_type()==Arg2.get_type()) )
		{
			Arg2.kill_data(); Arg2.move_data(Top); Top.init_data();
		}
		else
		{
			Arg2.set_error_locked();
		}
		DROP2;
	#endif

	case tARROW:			// 	<var> <var> tARROW --
		Assert2Args();
		TopAssertVar();
		Arg2AssertNonConst();
		assert(Arg2Ptr!=root);
		assert(TopPtr!=root);
		assert(t_root!=NULL);
		assert(Top.parent != NULL);
		if( Top.IsList() && Top.Contains(Arg2) ) goto ERR_VREFCHILDOFDEST;	// TODO: crosswise child of list

		if( Arg2.type_and_data_unlocked() || (Arg2.data_is_unlocked() && Top.get_type()==Arg2.get_type()) )
		{
			Arg2.SetVarRef(TopPtr);
		}
		else
		{
			Arg2.set_error_locked();
		}
		DROP2;

	case tPLGL:			// 	<var> <value> tPLGL --
		Assert2Args();
		Arg2AssertNonConst();
		if(Top.IsNumber() && Arg2.IsNumber()) Arg2.Value() += Top.Value();
		else								  Arg2		   += Top;
		DROP2;

	case tMIGL:			// 	<var> <value> tMIGL --
		Assert2Args();
		Arg2AssertNonConst();
		if(Top.IsNumber() && Arg2.IsNumber()) Arg2.Value() -= Top.Value();
		else								  Arg2		   -= Top;
		DROP2;

	case tMUGL:			// 	<var> <value> tMUGL --
		Assert2Args();
		Arg2AssertNonConst();
		if(Top.IsNumber() && Arg2.IsNumber()) Arg2.Value() *= Top.Value();
		else								  Arg2		   *= Top;
		DROP2;

	case tDVGL:			//  <var> <value> tDVGL --
		Assert2Args();
		Arg2AssertNonConst();
		if(Top.IsNumber() && Arg2.IsNumber()) Arg2.Value() /= Top.Value();
		else								  Arg2		   /= Top;
		DROP2;

	case tMODGL:		// 	<var> <value> tMODGL --
		Assert2Args();
		Arg2AssertNonConst();
		Arg2 %= Top;
		DROP2;

	case tHASHGL:		// 	<var> <value> tHASHGL --
		Assert2Args();
		Arg2AssertNonConst();
		if(Top.IsText()&&Arg2.IsText()) Arg2.Text() += Top.Text();
		else Arg2.AppendString(Top,DisassString);
		DROP2;

	case tGTGTGL:		// 	<var> <value> tGTGTGL --
		Assert2Args();
		Arg2AssertNonConst();
		Arg2 >>= Top;
		DROP2;

	case tLTLTGL:		// 	<var> <value> tLTLTGL --
		Assert2Args();
		Arg2AssertNonConst();
		Arg2 <<= Top;
		DROP2;

	case tXORGL:		// 	<var> <value> tXORGL --
		Assert2Args();
		Arg2AssertNonConst();
		Arg2 ^= Top;
		DROP2;

	case tANDGL:		// 	<var> <value> tANDGL --
		Assert2Args();
		Arg2AssertNonConst();
		Arg2 &= Top;
		DROP2;

	case tORGL:			// 	<var> <value> tORGL --
		Assert2Args();
		Arg2AssertNonConst();
		Arg2 |= Top;
		DROP2;

	case tHASHHASHGL:	// 	<var> <temp> tHASHHASHGL --
		Assert2Args();
		Arg2AssertNonConst();
		TopAssertTemp();
		Arg2.AppendListItems(Top);
		DROP2;

	case tADD:			//	<temp> <value> tADD -- <temp>
		Assert2Args();
		Arg2AssertTemp();
		if(Top.IsNumber())
			if(Arg2.IsNumber()) Arg2.Value() += Top.Value();
			else                Arg2         += Top.Value();
		else					Arg2		 += Top;
		DROP;

	case tSUB:			//	<temp> <value> tSUB -- <temp>
		Assert2Args();
		Arg2AssertTemp();
		if(Top.IsNumber())
			if(Arg2.IsNumber()) Arg2.Value() -= Top.Value();
			else                Arg2         -= Top.Value();
		else					Arg2		 -= Top;
		DROP;

	case tMUL:			//	<temp> <value> tMUL -- <temp>
		Assert2Args();
		Arg2AssertTemp();
		if(Top.IsNumber())
			if(Arg2.IsNumber()) Arg2.Value() *= Top.Value();
			else                Arg2         *= Top.Value();
		else					Arg2		 *= Top;
		DROP;

	case tDIV:			//	<temp> <value> tDIV -- <temp>
		Assert2Args();
		Arg2AssertTemp();
		if(Top.IsNumber())
			if(Arg2.IsNumber()) Arg2.Value() /= Top.Value();
			else                Arg2         /= Top.Value();
		else					Arg2		 /= Top;
		DROP;

	case tMOD:			//	<temp> <value> tMOD -- <temp>
		Assert2Args();
		Arg2AssertTemp();
		Arg2 %= Top;
		DROP;

	case tOR:			//	<temp> <value> tOR -- <temp>
		Assert2Args();
		Arg2AssertTemp();
		Arg2 |= Top;
		DROP;

	case tXOR:			//	<temp> <value> tXOR -- <temp>
		Assert2Args();
		Arg2AssertTemp();
		Arg2 ^= Top;
		DROP;

	case tAND:			//	<temp> <value> tAND -- <temp>
		Assert2Args();
		Arg2AssertTemp();
		Arg2 &= Top;
		DROP;

	case tLTLT:			//	<temp> <value> tLTLT -- <temp>
		Assert2Args();
		Arg2AssertTemp();
		Arg2 <<= Top;
		DROP;

	case tGTGT:			//	<temp> <value> tGTGT -- <temp>
		Assert2Args();
		Arg2AssertTemp();
		Arg2 >>= Top;
		DROP;

	{	bool f;

	case tEQ:			//	<value> <value> tEQ -- <const>
		Assert2Args();
		f = Arg2 == Top;
top_f:	Drop2Push(f?eins:zero);
		LOOP;

	case tNE:			//	<value> <value> tNE -- <const>
		Assert2Args();
		f = Arg2 != Top;
		goto top_f;

	case tGT:			//	<value> <value> tGT -- <const>
		Assert2Args();
		f = Arg2 > Top;
		goto top_f;

	case tGE:			//	<value> <value> tGE -- <const>
		Assert2Args();
		f = Arg2 >= Top;
		goto top_f;

	case tLT:			//	<value> <value> tLT -- <const>
		Assert2Args();
		f = Arg2 < Top;
		goto top_f;

	case tLE:			//	<value> <value> tLE -- <const>
		Assert2Args();
		f = Arg2 <= Top;
		goto top_f;

	}	// Bool f

	case tHASH:			//	<temp> <value> tHASH -- <temp>
		Assert2Args();
		Arg2AssertTemp();
		if(Top.IsText()&&Arg2.IsText()) Arg2.Text() += Top.Text();
		else Arg2.AppendString(Top,DisassString);
		DROP;

	case tHASHHASH:		//	<temp> <temp> tHASHHASH -- <temp>
		Assert2Args();
		Arg2AssertTemp();
		TopAssertTemp();
		Arg2.AppendListItems(Top);
		assert(Arg2.IsList());
		DROP;

	case tNEG:			// <temp> tNEG -- <temp>
		Assert1Arg();
		TopAssertTemp();
		TopReqNumber();
		Top.Value() = -Top.Value();		 	// TODO list expansion?
		QUICK;

	case tNOT:			// <temp> tNOT -- <temp>
		Assert1Arg();
		TopAssertTemp();
		TopReqNumber();
		Top.Value() = !Top.Value();			// TODO list expansion?
		QUICK;

	case tTILDE:		// <temp> tTILDE -- <temp>
		Assert1Arg();
		TopAssertTemp();
		TopReqNumber();
		Top.Value() = -1.0 -Top.Value(); 	// TODO list expansion?
		LOOP;

	case tABS:			// <temp> tABS -- <temp>
		Assert1Arg();
		TopAssertTemp();
		TopReqNumber();
		Top.Value() = abs(Top.Value());
		QUICK;

	case tSIGN:			// <temp> tSIGN -- <temp>
		Assert1Arg();
		TopAssertTemp();
		TopReqNumber();
		Top.Value() = sign(Top.Value());
		QUICK;

	case tROUND:		// <temp> tROUND -- <temp>
		Assert1Arg();
		TopAssertTemp();
		TopReqNumber();
		Top.Value() = floor(0.5+Top.Value());
		QUICK;

	case tFRACT:		// <temp> tFRACT -- <temp>
		Assert1Arg();
		TopAssertTemp();
		TopReqNumber();
		{ Double i; Top.Value() = modf(Top.Value(),&i); }
		QUICK;

	case tINT:			// <temp> tINT -- <temp>
		Assert1Arg();
		TopAssertTemp();
		TopReqNumber();
		{ Double i; modf(Top.Value(),&i); Top.Value()=i; }
		QUICK;

	case tSIN:			// <temp> tSIN -- <temp>
		Assert1Arg();
		TopAssertTemp();
		TopReqNumber();
		Top.Value() = sin(Top.Value());
		QUICK;

	case tCOS:			// <temp> tCOS -- <temp>
		Assert1Arg();
		TopAssertTemp();
		TopReqNumber();
		Top.Value() = cos(Top.Value());
		QUICK;

	case tTAN:			// <temp> tTAN -- <temp>
		Assert1Arg();
		TopAssertTemp();
		TopReqNumber();
		Top.Value() = tan(Top.Value());
	//	errno=ok;				// ERANGE for inf  -  actually never happened  ((~1e16))
		QUICK;

	case tASIN:			//	<temp> tASIN -- <temp>
		Assert1Arg();
		TopAssertTemp();
		TopReqNumber();
		Top.Value() = asin(Top.Value());	// EDOMAIN
		LOOP;

	case tACOS:			//	<temp> tACOS -- <temp>
		Assert1Arg();
		TopAssertTemp();
		TopReqNumber();
		Top.Value() = acos(Top.Value());	// EDOMAIN
		LOOP;

	case tATAN:			//	<temp> tATAN -- <temp>
		Assert1Arg();
		TopAssertTemp();
		TopReqNumber();
		Top.Value() = atan(Top.Value());	// EDOMAIN
		LOOP;

	case tSINH:			//	<temp> tASINH -- <temp>
		Assert1Arg();
		TopAssertTemp();
		TopReqNumber();
		Top.Value() = sinh(Top.Value());
		if(errno==ok) QUICK;
		if (errno==ERANGE) errno=ok;	// inf
		LOOP;

	case tCOSH:			//	<temp> tCOSH -- <temp>
		Assert1Arg();
		TopAssertTemp();
		TopReqNumber();
		Top.Value() = cosh(Top.Value());
		if(errno==ok) QUICK;
		if (errno==ERANGE) errno=ok;	// inf or 0
		LOOP;

	case tTANH:			//	<temp> tTANH -- <temp>
		Assert1Arg();
		TopAssertTemp();
		TopReqNumber();
		Top.Value() = tanh(Top.Value());		// error possible?
		LOOP;

	case tSQUARE:			//	<temp> tSQUARE -- <temp>
		Assert1Arg();
		TopAssertTemp();
		TopReqNumber();
		Top.Value() = Top.Value() * Top.Value();	// ERANGE ?
		LOOP;

	case tSQRT:			//	<temp> tSQRT -- <temp>
		Assert1Arg();
		TopAssertTemp();
		TopReqNumber();
		Top.Value() = sqrt(Top.Value());		// EDOMAIN ?
		LOOP;

	case tRANDOM:		//	<temp> tRANDOM -- <temp>		returns non-integer result!!
		Assert1Arg();
		TopAssertTemp();
		TopReqNumber();
		Top.Value() = random() * Top.Value() / 0x80000000u;
		LOOP;

	{	Double n;

	case tCOUNT:		// <value> tCOUNT -- <temp>
		Assert1Arg();
		if(Top.IsList()) n=Top.ListSize(); else
		if(Top.IsText()) n=Top.TextLen();  else n=0;
top_n:	if(TopIsNoTemp())
		{
			Top.decr_lock();
			TopPtr = new Var(n);
			Top.incr_lock();						// kio 2009-03-30
		}
		else { Top=n; }
		QUICK;

	case tINDEX:		// <value> tINDEX -- <temp>
		Assert1Arg();
							// non_ref values make no sense here, because index information get's lost!
		n = Top.Index()+1;	// no check for parent: all variables except 'globals' do have a parent
		goto top_n;			// Var's without a parent silently work on this call

	case tCHARCODE:		// <value> tCHARCODE -- <temp>
		Assert1Arg();
		TopReqString();
		n = Top.TextLen() ? Top.Text()[0] : 0;
		goto top_n;

	case tISTRIGGERED:	//	<value> tISTRIGGERED -- <temp>
		Assert1Arg();
		TopReqIrpt();
		n = Top.GetIrpt()->IsTriggered();
		goto top_n;

	case tISTEXT:		// <value> tISTEXT -- <temp>
		Assert1Arg();
		n = Top.IsText();
		goto top_n;

	case tISNUMBER:		// <value> tISNUMBER -- <temp>
		Assert1Arg();
		n = Top.IsNumber();
		goto top_n;

	case tISLIST:		// <value> tISLIST -- <temp>
		Assert1Arg();
		n = Top.IsList();
		goto top_n;

	case tISPROC:		// <value> tISPROC -- <temp>
		Assert1Arg();
		n = Top.IsProc();
		goto top_n;

	case tISFILE:		// <value> tISFILE -- <temp>
		Assert1Arg();
		TopReqString();
		n = IsFile(Top.Text());
		goto top_n;

	case tISDIR:		// <value> tISDIR -- <temp>
		Assert1Arg();
		TopReqString();
		n = IsDir(Top.Text());
		goto top_n;

	case tISLINK:		// <value> tISLINK -- <temp>
		Assert1Arg();
		TopReqString();
		n = IsLink(Top.Text());
		goto top_n;

	}	// Double n

//	log_B(X)  = ln(X) / ln(B)
//	log_2(X)  = ln(X) / ln(2)
//	log_10(X)= ln(X) / ln(10)
//	B^X  = e ^ ( X * ln(B) )
//	2^X  = e ^ ( X * ln(2) )
//	10^X = e ^ ( X * ln(10) )

	case tLOGE:			// <temp> tLOGE -- <temp>			// n = loge(value)
		Assert1Arg();
		TopAssertTemp();
		TopReqNumber();
		Top.Value() = log(Top.Value());
		errno=ok;	// note: i386 sets EDOMAIN and ERANGE; ppc none of both
		QUICK;			// result should be NAN for EDOMAIN and INF for ERANGE anywhere

	case tLOG10:		// <temp> tLOG10 -- <temp>			// n = log10(value)
		Assert1Arg();
		TopAssertTemp();
		TopReqNumber();
		Top.Value() = log10(Top.Value());
		errno=ok;	// note: i386 sets EDOMAIN and ERANGE; ppc none of both
		QUICK;			// result should be NAN for EDOMAIN and INF for ERANGE anywhere

	case tLOG2:			//	<temp> tLOG2 -- <temp>			// n = log2(value)
		Assert1Arg();
		TopAssertTemp();
		TopReqNumber();
		Top.Value() = log(Top.Value())/d_ln2;
		errno=ok;	// note: i386 sets EDOMAIN and ERANGE; ppc none of both
		QUICK;			// result should be NAN for EDOMAIN and INF for ERANGE anywhere

	case tLOG:			//	<temp> <value> tLOG -- <temp>	// n = log(base,value)
		Assert2Args();
		Arg2AssertTemp();
		Arg2ReqNumber();
		TopReqNumber();
		Arg2.Value() = Arg2.Value()==1.0 ? NAN		// +inf/-inf-Sprung bei 1: log(1)==0.
					: log(Top.Value())/log(Arg2.Value());
		errno=ok;	// note: i386 sets EDOMAIN and ERANGE; ppc none of both
		DROP;			// result should be NAN for EDOMAIN and INF for ERANGE anywhere

	case tEXPE:			//	<temp> tEXPE -- <temp>
		Assert1Arg();
		TopAssertTemp();
		TopReqNumber();
		Top.Value() = exp(Top.Value());
		errno=ok;	// note: i386 sets EDOMAIN and ERANGE; ppc none of both
		QUICK;			// result should be NAN for EDOMAIN and INF for ERANGE anywhere

	case tEXP2:			//	<temp> tEXP2 -- <temp>
		Assert1Arg();
		TopAssertTemp();
		TopReqNumber();
		Top.Value() = exp(Top.Value()*d_ln2);
		errno=ok;	// note: i386 sets EDOMAIN and ERANGE; ppc none of both
		QUICK;			// result should be NAN for EDOMAIN and INF for ERANGE anywhere

	case tEXP10:		//	<temp> tEXP10 -- <temp>
		Assert1Arg();
		TopAssertTemp();
		TopReqNumber();
		Top.Value() = exp(Top.Value()*d_ln10);
		errno=ok;	// note: i386 sets EDOMAIN and ERANGE; ppc none of both
		QUICK;			// result should be NAN for EDOMAIN and INF for ERANGE anywhere

	case tEXPN:			//	<temp> <value> tEXPN -- <temp>
		Assert2Args();
		Arg2AssertTemp();
		Arg2ReqNumber();
		TopReqNumber();
		Arg2.Value() = exp( Top.Value() * log(Arg2.Value()) );
		errno=ok;	// note: i386 sets EDOMAIN and ERANGE; ppc none of both
		DROP;			// result should be NAN for EDOMAIN and INF for ERANGE anywhere

	case tSTDIN:		// 	tSTDIN -- <var>
		Push(stdin0);
		QUICK;

	case tSTDOUT:		//	tSTDOUT -- <var>
		Push(stdout0);
		QUICK;

	case tSTDERR:		//	tSTDERR -- <var>
		Push(stderr0);
		QUICK;

	case tENV:			//	tENV -- <var>
		Push(env);
		QUICK;

	case tERROR:		//	tERROR -- <var>
		if(!thread->error) InitThreadError();
		Push(thread->error);
		QUICK;

	case tGLOBALS:		//	tGLOBALS -- <var>
		Push(thread->globals);
		assert(Top.type_is_locked());
		QUICK;

	case tROOT:			//	tROOT -- <var>
		assert(root->data_is_locked());
		assert(root->is_locked());
		Push(root);
		QUICK;

	case tLOCALS:		//	tLOCALS -- <var>
		Push(thread->locals);
		assert(Top.type_is_locked());
		QUICK;

	case tTHIS:			//	tTHIS -- <var>
		if(thread->proc->IsUnlinked()) goto ERR_PROCNOPARENT;
		Push(thread->proc->Parent());
		QUICK;

	case tMIN:			//	<value> <n*value> tMIN.n -- <value>		// min ( a, b )
	{	int n = *ip++;
		AssertNArgs(n+1);
		if(n>=1)
		{
			do
			{	if (&Arg2.MinVar(Top)==&Arg2) { Drop(); } 		// setzt ggf. error ?
				else { Nip(); }
			}
			while(--n>=1);
		}
		else if(Top.IsList())
		{
			Var* z = &Top.MinVar(); 					// setzt das ggf. error ?
			z->lock(); Top.unlock(); TopPtr = z;
		}
	}	LOOP;

	case tMAX:			//	<value> <n*value> tMAX.n -- <value>		// max ( a, b )
	{	int n = *ip++;
		AssertNArgs(n+1);
		if(n>=1)
		{
			do
			{	if (&Arg2.MaxVar(Top)==&Arg2) { Drop(); } 		// setzt ggf. error ?
				else { Nip(); }
			}
			while(--n>=1);
		}
		else if(Top.IsList())
		{
			Var* z = &Top.MaxVar(); 					// setzt das ggf. error ?
			z->lock(); Top.unlock(); TopPtr = z;
		}
	}	LOOP;

	case tSPACESTR:		//	<temp> [<value>] tSPACESTR.n -- <temp>
		if(*ip++==0)	//	<temp> tSPACESTR.0 -- <temp>
		{
			Assert1Arg();
			TopAssertTemp();
			TopReqNumber();
			Top = SpaceString(Top.LongValue());
		}
		else			//	<temp> <value> tSPACESTR.1 -- <temp>
		{
			Assert2Args();
			Arg2AssertTemp();
			Arg2ReqNumber();
			TopReqString();
			Arg2 = Top.Text() * Arg2.LongValue();
			Drop();
		}
		LOOP;

	case tCHARSTR:		// <temp> tCHARSTR -- <temp>
		Assert1Arg();
		TopAssertTemp();
		TopReqNumber();
		Top = String((UCS4Char)Top.LongValue());
		LOOP;

	case tLEFTSTR:		//	<temp> <value> tLEFTSTR -- <temp>
		Assert2Args();
		Arg2AssertTemp();
		Arg2ReqString();
		TopReqNumber();
		Arg2.Text() = LeftString(Arg2.Text(),Top.LongValue());
		DROP;

	case tRIGHTSTR:		//	<temp> <value> tRIGHTSTR -- <temp>
		Assert2Args();
		Arg2AssertTemp();
		Arg2ReqString();
		TopReqNumber();
		Arg2.Text() = RightString(Arg2.Text(),Top.LongValue());
		DROP;

	case tMIDSTR:		// 	<temp> <value> [<value>] tMIDSTR -- <temp>
		if(*ip++==0)
		{
			Assert2Args();
			Arg2AssertTemp();
			Arg2ReqString();
			TopReqNumber();
			Arg2.Text() = MidString( Arg2.Text(), Top.LongValue()-1 );
			DROP;
		}
		else
		{
			Assert3Args();
			Arg3AssertTemp();
			Arg3ReqString();
			Arg2ReqNumber();
			TopReqNumber();
			Arg3.Text() = MidString( Arg3.Text(), Arg2.LongValue()-1, Top.LongValue() );
			DROP2;
		}

	case tSUBSTR:		// 	<temp> <value> [<value>] tSUBSTR -- <temp>
		if(*ip++==0)
		{
			Assert2Args();
			Arg2AssertTemp();
			Arg2ReqString();
			TopReqNumber();
			Arg2.Text() = MidString( Arg2.Text(), Top.LongValue()-1 );
			DROP;
		}
		else
		{
			Assert3Args();
			Arg3AssertTemp();
			Arg3ReqString();
			Arg2ReqNumber();
			TopReqNumber();
			Arg3.Text() = SubString( Arg3.Text(), Arg2.LongValue()-1, Top.LongValue() );
			DROP2;
		}

	case tFIND:			// <value> <value> [<value>] tFIND -- <temp>
	{	long n = 0;
		if(*ip++==1)
		{
			Assert3Args();
			TopReqNumber();
			n = Top.LongValue() -1; Drop();
		}

		Assert2Args();
		if(Arg2.IsText())
		{
			TopReqString();
			n = Arg2.Text().Find(Top.Text(), n);
		}
		else if(Arg2.IsList())
		{
			n = Arg2.Find(Top,n);
		}
		else
		{
			goto ERR_STRORLISTREQ;
		}

		Drop2Push(new Var((Double)(n+1)));
		QUICK;
	}

	case tRFIND:			// <value> <value> [<value>] tRFIND -- <temp>
	{	long n = 0x7FFFFFFF;
		if(*ip++==1)
		{
			Assert3Args();
			TopReqNumber();
			n = Top.LongValue() -1; Drop();
		}

		Assert2Args();
		if(Arg2.IsText())
		{
			TopReqString();
			n = Arg2.Text().RFind(Top.Text(), n);
		}
		else if(Arg2.IsList())
		{
			n = Arg2.RFind(Top,n);
		}
		else
		{
			goto ERR_STRORLISTREQ;
		}

		Drop2Push(new Var((Double)(n+1)));
		QUICK;
	}

	case tREPLACE:			//	<temp|var> <value> <value> tREPLACE -- <temp|var>
		Assert3Args();
		Arg3ReqString();				// TODO: list expansion
		Arg3AssertNonConst();
		Arg2ReqString();
		TopReqString();
		Arg3.Text().Replace( Arg2.Text(), Top.Text() );
		DROP2;

	case tSPLIT:			//	<temp|var> [<value>] tSPLIT.n -- <temp|var>
		if(*ip++==0)
		{
			Assert1Arg();
			TopAssertNonConst();
			TopReqStringList();
			Top.SplitLines();
			LOOP;
		}
		else
		{
			Assert2Args();
			Arg2AssertNonConst();
			Arg2ReqStringList();
			TopReqString();
			Arg2.SplitLines(Top.Text());
			DROP;
		}

	case tJOIN:				//	<temp|var> <value> tJOIN  --  <temp|var>		2010-09-27
		Assert2Args();
		Arg2AssertNonConst();
		Arg2ReqList();
		TopReqString();
		Arg2.JoinLines(Top.Text());
		DROP;

	case tCONVERT_TO:		//	<temp|var>         tCONVERT_TO.nn  --  <temp|var>
	{						// 	<temp|var> <value> tCONVERT_TO.ff  --  <temp|var>
	// get idf
		int c = *ip++;
		if(c==0xff)
		{
			Assert2Args();
			TopReqString();
			c = GetEncodingFromName(Top.Text());
			if(errno) ERROR;
			Drop();
		}
		Assert1Arg();
		TopAssertNonConst();
		Top.ConvertTo((CharEncoding)c);
		LOOP;
	}

	case tCONVERT_FROM:		//	<temp|var>         tCONVERT_FROM.nn  --  <temp|var>
	{						// 	<temp|var> <value> tCONVERT_FROM.ff  --  <temp|var>
	// get idf
		int c = *ip++;
		if(c==0xff)
		{
			Assert2Args();
			TopReqString();
			c = GetEncodingFromName(Top.Text());
			if(errno) ERROR;
			Drop();
		}
		Assert1Arg();
		TopAssertNonConst();
		Top.ConvertFrom((CharEncoding)c);
		LOOP;
	}


	case tMATCH:		// 	<temp> <value> [<value>] tMATCH -- <temp>
	{
		MatchType flags = fnm_basic;
				//  | fnm_escape		// handle '\' as escape character
				//  | fnm_casefold		// case-insensitive
				//	| fnm_pathname		// '/' must match
				//	| fnm_pweriod		// leading '.'s must match
		if(*ip++==1)
		{
			Assert3Args();
			TopReqNumber();
			flags = (MatchType)(int)Top.Value(); Drop();
		}

		Assert2Args();
		Arg2AssertTemp();
		Arg2ReqString();
		TopReqString();
		Arg2 = FNMatch(CString(Arg2.Text()),CString(Top.Text()),flags);
		DROP;
	}

	case tHEXSTR:		// <temp> [<value>] tHEXSTR.n -- <temp>
	{
		int digits=0;
		if(*ip++==1)
		{
			Assert2Args();
			TopReqNumber();
			digits = Top.LongValue();
			Drop();
		}
		Assert1Arg();
		TopAssertTemp();
		TopReqNumber();
		Top = HexString(Top.Value(),digits);
		QUICK;
	}

	case tBINSTR:		// <temp> [<value>] tBINSTR.n -- <temp>
	{
		int digits=0;
		if(*ip++==1)
		{
			Assert2Args();
			TopReqNumber();
			digits = Top.LongValue();
			Drop();
		}
		Assert1Arg();
		TopAssertTemp();
		TopReqNumber();
		Top = BinString(Top.Value(),digits);
		QUICK;
	}

	case tTOUPPER:		// <temp> tTOUPPER -- <temp>
		Assert1Arg();
		TopAssertTemp();
		TopReqString();				// TODO: list expansion?
		Top.Text() = Top.Text().ToUpper();
		QUICK;

	case tTOLOWER:		// <temp> tTOLOWER -- <temp>
		Assert1Arg();
		TopAssertTemp();
		TopReqString();				// TODO: list expansion?
		Top.Text() = Top.Text().ToLower();
		QUICK;

	case tGETNAME:		// <value> tGETNAME -- <temp>
		Assert1Arg();
		DropPush( new Var( GetNameForHandle(Top.name) ) );
		QUICK;

	case tTOTEXT:		// <value> tTOTEXT -- <temp>
		Assert1Arg();
		DropPush( new Var( Top.ToString(no,DisassString) ) );
		LOOP;

	case tFULLPATH:		//	<temp> tFULLPATH -- <temp>
		Assert1Arg();
		TopAssertTemp();
		TopReqString();
		Top.Text() = FullPath(Top.Text(),1);		// ***TODO*** fullpath(<path>,<deref_symlink>)
		if(errno) { LogLine("FullPath returned error: %s",errorstr()); }
		LOOP;

	case tMDATE:		//	<temp> tMDATE -- <temp>
		Assert1Arg();
		TopAssertTemp();
		TopReqString();
		Top = FileMTime(FullPath(Top.Text(),1));	// TODO: mdate (<path>,<deref_symlink>)
		LOOP;

	case tDIR:			//	<value> tDIR tNEW|tVAR -- 		create directory
						//	<temp> tDIR tAT -- <temp>		read directory
		switch(*ip++)
		{
		case tAT:		// 	<temp> tDIR tAT -- <temp>		read directory
			Assert1Arg();
			TopAssertTemp();
			TopReqString();
			ReadDir(String(Top.Text()),Top);
			LOOP;
		case tNEW:		//	<value> tDIR tNEW|tVAR -- 		create new directory
		case tVAR:
			Assert1Arg();
			TopReqString();
			CreateDir(Top.Text(),ip[-1]==tNEW?"n":"w");
			DROP;
		case tDEL:		//	<value> tDIR tDEL --					delete empty directory
			Assert1Arg();
			TopReqString();
			DeleteDir(Top.Text());
			DROP;
		case tRENAME:		// rename dir. TODO: was, wenn symlink? besser rename link?
			Assert2Args();
			TopReqString();
			Arg2ReqString();
			RenameDir(Arg2.Text(),Top.Text());
			DROP2;
		case tEXISTS:		//	<value> tDIR tEXISTS -- <const>			exists node ?  ((may be file etc.!))
			goto texifile;
		default:
			goto ERR_IERR;
		}

	case tFILE:			//	<temp> tFILE tAT -- <temp>					read file
						//	<value> <value> tFILE tNEW|tVAR|tHASHGL --	create file
		switch(*ip++)
		{
		case tAT:		// 	<temp> tFILE tAT -- <temp>					read file
			Assert1Arg();
			TopAssertTemp();
			TopReqString();
			ReadFile(String(Top.Text()),Top.Text());
			LOOP;
		case tNEW:		//	<value> <value> tFILE tNEW|tVAR -- 			create file
		case tVAR:
		case tHASHGL:
			Assert2Args();
			TopReqString();
			Arg2ReqString();
			CreateFile(Arg2.Text(),Top.Text(),ip[-1]==tNEW?"n":ip[-1]==tVAR?"w":"a");
			DROP2;
		case tDEL:		//	<value> tFILE tDEL --						delete file
			Assert1Arg();
			TopReqString();
			DeleteFile(Top.Text());
			DROP;
		case tRENAME:		// rename file. TODO: was, wenn symlink? besser rename link?
			Assert2Args();
			TopReqString();
			Arg2ReqString();
			RenameFile(Arg2.Text(),Top.Text());
			DROP2;
		case tEXISTS:		//	<value> tFILE tEXISTS -- <const>			exists node ?  ((may be file etc.!))
texifile:	Assert1Arg();
			TopReqString();
			{ 	String fname = Top.Text();
				DropPush( NodeExists(fname,ip[-1]!=tLINK) ? eins : zero );
				assert(!(errno&&TopPtr==eins));
				errno=ok;
			}
			LOOP;
		case tFREEZE:		// TODO
		case tMELT:			// TODO
			goto ERR_NIMP;
		default:
			goto ERR_IERR;
		}

	case tLINK:			//	<value> <value> tLINK tNEW|tVAR -- 		create link
						//	<temp> tLINK tAT -- <temp>				read link
		switch(*ip++)
		{
		case tAT:		// 	<temp> tLINK tAT -- <temp>				read link
			Assert1Arg();
			TopAssertTemp();
			TopReqString();
			ReadLink(String(Top.Text()),Top.Text());
			LOOP;
		case tNEW:		//	<value> <value> tLINK tNEW|tVAR -- 		create link
		case tVAR:
			Assert2Args();
			TopReqString();
			Arg2ReqString();
			CreateLink(Arg2.Text(),Top.Text(),ip[-1]==tNEW?"n":"w");
			DROP2;
		case tDEL:		//	<value> tLINK tDEL --					delete link
			Assert1Arg();
			TopReqString();
			DeleteLink(Top.Text());
			DROP;
		case tRENAME:
			Assert2Args();
			TopReqString();
			Arg2ReqString();
			RenameLink(Arg2.Text(),Top.Text());
			DROP2;
		case tEXISTS:		//	<value> tLINK tEXISTS -- <const>			exists node ?  ((may be file etc.!))
			goto texifile;
		default:
			goto ERR_IERR;
		}


	case tDATESTR:		//	<temp> tDATESTR -- <temp>
		Assert1Arg();
		TopAssertTemp();
		TopReqNumber();
		Top = datetimestr((time_t)Top.Value());
		LOOP;

	case tDATEVAL:		// 	<temp> tDATEVAL -- <temp>
		Assert1Arg();
		TopAssertTemp();
		TopReqString();
		Top = dateVal(CString(Top.Text()));		// unix/os_utilities.h
		LOOP;

	case tNOW:			//	tNOW -- <temp>
		Push( new Var(now()) );
		QUICK;

	case tLIST:			//	<n*temp> tLIST.n -- <temp>
	{
		int n = *ip++;
		AssertNArgs(n);
		Var*z = new Var(isList);
		while(--n>=0)
		{
			TopAssertTemp();
			TopPtr->Link(z,n);
			Drop();
		}
		Push(z);
		LOOP;
	}

	case tWORDS:		//	tWORDS -- <temp>
		Push( new Var(isList) );
		for(uint i=0;i<tokens;i++)
		{
			Top.AppendItem(new Var(TokenName(i)));
		}
		LOOP;

	case tINFO:			//	<temp> tINFO -- <temp>
		Assert1Arg();
	{	TopAssertTemp();
		TopReqString();
		NameHandle nh = NewNameHandle(Top.Text());
		int i=tokens;
		while(--i>=0)
		{
			if (nh==tokennamehandle[i])
			{
				Top.Text() = String("syntax: ") + TokenSyntax(i) + "\n" + TokenInfo(i);
				LOOP;
			}
		}
		SetError( tokennotfound, String("token/word ")+Top.Text().ToQuoted()+" not found" );
		ERROR;
	}

	case tSORT:		//	<var> tSORT --
		Assert1Arg();
		TopReqVar();
		TopReqList();
		Top.Sort();
		DROP;

	case tRSORT:	//	<var> tRSORT --
		Assert1Arg();
		TopReqVar();
		TopReqList();
		Top.RSort();
		DROP;

	case tSHUFFLE:	//	<var> tSHUFFLE --
		Assert1Arg();
		TopReqVar();
		TopReqList();
		Top.Shuffle();
		DROP;

	case tMELT:		//	<var> tMELT --
		Assert1Arg();
		TopReqVar();
		TopReqString();
		MeltString(Top.Text());
		DROP;

	case tFREEZE:	//	<var> tMELT --
		Assert1Arg();
		TopReqVar();
		TopReqString();
		FreezeString(Top.Text());
		DROP;


	case tSELF:			//	tSELF -- <temp>
		Push( thread->GetVar() );
		QUICK;

	case tSUSPEND:		// <value> tSUSPEND --
		Assert1Arg();
		TopReqTid();	// suspend self / suspend other
		Top.GetThread()->Suspend();
		Drop();
		SWITCH;

	case tRESUME:		// 	<value> tRESUME --
		Assert1Arg();
		TopReqTid();
		Top.GetThread()->link_running();
		Drop();
		SWITCH;

	case tTRIGGER:		//	<value> tTRIGGER --
		Assert1Arg();
		TopReqIrpt();
		Top.GetIrpt()->Trigger();
		Drop();
		SWITCH;

	case tWAITIRPT:		//	<value> tWAITIRPT --
		Assert1Arg();
		TopReqIrpt();
		Top.GetIrpt()->Shedule();		// shedule self
	//	DROP;			<-- das ist doch falsch!?
		Drop();
		SWITCH;

	case tWAITIRPT_WTO:	//	<value> <value> tWAITIRPT_WTO --
		Assert2Args();
		TopReqNumber();
		Arg2ReqIrpt();
		Arg2.GetIrpt()->Shedule(Top.Value());
	//	DROP2;			<-- das ist doch falsch?!
		Drop();
		Drop();
		SWITCH;

	case tCLEARIRPT:	//	<value> tCLEARIRPT --
		Assert1Arg();
		TopReqIrpt();
		Top.GetIrpt()->Clear();
		DROP;

	case tSETPRIO:		//	<value> <value> tSETPRIO --
		Assert2Args();
		Arg2ReqTid();	// change prio of self / other thread
		TopReqNumber();
		Arg2.GetThread()->ChangePrio(Top.LongValue());
		Drop2();
		SWITCH;

	case tWAIT:			//	<value> tWAIT --
		Assert1Arg();
		TopReqNumber();	//	wait self for duration
		thread->Wait(Top.Value());
		Drop();
		SWITCH;

	case tSHEDULE:		//	<value> tSHEDULE --
		Assert1Arg();
		TopReqNumber();	//	shedule self for time
		thread->Shedule(Top.Value());
		Drop();
		SWITCH;

	case tREQUEST:		//	<value> tREQUEST --
		Assert1Arg();
		TopReqSema();
		if( Top.GetSema()->Request() ) DROP;
		ip--;
		SWITCH;

	case tREQUEST_WTO:	//	<value> <value> tREQUEST --
		Assert2Args();
		Arg2ReqSema();
		TopReqNumber();
		if( Arg2.GetSema()->IsAvailable() ) if(Arg2.GetSema()->Request()) DROP2;
		if( SystemTime() >= Top.Value() ) goto ERR_TIMEOUT;
		if( Arg2.GetSema()->Request_WTO(Top.Value()) ) DROP2;	// impossible...
		ip--;
		SWITCH;

	case tRELEASE:		//	<value> tRELEASE --
		Assert1Arg();
		TopReqSema();
		Top.GetSema()->Release();
		Drop();
		SWITCH;

	case tCLEARSEMA:	//	<value> tCLEARSEMA --
		Assert1Arg();
		TopReqSema();
		Top.GetSema()->Clear();
		Drop();
		SWITCH;

/*
	case tSEMAISFREE:
	case tSEMAISBLOCKED:
	case tSEMAISAVAILABLE:
	case tSEMAISNOTAVAILABLE:
	case tSEMAISMINE:
	case tSEMAISNOTMINE:
	case tSEMAISWAITING:
	case tSEMAOWNER:
*/

	{	displacement d;
		#define PeekDist()	if(sizeof(d)==2) d=peek2(ip); else d=peek4(ip)

	// proc literal:
	case tPROC:		//	tPROC.dist.n.m <n*namehandle> <statements> -- <temp>
					// 	dist = dist to skip proc body; n = args total; m = args before '*'
		Push( new Var(Proc(thread->proc->ProcBundle(),ip-1)) );
		PeekDist();
		ip += d;
		QUICK;


	case tBOOL:			//	<var|temp> tBOOL -- <var|temp>
		Assert1Arg();
		TopAssertNonConst();
		TopReqNumber();
		Top.Value() = bool(Top.Value());
		QUICK;

	case tANDAND:		// 	<value:0> tANDAND.dist -- <value:0>   	and branch
						//	<value:1> tANDAND.dist -- 				and no branch
		Assert1Arg();
		TopReqNumber();
		if(Top.Value())  { Drop(); ip += sizeof(d); }
		else { PeekDist(); ip += d; }
		QUICK;

	case tOROR:			// 	<value:1> tOROR.dist -- <value:1>   	and branch
						//	<value:0> tOROR.dist -- 				and no branch
		Assert1Arg();
		TopReqNumber();
		if(Top.Value()) { PeekDist(); ip += d; }
		else            { Drop(); ip += sizeof(d); }
		QUICK;


	case tIF:			//	<value> tIF.dest --
	case tQMARK:		// 	<value> tQMARK.dest --
		Assert1Arg();
		TopReqNumber();
		if(Top.Value())
		{
			xxlog(" nojump ");
			ip += sizeof(d);
		}
		else
		{
			xxlog(" d=");
			PeekDist();
			xxlog("%li ",(long)d);
			ip += d;
		}
		DROP;


#define	RType			rp->type
#define	RArg1			rp[1]
#define	RArg2			rp[2]
#define	RArg3			rp[3]
#define	RArg4			rp[4]
#define	RArg5			rp[5]

#define	RPush(DATA)		rp--, rp->data = size_t(DATA)
#define	RDrop(N)		rp+=(N)


	case tDO:			//	tDO.dist --
		PeekDist();
		RPush(ip+d);		//	RArg2: ip	for until, while & exit
		ip += sizeof(d);
		RPush(ip);			//	RArg1: ip 	for loop & next
		RPush(rtDO);		//	RType: type
		QUICK;
	#define	RCountDo	3
	#define	RDropDo()	RDrop(3)
	#define	RPeekLoop()	ip = RArg1.ip;
	#define	RPopExit()	ip = RArg2.ip; RDrop(3)


	case tTRY:			//	tTRY.dest --
		PeekDist();
		RPush(vp);			//	RArg2: vp for error catch
		RPush(ip+d);		//	RArg1: ip for error catch
		RPush(rtTRY);		//	RType: type
		ip += sizeof(d);
		QUICK;
	#define	RCountTry		3
	#define	RDropTry()		RDrop(3)
	#define	RDropTryOrDo()	RDrop(3)
	#define	RPopTry()		ip = RArg1.ip; while(vp>RArg2.vp) Drop(); RDrop(3)
	#define	RPeekTryVp()	RArg2.vp


	case tGKauf:		//	tGKauf --
		RPush(thread->locals); 		//	RArg1: old locals	(locked as is)
		RPush(rtGKauf); 			//	RType: type
		thread->locals = new Var(thread->locals,at_end,isList,nh_privates);
		thread->locals->lock();
		thread->locals->LockType();
		LOOP;
	#define	RCountGKauf	 2
	#define	RPopGKauf()	 thread->locals->unlock(); thread->locals->Vanish(); \
						 thread->locals = RArg1.var; RDrop(2)
	#define	RDropGKauf() thread->locals->unlock(); \
						 thread->locals = RArg1.var; RDrop(2)


	case tCATCH:		//	tCATCH.dest --			try .. catch .. then
		assert(RType==rtTRY);
		RDropTry();
		goto bra;

	case tNOCATCH:		//	tNOCATCH --				try .. nocatch
		assert(RType==rtTRY);
		RDropTry();
		QUICK;


	case tELSE:			//	tELSE.dest --
	case tCOLON:		// 	tCOLON.dest --
	case tELIF:			//	tELIF.dest --
bra:	PeekDist();
		ip += d;
		QUICK;

	}	// displacement d


	case tLOOP:			//	tLOOP --
		assert(RType==rtDO);
		RPeekLoop();
		DISP;


	case tNEXT:			//	tNEXT --
		for(;;)
			switch(RType)
			{
			case rtDO:		RPeekLoop(); DISP;
			case rtGKauf:	RPopGKauf(); continue;
			case rtTRY:		RDropTry();  continue;
			default:		goto ERR_IERR;
			}


	case tUNTIL:		//	<value> tUNTIL --
		Assert1Arg();
		TopReqNumber();
		if(!Top.Value()) DROP;
		goto twh;


	case tWHILE:		//	<value> tWHILE --
		Assert1Arg();
		TopReqNumber();
		if(Top.Value()) DROP;
twh:	Drop();
		goto tex;


	case tEXIT:			//	tEXIT --
tex:	for(;;)
			switch(RType)
			{
			case rtDO:	  	RPopExit();  DISP;
			case rtGKauf: 	RPopGKauf(); continue;
			case rtTRY:	  	RDropTry();  continue;
			default:	  	goto ERR_IERR;
			}


	case tGKzu:			//	tGKzu --
		assert(RType==rtGKauf);
		RPopGKauf();
		LOOP;


	case tEXE:			// <value> tEXE -- <value>
	case tEVAL:			// <value> tEVAL -- <value>
		Assert1Arg();
		TopReqString();
		compiler.Compile(Top.Text(),ip[-1]==tEXE);
		if(errno)
		{
			StartCompileError( ip[-1]==tEXE?"execute text":"eval text" );
			goto compile_error;
		}
		Drop();
		// TODO: bundle in root.rsrc einhängen

		RPush(vp);					//	RArg3	vp
		RPush(thread->proc);		//	RArg2	proc
		RPush(ip);					//	RArg1	ip
		RPush(rtEVAL);				//	RType

exe_cp:	thread->proc = compiler.GetProc(); thread->proc->lock();
		constants	 = thread->proc->ProcConstants();
		ip			 = thread->proc->ProcStart();
		DISP;

		//	rtINCLUDE: 	expect result. if no result, then return 0.
		//	rtEVAL: 	require result. the compiler should make sure
		// 				that the compiled string will always end with tRETURN1
	#define RPopEvalOrInclude()								\
		thread->proc->unlock(); thread->proc = RArg2.var; 	\
		constants = thread->proc->ProcConstants();			\
		while(vp>RArg3.vp) Drop();							\
		ip = RArg1.ip; RDrop(4);

	#define	RCountEvalOrInclude		4
	#define RDropEvalOrInclude() 	RArg2.var->unlock(); RDrop(4);
	#define	RPeekEvalOrIncludeVp()	RArg3.vp


	case tINCLUDE:	// 	<value> tINCLUDE -- <value>
		Assert1Arg();
		TopReqString();
		compiler.CompileFile(Top.Text());
		if(errno)
		{
			StartCompileError( "include file" );
			goto compile_error;
		}
		Drop();
		// TODO: bundle in root.rsrc einhängen

		RPush(vp);					//	RArg3	vp
		RPush(thread->proc);		//	RArg2	proc
		RPush(ip);					//	RArg1	ip
		RPush(rtINCLUDE);			//	RType
		goto exe_cp;				// 	execute compiler.proc


	case tCALL:		// 	<n*value> <value> tCALL.n -- <value>
		goto ERR_NIMP;		// TODO: falsche argumenten reihenfolge
		Assert1Arg();
		TopReqString();
		compiler.CompileFile(Top.Text());
		if(errno)
		{
			StartCompileError( "call file" );
			goto compile_error;
		}
		Drop();
		// TODO: bundle in root.rsrc einhängen

	{	int  argc = *ip++;
		Var* argv = new Var(isList,"globals"); argv->ResizeList(argc);
		while(argc--) { Assert1Arg(); TopReqTemp(); Top.Link(argv,argc); Drop(); }

		RPush(thread->globals);		//	RArg5	globals
		RPush(thread->locals);		//	RArg4	locals
		thread->globals = argv; argv->lock();
		thread->locals  = argv; argv->lock();
		RPush(vp);					//	RArg3	vp
		RPush(thread->proc);		//	RArg2	proc
		RPush(ip);					//	RArg1	ip
		RPush(rtCALL);				//	RType
	}
		goto exe_cp;				// execute compiled proc

		//	tCALL:	expect result. if no result, then return 0.
	#define RPopCall()											\
		thread->globals->unlock(); thread->globals = RArg5.var;	\
		thread->locals->unlock();  thread->locals  = RArg4.var;	\
		thread->proc->unlock();    thread->proc    = RArg2.var; \
		constants = thread->proc->ProcConstants();				\
		while(vp>RArg3.vp) Drop();								\
		ip = RArg1.ip; RDrop(6);

	#define	RCountCall		6
	#define RDropCall() 	RArg5.var->unlock(); RArg4.var->unlock(); \
							RArg2.var->unlock(); RDrop(6)
	#define	RPeekCallVp()	RArg3.vp


	// execute proc:
	{	RStackType rt_type; Var* argv;
		int i,argc,argn,argm; uptr dest;

	case tEXECINSTR:	//	<value> <n*temp> tEXECPROC.n --
		rt_type = rtINSTR;
		goto txp;

	case tEXECPROC:		//	<value> <n*temp> tEXECPROC.n -- <value>
		rt_type = rtPROC;	//	tPROC.dist.n.m <n*namehandle> <statements> -- <temp>
							// 	dist = dist to skip proc body; n = args total; m = args before '*' or -1

txp:	argc = *ip++;				// anz. args
		AssertNArgs(argc+1);
		ArgNReqProc(argc+1);
		dest = ArgN(argc+1).ProcStart();
		assert(*dest==tPROC);
		dest += sizeof(displacement) +1;		// 	skip tPROC.dist
		argn = *dest++;				//	.n
		argm = *dest++;		//	.m
		xxlog(" (%i:%i%s)", argc,argn,argm>argn?"":":*");

		if(argc!=argn)	{ if(argc<argn) goto ERR_TOOFEWARGUMENTS;
						  else if(argm>argn) goto ERR_TOOMANYARGUMENTS; }

		argv = new Var(isList,nh_locals); /* argv->ResizeList(argc); */
		i = argc; while(i--) { TopAssertTemp(); Top.Link(argv,i); Drop(); }

		if(argm>argn) argm=argn;
		for(i = 0;       i<argm;i++) { (*argv)[i].SetName(peek4(dest)); dest+=4; }
		for(i+=argc-argn;i<argc;i++) { (*argv)[i].SetName(peek4(dest)); dest+=4; }

		RPush(vp-1);				//	RArg4	vp			wird nur für tCHECKSTACKS benötigt!
		RPush(thread->locals);		//	RArg3	locals
		RPush(thread->proc);		//	RArg2	proc
		RPush(ip);					//	RArg1	ip
		RPush(rt_type);				//	RType

		thread->locals	= argv; argv->lock(); argv->lock_type();
		thread->proc 	= TopPtr; vp--;
		constants	 	= thread->proc->ProcConstants();
		ip				= dest;
	}	DISP;

		// tEXECPROC:  require result. error if no result.
		// tEXECINSTR: require no result. dump it if present.
	#define RPopProcOrInstr()											\
				thread->locals->unlock(); thread->locals = RArg3.var;	\
				thread->proc->unlock();   thread->proc   = RArg2.var; 	\
				constants = thread->proc->ProcConstants();				\
				ip = RArg1.ip; RDrop(5);

	#define RDropProcOrInstr() 											\
				thread->locals->unlock(); thread->locals = RArg3.var; 	\
				RArg2.var->unlock(); RDrop(5)

	#define	RPeekProcOrInstrVp()	RArg4.vp
	#define	RPeekProcOrInstrProc()	RArg2.var
	#define RCountProcOrInstr		5


	case tCHECKSTACK:		//	tCHECKSTACK.n --
		if( rp-ra >= *ip++ ) QUICK;

	{	xlog("(*resize stack*)");

		long  nl = (vp+1-va) + (re-rp) +ip[-1]; IFNDEBUG( nl += nl/2; )
		StackData* vn = new StackData[nl];

		memcpy( vn,            va, (vp+1-va)*sizeof(Var*)    );
		memcpy( vn+nl-(re-rp), rp, (re-rp)*sizeof(StackData) );

		// cave-at: va, re are #defines based on thread->stack[_end]
		//			ve, ra are #defines based on rp and vp

		ssize_t d = vn - thread->stack;

		vp += d;
		rp += vn+nl-re;

		delete[] thread->stack;
		thread->stack     = vn;
		thread->stack_end = vn+nl;

		StackData* rpz = rp;
		#define rp rpz
		for(;;)
		{
			switch(RType)
			{
			case rtDO:		RDrop(RCountDo); continue;
			case rtGKauf: 	RDrop(RCountGKauf); continue;
			case rtTRY:		RPeekTryVp() += d; RDrop(RCountTry); continue;
			case rtCALL:	RPeekCallVp() += d; RDrop(RCountCall); continue;
			case rtINSTR:
			case rtPROC:	RPeekProcOrInstrVp() += d;
							RDrop(RCountProcOrInstr); continue;
			case rtEVAL:
			case rtINCLUDE:	RPeekEvalOrIncludeVp() += d;
							RDrop(RCountEvalOrInclude); continue;
			case rtQUITAPPL:
			case rtTERMI:	LOOP;
			default:		goto ERR_IERR;
			}
			goto ERR_VSTACKBOGUS;
		}
		#undef rp
	}


	case tRETURN0:		//	tRETURN0 --|
		for(;;)
			switch(RType)
			{
			case rtDO:
			case rtTRY:		RDropTryOrDo(); continue;
			case rtGKauf: 	RDropGKauf(); continue;
			case rtPROC:	goto ERR_NORESULT;
			case rtINSTR:	RPopProcOrInstr(); DISP;
			default:		Push(zero); goto tend;
			}

	case tRETURN:		//	<value> tRETURN --|
		Assert1Arg();
		for(;;)
			switch(RType)
			{
			case rtDO:
			case rtTRY:		RDropTryOrDo(); continue;
			case rtGKauf: 	RDropGKauf(); continue;
			case rtINSTR:	Drop();
			case rtPROC:	RPopProcOrInstr(); DISP;
			default:		goto tend;
			}

	case tEOF:		//	tEOF --|
		Push(zero);
		goto tend;

	case tEND:		//	<value> tEND --|
	{	Assert1Arg();
tend:	Var* result = TopPtr; vp--;
		for(;;)
		{
			switch(RType)
			{
			case rtDO:
			case rtTRY:		RDropTryOrDo(); continue;
			case rtGKauf: 	RDropGKauf(); continue;
			case rtINSTR:
			case rtPROC:	RDropProcOrInstr(); continue;

			case rtCALL:	RPopCall(); break;
			case rtEVAL:
			case rtINCLUDE:	RPopEvalOrInclude(); break;

			case rtQUITAPPL:vp++; TopPtr=result; goto QUIT_APPL;
			case rtTERMI:	goto termi_self;
			default:		goto ERR_IERR;
			}
			break;
		}
		vp++; TopPtr = result;
	 	DISP;
	}

	case tCHECKSTACKS:
	{
		StackData* rpz = rp;
		#define rp rpz
		for(;;)
		{
			switch(RType)
			{
			case rtDO:		RDrop(RCountDo); continue;
			case rtGKauf: 	RDrop(RCountGKauf); continue;
			case rtTRY:		if(RPeekTryVp()==vp) QUICK; else break;
			case rtCALL:	if(RPeekCallVp()==vp) QUICK; else break;
			case rtINSTR:
			case rtPROC:	if(RPeekProcOrInstrVp()==vp) QUICK; else break;
			case rtEVAL:
			case rtINCLUDE:	if(RPeekEvalOrIncludeVp()==vp) QUICK; else break;
			case rtQUITAPPL:
			case rtTERMI:	if(vp+1==va) QUICK; else break;
			default:		goto ERR_IERR;
			}
			goto ERR_VSTACKBOGUS;
		}
		#undef rp
	}


	case tTERMI:		//	<value> tTERMI --
		Assert1Arg();
	{	TopReqTid();
		Thread* t = Top.GetThread();
		if(t==t_root) goto ERR_TERMIROOT;
		if(t!=thread) 			// terminate other
		{
			// TODO: delete thread  =>  handle thread Var
			delete t;
			DROP;
		}
	}

	// terminate self
	// entry also from: tEND
termi_self:
		assert(t_zombie==NULL);	// kann nicht sein: t_zombie wird nur hier gesetzt
		t_zombie = thread;			// => DISP => Threadwechsel bevor irgendwer weiter läuft
									// => Registerwechsel => kill zombie
		thread->unlink_running();
		ip--;						// if we are restarted: must work for tTERMI and tEND
		SWITCH;


	case tSPAWN:	//	<value> <n*temp> <value> tSPAWN.n.m <m*opcode> -- <temp>
	{				// TODO: tSPAWN: optimize: direkt in proc starten
		Assert1Arg();
		TopReqNumber();
		float prio = Top.Value(); Drop();
		int   argc = *ip++;				// 	n
		int   skip = *ip++;				// 	m

		AssertNArgs(argc+1);
		for(int i=1;i<=argc;i++) ArgNAssertTemp(i);

		Thread* t = new Thread(thread,prio,ip,argc+1+RCountProcOrInstr);
		assert( t->vp+argc+1+RCountProcOrInstr < (Var**)t->rp );

		// transfer  proc addr <value>  &  arguments <n*temp>
		t->vp += argc+1;
		for(int i=0;i<=argc;i++) { t->vp[-i] = TopPtr; vp--; }

		Push( new Var(t) );				//	result: tid
		threads->AppendItem( TopPtr );	//	also prevents extinction if the returned tid is discarded

		assert(*ip==tEXECINSTR);
		assert(ip==t->ip);

		ip += skip;						// skip <m*opcode>
		SWITCH;
	}


	{	NameHandle nh; displacement d; Var* r;

						//	search for name everywhere
	case tEXISTS:		//	tEXISTS.namehandle.dist -- <var>
						//							-- <zero>  & branch
		nh = peek4(ip); ip+=4;
texi:	r = FindVar(nh,thread->locals); 				if(r) goto xpr;
		if( thread->proc->IsLinked() )
		{
			r = thread->proc->Parent()->FindItem(nh); 	if(r) goto xpr;
			r = FindVar(nh,thread->globals); 			if(r) goto xpr;
		}
		goto xp0;

						//	search for name everywhere
	case tEXISTSAT:		// 	<value> tEXISTSAT.dist -- <var>
		Assert1Arg();
		TopReqString();
		nh = FindNameHandle(Top.Text()); 	// unlocked NH
		if(nh!=0||Top.TextLen()==0) { Drop(); goto texi; }	// NH exists. somewhere.
		goto xd1p0;							// NH does not exist => no so-named Var exists

						//	search for name in supplied list
	case tEXIIDF:		//	<var> tEXIIDF.namehandle.dist -- <var>
						//								  -- <zero>  & branch
		nh = peek4(ip); ip+=4;
		Assert1Arg();
		TopAssertVar();	// wg. DropPush(r)		TODO: das sollte jetzt überflüssig sein
texidf:	r = Top.FindItem(nh);	 	// note: FindItem() returns NULL if this is no list
		if(r) goto xd1pr;			// drop, push r
		goto xd1p0;

	case tEXIAT:		//	<var> <value> tEXIAT.dist -- <var>
						//							  -- <zero>  & branch
		Assert2Args();
		TopReqString();
		Arg2AssertVar();	// wg. DropPush(r)		TODO: das sollte jetzt überflüssig sein
		nh = FindNameHandle(Top.Text()); 	// unlocked NH
		if(nh!=0||Top.TextLen()==0) { Drop(); goto texidf; }	// NH exists. somewhere.
xd2p0:	Drop();
xd1p0:	Drop();
xp0:	Push(zero);
		PeekDist(); ip+=d;
		QUICK;

	case tEXIIDX:		//	<var> <value> tEXIIDX.dist -- <var>
						//							   -- <zero>  & branch
		Assert2Args();
		TopReqNumber();
		Arg2AssertVar();	// wg. DropPush(r)		TODO: das sollte jetzt überflüssig sein
		if(Arg2.IsNoList()) goto xd2p0;
		{	ulong n = Top.LongValue() -1;
			if( n>=Arg2.ListSize() ) goto xd2p0;
			r = &Arg2[n];
		}
		Drop();
xd1pr:	//TopReqVar();
		Drop();
xpr:	Push(r);
		ip+=sizeof(d);
		QUICK;

	}	// nh, d, r


	{	NameHandle nh; Var* r;

					//	search for name everywhere
					//	name must exist
	case tIDF:		//	tIDF.namehandle -- <var>
					//	Suche in: privates..locals, locals, proc, globals..root, root
		nh = peek4(ip); ip+=4;
tidf:	r = FindVar(nh,thread->locals); if(r) { pir: Push(Deref(r)); QUICK; }
		if(thread->proc->IsLinked())
		{ 	r = thread->proc->Parent()->FindItem(nh);	if(r) goto pir;
			r = FindVar(nh,thread->globals); 			if(r) goto pir;
		}
		Push(new Var(GetNameForHandle(nh)));
		goto ERR_VARNOTFOUND;

					//	search for name everywhere
					//	name must exist
					//	@<text>
	case tAT:		// 	<value> tAT -- <var>
		Assert1Arg();
		TopReqString();
		nh = FindNameHandle(Top.Text()); 	// unlocked NH
		if(nh!=0||Top.TextLen()==0) { Drop(); goto tidf; }	// NH exists. somewhere.
		// NH does not exist => no so-named Var exists
		goto ERR_VARNOTFOUND;

					// 	search for name in supplied list
					//	auto-create
					//	var <var>.<idf>
	case tVARIDF:	// 	<var> tVARIDF.namehandle -- <var>
		Assert1Arg();
		nh = peek4(ip); ip+=4;
		TopReqVar();	// wg. DropPush()		TODO: das sollte jetzt überflüssig sein
	//	TopReqList();
tvari:	r = Top.FindItem(nh);
		if(r) { DropPush(Deref(r)); QUICK; }
		else  { if(Top.IsNoList()) { Top.SetList(); if(errno)
															ERROR; }
				DropPush(new Var(TopPtr,at_end,isNumber,nh)); QUICK; }

					// 	search for name in supplied list
					//	auto-create
					//	var <var>.@<text>
	case tVARAT:	//	<var> <value> tVARAT -- <var>
		Assert2Args();
		TopReqString();
		Arg2ReqVar();	// wg. Drop2Push()
	//	Arg2ReqList();
		nh = FindNameHandle(Top.Text()); 	// unlocked NH
		if(nh!=0||Top.TextLen()==0) { Drop(); goto tvari; }		// NH exists. somewhere.
		// NH does not exist => no so-named Var exists
tvat:	if(Arg2.IsNoList()) Arg2.SetList(); if(errno)
														ERROR;
		r = new Var(Arg2Ptr,at_end,isNumber,Top.Text());
		Drop2Push(r);
		QUICK;

					// 	search for name in supplied list
					//	name must not exist  -> create
					//	new <var>.<idf>
	case tNEWIDF:	//	<var> tNEWIDF.namehandle -- <var>
		Assert1Arg();
		nh = peek4(ip); ip+=4;
		TopReqVar();	// wg. DropPush			TODO: das sollte jetzt überflüssig sein (aber nicht sinnlos)
	//	TopReqList();
tnewi:	r = Top.FindItem(nh); if(r) goto ERR_VAREXISTS;
		if(Top.IsNoList()) Top.SetList(); if(errno)
													ERROR;
		r = new Var(TopPtr,at_end,isNumber,nh);
		DropPush(r);
		QUICK;

					// 	search for name in supplied list
					//	name must not exist  -> create
					//	new <var>.@<text>
	case tNEWAT:	//	<var> <value> tNEWAT -- <var>
		Assert2Args();
		TopReqString();
		Arg2ReqVar();
	//	Arg2ReqList();
		nh = FindNameHandle(Top.Text()); 	// unlocked NH
		if(nh!=0||Top.TextLen()==0) { Drop(); goto tnewi; }		// NH exists. somewhere.
		// NH does not exist => no so-named Var exists
		goto tvat;


					// 	search for name in supplied list
					//	must exist
					//	var <var>.<idf>
	case tDOT:		// 	<value|temp|const> tDOT.namehandle -- <value|temp|const>
		Assert1Arg();
		TopReqList();
		nh = peek4(ip); ip+=4;
tdoti:	r = Top.FindItem(nh);
		if (r) { DropPush(Deref(r)); QUICK; }
		r = new Var( Top.Name() + "." + GetNameForHandle(nh) );		// -> for printing error msg
		DropPush(r);				// 2005-07-11 DropPush() statt Push(): stack size for one more item is not asserted!
		goto ERR_VARNOTFOUND;

					// 	search for name in supplied list
					//	must exist
					//	new <var>.@<text>
	case tDOTAT:	//	<var|temp|const> <value> tDOTAT -- <var|temp|const>
		Assert2Args();
		Arg2ReqList();
		TopReqString();
		nh = FindNameHandle(Top.Text()); 	// unlocked NH
		if(nh!=0||Top.TextLen()==0) { Drop(); goto tdoti; }		// NH exists. somewhere.
		// NH does not exist => no so-named Var exists
		TopReqTemp();
		Top.Text() = Arg2.Name() + "." + Top.Text();
		goto ERR_VARNOTFOUND;

	}	// nh, r

					// 	search for name in supplied list
					//	name may or may not exist  -> create
					//	var <var>[<idx>]
	case tVARIDX:	//	<var> <value> tVARIDX -- <var>
	{	Assert2Args();
		TopReqNumber();
		Arg2ReqVar();
	//	Arg2ReqList();
		if(Arg2.IsNoList()) Arg2.SetList(); if(errno) ERROR;
		ulong n = Top.LongValue() -1;
		if( n>=Arg2.ListSize() )
		{ 	if(n>>26) goto ERR_IDXOORANGE;
			Arg2.ResizeList(n+1);
		}
		Drop2Push(Deref(&Arg2[n]));
	}	QUICK;

					// 	create item at index in supplied list
					//	item must not exist  -> create
					//	new <var>[<idx>]
	case tNEWIDX:	// 	<var> <value> tNEWIDX -- <var>
	{	Assert2Args();
		TopReqNumber();
		Arg2ReqVar();
	//	Arg2ReqList();
		if(Arg2.IsNoList()) Arg2.SetList(); if(errno) ERROR;
		ulong n = Top.LongValue() -1;
		if( n<Arg2.ListSize() ) goto ERR_VAREXISTS;
		if(n>>26) goto ERR_IDXOORANGE;
		Arg2.ResizeList(n+1);
		Drop2Push(&Arg2[n]);
	}	QUICK;


	case tIDX:			//	<value> <value> tIDX -- <value>
						//	<itemref> = <list>[idx]
	{					//	<char> = <text>[idx]
		Assert2Args();
		TopReqNumber();
		long n = Top.LongValue() -1;
		if(n>>26) goto ERR_IDXOORANGE;

		if(Arg2.IsList())
		{
			Var*r = Deref(&Arg2[n]);
			r->incr_lock();		// if Arg2IsTemp, eg.: {1,2,3}[1], then r is not implicitely protected from destruction!
			Drop2Push(r);
			r->decr_lock();
			LOOP;
		}
		else if(Arg2.IsText())
		{
			Arg2ReqTemp();
			Arg2.Text() = String(Arg2.Text(),n,n+1);
			DROP;
		}
		else goto ERR_STRORLISTREQ;
	}

	{	long a,e;

	case tTO_IDX:		//	<value> <value> tTO_IDX -- <temp>
		Assert2Args();
		TopReqNumber();
		a=0; e = Top.LongValue();
		goto toto;

	case tIDX_TO:		//	<value> <value> tIDX_TO -- <temp>
		Assert2Args();
		TopReqNumber();
		e = 1<<26; a = Top.LongValue() -1;
		goto toto;

	case tIDX_TO_IDX:	//	<value> <value> <value> tIDX_TO_IDX -- <temp>
		Assert3Args();
		TopReqNumber();
		Arg2ReqNumber();
		a = Arg2.LongValue() -1;
		e = Top.LongValue();
		Drop();
toto:
		if(Arg2.IsList())
		{
			Var*v=new Var(isList);
			if(a<0) a=0;
			if(e>(long)Arg2.ListSize()) e=Arg2.ListSize();
			for(;a<e;a++)
			{
				Var* z = new Var(Arg2[a]); z->SetName(Arg2[a].name);
				v->AppendItem(z);
			}
			Drop2Push(v);
			LOOP;
		}
		else if(Arg2.IsText())
		{
			Arg2ReqTemp();
			Arg2.Text() = String(Arg2.Text(),a,e);
			DROP;
		}
		else goto ERR_STRORLISTREQ;

	}	// long a,e



// missing impl:
    case tREQUIRE:		// TODO
    case tIOCTL:		// TODO
		goto ERR_NIMP;

// dürfen nicht mehr vorkommen:
	case tANDANDGL:		// kann nicht wg. pruning
	case tORORGL:		// kann nicht wg. pruning
	case tEC:			// compiled as tNUM
	case tPI:			// compiled as tNUM
	case tNEWLINE:		// compiled as tNUM
	case tTAB:			// compiled as tNUM
	case tNOP:
	case tSEMIK:
	case tKOMMA:
	case tTHEN:
	case tRKauf:
    case tRKzu:
    case tEKauf:
    case tEKzu:
	case tFROM:
    case tTO:
    case tPIPE:
    case tSOCK:
    case tCONVERT:
    case tKOMM:
    case tKANF:
    case tKEND:
//    case tPWD:
    case tNEW:
    case tVAR:
		goto ERR_IERR;


    case tPRIO:			//	<value> tPRIO -- <temp>
		Assert1Arg();
		TopReqTid();
	{	Thread* t = Top.GetThread(); Drop(); Push(new Var(t->GetPrio()));
	}	QUICK;

	case tDEL:			//	<var> tDEL --
	/*	tDEL löscht keine Variablen, sondern hängt sie aus dem Variablenbaum aus,
		wodurch sie zumeist letztmalig entriegelt und somit auch gelöscht werden.
		Jeder, der sich für irgendeine Variable interessiert, muss sie locken!
	*/
		Assert1Arg();
		TopReqVar();
		if(Top.IsUnlinked()) goto ERR_TOPNOPARENT;
		Top.Vanish();				// Aushängen und Lücke schließen
		DROP;						// und runter vom Stack (und löschen)

	case tRENAME:		//	<var|temp> <value> tRENAME -- <var|temp>
		Assert2Args();
		TopReqString();
		Arg2AssertNonConst();
		Arg2.SetName(Top.Text());
		DROP;

	case tRENAME_NH:	//	<temp|var> tRENAME_NH.namehandle -- <temp|var>
		Assert1Arg();
		TopAssertNonConst();
		Top.SetName(NameHandle(peek4(ip))); ip+=4;
		LOOP;

	case tCD:			//	<value> tCD --
		Assert1Arg();
		TopReqString();
		ChangeWorkingDir( Top.Text() );			// TODO: set wdir for FullPath()
		thread->wdir = WorkingDirString();
		DROP;

	case tSWAP:			//	<var> <var> tSWAP --
		Assert2Args();
		TopAssertVar();
		Arg2AssertVar();
		if(TopPtr!=Arg2Ptr) Top.SwapData(Arg2);
		DROP2;


	{	Stream* s;

	case tPUTBACK:			//	<stream> <value> tPUTBACK -- <stream>
		Assert2Args();
		Arg2ReqStream();

		Arg2.GetStream()->PutbackString( Top.IsText() ? Top.Text() : Top.ToString(no,DisassString) );
		DROP;


	case tPUT:				//	<stream> <value> tPUT -- <stream>
		Assert2Args();

		if( Top.IsNoText() )
		{
			String t = Top.ToString(no,DisassString);
			Drop();
			Push( new Var(t) );
		}
		goto tw;


	case tWRITE:			//	<stream> <value> tWRITE -- <stream>
		Assert2Args();
		TopReqString();
tw:		Arg2ReqStream();

		s = Arg2.GetStream();
		if( s->OutputInProgress() )		// -> wait for free
		{
			double d = s->LastOutput()+s->OutputTimeout() - SystemTime();
			if( d<=0.0 ) goto io_timeout;
			s->OutputInterrupt().Shedule( d );
			ip--;						// when resumed execute tWRITE again
			SWITCH;
		}

		if(ip[-1]==tPUT) s->PutString( Top.Text() );
		else if( Top.Text().Csz()!=csz1 && Top.Text().ReqCsz()!=csz1 ) goto ERR_CSZ1REQ;
		else s->Write( Top.Text() );
		if(errno==ok) DROP;
		goto io_error;

	case tGETCHAR:		//	<stream> <var> tGETCHAR -- <stream>
		Assert2Args();
		Arg2ReqStream();
		TopReqVar();
		if(Top.IsNoText()) { Top = emptyString; if(errno) ERROR; }

		s = Arg2.GetStream();
		if(s->GetCharAvail()) s->GetChar(Top.Text(),test_io);
		else				  Top.Text().Clear();
		if(errno==ok) DROP;
		goto io_error;

	case tREAD:				//	<stream> <var> <const:0:1> tREAD -- <stream>
		Assert3Args();
		Arg3ReqStream();
		Arg2ReqVar();
		Arg2ReqString();
		TopAssertNumber();

		s = Arg3.GetStream();
		s->Read( Arg2.Text(), TopPtr==eins ? resume_io : start_io );
		goto ww;

	case tGET:				//	<stream> <var> <const:0:1> tGET -- <stream>
		Assert3Args();
		Arg3ReqStream();
		Arg2ReqVar();
		if(Arg2.IsNoText()) { Arg2.SetString(); if(errno) ERROR; }
		TopAssertNumber();

		s = Arg3.GetStream();
		s->GetString( Arg2.Text(), TopPtr==eins ? resume_io : start_io );
		goto ww;

ww:		if( errno==ok ) DROP2;
		if( errno!=EAGAIN ) goto io_error;

		{
		double d = s->LastInput()+s->InputTimeout() - SystemTime();
		if( d<0.0 ) goto io_timeout;
		s->InputInterrupt().Shedule( d );
		}
		errno=ok;
		if(TopPtr!=eins) DropPush(eins);
		ip--;
		SWITCH;

	case tEDIT:				//	<stream> <var> <const:0:1> tEDIT -- <stream>
		Assert3Args();
		Arg3ReqStream();
		Arg2ReqVar();
		Arg2ReqString();
		TopAssertNumber();

		s = Arg3.GetStream();
		if( s->NoTTY() ) goto ERR_TTYREQ;

		s->GetString( Arg2.Text(), TopPtr==eins ? resume_io : edit_io );
		goto ww;

	case tOPEN:		//	<var> <value> tOPEN --
		Assert2Args();
		Arg2AssertVar();
		TopReqString();
		Arg2 = new Stream( Top.Text(), O_RDWR|O_CREAT );
		if(errno==ok) DROP2;
		goto io_error;

	case tOPENOUT:	//	<var> <value> tOPENOUT --
		Assert2Args();
		Arg2AssertVar();
		TopReqString();
		Arg2 = new Stream( Top.Text(), O_WRONLY|O_CREAT|O_TRUNC );
		if(errno==ok) DROP2;
		goto io_error;

	case tOPENIN:	//	<var> <value> tOPENIN --
		Assert2Args();
		Arg2AssertVar();
		TopReqString();
		Arg2 = new Stream( Top.Text(), O_RDONLY );
		if(errno==ok) DROP2;
		goto io_error;

	case tCLOSE:	//	<stream:var> tCLOSE --			TODO: wait for OutputFinished()  !!!
		Assert1Arg();
		TopAssertVar();
		TopReqStream();
		Top = 0.0;		// -> delete Stream			TODO
		DROP;

io_timeout:
		errno = timeoutexpired;
io_error:
		LogLine("error in file: ",s->FilePath().CString());		// TODO: remove
		PrependToError( s->FilePath() + ": " );
		ERROR;

	}	// Stream* s

	case tSYS:		// 	<value> <n*value> tSYS.n -- <temp>
	{
		int argc = *ip++;
		AssertNArgs(argc+1);
		ArgNReqString(argc+1);

		str* argv = new str[argc+2];

		argv[argc+1] = NULL;
		for (int i=argc;i>=0;i--)
		{
			argv[i] = newcopy(Top.ToString().CString()); Drop();
		}
		str result = execCmd(argv);
		Push(new Var(String(result?result:"")));
		delete[] result;

		for (int i=0;i<=argc;i++) delete[] argv[i];
		delete[] argv;
		LOOP;
	}


	}	// switch( *ip++ )




ERR_VARNOTFOUND:	SetError( variablenotfound,
						String("Variable ")+ToQuoted(Top.Text())+" not found" );
					ERROR;

ERR_TOPNOPARENT:	SetError( topnoparent,
						String("Variable ")+Top.Name().ToQuoted()+" cannot be deleted" );
					ERROR;


ERR_NIMP:			errno = notyetimplemented;				ERROR;

#if defined(DEBUG)
ERR_STACKOVERFLOW:	SetError("stack overflow"); 				ERROR;
ERR_VSTACKUNDERFLOW:SetError("variables stack underflow"); 		ERROR;
ERR_RSTACKUNDERFLOW:SetError("return stack underflow"); 		ERROR;
#endif
ERR_VSTACKBOGUS:	SetError("variable stack bogus"); 			ERROR;

ASSERT_ISTEMP:		SetError("assertion failed: IsTemp()"); 	ERROR;
ASSERT_ISVAR:		SetError("assertion failed: IsVar()"); 		ERROR;
ASSERT_ISNONCONST:	SetError("assertion failed: IsNonConst()");	ERROR;
//ASSERT_ISSTRING:	SetError("assertion failed: IsString()");	ERROR;
//ASSERT_ISSTREAM:	SetError("assertion failed: IsStream()");	ERROR;
ASSERT_ISNUMBER:	SetError("assertion failed: IsNumber()");	ERROR;
//ASSERT_ISLIST:   	SetError("assertion failed: IsList()");		ERROR;
ASSERT_VSTACK:		SetError("assertion failed: variables stack underflow");	ERROR;

ERR_VARREQ:			errno = varreq;			ERROR;
ERR_NUMREQ:			errno = numreq;			ERROR;
ERR_STRREQ:			errno = stringreq;		ERROR;

ERR_STRORLISTREQ:	errno = stringorlistreq;	ERROR;
ERR_LISTREQ:		errno = listreq;			ERROR;
ERR_TIDREQ:			errno = tidreq;				ERROR;
ERR_IRPTREQ:		errno = irptreq;			ERROR;
ERR_SEMAREQ:		errno = semareq;			ERROR;
ERR_TIMEOUT:		errno = timeoutexpired;		ERROR;
ERR_IERR:			errno = internalerror;		ERROR;
ERR_PROCREQ:		errno = procreq;			ERROR;
ERR_TOOFEWARGUMENTS:errno = toofewarguments;	ERROR;
ERR_TOOMANYARGUMENTS:errno = toomanyarguments;	ERROR;
ERR_NORESULT:		errno = noresult;			ERROR;
ERR_TERMIROOT:		errno = termiroot;			ERROR;
ERR_IDXOORANGE:		errno = indexoorange;		ERROR;
ERR_VAREXISTS:		errno = varexists;			ERROR;
//ERR_VARINUSE:		errno = varinuse;			ERROR;
//ERR_INTERSECT:  	errno = listsintersect;		ERROR;
ERR_STREAMREQ:		errno = streamreq;			ERROR;
ERR_TTYREQ:			errno = ttyreq;				ERROR;
ERR_CSZ1REQ:		errno = csz1req;			ERROR;
ERR_PROCNOPARENT:	errno = procnoparent;		ERROR;
ERR_VREFCHILDOFDEST:SetError("reference variable is child of target");	ERROR;

	{

error_exit:

		SetThreadError();

		xlogline("%s",errorstr());

compile_error:

		while(rp<re)
		{
			switch(RType)
			{
			case rtDO:		RDropDo(); continue;
			case rtGKauf: 	RPopGKauf(); continue;
			case rtTRY:
				AddToThreadError( ip, emptyString );
				errno=ok;
				RPopTry();
				LOOP;
			case rtPROC:
			case rtINSTR:			// [in file "foo"] in proc "bar" at row=33, col=66: foobar expected
				AddToThreadError( ip, thread->proc->Name()+"()" );
				RPopProcOrInstr();
				continue;
			case rtCALL:			// in file "foo" at row=33, col=66: foobar expected
				AddToThreadError( ip, "called file" );
				RPopCall();
				continue;
			case rtINCLUDE:			// in file "foo" at row=33, col=66: foobar expected
				AddToThreadError( ip, "included file" );
				RPopEvalOrInclude();
				continue;
			case rtEVAL:			// eval: at [row=33,] col=66: foobar expected
				AddToThreadError( ip, "eval expression" );
				RPopEvalOrInclude();
				continue;

			case rtTERMI:
			{
				AddToThreadError( ip, emptyString );
				Log( "\nThread %s died after error: %s\n%s\n", thread->GetName().ToQuoted().CString(), errorstr(), (*thread->error)[2].Text().CString() );

	ee_termi:	errno=ok;

				while(vp>=va) Drop();			// 	just in case the vstack overflowed
				Push( thread->GetVar() );		//	tSELF
				static uchar ts[] = { tTERMI };	//	tTERMI
				ip = ts +1;						//	setup for termi_self to prevent restart
				goto termi_self;
			}

			case rtQUITAPPL:
			{
				AddToThreadError( ip, emptyString );
				AppendToError( "\n" );
				AppendToError( (*thread->error)[2].ToString() );

	ee_quit:	while(vp>=va) Drop();			// 	just in case the vstack overflowed
				Push( thread->GetVar() );		//	tSELF	((serves also as return value))
				static uchar ts[] = { tTERMI };	//	tTERMI
				ip = ts +1;						//	setup to prevent restart
				goto QUIT_APPL;					// 	error still set -> appl will return errno
			}

			default:
				rp=re;
				LogLine("Execute(): error_exit: RStack corrupted.");
				if(thread==t_root) goto ee_quit; else goto ee_termi;
			}
		}
	}

QUIT_APPL:
	xxlogline("/*VScript::Execute()*/");
	assert(thread==t_root);
	ip--;						// -><- restart
	SaveRegisters();			// -> proper purge resources
	return TopPtr;
}



void VScript::InitThreadError ( )		// error = { 0, "", {} }
{
	Var*& v = MyTid()->error;
	assert(v==NULL);

	Init( v, new Var( isList, nh_error ) );
	v->AppendItems (
			new Var( isNumber,	nh_error_number ),
			new Var( isText,	nh_error_text ),
			new Var( isList,	nh_error_stack) );
	v->DeepLockType();
	v->LockData();
}

void VScript::SetThreadError ( )
{
	Var*& v = MyTid()->error;
	if(!v) InitThreadError();

	(*v)[0].Value() = errno;
	(*v)[1].Text()  = ErrorString();
	(*v)[2].SetList();			// clear stack list
}

void VScript::AddToThreadError ( int row, int col, cString& file, cString& info )
{
	Var* v = new Var( isList );
	v->AppendItems (
				new Var( row,  nh_error_row  ),
				new Var( col,  nh_error_col  ),
				new Var( file, nh_error_file ),
				new Var( info, nh_error_info ) );
	(*MyTid()->error)[2].InsertItem( 0, v );
}

void VScript::AddToThreadError ( uptr ip, cString& msg )
{
	Var*  myproc = MyTid()->proc;
	ulong rowcol = myproc->ProcXrefBase()[ ip-1 - myproc->ProcCoreBase() ];
	bool  ex     = myproc->ProcBundle()->ListSize() > bundle_sourcefile;
	AddToThreadError( Row(rowcol)+1, Col(rowcol)+1, ex ? myproc->ProcSourcefile() : emptyString, msg );
}

void VScript::StartCompileError ( cstr msg )
{
	SetThreadError();
	AddToThreadError( compiler.ErrorRow()+1, compiler.ErrorCol()+1, compiler.GetSourcefile(), msg );

}



/* ----	purge resources of thread -------------------
		purge rstack and vstack
		rstack: tTERMI or tROOT left on stack
		vstack: empty
*/
void Thread::PurgeResources ( )
{
	#define thread this
	#define unlock Unlock
// 	note: die macros für lokale variablen in Execute()
//	funktionieren hier auch wg.
	while(rp<re-1)
		switch(RType)
		{
		case rtDO:		 RDropDo(); continue;
		case rtTRY:		 RDropTry(); continue;
		case rtGKauf: 	 RDropGKauf(); continue;
		case rtPROC:
		case rtINSTR:	 RDropProcOrInstr(); continue;
		case rtCALL:	 RDropCall(); continue;
		case rtINCLUDE:
		case rtEVAL:	 RDropEvalOrInclude(); continue;
		case rtTERMI:
		case rtQUITAPPL: rp=re;		// => break
		default:		 LogLine("Thread::PurgeResources(): RStack corrupted ");
						 rp=re;		// => break
		}

	while(vp>=va) Drop();
}








/*	Copyright  (c)	Günter Woigk   2001 - 2019
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


#ifndef Thread_h
#define Thread_h

#include "kio/kio.h"
#include "VString/String.h"
#include "unix/os_utilities.h"
#include "BPObj.h"


class					Var;
class					Sema;			// semaphor
class					Irpt;			// interrupt
class					Thread;
typedef	Thread *		Tid;			// thread identifier


extern long volatile	disp_flag;		// the central dispatcher flags

extern Tid				t_running;		// linked list of all running threads sorted by priority
extern Tid				t_timeshed;		// linked list of all time sheduled threads sorted by alarm time
extern Tid				t_suspended;	// linked list of all threads not running and not time sheduled
extern Tid				t_root;			// main application thread
extern Tid				t_zombie;		// thread to dispose of

//extern ulong			sysLoad[33];	// systerm load sorted by log(2) of prio

#define	MyTid() 		t_running
#define SystemTime()	now()
#define SetDispFlag(M)	disp_flag|=(M)
extern void				StartRTimer	( double delay );


/* ----	bits in disp_flag and Twsp::state ----------------------------
*/
enum									// disp_flag:				Twsp.state:
{										// ----------------			-----------------
	timeshed		= 0x80000000,		// timer expired			sheduled for timer
	irptshed		= 0x40000000, 		// async. interrupt			sheduled for interrupt
	semashed		= 0x20000000, 		//							waiting for semaphor
	suspended		= 0x10000000,		//							thread is suspended
	running			= 0x08000000,		// 							thread willing to run
	control_c		= 0x04000000,		// user entered <ctrl-C>
	signal_input	= 0x02000000,		// async input available
	signal_output	= 0x01000000,		// async output possible
	signal_io		= signal_input | signal_output,
};



enum RStackType
{ 	rtDO,			//
	rtTRY, 			//
	rtGKauf,		// local context
	rtPROC,			// call proc, expect return value
	rtINSTR,		// call proc, expect no return value
	rtCALL,
	rtINCLUDE,
	rtEVAL,
	rtTERMI,		// last entry on rstack: terminate self
	rtQUITAPPL,		// same for initial thread
};
union StackData
{
	RStackType	type;
	size_t		data;
	uptr 		ip;
	Var* 		var;
	Var** 		vp;
};


class Thread : private BPObj
{
friend class VScript;
friend class Var;
friend class Irpt;
friend class Sema;

// dispatcher data:
	long		state;				// running, suspended, irptshed, semashed, timeshed
	float		prio;				// thread priority: lower is better
	float		prio_org;			// nominal priority (most times same as prio)
	Sema*		blocked_sems;		// linked list of Semas blocked by this thread

	Tid			next,prev;			// linking in t_running, t_suspended or t_timeshed list
	Tid			next_w, prev_w;		// linking in sema.wait or Irpt.wait

	union
	{	Sema *	sema;				// Sema for which thread is waiting
		Irpt  *	irpt;		// Irpt for which thread is sheduled
	};
	double		time;				// alarm time for which thread is sheduled
	ulong		ptime;				// accumulated process time

// interpreter data:
	Var*		proc;				// current proc -> bundle, constants, core
	Var*		globals;
	Var*		locals;

	StackData*	stack_end;			// return stack start
	StackData*	rp;					// return stack pointer
	Var**		vp;					// variables stack pointer
	StackData*	stack;				// variables stack start

	uchar*		ip;					// instruction pointer

// thread data:
	String		wdir;				// akt. arbeitsverzeichnis
	Var*		error;				// { number=<number>, message=<text>,
									//	 stack={ { row=<number>, col=<number>, file=<text>, info=<text> } [, …] }
									// }


	void		unlink_running 		( );
	void 		unlink_suspended	( );
	void 		unlink_timeshed 	( );
	void 		unlink_semashed 	( );
	void 		unlink_irptshed 	( );
	void		link_running 		( );
	void 		link_suspended 		( );
	void 		link_timeshed 		( double alarmtime );
	void 		link_semashed 		( Sema* );
	void 		link_irptshed 		( Irpt  * );
	void		change_prio 		( float newprio );

	void		PurgeResources		( );					// in Execute.cp


				Thread				( const Thread& );		// prohibit
	Thread&		operator=			( const Thread& );		// prohibit

public:
				~Thread				( );
	// initial thread:
				Thread				( Var* proc, Var* globals, float prio );
	// spawned thread: starts operation in parent's context at parent's ip
				Thread				( Thread* parent, float prio, uptr ip, ulong minstacksize );

	double&		Timeout				( )			{ return time; }
	ulong&		Ptime				( )			{ return ptime;}

	float		GetPrio				( )			{ return prio_org; }

// the following methods may change the currently served thread:
// => remember old t_running and switch registers to new t_running!
	void		Termi				( );
	void		Suspend				( );
	void		Resume				( );
	void		Wait				( double );	// wait duration
	void		Shedule				( double );	// wait until clock
	void		ChangePrio			( float );	// change thread's priority
};





#endif













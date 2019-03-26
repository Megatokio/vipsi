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

	2002-04-28 kio	changed Stream io[] to Stream* io[] to allow use of subclasses of Stream
					added OpenStream() and CloseStream()
	2003-07... kio	changes to use UCS4String library
*/

#define SAFE 3
#define LOG 1
#include "config.h"
#include "unix/os_utilities.h"
#include "Thread.h"
#include "token.h"
#include "SystemIO.h"
#include "Var/Var.h"


INIT_MSG


#define	XXCHECK(N)
#ifndef NDEBUG
#define	STACKSIZE		1
#else
#define	STACKSIZE		500
#endif

/* ----	data ---------------------------------------------------
*/
int32 volatile	disp_flag	= 0;	// the central dispatcher flags

Tid				t_running	= 0;	// linked list of all running threads sorted by priority
Tid				t_timeshed	= 0;	// linked list of all time sheduled threads sorted by alarm time
Tid				t_suspended	= 0;	// linked list of all threads not running and not time sheduled

Tid				t_root		= 0;	// root thread: twsp is the application's heap: must not be deleted!
Tid				t_zombie	= 0;	// thread to dispose of in Switch()

//uint32			sysLoad[33];		// systerm load sorted by log(2) of prio




/* ----	setup root thread -------------------------------------
*/
Thread::Thread ( Var* pro, Var* globs, float pri )
{
// twsp:
	wdir  = WorkingDirString();
	error = nullptr;

// dispatcher:
	t_root     	 = this;			// global
	state		 = 0;
	prio		 = pri;
	prio_org	 = pri;
	blocked_sems = nullptr;
//	next 	= prev   = nullptr;
//	next_w 	= prev_w = nullptr;
//	sema	= nullptr;
//	irpt	= nullptr;
//	time 	= 0.0;
	ptime 	= 0;
	link_running();

// interpreter:
	assert(globs->TypeIsLocked());
	Init(proc,pro);
	Init(locals,globs);
	Init(globals,globs);

	stack 	  = new StackData[STACKSIZE];
	stack_end = stack +STACKSIZE;
	rp 	= stack_end-1; rp->type = rtQUITAPPL;		// last stack entry: => terminate => quit_appl
	vp  = (Var**)stack-1;

	ip 	= proc->ProcCoreBase();
}


/* ----	setup spawned Thread -------------------------------------
*/
Thread::Thread ( Thread* starter, float pri, uptr ip0, uint32 stacksize )
:	wdir(starter->wdir)
{
	error = nullptr;

// setup for dispatcher:
	state		 = 0;
	prio		 = pri;
	prio_org	 = pri;
	blocked_sems = nullptr;
//	next   = prev   = nullptr;
//	next_w = prev_w = nullptr;
//	sema	= nullptr;
//	irpt	= nullptr;
//	time 	= 0.0;
	ptime	= 0;
	link_running();

// setup stacks:
	stacksize += STACKSIZE;
	stack 	  = new StackData[stacksize];
	stack_end = stack +stacksize;
	rp 	= stack_end-1; rp->type = rtTERMI;	// security: last stack entry: => just die
	vp  = (Var**)stack-1;

/*	setup interpreter:
	setup identisch zum starter
	=> 	legale werte sind leicht puplizierbar
	=> 	nachteil: insbesondere starter-> proc und starter->locals
		bleiben während der lebenszeit dieses threads gelockt.
*/
	Init(proc,starter->proc);
	Init(globals,starter->globals);
	Init(locals,starter->locals); 		// evtl. auch starter->globals
	ip = ip0;
}


/* ----	Destructor ------------------------
*/
Thread::~Thread()
{
	if(this==t_root) t_root=nullptr;

// remove from all dispatcher lists
	{
		int32 s=state; if(s)
		{
			if (s&running)   unlink_running();	else
			if (s&timeshed)  unlink_timeshed(); else
			if (s&suspended) unlink_suspended();
	}	}

// clear owner in all owned semaphors
	for( Sema*n,*s=blocked_sems; s; s=n )
	{
		n=s->next;
		s->owner = nullptr;
		s->next  = nullptr;
		s->prev  = nullptr;
	}

	PurgeResources();

	Kill(locals);
	Kill(globals);
	Kill(proc);
	Kill(error);

	delete[] stack;
}






/* ----	unlink thread from list of running threads ------------------------------
*/
void Thread::unlink_running ( )
{
	assert(state==running);

	state = 0;

	if (prev)	prev->next = next;
	else		t_running  = next;

	if (next)	next->prev = prev;
}


/* ----	unlink thread from list of suspended threads ----------------------------
*/
void Thread::unlink_suspended ( )
{
	assert((state&~(irptshed|semashed)) == suspended);

	state &= ~suspended;

	if (prev)	prev->next  = next;
	else		t_suspended = next;

	if (next)	next->prev  = prev;
}


/* ----	unlink thread from list of time sheduled threads -------------------------
*/
void Thread::unlink_timeshed ( )
{
	assert((state&~(irptshed|semashed)) == timeshed);

	state &= ~timeshed;

	if (next)	next->prev = prev;
	if (prev)	prev->next = next;
	else	  	t_timeshed = next;
}


/* ----	link thread into list of suspended threads --------------------------
		this list is unsorted
		thread may be additionally linked to a semashed or irptshed list
*/
void Thread::link_suspended ( )
{	int32 s = state;
{
	if (s)
	{
		if (s&suspended) goto x;
		if (s&running)  unlink_running();	else
		if (s&timeshed) unlink_timeshed();
	}

	state |= suspended;

	Tid t = t_suspended;
	t_suspended = this;
	next = t;
	prev = nullptr;
	if (t) t->prev = this;
}
x:	XXCHECK();
}



/* ----	link thread into list of time sheduled threads ---------------------------
		this list is sorted by alarm time
		thread may be additionally linked to a semashed or irptshed list
*/
void Thread::link_timeshed ( double alarmtime )
{
	double delay = alarmtime -SystemTime();

	if( delay <= 0.000010 )
	{
		link_running();
	}
	else
	{
		int32 s = state;

		if (s)
		{
			if (s&running)   unlink_running();		else
			if (s&suspended) unlink_suspended();	else
			if (s&timeshed)  unlink_timeshed();
		}

		state |= timeshed;
		time   = alarmtime;

		Tid t = t_timeshed;
		Tid p = nullptr;
		while ( t && alarmtime>t->time ) { p=t; t=t->next; }

		next = t;
		prev = p;

		if (t) t->prev    = this;
		if (p) p->next    = this;
		else { t_timeshed = this; StartRTimer(delay); }
	}

	XXCHECK();
}


/* ----	link thread into list of running threads ----------------------------------
		this list is sorted by priority
*/
void Thread::link_running ( )
{	int32 s = state;
{
	if (s)
	{
		if (s&running)   goto x;
		if (s&suspended) unlink_suspended(); else
		if (s&timeshed)  unlink_timeshed();
		if (s&irptshed)  unlink_irptshed();  else
		if (s&semashed)  unlink_semashed();
	}

	state = running;

	Tid t = t_running;
	Tid p = nullptr;
	while ( t && prio>t->prio ) { p=t; t=t->next; }

	next			= t;
	prev			= p;

	if (t) t->prev 	= this;
	if (p) p->next 	= this;
	else t_running 	= this;
}
x:	XXCHECK();
}


/* ----	change priority of thread --------------------------------20.dec.2000-------
*/
void Thread::change_prio ( float newprio )
{
// running threads are sorted by priority
// if priority change results in a position change in the running list
// then the thread must be relinked
	if (state&running)
	{
		if ( newprio>prio ?
			 next&&newprio>next->prio :	// now worse than lower neighbour?
			 prev&&newprio<prev->prio )	// or better than upper neighbour?
		{
			unlink_running();
			prio = newprio;
			link_running();
			return;
		}
	}

// suspended and time-sheduled list are not sorted by priority
// and if priority change does not result in a position change in the running list
// then only update this->prio
	prio = newprio;
}


/* ----	unlink thread from list of threads waiting for a semaphor -----------------
		20.dec.2000: throttle blocking thread if it was boosted
*/
void Thread::unlink_semashed ( )
{
	assert(state&semashed);

	state &= ~semashed;

	if (prev_w)	prev_w->next_w = next_w;
	else		sema->wait     = next_w;

	if (next_w)	next_w->prev_w = prev_w;

// throttle blocking thread if it was boosted
	Tid t = sema->owner;
	if ( t && t->prio==prio && t->prio<t->prio_org )
	{
		float p = t->prio_org;
		for ( Sema*s=t->blocked_sems; s; s=s->next )
		{
			for ( Tid tt=s->wait; tt; tt=tt->next_w )
			{
				if (tt->prio<p) p = tt->prio;
			}
		}
		t->change_prio(p);
	}
}


/* ----	link thread into list of threads waiting for a semaphor ------------
		the list starts in Sema->wait and links through Thread->prev_w/next_w
		the list is no longer sorted by priority		18.dec.2000 kio
		20.dec.2000: boost blocking thread
*/
void Thread::link_semashed ( Sema* sem )
{
	assert(!(state&semashed));
	assert(!(state&irptshed));

	state |= semashed;
	sema = sem;

	Tid t = sem->wait;
	sem->wait = this;
	next_w = t; if (t) t->prev_w = this;
	prev_w = nullptr;

// boost blocking thread
	t = sem->owner;
	if (t && t->prio > prio)
	{
		t->change_prio(prio);
	}

	XXCHECK();
}


/* ----	link thread into list of threads sheduled for an interrupt --------------
		the list starts in the interrupt cell
		the list is unsorted
*/
void Thread::link_irptshed ( Irpt  * i )
{
	assert(!(state&semashed));
	assert(!(state&irptshed));

	state |= irptshed;
	irpt = i;

	Tid t = i->wait;
	i->wait = this;
	next_w = t; if (t) t->prev_w = this;
	prev_w = nullptr;

	XXCHECK();
}


/* ----	unlink thread from list of interrupt sheduled threads ---------------------
*/
void Thread::unlink_irptshed ( )
{
	assert(state&irptshed);

	state &= ~irptshed;

	if (prev_w)	prev_w->next_w = next_w;
	else		irpt->wait     = next_w;

	if (next_w)	next_w->prev_w = prev_w;
}



/* ----	suspend this thread -------------------------
		suspend self or another thread
		-running -timeshed +suspended -irptshed -semashed
		if thread was semashed it will re-request semaphor when resumed or timeout
*/
void Thread::Suspend()
{
	XXCHECK(this);
	xxlogline ( "Suspend(\"%s\")", CString(GetName()) );

//	Tid t = t_running;
	link_suspended();
	if (state&(semashed|irptshed)) state&semashed ? unlink_semashed() : unlink_irptshed();
//	Switch(t);
}


/* ----	resume this thread -------------------------
		resume another thread
		resuming self is legal but has no effect
		+running -timeshed -suspended -irptshed -semashed
		if thread is semashed or was semashed before beeing suspended
		it will re-request semaphor or timeout
*/
void Thread::Resume()
{
	XXCHECK(this);
	xxlogline ( "Resume(\"%s\")", CString(GetName()) );

//	Tid t = t_running;
	link_running();
//	Switch(t);
}


/* ----	time-shedule this thread -------------------------
		time-shedule self or another thread
		-running +timeshed -suspended -irptshed -semashed
		if thread was semashed it will re-request semaphor when resumed or timeout
*/
void Thread::Shedule( double alarmtime )
{
	XXCHECK(this);
	xxlogline ( "Shedule(\"%s\")", CString(GetName()) );

//	Tid t = t_running;
	link_timeshed(alarmtime);
	if (state&(semashed|irptshed)) state&semashed ? unlink_semashed() : unlink_irptshed();
//	Switch(t);
}


/* ----	time-shedule this thread -------------------------
		time-shedule self or another thread
		-running +timeshed -suspended -irptshed -semashed
		if thread was semashed it will re-request semaphor when resumed or timeout
*/
void Thread::Wait( double delay )
{
	XXCHECK(this);
	xxlogline ( "Wait(\"%s\",%f)", CString(GetName()), delay );

//	Tid t = t_running;
	link_timeshed(SystemTime()+delay);
	if (state&(semashed|irptshed)) state&semashed ? unlink_semashed() : unlink_irptshed();
//	Switch(t);
}


/* ----	change priority of thread ---------------------
		prio_org is changed and
		prio is recalculated as necessary
*/
void Thread::ChangePrio ( float newprio )
{
	XXCHECK(this);
	xxlogline ( "ChangePrio(\"%s\",%lu)", CString(GetName()), newprio );

//	Tid t = t_running;

	if (newprio!=prio_org)
	{
		if ( newprio<prio_org )
		{
		// increase priority
		// => no problems with threads boosted by semaphors
			if (newprio<prio) change_prio(newprio);
		}
		else
		{
		// decrease priority
		// => be careful with threads boosted by semaphors
			if (blocked_sems && prio==prio_org)
			{
			// recalculate effective thread priority:
			// lower eff. prio to max of own prio and blocked threads
				for ( Sema*s=blocked_sems; s; s=s->next )
					for ( Tid t=s->wait; t; t=t->next_w )
						if (t->prio<newprio) newprio = t->prio;
			}
			change_prio(newprio);
		}
		prio_org = newprio;
	}

//	Switch(t);
}




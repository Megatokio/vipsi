/*	Copyright  (c)	Günter Woigk 1990 - 2020
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


/* =================================================================
						Semaphors
================================================================= */


/* ----	request a semaphor -----------------------------------------

	basically semaphors are 'binary':
		sema.flag = 0   =>  free
		sema.flag = 1   =>  occupied

	The implementation here expands the posibilities:
		sema.flag <= 0  =>  free
		sema.flag >= 1  =>  occupied

	a semaphor can be released or requested multiple times in one call. this allows
	some 'counting' implementations, e.g. a semaphor which allows access to a certain
	resource for 20 threads simultaneously or, more important, this allows, that the
	current owner of a semaphor is not blocked if it requests a semaphor multiple times.
	this makes encapsulating of modules by semaphors a little bit more easy.

	note:
		semaphors are not automatically released when a thread which blocks them is
		terminated. in an environment which supports access to such locked semaphors you
		could call Release() manually.

	you can request a semaphor with or without timeout:

	without timeout:
		Request() waits unlimited until the semaphor becomes available.
		if it returns, you _have_ the semaphor.
		even if you manually resume a thread which waits for a semaphor without timeout,
		it will not return but resume waiting for the semaphor instead.

	with timeout:
		Request() waits only the given time for the semaphor to become available.
		after that it returns anyway.
		when it returns, you _must_ verify that you have got the semaphor: Sema::IsMine()
		if you manually resume a thread which waits for a semaphor with timeout,
		it will return, most likely without having got the semaphor.
		this is different to the behavior of requests without timeout!

*/

#define SAFE 3
#define LOG 1
#include "kio/kio.h"
#include "Sema.h"

#define XXCHECK(N)
DEBUG_INIT_MSG


/* ----	remove sema from old owner's blocked_sems list -------------------
*/
void Sema::unlink_owner ( )
{
	assert(owner);

	if (prev) { prev->next=next; prev=nullptr; } else owner->blocked_sems = next;
	if (next) { next->prev=prev; next=nullptr; }
	owner = nullptr;
}


/* ----	put sema on new owner's blocked_sems list -----------------------
*/
void Sema::link_owner ( Tid newowner )
{
	if (newowner != owner)
	{
		assert(!owner);
		assert(!next);
		assert(!prev);
		assert(IsBlocked());

		owner = newowner;

		if (newowner->blocked_sems)
		{
			next = newowner->blocked_sems;
			next->prev = this;
		}
		newowner->blocked_sems = this;
	}
}

#define	UNLINK_OWNER()	if (owner) unlink_owner()
#define	LINK_OWNER(t)	if (IsBlocked()) link_owner(t)



/* ----	try to request semaphor --------------------
		2002-07-10 kio version for vipsi
*/
bool Sema::Request ( int n )
{
	Tid t = MyTid();
//	XXCHECK(t);
	xxlogline ( "Sema::Request(%i)", n );

	if (n>0)
	{
		if (IsBlocked() && owner!=t)			// currently blocked?
		{
			t->link_suspended();
			t->link_semashed ( this );
			return no;
		}
		flag += n;
		LINK_OWNER(t);
	}
	return yes;

//	Switch(t);
}


bool Sema::Request_WTO ( double timeout/*Zeitpunkt!*/, int n)
{
	Tid t = MyTid();
//	XXCHECK(t);
	xxlogline ( "Sema::Request(%f,%i)", timeout,n );

	if (n>0)
	{
		if (IsBlocked() && owner!=t)			// currently blocked?
		{
			t->link_semashed ( this );
			t->link_timeshed(timeout);
			return no;
		}
		flag += n;
		LINK_OWNER(t);
	}
	return yes;

//	Switch(t);
}


/* ----	release a semaphor ---------------------------- 07.08.95 --- 28.7.98 ------
		18.dec.2000 reworked kio:
		no longer releasing only minimum number of waiting threads
		because they could get stopped before re-requesting the semaphor
		leaving it free but with threads waiting, possibly never getting
		kicked again...
		=> threads now need no longer to be sorted by priority in the sema's wait list
*/
void Sema::Release ( int n )
{
//	Tid t = MyTid();
//	XXCHECK(t);
	xxlogline ( "Sema::Release(%i)", n );

	if (n>0)
	{
		flag -= n;

		if (IsFree())
		{
			while (wait) wait->link_running();
			UNLINK_OWNER();
		}
	}

//	Switch(t);
}


/* ----	clear semaphor --------------------------18.dec.2000---------
		waiting threads are kicked, if any.
		those without timeout will immediately re-request the semaphor
*/
void Sema::Clear ( )
{
//	Tid t = MyTid();
//	XXCHECK(t);
	xxlogline ( "Sema::Clear()" );

	flag = 0;
	while (wait) wait->link_running();
	UNLINK_OWNER();

//	Switch(t);
}


/* ----	destroy semaphor ------------------------------------
		waiting threads are suspended for security
*/
Sema::~Sema()
{
	while(wait)
	{
		wait->link_suspended();
		wait->unlink_semashed();
	}
	UNLINK_OWNER();
}









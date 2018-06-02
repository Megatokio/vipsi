/*	Copyright  (c)	Günter Woigk   1990-2017
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

#define SAFE 3
#define LOG 1
#include "Irpt.h"
#include "Thread.h"

INIT_MSG


Irpt* volatile		asyncIrpt[32];		// store addr. of asynchronously triggered interrupts
int volatile		asyncCnt[32];		// store interrupt count



/* =================================================================
						Interrupts
================================================================= */


/* ----	trigger interrupt synchronously ---------------
		used while in proper thread context
*/
void Irpt::Trigger ( int n )
{
	xxlogline( "Irpt::Trigger(%i)",n );

	if( n>0 && (flag+=n)>0 )
	{
		while( wait ) { wait->link_running(); }
	}
}


/* ----	trigger interrupt asynchronously -----------------------
		used from interrupts and completion routines etc.
		note: if more than NELEM(asyncIrpt) interrupt sources
		are present in the system and if they all raise interrupts
		some interrupts may be lost. •••we could raise an alert flag here.
*/
void Irpt::AsyncTrigger ( int n )
{
	if( n<=0 ) return;

	for( uint i=0; i<NELEM(asyncIrpt); i++ )
	{
		if( asyncIrpt[i]==NULL )	// slot seems to be free?
		{
			asyncCnt[i] += n;		// request it

			if( asyncCnt[i] == n )	// and we've got it
			{
				asyncIrpt[i] = const_cast<Irpt*>(this);
				SetDispFlag(irptshed);
				return;
			}

			asyncCnt[i] -= n;		// we've interrupted s.o. else, probably Switch()
			continue;
		}

		if( asyncIrpt[i]==this )	// huh fine, we were here a nano second ago
		{
			asyncCnt[i] += n;		// just add  more interrupts
		//	asyncIrpt[i] = this;	// already set
		//	SetDispFlag(irptshed);	// already set
			return;
		}
	}

// if we come here the interrupt is lost
	LogLine( "Irpt::AsyncTrigger(): too many interrupts pending" );
}


/* ----	shedule current thread for interrupt -------------------------
		without timeout
*/
void Irpt::Shedule ( )
{
	xxlogline ( "Irpt::Shedule()" );

	if( IsNotTriggered() )
	{
		Tid t = MyTid();
		t->link_suspended();
		t->link_irptshed(this);
	}
}


/* ----	shedule current thread for interrupt -------------------------
		with timeout
*/
void Irpt::Shedule ( double timeout )
{
	xxlogline( "Irpt::Shedule(%f)", timeout );

	if( IsNotTriggered() && timeout>0 )
	{
		Tid t = MyTid();
		t->link_timeshed(SystemTime()+timeout);
		t->link_irptshed(this);
	}
}


/* ----	destroy interrupt ------------------------------------
		waiting threads remain suspended or time sheduled
*/
Irpt::~Irpt ( )
{
	while( wait ) { wait->unlink_irptshed(); }
}












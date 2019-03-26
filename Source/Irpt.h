/*	Copyright  (c)	Günter Woigk   1990 - 2019
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


#ifndef Irpt_h
#define	Irpt_h

#include "BPObj.h"

typedef class Thread* Tid;
class Irpt;


extern Irpt * volatile	asyncIrpt[32];	// store addr. of asynchronously triggered interrupts
extern int volatile		asyncCnt[32];	// store interrupt count


/* ----	Interrupts --------------------------------------------------
		Irpt.flag	>  0	triggered
		Irpt.flag	<= 0	not yet triggered
*/
class Irpt : private BPObj
{
friend class Var;
friend class Thread;

		long	flag;			// interrupt cell
		Tid		wait;			// linked list of waiting threads

				Irpt			( const Irpt& );					// prohibit
		Irpt&	operator=		( const Irpt& );					// prohibit

public:
				Irpt			( )				:flag(0),wait(0){}	// create cleared interrupt
				~Irpt			( );								// destructor: unlink waiting tasks!

		bool	IsTriggered		( )  const		{ return flag>0;  }	// interrupts pending ?
		bool	IsNotTriggered	( )  const		{ return flag<=0; }	// no interrupts pending ?
		long	Interrupts		( )  const		{ return flag; 	  }	// num. of pending irpts (may be neg.)
		bool	IsWaiting		( )  const		{ return wait!=0; }	// s.o. waiting ?

		void	Clear			( )				{ flag=0; 		  }	// reset to initial state
		void	Clear			( int n )		{ if(n>0)flag-=n; }	// clear n interrupts

		void	AsyncTrigger	( int n=1 );						// trigger interrupt asynchronously

// the following methods may/will change the current thread
// => check t_running after call!
		void	Trigger			( int n=1 );						// trigger interrupt
		void	Shedule			( );								// shedule for interrupt
		void	Shedule			( double timeout );					// shedule with timeout
};






#endif





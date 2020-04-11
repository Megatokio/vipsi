/*	Copyright  (c)	Günter Woigk   1990 - 2020
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


#ifndef Sema_h
#define	Sema_h

#include "kio/kio.h"
#include "Thread.h"
#include "BPObj.h"


class Var;



/* ----	Semaphors ----------------------------------------------------
		sema.flag	> 0		blocked
   		sema.flag 	<=0		free
*/
class Sema : private BPObj
{
friend class Var;
public:

		int32		flag; 		// >0 blocked; <=0 free
		Tid			wait;		// linked list of waiting threads
		Tid			owner;		// Tid of blocking thread

		Sema*		next;		// linked list of all Semas blocked by same owner
		Sema*		prev;

//		Var*		sema_var;	// representer in global tree; unlocked

		void		link_owner	( Tid newowner );
		void		unlink_owner( );

					Sema		( const Sema& );	// prohibit
		Sema&		operator=	( const Sema& );	// prohibit

public:
					Sema		( )				: flag(0),wait(nullptr),owner(nullptr),next(nullptr),prev(nullptr) {}
					~Sema		( );			// destructor: unlink waiting tasks!

		bool		IsFree		( ) const		{ return flag<=0; }
		bool		IsBlocked	( ) const		{ return flag>0;  }
		int32		BlockCount	( ) const		{ return flag; }
		bool		IsAvailable	( ) const		{ return IsFree()    || owner==MyTid(); }
		bool		IsNotAvailable()const		{ return IsBlocked() && owner!=MyTid(); }
		bool		IsMine		( ) const		{ return IsBlocked() && owner==MyTid(); }
		bool		IsNotMine	( ) const		{ return IsFree()    || owner!=MyTid(); }
		bool		IsWaiting	( ) const		{ return wait!=nullptr; }
		Tid			Owner		( ) const		{ return owner; }

		void		Clear		( );			// reset to initial state
		bool		Request		( int n=1 );	// request semaphor
		bool		Request_WTO	( double timeout, int n=1 );	// request semaphor
		void		Release		( int n=1 );	// release semaphor
};







#endif


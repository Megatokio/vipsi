/*	Copyright  (c)	Günter Woigk 2015 - 2015
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

#ifndef _Object_h_
#define _Object_h_

#include "kio/kio.h"


/* 	Object with reference counter and reference counting pointer

	Objekte werden mit refcnt=0 erzeugt
	Objekte dürfen nur mit refcnt=0 gelöscht werden.
	=> Wenn keine Objektpointer für dieses Objekt benutzt werden, kann es ganz normal mit delete gelöscht werden.
	=> Objekte können ggf. auch als lokale Variablen einer Funktion auf dem Heap angelegt werden.

	Wird dem Object ein erster ObjectPtr zugewiesen, darf das Objekt nur noch über diese gelöscht werden.
	Alle normalen, nicht zählenden Pointer auf das Objekt werden per Definition zu Weak Pointern.
	Wird der letzte ObjectPtr für ein Objekt gelöscht (oder der ptr darin überschrieben), wird das Objekt gelöscht.

	Die Template-Klassen Ptr und MTPtr sind solche ObjectPtr und implementieren das Reference-Counting.

	Es sind 3 Variationen möglich:

	Object + Ptr		single-threaded app
	MTObject + Ptr		concurrent access to data members, but creation/destruction of pointers is thread-safe
						e.g. Objects are never passed to/from functions with an ObjectPtr.
	 					Note: weak (bare) pointers may be used to pass Objects to functions.
	MTObject + MTPtr	concurrent access to data members and creation/destruction of pointers,
						e.g. Objects may be passed to/from functions with an ObjectPtr.
*/


namespace kio
{

class Object
{
	template<class T> friend class Ptr;

protected:
mutable volatile uint cnt;

inline void		retain()					{ ++cnt; }
inline void		release()					{ if(--cnt==0) delete this; }

public:
				Object()					:cnt(0){}
virtual			~Object()					{ XXXASSERT(cnt==0); }
				Object(Object const&)		:cnt(0){}
virtual	Object&	operator=(Object const&)	{ return *this; }

inline	uint	refcnt()					{ return cnt; }
};


}

#endif


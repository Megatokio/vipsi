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


#ifndef _NVPtr_h_
#define _NVPtr_h_

#include "kio/kio.h"
#include "unix/pthreads.h"


/*	Volatile Objekte, Reference Counter und Locking Pointer:

	RCPtr	Pointer auf ein Objekt, das retain() / release() implementiert,
			idR. mithilfe eines Referenzzählers.
			Dieser kann auch in single-threaded Anwendungen nützlich sein.

			Für RCPtr sind auch der Copy Creator und operator=() definiert.
			Außerdem können ihm direkt Objekte oder auch NULL zugewiesen werden.

			In multi-threaded Anwendungen darf der RCPtr ein volatiles Objekt enthalten.
			Dann rückt er entsprechend auch immer nur ein volatiles Objekt heraus.
			Die Funktionen retain() und release() des Objekts müssen dann volatile sein.
			Sie müssen also das Objekt selbst locken, bevor der Referenzzähler geändert wird.

	NVPtr	Lockt das Objekt solange er existiert und rückt einen nicht-volatilen Pointer heraus.
			Ein NVPtr kann nur aus einem RCPtr erzeugt werden.
			Alle volatilen Pointer müssen also in einem RCPtr gespeichert sein!
			Nur dann können volatile Objekte ohne zusätzliche Logik sicher gelöscht werden.
			Das verwendete Objekt-Lock sollte NICHT REKURSIV sein!

			Einem NVPtr kann mit operator=() ein neuer RCPtr oder auch NULL zugewiesen werden.

	Soll ein volatiles Objekt gelöscht werden, muss man alle RCPtr darauf löschen oder NULLen.
	Dann sinkt der Referenzzähler auf 0 und das Objekt wird in Objekt::release() gelöscht.
	note: Das Löschen des Objekts blockiert, solange es geblockt ist.
	Solange ein NVPtr auf das Objekt existiert, ist der Löschvorgang blockiert.
	Ist man selbst Besitzer dieses NVPtrs und das Lock ist nicht-rekursiv, hat man ein Dead Lock.
	Ist man selbst Besitzer dieses NVPtrs und das Lock ist rekursiv, zeigt der NVPtr auf ein gelöschtes Objekt.
*/



/*	reference counting pointer to object

	class T must provide:
		retain()	const
		release()
		refcnt()	(optional)

	if T is volatile:
		retain() const volatile
		release() volatile
		refcnt() volatile	(optional)
*/
template<class T>
class RCPtr
{
protected:
	T*		p;

	void		retain		()	const			{ if(p) p->retain(); }
	void		release		()	const			{ if(p) p->release(); }

public:
				RCPtr		()					:p(NULL){}
				RCPtr		(T* p)      		:p(p)   { retain(); }
				RCPtr		(RCPtr const& q)	:p(q.p) { retain(); }
				~RCPtr		()					{ release(); }

	void		operator=	(RCPtr const& q) 	{ q.retain();  release(); p = q.p; }
	void		operator=	(T* q)      		{ if(q) q->retain(); release(); p = q; }

	T*			operator->	()					{ return p; }
	T&			operator*	()					{ return *p; }
	T*			ptr			()					{ return p; }
	T&			ref			()					{ return *p; }

	T*			operator->	() const			{ return p; }
	T&			operator*	() const			{ return *p; }
	T*			ptr			() const			{ return p; }
	T&			ref			() const			{ return *p; }

	uint		refcnt		()					{ return p ? p->refcnt() : 0; }
	void		swap		(RCPtr& q)			{ T* z=p; p=q.p; q.p=z; }

	bool		isNotNull	() volatile const	{ return p!=NULL; }
	bool		isNull		() volatile const	{ return p==NULL; }
};


/*	"non-volatile" locking pointer to volatile object

	class T must provide:
		lock()   volatile
		unlock() volatile
		retain() volatile  (required by NVPtr)
		release() volatile (required by NVPtr)

	note: the NVPtr does not increase the reference count!
*/
template <class T>
class NVPtr
{
	T*		p;

	typedef volatile T	VolT;

			NVPtr		(NVPtr const&);		// prohibit: this is impossible!
	NVPtr&	operator=	(NVPtr const&);		// prohibit: this is impossible!


	void	lock		()					{ if(p) p->lock(); }
	void	unlock		()					{ if(p) p->unlock(); }

public:
			NVPtr		()					:p(NULL){}
			NVPtr		(RCPtr<VolT>& q)	:p(const_cast<T*>(q.ptr())){ lock(); }
			~NVPtr		()					{ unlock(); }
			NVPtr		(VolT& o)			:p(const_cast<T*>(&o)) { lock(); }		// for static variables only!
			NVPtr		(VolT* o)			:p(const_cast<T*>(o))  { lock(); }

	void	operator=	(RCPtr<VolT>& q)	{ if(p!=q.ptr()) { unlock(); p = q.ptr(); lock(); } }

	void	operator=	(ptr p)				{ XXXASSERT(p==NULL); unlock(); p = NULL; }

	T*		operator->	()					{ return p; }
	T&		operator*	()					{ return *p; }
	T*		ptr			()					{ return p; }
	T&		ref			()					{ return *p; }
};



class RCObject
{
	template<class T> friend class RCPtr;
	template<class T> friend class NVPtr;

	mutable uint	_cnt;
	mutable PLock	_lock;

	void	lock()	  volatile const	{ ((PLock&)_lock).lock(); }
	void	unlock()  volatile const	{ ((PLock&)_lock).unlock(); }
	void	retain()  const				{ ++_cnt;}
	void	release()					{ if(--_cnt==0) delete this; }
	void	retain()  volatile const	{ lock(); ++_cnt; unlock(); }
	void	release() volatile			{ lock(); if(--_cnt==0) delete this; else unlock(); }

public:
			RCObject()				:_cnt(0){}
};



#endif


#if 0

//#define MachinePtr VolatileMachineRCPtr

typedef RCPtr<volatile Machine> MachinePtr;

class MachineList : public Array<MachinePtr>, public RCObject
{
	using Array<MachinePtr>::cnt;
	using Array<MachinePtr>::data;

public:
	uint	count()	volatile		{ return cnt; }
};

volatile MachineList machines;
volatile Machine* front_machine = NULL;

void runMachinesForSound()
{
	for(uint i=machines.count(); i--;)
	{
		NVPtr<MachineList> ml(machines);
		if(i>=ml->count()) continue;	// security
		NVPtr<Machine> m((*ml)[i]);
		if(m->can_run_for_sound) m->runForSound();
	}
}

void setFrontMachine (volatile Machine* m)
{
	front_machine = m;
}

#endif


#if 0
// ============================================================
//			MachineController list:
// ============================================================

MachineController* front_machine_controller = NULL;

class MachineControllerList : public Array<MachineController*>
{
public:
	void remove(MachineController* mc)
	{
		if(mc==front_machine_controller) front_machine_controller = NULL;

		for(uint i=0;i<count();i++)
			if(data[i]==mc) { Array<MachineController*>::remove(i); return; }
	}
};

MachineControllerList machine_controllers;


void setFrontMachineController(MachineController*m)
{
    XLogIn("setFrontMachineController");
    if(m==front_machine_controller) return;

	if(front_machine_controller)
		front_machine_controller->tool_windows->hideAllInspectors();

	front_machine = m->machine.ptr();
	front_machine_controller = m;

	front_machine_controller->tool_windows->showAllInspectors();
}

#endif

















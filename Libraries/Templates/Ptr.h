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


#ifndef _Ptr_h_
#define _Ptr_h_

#include "Object.h"


/* 	Reference counting pointer for Objects with reference counter
	see description in Object.h

	class Ptr<T> is used for objects with no concurrent creation/destruction of such pointers.
	E.g. in single-threaded apps or if higher logic says that this cannot happen.
	If concurrent creation/destruction of such pointers is possible then use MTPtr<T>.
*/

namespace kio
{

template<class T>		// T must be based on Object
class Ptr
{
	static_assert(std::is_base_of<Object, T>::value, "T must be derived from Object");

	T*		p;

	// retain/release: auch wenn p auf ein MTObject zeigt, wird Object::retain/release benutzt!
	static inline void	retain	(T* p)	{ if(p) p->Object::retain(); }
	static inline void	release	(T* p)	{ if(p) p->Object::release(); }

public:
			Ptr			()				:p(NULL){}
    		Ptr			(T* p)      	:p(p)  { retain(p); }
			Ptr			(Ptr const& q)	:p(q.p){ retain(p); }
			~Ptr		()				{ release(p); }
	Ptr&	operator=	(Ptr const& q) 	{ retain(q.p); release(p); p = q.p; }
	Ptr&	operator=	(T* q)      	{ retain(q);   release(p); p = q;   }

	T*		operator->	()				const { return p; }
	T&		operator*	()				const { return *p; }

	uint	refcnt		()				const { return p ? p->cnt : 0; }
	T*		ptr			()				const { return p; }
	T&		ref			()				const { return *p; }
	void	swap		(Ptr& q)		{ T* z=p; p=q.p; q.p=z; }

	bool	operator<	(T const& q)	const { return p->operator< (q); }	// operator<() is typically required by sort etc.
	bool	operator>	(T const& q)	const { return p->operator> (q); }
	bool	operator>=	(T const& q)	const { return p->operator>=(q); }
	bool	operator<=	(T const& q)	const { return p->operator<=(q); }
	bool	operator==	(T const& q)	const { return p->operator==(q); }
	bool	operator!=	(T const& q)	const { return p->operator!=(q); }

	bool	operator<	(Ptr const& q)	const { return operator< (q.p); }
	bool	operator>	(Ptr const& q)	const { return operator> (q.p); }
	bool	operator>=	(Ptr const& q)	const { return operator>=(q.p); }
	bool	operator<=	(Ptr const& q)	const { return operator<=(q.p); }
	bool	operator!=	(Ptr const& q)	const { return operator!=(q.p); }
	bool	operator==	(Ptr const& q)	const { return operator==(q.p); }

	// bool	operator bool()				{ return p!=NULL; }
	// Foo	operator[]	(uint i)		{ return (*p)[i]; }
	// Foo	operator()	(…)

	// bool	operator!	()
	// T	operator~	()
	// T	operator+	()
	// T	operator-	()

	// T&	operator++	()		pre-increment
	// T&	operator--	()
	// T	operator++	(int)	post-increment
	// T	operator--	(int)

	// T&	operator+=	(Foo)	Foo,Foo&,Foo const&
	// T&	operator-=	(Foo)
	// T&	operator*=	(Foo)	etc.

	// friends defined inside class body are inline and are hidden from non-ADL lookup
	// passing first arg by value helps optimize chained a+b+c
	// alternatively, both parameters may be const references.
	// friend T operator+ (T,Foo)	T, T&, T const&; Foo, Foo&, Foo const&
	// friend T	operator- (T,Bar)
	// friend T	operator* (T,Bar)	etc.
	template<class T2>
	friend T	operator+ (T a, T2 const& b)	{ return a += b; }
};


}

#endif

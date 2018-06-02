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


#include "Object.h"
#include "unix/pthreads.h"


/* 	Object with reference counter and reference counting pointer
	see description in Object.h
*/


namespace kio
{

class MTObject : public Object
{
	template<class T> friend class Ptr;
	template<class T> friend class MTPtr;

public:
	RPLock	_lock;						// public for easy use in a PLocker

private:
inline void		retain()					{ _lock.lock(); ++cnt; _lock.unlock(); }
inline void		release()					{ if(cnt==1) { if(XXXSAFE) cnt = 0; delete this; }
														   else { _lock.lock(); --cnt; _lock.unlock(); } }

public:
				MTObject()					{}
virtual			~MTObject()					{}
				MTObject(Object const&)		{}
		MTObject& operator=(MTObject const&){ return *this; }

inline uint	refcnt()						{ return cnt; }

inline void	lock()							{ _lock.lock(); }
inline void	unlock()						{ _lock.unlock(); }
};


}


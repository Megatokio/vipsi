/*	Copyright  (c)	Günter Woigk 1995 - 2015
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


---	byte order independent and unaligned peek & poke ---

	PeekX => 'abcd' == *(uint32*)"abcd"	--> access data written by m68k, ppc, ...	<-- this is the internet order
	PeekZ => 'abcd' == *(uint32*)"dcba"	--> access data written by i386, z80, ...	<-- avoid if possible
	peek  =>  machine order, unaligned. --> glue for machines which don't support unaligned memory access.
*/


#ifndef _peekpoke_h_
#define _peekpoke_h_

#include "kio/kio.h"



#define P(i)	((uint8*)p)[i]


#if defined(_BIG_ENDIAN) && !_ALIGNMENT_REQUIRED
	INL uint8  peek1X ( void const* p )		{ return *(uint8* )p; }
	INL uint16 peek2X ( void const* p )		{ return *(uint16*)p; }
	INL uint32 peek4X ( void const* p )		{ return *(uint32*)p; }
	INL uint64 peek8X ( void const* p )		{ return *(uint64*)p; }
	INL void   poke1X ( void* p, uint8 n )	{		 *(uint8* )p = n; }
	INL void   poke2X ( void* p, uint16 n)	{		 *(uint16*)p = n; }
	INL void   poke4X ( void* p, uint32 n )	{		 *(uint32*)p = n; }
	INL void   poke8X ( void* p, uint64 n )	{		 *(uint64*)p = n; }
#else
	INL uint8  peek1X ( void const* p )		{ return P(0); }
	INL uint16 peek2X ( void const* p )		{ return P(0)<<8  | P(1); }
	INL uint32 peek4X ( void const* p )		{ return P(0)<<24 | P(1)<<16 | P(2)<<8 | P(3); }
	INL uint64 peek8X ( void const* p )		{ return ((uint64)peek4X(p)<<32) + peek4X((ptr)p+4); }
	INL void   poke1X ( void* p, uint8 n )	{		 P(0)=n; }
	INL void   poke2X ( void* p, uint16 n)	{		 P(0)=n>>8;  P(1)=n; }
	INL void   poke4X ( void* p, uint32 n )	{		 P(0)=n>>24; P(1)=n>>16; P(2)=n>>8; P(3)=n; }
	INL void   poke8X ( void* p, uint64 n )	{ for(uint i=8;i--;) P(7-i) = n>>(i<<3); }
#endif
	INL void   poke3X ( void*p, uint32 n )	{        poke1X(p,n>>16); poke2X((ptr)p+1,n); }
	INL uint32 peek3X ( void const*p )		{ return peek1X(p) <<16 | peek2X((ptr)p+1);   }

#if defined(_LITTLE_ENDIAN) && !_ALIGNMENT_REQUIRED
	INL uint8  peek1Z ( void const* p )		{ return *(uint8* )p; }
	INL uint16 peek2Z ( void const* p )		{ return *(uint16*)p; }
	INL uint32 peek4Z ( void const* p )		{ return *(uint32*)p; }
	INL uint64 peek8Z ( void const* p )		{ return *(uint64*)p; }
	INL void   poke1Z ( void* p, uint8 n )	{		 *(uint8 *)p = n; }
	INL void   poke2Z ( void* p, uint16 n)	{		 *(uint16*)p = n; }
	INL void   poke4Z ( void* p, uint32 n )	{		 *(uint32*)p = n; }
	INL void   poke8Z ( void* p, uint64 n )	{		 *(uint64*)p = n; }
#else
	INL uint8  peek1Z ( void const* p )		{ return								 P(0); }
	INL uint16 peek2Z ( void const* p )		{ return					   P(1)<<8 | P(0); }
	INL uint32 peek4Z ( void const* p )		{ return P(3)<<24 | P(2)<<16 | P(1)<<8 | P(0); }
	INL uint64 peek8Z ( void const* p )		{ return ((uint64)peek4X((ptr)p+4)<<32) + peek4X(p); }
	INL void   poke1Z ( void* p, uint8 n )	{								     P(0)=n; }
	INL void   poke2Z ( void* p, uint16 n)	{					      P(1)=n>>8; P(0)=n; }
	INL void   poke4Z ( void* p, uint32 n )	{ P(3)=n>>24; P(2)=n>>16; P(1)=n>>8; P(0)=n; }
	INL void   poke8Z ( void* p, uint64 n )	{ for(uint i=8;i--;) P(i) = n>>(i<<3); }
#endif
	INL void   poke3Z ( void* p, uint32 n )	{        poke1Z((ptr)p+2,n>>16); poke2Z(p,n); }
	INL uint32 peek3Z ( void const* p )		{ return peek1Z((ptr)p+2) <<16 | peek2Z(p);   }


#undef P


/* ----	unaligned peek & poke -------------------------------
		note: on PDP machines data is stored in little endian order.
		unaligned peek and Poke on PDP machines is braindead anyway.
*/
#ifdef _BIG_ENDIAN
	INL uint8  peek1 ( void const* p )	{ return Peek1X(p); }
	INL uint16 peek2 ( void const* p )	{ return peek2X(p); }
	INL uint32 peek3 ( void const* p )	{ return Peek3X(p); }
	INL uint32 peek4 ( void const* p )	{ return peek4X(p); }
	INL uint64 peek8 ( void const* p )	{ return peek8X(p); }

	INL void   poke1 ( void* p, uint8 n  ) { Poke1X(p,n); }
	INL void   poke2 ( void* p, uint16 n ) { poke2X(p,n); }
	INL void   poke3 ( void* p, uint32 n ) { Poke3X(p,n); }
	INL void   poke4 ( void* p, uint32 n ) { poke4X(p,n); }
	INL void   poke8 ( void* p, uint64 n ) { poke8X(p,n); }
#else
	INL uint8  peek1 ( void const* p )	{ return peek1Z(p); }
	INL uint16 peek2 ( void const* p )	{ return peek2Z(p); }
	INL uint32 peek3 ( void const* p )	{ return peek3Z(p); }
	INL uint32 peek4 ( void const* p )	{ return peek4Z(p); }
	INL uint64 peek8 ( void const* p )	{ return peek8Z(p); }

	INL void   poke1 ( void* p, uint8  n ) { poke1Z(p,n); }
	INL void   poke2 ( void* p, uint16 n ) { poke2Z(p,n); }
	INL void   poke3 ( void* p, uint32 n ) { poke3Z(p,n); }
	INL void   poke4 ( void* p, uint32 n ) { poke4Z(p,n); }
	INL void   poke8 ( void* p, uint64 n ) { poke8Z(p,n); }
#endif


#endif




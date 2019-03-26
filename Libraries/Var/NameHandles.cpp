/*	Copyright  (c)	Günter Woigk   2004 - 2019
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

	2004-05-27 kio	started work on this file
	2004-05-29 kio	first version ready
	2004-06-01 kio	CONCEPTUAL PROBLEM ...
	2004-06-05 kio	COMPILER BUG in gcc 3.3.3 ...
	2004-06-06 kio	modified version ready
*/

#define	SAFE	2
#define	LOG 	1

#include "cstrings/cstrings.h"
#include "NameHandles.h"

INIT_MSG



/* Settings
*/
#ifndef NH_HASHRING_BITS
#define	NH_HASHRING_BITS	12			/* suggested value: 12  -->  HashRing[4k] */
#endif



/* ----------------------------------------------------------------------------
*/
const uint	handlebits		= sizeof(NameHandle)*8;		// size of NameHandle		((32))

const uint	hr_bits 		= NH_HASHRING_BITS;			// size of HashRing[]		((12))
const ulong	hr_size			= 1<<hr_bits;

const uint	hr_shift		= handlebits-hr_bits;		// shift for Handle <--> HashRingIndex
//const ulong hr_lo_mask  	= hr_size-1;				// mask if shifted right (index position)
//const ulong hr_hi_mask  	= hr_lo_mask << hr_shift;	// mask if shifted left  (handle position)

const uint	hd_bits			= handlebits - hr_bits;		// max.size of HashData[] in HashRing[].data[]		((20))
const ulong	hd_mask			= (1ul<<hd_bits)-1;



/* ----	data container for Handle/Name pair ----------------------
*/
struct HashData
{
	uint		usage;	// usage count
	ulong		hash;
	String		name;

	HashData ( ) : usage(0) { };	// creator
};



/* ----	HashRing array for Handle/Name pair arrays --------------------
		note: HashRing[i].data[j]  <=>  handle = (i<<hr_shift) + j
*/
static struct { uint size; HashData* data; } HashRing[hr_size];


/* ---- Initialization ------------------------------------------------
		Set NameHandle(emptyString) := 0
		note: initialization may be performed after we were already
			  used by other modules for their initialization.
			  All we have to do here is to make sure that the
			  emptyString is assigned to NameHandle 0.
*/
void InitHR()
{
	static int done=0; if(done) return; else done++;
	XXLogIn("init HashRing[0]");
	XXASSERT(emptyString.CalcHash()==0);
	XXASSERT(HashRing[0].size==0);		// NH==0 already used?
// manual setup:
// NewNameHandle() calls cstring.cp which may be not yet initialized
	HashData* hd;
	HashRing[0].size = 1;
	HashRing[0].data = hd = new HashData[1];
	hd[0].usage = 1;					// lock NH and leave it locked 4ever
	hd[0].hash  = 0;
	hd[0].name  = emptyString;
};
static struct INIT_HR { INIT_HR(){InitHR();} } dummyname;





/* ===================================================================
		The Procedures
=================================================================== */



/* ----	Release existing Handle ------------------------------------
		Handle 1x entriegeln
*/
void UnlockNameHandle( NameHandle handle )
{
	if (handle==0) return;

	uint     idx = handle >> hr_shift;		// HashRing[idx]
	HashData* hd = HashRing[idx].data;
	uint       i = handle & hd_mask;		// hd[i]

	XASSERT(i<HashRing[idx].size);
	XXXASSERT(hd!=nullptr);
	XASSERT(hd[i].usage>0);

	hd[i].usage--;							// may become 0
}


/* ----	Request existing Handle ----------------------------------------
		Handle 1x verriegeln
*/
void LockNameHandle ( NameHandle handle )
{
	if (handle==0) return;

	uint     idx = handle >> hr_shift;		// HashRing[idx]
	HashData* hd = HashRing[idx].data;
	uint       i = handle & hd_mask;		// hd[i]

	XASSERT(i<HashRing[idx].size);			// bogus NH
	XXXASSERT(hd!=nullptr);					// internal error
	XASSERT(hd[i].usage>0);					// try to lock an old but no longer valid NH

	hd[i].usage++;
}


/* ----	Namen auslesen ------------------------------------------
*/
cString& GetNameForHandle ( NameHandle handle )
{
//	if (handle==0) return emptyString;

	uint     idx = handle >> hr_shift;		// HashRing[idx]
	HashData* hd = HashRing[idx].data;
	uint       i = handle & hd_mask;		// hd[i]

	XASSERT(i<HashRing[idx].size);
	XXXASSERT(hd!=nullptr);
	XASSERT(hd[i].usage>0);

	return hd[i].name;
}


/* ----	Search for Name ------------------------------------------
		returns Handle if found
		returns 0 if name unknown  OR  name == empty string
*/
NameHandle FindNameHandle ( cString& s )
{
	if (s.Len()==0) return 0;

	ulong   hash = s.CalcHash();
	ulong    idx = hash >> hr_shift;
	uint      sz = HashRing[idx].size;  if (sz==0) return 0;		// not found
	HashData* hd = HashRing[idx].data;

	for (uint i=0;i<sz;i++)
	{
		if (hd[i].hash==hash && hd[i].usage && hd[i].name==s) return (idx<<hr_shift)+i;
	}

	return 0;		// not found
}


/* ----	Request Name ------------------------------------------
		Namen auf 'neu' prüfen
		Handle verriegeln
*/
NameHandle NewNameHandle ( cString& s )
{
	if (s.Len()==0) return 0;

	XXLogIn("NewNameHandle(\"%s\")",s.CString());

#if XXXSAFE
	NameHandleCheck();
#endif

	ulong   hash = s.CalcHash();
	ulong     idx = hash >> hr_shift;
	uint      sz = HashRing[idx].size;
	HashData* hd;
	uint	   i;

	XXLog("#%lu#",idx);

// first name for this slot?  =>  handle == hash
	if (sz==0)
	{
		HashRing[idx].size = 1;
		HashRing[idx].data = hd = new HashData[1];
		i = 0;
		XXLog("{+}");
		goto i;		// --> HashRing[idx].data[i]
	}

// already exists?
	hd = HashRing[idx].data;	XXXASSERT(hd!=nullptr);
	for (i=0;i<sz;i++)
	{
		if (hd[i].hash==hash && hd[i].usage && hd[i].name==s)
		{
			hd[i].usage++;
			return (idx<<hr_shift) +i;
		}
	}

// slot already in use, but string not found  =>  search free slot
	for (i=0;i<sz;i++)
	{
		if (hd[i].usage==0)
		{
			XXLog("{o}");
	i:		hd[i].usage = 1;
			hd[i].hash  = hash;
			hd[i].name  = s;
			return (idx<<hr_shift) +i;
		}
	}

// no free slot  =>  grow vector
	HashData* ohd = hd;
	uint      osz = sz;

	if(XXLOG) { static int n=0; Log("{*%i*}",++n); }

	HashRing[idx].size = sz = sz*4/3+1;
	HashRing[idx].data = hd = new HashData[sz];
	for (i=0; i<osz; i++) { hd[i] = ohd[i]; }
	delete[] ohd;

//	i = osz;
	goto i;		// --> HashRing[idx].data[i]
}


/* ----	cstr version -------------------------------------------------
		required wg. bug in gcc 3.3.3
*/
NameHandle NewNameHandle ( cstr s )
{
	return NewNameHandle(String(s));
}



/* ----	free unused memory ---------------------------------
		called in case of memory shortage
*/
void NameHandlesPurgeCaches ( )
{
	for (uint idx=0; idx<hr_size; idx++ )
	{
		uint      sz = HashRing[idx].size;	if(sz==0) continue;
		HashData* hd = HashRing[idx].data;

		int n=-1;
		for (uint i=0;i<sz; i++)
		{
			if (hd[i].usage) n=i;
			else hd[i].name=emptyString;
		}

		if (n==-1)
		{
			delete[] hd;
			HashRing[idx].size = 0;
			HashRing[idx].data = nullptr;
			continue;
		}

		if (uint(n)+1<sz)
		{
			// truncate vector ***TODO***
		}
	}
}


void NameHandleCheck()
{
	char bu[5000];
	memset(bu,0,sizeof(bu));

	for(uint i=0;i<hr_size;i++)
		for(uint j=0;j<HashRing[i].size;j++)
			HashRing[i].data[j].name.Check( __FILE__, __LINE__ );
}

















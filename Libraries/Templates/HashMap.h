/*	Copyright  (c)	Günter Woigk 2014 - 2015
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
	OR BUSINESS IDXERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
	WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
	OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
	ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef HashMap_h
#define HashMap_h

#include "kio/kio.h"
#include "Templates/Array.h"
#include "hash/sdbm_hash.h"
#include "kio/util/msbit.h"


/*	Template class HashMap stores Objects with Keys.

	Keys (Names) must be unique.
	The HashMap retains ownership of the Objects.

	Keys are not copied.
	The passed keys are stored internally and must remain valid throughout lifetime of the HashMap.
	This may be accomplished by using Objects with a data member 'name' and use this 'name' as key.

	The HashMap creator should be passed the expected size of the final map.
	If the map grows larger then it will be resized on the fly which will take some time.
	Better be generous because a larger hashmap results in less collisions and therefore faster access;
	the fixed costs are hashmap.count() * sizeof(int).


	in hashmap[] sind die indizes der zugehörigen daten in keys/items gespeichert.
	wg. uneindeutigkeit muss bei zugriff immer noch der key aus keys[] verglichen werden.
	der hashwert gibt den startpunkt in hashmap[] an, ab dem nach den zugehörigen daten gesucht werden kann
	ist der erste wert falsch, wird mit dem nächsten eintrag in hashmap weiterprobiert bis ein endmarker erreicht wurde. dann: n.ex.
	endmarker ist gesetztes bit31 auf einem index.
	unbenutzte slots in hashmap haben den wert FREE und haben auch bit31 gesetzt
	wird ein eintrag gelöscht, wird der index auf FREE gesetzt, bit31 bleibt aber erhalten.
	ggf. wird bit31 im vor-index ebenfalls gesetzt. (s.u.)


	template arguments:

		class KEY		must be a flat type for HashMap. Keys are compared with '='.
		class ITEM		must be a flat type for HashMap.
*/


#ifndef ArrayMAX
#define ArrayMAX	0x40000000u		/* max size  ((not count)) */
#endif



template<class KEY, class ITEM>
class HashMap
{
protected:
	Array<ITEM>	items;			// stored items
	Array<KEY>	keys;			// their keys

	int*		map;			// hash -> index conversion array
	uint		mask;			// map size -1; map size must be 1<<N

	const int BIT31 = INT_MIN;	// 0x80000000 mask for 'end-of-thread' marker bit
	const int FREE  = -1;		// 			  value for free slots in map[] (BIT31 set)

protected:
	void		clearmap	()					{ memset(map,-1,(mask+1)*sizeof(*map)); }	// set all to FREE
	int			indexof		(KEY) const;		// find index in items[]; -1 if not found
	void		resizemap	(uint)				throw(limit_error);

static inline uint hash 	(cstr key)			{ return sdbm_hash(key);  }
static inline uint hash 	(int16& key)		{ return key ^ (key>>5);  }
static inline uint hash 	(uint32 key)		{ return key ^ (key>>16); }
static inline uint hash 	(uint64 key)		{ return hash( (uint32)key ^ (uint32)(key>>32) ); }
static inline uint hash 	(int32 key)			{ return hash((uint32)key); }
static inline uint hash 	(int64 key)			{ return hash((uint64)key); }
static inline uint hash 	(void* key)			{ return hash((ssize_t)key); }

static inline bool eq		(cstr a,cstr b)		{ return strcmp(a,b)==0; }
static inline bool eq		(int a, int b)		{ return a==b; }
static inline bool eq		(uint a, uint b)	{ return a==b; }
static inline bool eq		(long a, long b)	{ return a==b; }
static inline bool eq		(ulong a, ulong b)	{ return a==b; }
static inline bool eq		(void* a, void* b)	{ return a==b; }

				HashMap		(HashMap const&);	// prohibit
	void		operator=	(HashMap const&);	// prohibit

public:
				HashMap		(uint max=1<<10);	// default: for up to 1024 items without resizing
				HashMap		(HashMap&);
	void		operator=	(HashMap&);
 				~HashMap	()					{ delete[] map; }

// get internal data:
	uint		getMapSize	() const			{ return mask+1; }
	int const*	getMap		() const			{ return map; }

	Array<KEY> const&  getKeys	() const		{ return keys; }
	Array<ITEM>&	   getItems	()				{ return items; }
	Array<ITEM> const& getItems	() const		{ return items; }

// get items:
	uint		count		() const			{ return items.count(); }
	bool		contains	(KEY key) const		{ return indexof(key) != -1; }
	ITEM		get			(KEY key, ITEM dflt) const	{ int idx = indexof(key); return idx==-1 ? dflt : items[idx]; }
	ITEM&		operator[]	(KEY key) 			throw(index_error) { return items[indexof(key)]; }		// XXCHECK_IDX -> throws index_error
	ITEM const&	operator[]	(KEY key) const		throw(index_error) { return items[indexof(key)]; }		// XXCHECK_IDX -> throws index_error

// add / remove items:
	void		purge		();
	void		add			(KEY, ITEM);		// overwrites if key already exists
	void		remove		(KEY);				// silently does nothing if key does not exist

// read / write file:
	void		writeToFile	(FD&) const 		throw(file_error);
	void		readFromFile(FD&) 				throw(file_error);		// caveat: cstr keys read back in tempmem!
};




// -----------------------------------------------------------------------
//				   	I M P L E M E N T A T I O N S
// -----------------------------------------------------------------------


// 	create HashMap with preallocated items[] and initial map[] size
//	there will be no reallocation of items[] and  no reallocation and reindexing of map[] up to max items
//	log2mapsize is calculated from max if 1<<log2mapsize <= max
//	note: if int==int16 then maximum for max is 0x7FFE (32766)
//
template<class KEY,class ITEM>
HashMap<KEY,ITEM>::HashMap(uint max)
:
	items((uint)0,max),
	keys((uint)0,max),
	map(NULL),
	mask(0)
{
	XXASSERT(max>0);
	XXLIMIT(max,ArrayMAX/sizeof(ITEM));

	int mapsize = 4 << msbit(max-1);	// mapsize = 2 * max!

	map = new int[mapsize];
	mask = mapsize-1;
	clearmap();
}


// copy creator
// *moves* data from source to this HashMap!
//
template<class KEY,class ITEM>
HashMap<KEY,ITEM>::HashMap(HashMap& q)
:
	items(q.items),
	keys(q.keys),
	map(q.map),
	mask(q.mask)
{
	//q.mask = 0;
	q.map = NULL;
	q.map = new KEY[mask+1];
	//q.mask = mask;
	q.clearmap();
}


// assignment operator
// *moves* data from source to this Hashmap!
//
template<class KEY,class ITEM>
void HashMap<KEY,ITEM>::operator=(HashMap& q)
{
	if(this!=&q)
	{
		this->~HashMap();
		new(this) HashMap(q);
	}
}


//	clear HashMap
//	the map[] is not resized
//	but all entries are cleared to FREE instead
//
template<class KEY,class ITEM>
void HashMap<KEY,ITEM>::purge()
{
	items.purge();
	keys.purge();
	clearmap();
}


template<class KEY,class ITEM>
void HashMap<KEY,ITEM>::resizemap(uint newsize) throw(limit_error)
{
	XXLogLine("HashMap: grow map to %i",newsize);

	XXASSERT(newsize>=items.count());
	XXASSERT(newsize == 1 << msbit(newsize));

	mask = newsize-1;

	LIMIT(mask,min((uint)BIT31,(uint)(ArrayMAX/sizeof(*map))));	// also catches size=0

// allocate and clear map:
	delete[] map;
	map = NULL;
	map = new int[newsize];
	clearmap();

// put all items back into map:
	for(uint idx=0,e=items.count(); idx<e; idx++)
	{
		uint i = hash(keys[idx]);
		while(map[i&mask] != FREE)
			map[i++&mask] &= ~BIT31;	// clear end-of-thread marker on this index
		map[i&mask] = idx + BIT31;		// store index, set end-of-thread marker
	}
}


//	search for key
//	returns index in items[] or -1
//
template<class KEY,class ITEM>
int HashMap<KEY,ITEM>::indexof(KEY key) const
{
	uint i = hash(key);
	int idx = map[i&mask];
	if(idx==FREE) return -1;

	for(;;)
	{
		bool fin = idx<0;
		idx &= ~BIT31;
		if(eq(keys[idx],key)) return idx;	// found
		if(fin) return -1;					// end of thread => not found
		idx = map[++i&mask];
		XXXASSERT(idx!=FREE);
	}
}


//	add item for key
//	if key alredy exists, then overwrite
//
template<class KEY,class ITEM>
void HashMap<KEY,ITEM>::add(KEY key, ITEM item)
{
a:	uint mask = this->mask;					// for rapid access
	uint i = hash(key);						// i = index in map[]
	int  idx = map[i&mask];					// idx = index in items[]
	if(idx==FREE) goto b;					// map[i] is free => quick action!

// search for existing key:

	for(;;)
	{
		bool fin = idx<0;
		idx &= ~BIT31;
		if(eq(keys[idx],key)) 				// key exists => overwrite & exit:
		{
			items[idx] = item;				// overwrite item at idx
			keys[idx] = key;				// also overwrite key, if KEY==cstr then the key may be kept alive by it's item
			return;
		}
		if(fin) break;						// no more chances: key does not yet exist
		idx = map[++i&mask];				// inspect next map[i] / items[idx]
		XXXASSERT(idx!=FREE);				// BIT31 must be set on last entry of thread: we can't run into a free slot
	}

// check whether it's time to grow the map[]:
	if(items.count()*2 > mask)
	{
		resizemap(mask*2+2);
		goto a;
	}

// find free slot in map[]:
	do { map[i&mask] &= ~BIT31; } while(map[++i&mask] != FREE);	// and clear end-of-thread markers until free slot found

// map[i] is free
// get free index in items[]:
// store index in map[i],
// store key+item in keys[] and items[]:

//	map[(i-1)&mask] &= ~BIT31;				// clear end-of-thread marker on previous index
b:	map[i&mask] = items.count() + BIT31;	// store index, set end-of-thread marker
	items.append(item);						// store item at index
	keys.append(key);						// store key at index
}


// remove key
// silently ignores if key does not exist
//
template<class KEY,class ITEM>
void HashMap<KEY,ITEM>::remove(KEY key)
{
	uint mask = this->mask;					// for rapid access
	bool fin;
	int idx;

	uint i = hash(key);						// i = index in map[]
	idx = map[i&mask];						// idx = index in items[]
	if(idx==FREE) return;					// not found

	for(;;)
	{
		fin = idx<0;						// end-of-thread marker
		idx &= ~BIT31;						// real index
		if(eq(keys[idx],key)) break;		// item found at map[i] / items[idx]
		if(fin) return;						// end of thread => not found
		idx = map[++i&mask];				// next i / idx
	}

	// item has been found at map[i] / items[idx]:
	// fin indicates that map[i] is at the end of the thread

	// move items.last() into gap:
	if((uint)idx!=items.count()-1)
	{
		items[idx] = items.last();
		KEY key2 = keys[idx] = keys.last();	// key2 = key of moved item

		// find index i2 in map[] for moved item:
		uint i2;
		for(i2=hash(key2);; ++i2)			// i2 = index in map[]
		{
			int idx2 = map[i2&mask];		// idx2 = index in items[]
			XXXASSERT(idx2!=FREE);
			XXXASSERT(idx2>=0||eq(keys[idx2&~BIT31],key2));	// must exist
			if(idx2<0 || eq(keys[idx2],key2)) break;		// found
		}

		// point map[i2] to new location:
		map[i2&mask] += idx - (items.count()-1);	// replace index but keep bit31
	}
	items.drop();
	keys.drop();

	// map[i] is a free slot
	// fin tells whether map[i] is at the end of the thread
	// search remainder of thread for entries which should be moved closer to their nominal position in map[]:

	uint j = i;
	while(!fin)								// fin tells whether map[j] is the end of the thread
	{
		idx = map[++j&mask];				// inspect map[j] / items[idx]
		XXXASSERT(idx!=FREE);
		fin = idx<0;						// update fin for j
		idx &= ~BIT31;

		uint j0 = hash(keys[idx]);			// j0 = nominal position of j in map[]
		if(j==j0) continue;					// on it's nominal position

		if(((j-j0)&mask) >= ((j-i)&mask))	// j0<=i ?
		{
			map[i&mask] = idx;				// move idx from map[j] to map[i], bit31 cleared
			i = j;							// now map[i] is a free slot
		}
	}

	// map[i] is a free slot.
	// after i there is no entry with a nominal position before i
	// => truncate thread here

	map[(i-1)&mask] |= BIT31;				// map[i-1] := end of thread
	map[i&mask] = FREE;						// map[i] := free
}


template<class KEY,class TYPE>
void HashMap<KEY,TYPE>::writeToFile(FD& fd) const throw(file_error)
{
	items.writeToFile(fd);
	keys.writeToFile(fd);
	fd.write(mask);
	fd.write_data(map,mask+1);
}


template<class KEY,class TYPE>
void HashMap<KEY,TYPE>::readFromFile(FD& fd) throw(file_error)
{
	items.readFromFile(fd);
	keys.readFromFile(fd);
	fd.read(mask);
	fd.read_data(map,mask+1);
}




// -------------------------------------------------------------
//			template class "ObjHashMap"
// -------------------------------------------------------------




template<class KEY, class ITEM>
class ObjHashMap : public HashMap<KEY,ITEM*>
{
protected:
	using HashMap<KEY,ITEM*>::items;		// stored items
	using HashMap<KEY,ITEM*>::keys;			// their keys
	using HashMap<KEY,ITEM*>::map;			// hash -> index conversion array
	using HashMap<KEY,ITEM*>::mask;			// map size -1; map size must be 1<<N

//	const int BIT31 = INT_MIN;	// 0x80000000 mask for 'end-of-thread' marker bit
//	const int FREE  = -1;		// 			  value for free slots in map[] (BIT31 set)

public:
//	using HashMap<KEY,ITEM*>::notfound;

protected:
//	void		clearmap	()					{ memset(map,-1,(mask+1)*sizeof(int)); }	// set all to FREE

//static inline uint hash 	(cstr key)			{ return sdbm_hash(key);  }
//static inline uint hash 	(int16& key)		{ return key ^ (key>>5);  }
//static inline uint hash 	(int32 key)			{ return key ^ (key>>16); }
//static inline uint hash 	(int64 key)			{ return hash( (int32)key ^ (int32)(key>>32) ); }
//static inline uint hash 	(void* key)			{ return hash((ssize_t)key); }
//
//static inline bool eq		(cstr a,cstr b)		{ return strcmp(a,b)==0; }
//static inline bool eq		(int a, int b)		{ return a==b; }
//static inline bool eq		(long a, long b)	{ return a==b; }
//static inline bool eq		(void* a, void* b)	{ return a==b; }


				ObjHashMap	(ObjHashMap const&);	// prohibit
	void		operator=	(ObjHashMap const&);	// prohibit

public:
				ObjHashMap	(uint max=1<<10)	:HashMap<KEY,ITEM*>(max){}
				ObjHashMap	(ObjHashMap& q)		:HashMap<KEY,ITEM*>(q){}
	void		operator=	(ObjHashMap& q)		{ HashMap<KEY,ITEM*>::operator=(q); }
 				~ObjHashMap	()					{ while(items.count()) delete items.pop(); }

//	uint		count		() const			{ return items.count(); }
	void		purge		()					{ while(items.count()) delete items.pop(); HashMap<KEY,ITEM*>::purge(); }

	int			indexof		(KEY k) const		{ return HashMap<KEY,ITEM*>::indexof(k); }
//	bool		contains	(KEY key)			{ return indexof(key) != -1; }

	void		add			(KEY k, ITEM* v)	{ int i = indexof(k); if(i!=-1) delete items[i]; HashMap<KEY,ITEM*>::add(k,v); }
	void		remove		(KEY k)				{ int i = indexof(k); if(i!=-1) { delete items[i]; HashMap<KEY,ITEM*>::remove(k); } }
	ITEM const&	get			(KEY k) const		{ int i = indexof(k); return i==-1 ? *(ITEM*)NULL : *items[i]; }
	ITEM&		get			(KEY k) 			{ int i = indexof(k); return i==-1 ? *(ITEM*)NULL : *items[i]; }
	ITEM&		operator[]	(KEY k) 			{ return *items[indexof(k)]; }
	ITEM const&	operator[]	(KEY k) const		{ return *items[indexof(k)]; }

	void		writeToFile	(FD&) const 		throw(file_error);
	void		readFromFile(FD&) 				throw(file_error);
};


template<class KEY,class TYPE>
void ObjHashMap<KEY,TYPE>::writeToFile(FD& fd) const throw(file_error)
{
	items.purge();
	uint n; fd.read(n);
	if(n>ArrayMAX/sizeof(TYPE)) throw file_error(fd,limiterror);
	items.data = (TYPE**)new uint8[n*sizeof(TYPE*)]; items.cnt = n;
	memset(items.data,0,n*sizeof(TYPE));

    for(uint i=0; i<n; i++)
    {
		uint8 f; fd.read(f);
		if(f) (items.data[i] = new TYPE())->readFromFile(fd);
    }

	keys.writeToFile(fd);
	fd.write(mask);
	fd.write_data(map,mask+1);
}


template<class KEY,class TYPE>
void ObjHashMap<KEY,TYPE>::readFromFile(FD& fd) throw(file_error)
{
	fd.write(items.count());

    for(uint i=0; i<items.count(); i++)
    {
		TYPE* obj = &items[i];
		uint8 f =obj!=NULL;
		fd.write(f);
		if(f) obj->writeToFile(fd);
    }

	keys.readFromFile(fd);
	fd.read(mask);
	fd.read_data(map,mask+1);
}

#endif

































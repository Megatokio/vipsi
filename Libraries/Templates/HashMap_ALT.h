/*	Copyright  (c)	Günter Woigk 2014 - 2014
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


#ifndef HashMap_h
#define HashMap_h

#include "kio/kio.h"
#include "Array.h"
#include "hash/sdbm_hash.h"
#include "kio/msbit.h"



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

	Description:

	Objects are stored in objects[]
	Corresponding keys are store in keys[]
	The hashmap[] provides mappings from key hashes to indexes in objects[]+keys[]
	unused mappings are set to -1 (0xffffffff)
	if multiple keys resulted in the same hash then the value in hashmap[] is an index in more[] instead.
	this is flagged by setting bit31 of the index.
	then in more[index++] indexes in objects[]+keys[] for this hash are stored.
	again, the last index of each list is flagged with bit31 set.
*/


const int BIT31 = INT_MIN;	// 0x80000000



template<class T>
class HashMap
{
	ObjArray<T>	objects;
	Array<cstr>	keys;
	Array<int>	hashmap;
	Array<int>	more;		// collisions
	uint		mask;

private:
	void		grow		();
	static uint	calc_hash	(cstr key)			{ return sdbm_hash(key); }

public:
				HashMap		(uint size = 0)		{ int b=size?msbit(size*2-1):10; size=1<<b; mask = size-1; hashmap.grow(size); memset(hashmap.getData(),-1,sizeof(int)<<b); }
//				HashMap<T>	(HashMap<T> const&);	default copy should work. this will *move* all objects from q to this HashMap
//	void		operator=	(HashMap<T> const&);	default copy should work. this will *move* all objects from q to this HashMap
//				~HashMap	()						default destructor should work

	uint		count		() const			{ return objects.count(); }
	void		purge		();

	T*			get			(cstr key);
	void		add			(cstr key, T* object);
};



/*	clear HashMap
	the hashmap[] is not resized
	but all entries are cleared to -1 instead
*/
template<class T>
void HashMap<T>::purge()
{
	objects.purge();
	keys.purge();
	memset(hashmap.getData(), -1, hashmap.count()*sizeof(int));
	more.purge();
}


/*	The HashMap should be allocated with a large-enough size.
	But if however the count of the stored objects exceeds the size of the hashtab[]
	then the hashmap[] will be doubled
	note: there is no hard requirement to grow() at exactly this point,
		  but then the collissions will have a growing impact on the performance of this map.
	Indexes of objects don't change
	Assumes there are no deleted objects (gaps) in objects[]+keys[]
		  currently there is no function for removing objects
	note: to avoid the hassle with handling collissions we simply
		  purge all lists and add all keys+objects again
*/
template<class T>
void HashMap<T>::grow()
{
	mask += mask+1;
	hashmap.purge();
	hashmap.grow(mask+1);
	memset(hashmap.getData(), -1, sizeof(int)*(mask+1));
	more.purge();

	Array<T*> zobjects = (Array<T*>&)objects;	// move all objects to zobjects[] which does not retain ownership! source becomes empty
	Array<cstr> zkeys  = keys;		// move all keys, source becomes empty
	assert(objects.count()==0);
	assert(keys.count()==0);

	cstr* key = zkeys.getData();
	T**   obj = zobjects.getData();
	for(uint i=0; i<zobjects.count(); i++)
	{
		add(*key++,*obj++);			// store and retain ownership
	}
}


/*	retrieve object from HashMap
	returns NULL if no object is stored for the passed key

	if exactly one object is stored for the key's hash
		then the object is retrieved from objects[] directly
	else the list of indexes is looked up in more[]
*/
template<class T>
T* HashMap<T>::get(cstr key)
{
	int oi = hashmap[calc_hash(key) & mask];	// oi = object index in objects[]+keys[]

	if(oi>=0)				// single key for this hash already stored?
	{
		if(eq(key,keys[oi])) return &objects[oi];	// if the stored key is the lookup key then return the stored object
		//else return NULL;
	}

	else if(~oi)			// multiple keys for this hash already stored?
	{
		int mi = oi & ~BIT31;	// mi = index in more[]
		do
		{	oi = more[mi++];	// oi = object index in keys[] and objects[]
			if(eq(key,keys[oi & ~BIT31])) return &objects[oi & ~BIT31];
		}
		while(oi>=0);
	}

	//else if(oi==-1)		// no key for this hash stored
	//{}					// => return NULL

	return NULL;			// not found
}


/*	add key+object to HashMap
	if the key already exists then the object is replaced, deleting the old object
	else the object+key is appended to objects[] and keys[]

	if the hashmap[] at the key's hash position is empty,
		then we store the object index directly in the hashmap[] and we are done
	else if there is already a single mapping for this hash,
		then we create an extension entry in more[] and store both object indexes there
	else we append the object's index to the existing list in more[]
		for which we move the list to the end of more[] so we don't have to shift all indexes in hashmap[]
		this creates gaps which are currently never closed
*/
template<class T>
void HashMap<T>::add( cstr key, T* object )
{
a:	int hi = calc_hash(key)&mask;		// hashmap_index in hashmap[]
	int oi = hashmap[hi];				// object_index	 in objects[] and keys[]

	if(oi==-1)				// slot is free
	{
		hashmap[hi] = count();			// index of new entry in objects[] and keys[]
s:		objects.append(object);			// append key+object
		keys.append(key);
	}

	else if(oi>=0)			// unique result for this hash
	{
		if(eq(key,keys[oi]))			// same key => replace object
		{
i:			assert(&objects[oi]!=object);	// 2x das gleiche Objekt speichern ist auf jeden Fall ein Fehler, da die HashMap die Ownership übernimmt
			delete &objects[oi];			// insert key+object at oi
			objects.getData()[oi] = object;
			keys[oi] = key;
		}
		else							// hash collision => Einträge an more[] anhängen:
		{
			int mi = more.count();		// index of extended entry in more[]
			hashmap[hi] = mi | BIT31;	// store more_index in hashmap[] and set msb for 'more index'
			more.append(count());		// store object index of new entry
			more.append(oi | BIT31);	// store object index of old entry and set msb for 'end of list'
			goto s;						// store object and key ((append))
		}
	}

	else					// multiple keys for this hash stored
	{
		if(count()>mask) { grow(); goto a; }

		int mi = oi & ~BIT31;			// index of extended entry in more[]

		int n=0,m;						// n = size of more map
		do
		{	m = more[mi + n++];			// m = value from more[]
			oi = m & ~BIT31;			// oi = index in keys[] and objects[];  note: ~BIT31 = 0x7fffffff
			if(eq(key,keys[oi])) goto i;// gleicher key => altes Objekt ersetzen
		}
		while(m>=0);

		hashmap[hi] = more.count() | BIT31;	// append new index list to more[]
		more.append(count());					// append index of new entry
		while(n--) { more.append(more[mi++]); }	// append old entries; last entry has already bit31 set
		goto s;									// store object and key ((append))
	}
}


#endif

































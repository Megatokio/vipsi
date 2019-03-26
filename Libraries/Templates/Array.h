/*	Copyright  (c)	Günter Woigk 2010 - 2019
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


#ifndef Array_h
#define Array_h


//#define SAFE 3			note: a template is only instantiated for ONE arbitrary file inclusion and uses SAFE and LOG of this inclusion
//#define LOG 1					  in all other inclusions as well, regardless of a potentially different setting there!

#include "../kio/kio.h"
#include "../unix/FD.h"


/*	Array for flat items

	items are initialized with zero.
	Array<T> does not call new or delete of items
	append() implicitely calls operator=() of items
	operator[] throws index_error (XXSAFE only)
	the copy creator and operator=()  *move* the data from source to dest, leaving an empty source with size=0
	grow() and insertrange() init new array items with ALL-ZERO.

	• template Array unterstützt auch cstr in tempmem
	• will man auch str in tempmem muss man entsprechende Spezialisierungen für sort, rsort, readFromFile und writeToFile schreiben, analog zu cstr
*/


#ifndef ArrayMAX
#define ArrayMAX	0x40000000		/* max size  ((not count)) */
#endif


// helper: check cnt
// helper: check item idx
// helper: check range end idx
#define CHECK_IDX(I)		XXINDEX(I,this->cnt)
#define CHECK_END(I)		XXINDEX(I,this->cnt+1)
#define CHECK_SIZE(SZ)		XXLIMIT(SZ,ArrayMAX/sizeof(T))


// Define Sorter Templates:

	#define 	SORTER 		sort
	#define 	CMP(A,B)	((A)>(B))
	template<class TYPE>
	#include 	"sort.h"

	#define 	SORTER 		rsort
	#define 	CMP(A,B)	((A)<(B))
	template<class TYPE>
	#include 	"sort.h"

	#define 	FUNCTION_SORTER
	#define 	SORTER 		sort
	template<class TYPE>
	#include 	"sort.h"

// Spezialisierungen für cstr:

#if 0
	inline bool gt(cstr& a, cstr& b) { return strcmp(a,b)>0; }
	inline bool lt(cstr& a, cstr& b) { return strcmp(a,b)<0; }
	template<> inline void sort(cstr* a, cstr* e) noexcept(false) { sort( a, e, (bool(*)(cstr&,cstr&))gt); }
	template<> inline void rsort(cstr* a, cstr* e) noexcept(false) { sort( a, e, (bool(*)(cstr&,cstr&))lt); }
#else
	#define 	SORTER 		sort
	#define 	CMP(A,B)	(strcmp(A,B)>0)
	#define 	TYPE		cstr
	template<>
	inline
	#include 	"sort.h"

	#define 	SORTER 		rsort
	#define 	CMP(A,B)	(strcmp(A,B)<0)
	#define		TYPE		cstr
	template<>
	inline
	#include 	"sort.h"
#endif

// Spezialisierungen für str:

#if 0
	inline bool gt(str& a, str& b) { return strcmp(a,b)>0; }
	inline bool lt(str& a, str& b) { return strcmp(a,b)<0; }
	template<> inline void sort(str* a, str* e) noexcept(false) { sort( a, e, (bool(*)(str&,str&))gt); }
	template<> inline void rsort(str* a, str* e) noexcept(false) { sort( a, e, (bool(*)(str&,str&))lt); }
#else
	#define 	SORTER 		sort
	#define 	CMP(A,B)	(strcmp(A,B)>0)
	#define 	TYPE		str
	template<>
	inline
	#include 	"sort.h"

	#define 	SORTER 		rsort
	#define 	CMP(A,B)	(strcmp(A,B)<0)
	#define		TYPE		str
	template<>
	inline
	#include 	"sort.h"
#endif




// ------------------------------------------------------------
//				template class "Array"
// ------------------------------------------------------------



template<class T>
class Array
{
protected:
		uint	max,cnt;
		T*		data;

		void	kill		()						{ delete[] (ptr)data; }
		void	init		()						{ max=cnt=0; data=nullptr; }
		void	init		() const				{ const_cast<Array<T>&>(*this).init(); }
		void	init		(Array const& q)		{ max=q.max; cnt=q.cnt; data=q.data; q.init(); }

public:

				Array<T>	()						:max(0),cnt(0),data(nullptr){}
				Array<T>	(T* q, uint cnt)		:max(cnt),cnt(cnt),data(q){}
				Array<T>	(Array const& q)		{ init(q); }
				Array<T>	(uint cnt, uint max=0);
    virtual     ~Array<T>	()						{ kill(); }
	Array<T>&	operator=	(Array const& q)		{ if(this!=&q) { kill(); init(q); } return *this; }

	Array<T>	copy		() const;
	Array<T>	copyofrange	(uint a, uint e) const;
	void		swap		(Array& q)				{ kio::swap(max,q.max); kio::swap(cnt,q.cnt); kio::swap(data,q.data); }

// access data members:
	uint const&	count   	() const				{ return cnt; }						// & wird in vicci benutzt TODO: eliminate
	T* const&	getData		() const				{ return data; }					// & wird in vicci benutzt TODO: eliminate
	T const&	operator[]	(uint i) const			noexcept(false) { CHECK_IDX(i); return data[i]; }
	T&			operator[]	(uint i)				noexcept(false) { CHECK_IDX(i); return data[i]; }	// non-auto-growing
	T const&	first		() const				noexcept(false) { CHECK_IDX(0); return data[0]; }
	T&			first		()						noexcept(false) { CHECK_IDX(0); return data[0]; }
	T const&	last		() const				noexcept(false) { CHECK_IDX(0); return data[cnt-1]; }
	T&			last		()						noexcept(false) { CHECK_IDX(0); return data[cnt-1]; }

	bool		operator==	(Array const& q) const;
	bool		operator!=	(Array const& q) const;
	bool		contains	(T item) const			{ for(uint i=0;i<cnt;i++) if(data[i]==item) return true; return false; }

// resize:
	void	    grow        ()					    noexcept(false)	{ grow(cnt+1); }
	void		grow		(uint newcnt)			noexcept(false);
	void		shrink		(uint newcnt);
	void		resize		(uint newcnt)			noexcept(false)	{ shrink(newcnt); grow(newcnt); }
	void		drop		()						noexcept(false) { CHECK_IDX(0); --cnt; }
	T			pop			()						noexcept(false) { CHECK_IDX(0); return data[--cnt]; }
	void		purge		()										{ kill(); init(); }
	void		append		(T n)					noexcept(false)	{ grow(cnt+1); data[cnt-1]=n; }
	Array&		operator<<	(T n)					noexcept(false)	{ grow(cnt+1); data[cnt-1]=n; return *this; }
	void		remove		(uint idx)				noexcept(false);
	void		removeItem	(T);
	void		insertat	(uint idx, T)			noexcept(false);
	void		insertat	(uint idx, Array const&)noexcept(false);
	void		removerange	(uint a, uint e)		;
	void		insertrange	(uint a, uint e)		noexcept(false);
	void		append		(Array const& q)		noexcept(false);

	void 		revert		();
	void 		rol			()						{ rol(0,cnt); }
	void 		ror			()						{ ror(0,cnt); }
	void 		shuffle		()						{ shuffle(0,cnt); }
	void 		sort		()						{ sort(0,cnt); }
	void 		rsort		()						{ rsort(0,cnt); }
	void 		sort		(bool(*gt)(T&,T&))		{ sort(0,cnt,gt); }

	void 		rol			(uint a, uint e);		// roll left  range a..e
	void 		ror			(uint a, uint e);		// roll right range a..e
	void 		shuffle		(uint a, uint e);		// shuffle range [a..[e
	void 		sort		(uint a, uint e)					{ if(e>cnt) e=cnt; if(a<e) ::sort(data+a, data+e); }
	void 		rsort		(uint a, uint e)					{ if(e>cnt) e=cnt; if(a<e) ::rsort(data+a, data+e); }
	void 		sort		(uint a, uint e, bool(*gt)(T&,T&))	{ if(e>cnt) e=cnt; if(a<e) ::sort(data+a, data+e, gt); }

	void		writeToFile		(FD& fd) const		noexcept(false)		{ fd.write(cnt); fd.write_data(data,cnt); }
	void		readFromFile	(FD& fd)			noexcept(false);
	void		appendFromFile	(FD& fd, uint cnt)	noexcept(false);		// used in zxsp for uint8

private:		// for easier specialisation: Array<str|cstr> reads|writes nstr
	void		read_from_file	(FD& fd, uint a, uint e) noexcept(false)	{ fd.read_data(data+a,e-a); }
	void		write_to_file	(FD& fd, uint a, uint e) noexcept(false)	{ fd.write_data(data+a,e-a); }
};




// -----------------------------------------------------------------------
//					  I M P L E M E N T A T I O N S
// -----------------------------------------------------------------------


// create Array with data[cnt] (cleared to 0)
// and optional preallocation max
// max ≤ ArrayMAX
//
template<class T>
Array<T>::Array(uint cnt, uint max)
:	max(0),
	cnt(0),
	data(nullptr)
{
	if(max<cnt) max=cnt;
	CHECK_SIZE(max);

	data = reinterpret_cast<T*> ( new int8[max*sizeof(T)] );
	memset(data,0,cnt*sizeof(T));
	this->cnt = cnt;
	this->max = max;
}


// compare two arrays
//
template<class T>
bool Array<T>::operator== ( Array<T>const& q ) const
{
	if(cnt!=q.cnt) return no;
	for(uint i=cnt; i--;) { if(data[i]!=q.data[i]) return no; }
	return yes;
}


// compare two arrays
//
template<class T>
bool Array<T>::operator!= ( Array<T>const& q ) const
{
	if(cnt!=q.cnt) return yes;
	for(uint i=cnt; i--;) { if(data[i]!=q.data[i]) return yes; }
	return no;
}


// create copy of this
// note: the copy c'tor *moves* data,
//		 while copy() duplicates data
//
template<class T>
Array<T> Array<T>::copy() const
{
	Array<T> z;

	if(cnt)
	{
		z.data = reinterpret_cast<T*> ( new int8[cnt*sizeof(T)] );
		memcpy(z.data,data,cnt*sizeof(T));
		z.max = z.cnt = cnt;
	}

	return z;
}


// create a copy of a range of data of this
// e ≤ cnt
//
template<class T>
Array<T> Array<T>::copyofrange(uint a,uint e) const
{
	Array<T> z;

	if(a<e)
	{
		CHECK_END(e);

		z.data = reinterpret_cast<T*> ( new int8[(e-a)*sizeof(T)] );
		memcpy(z.data, data+a, (e-a)*sizeof(T));
		z.max = z.cnt = e-a;
	}

	return z;
}


// grow data
// does nothing if new cnt ≤ current cnt
// checks new size against ArrayMAX
// preallocates ~12% more
//
template<class T>
void Array<T>::grow(uint newcnt) noexcept(false)
{
	if(newcnt<=cnt) return;

	if(newcnt>max)		// need to grow data?
	{
		LIMIT(newcnt,ArrayMAX/sizeof(T));

		max = newcnt+newcnt/8+4;
		T* newdata = (T*) new int8[max*sizeof(T)];
		memcpy(newdata,data,cnt*sizeof(T));
		delete[] (ptr)data;
		data = newdata;
	}

	memset(data+cnt,0,(newcnt-cnt)*sizeof(T));
	cnt = newcnt;
}


// shrink data
// does nothing if new cnt ≥ current cnt
// may reallocate data
//
template<class T>
void Array<T>::shrink(uint newcnt)
{
	if(newcnt>=cnt) return;

	if(newcnt<max-max/8)	// time to shrink data?
	{
		T* newdata = newcnt ? (T*) new int8[newcnt*sizeof(T)] : nullptr;
		memcpy(newdata,data,newcnt*sizeof(T));
		delete[] (ptr)data;
		data = newdata;
		max = newcnt;
	}

	cnt = newcnt;
}


// remove item
// silently ignores if item not found
//
template<class T>
void Array<T>::removeItem(T item)
{
	for(uint i=0; i<cnt; i++)
	{
		if(data[i]==item)
		{
			memmove( data+i, data+(i+1), (cnt-i-1)*sizeof(T) );
			cnt--;
			return;
		}
	}
}

// remove item at index
// idx < cnt
//
template<class T>
void Array<T>::remove(uint idx) noexcept(false)
{
	CHECK_IDX(idx);

	memmove( data+idx, data+(idx+1), (cnt-idx-1)*sizeof(T) );
	cnt--;
}


// insert item at index
// idx ≤ cnt
//
template<class T>
void Array<T>::insertat(uint idx, T t) noexcept(false)
{
	CHECK_END(idx);

	grow();
	memmove( data+(idx+1), data+idx, (cnt-idx-1)*sizeof(T) );
	data[idx] = t;
}


// insert source array at index
// *moves* data, source will be cleared!
// idx ≤ cnt
//
template<class T>
void Array<T>::insertat(uint idx, Array const& q) noexcept(false)
{
	insertrange(idx,idx+q.count());
	memcpy(data+idx,q.data,q.count()*sizeof(T));
	q.init();
}


// remove range of data
//
template<class T>
void Array<T>::removerange(uint a, uint e)
{
	if(e>cnt)  e=cnt;
	if(a>=e)   return;

	memmove(data+a, data+e, (cnt-e)*sizeof(T));
	cnt -= e-a;
}


// insert space cleared with 0 into data
// a ≤ cnt
//
template<class T>
void Array<T>::insertrange(uint a, uint e) noexcept(false)
{
	CHECK_END(a);
	if(a>=e) return;

	grow(cnt+(e-a));
	memmove(data+e,data+a,(cnt-e)*sizeof(T));
	memset(data+a,0,(e-a)*sizeof(T));
}


// append other array to this array
// *moves* data, source array is cleared
//
template<class T>
void Array<T>::append(Array const& q) noexcept(false)
{
	grow(cnt+q.cnt);
	memcpy(data+cnt-q.cnt, q.data, q.cnt*sizeof(T));
	q.init();
}


// append n items from file
//
template<class T>
void Array<T>::appendFromFile(FD& fd, uint n) noexcept(false)
{
	grow(cnt+n);
	read_from_file(fd,cnt-n, cnt);
}


// revert order of items in data[]
// uses operator=()
//
template<class T>
void Array<T>::revert()
{
	if(cnt<=1) return;

	T* a = data;
	T* e = a+cnt-1;
	do { kio::swap(*a,*e); } while(++a<--e);
}


// roll left range [a..[e
// e ≤ cnt
//
template<class T>
void Array<T>::rol(uint a, uint e)
{
	CHECK_END(e);
	if(a>=e) return;

	int8 z[sizeof(T)];
	memcpy(z,data+a,sizeof(T));		// z = data[a];
	memmove(data+a, data+(a+1), (e-a-1)*sizeof(T));
	memcpy(data+(e-1),z,sizeof(T));	// data[e-1] = z;
}


// roll right range [a..[e
// e ≤ cnt
//
template<class T>
void Array<T>::ror(uint a, uint e)
{
	CHECK_END(e);
	if(a>=e) return;

	int8 z[sizeof(T)];
	memcpy(z,data+(e-1),sizeof(T));	// z = data[e-1];
	memmove(data+(a+1), data+a, (e-a-1)*sizeof(T));
	memcpy(data+a,z,sizeof(T));		// data[a] = z;
}


// shuffle data in range [a..[e
// e ≤ cnt
// uses operator=()
//
template<class T>
void Array<T>::shuffle(uint a, uint e)	// shuffle range [a..[e
{
	CHECK_END(e);
	if(a>=e) return;

	for(uint i=cnt; i--;) kio::swap(data[i],data[random()%cnt]);
}


// Spezialisierungen für str und cstr:
template<>
inline void Array<cstr>::read_from_file(FD& fd, uint a, uint e) noexcept(false)	{ while(a<e) data[a++] = fd.read_str(); }
template<>
inline void Array<cstr>::write_to_file(FD& fd, uint a, uint e) noexcept(false)	{ while(a<e) fd.write_str(data[a++]); }
template<>
inline void Array<str>::read_from_file(FD& fd, uint a, uint e) noexcept(false)	{ while(a<e) data[a++] = fd.read_str(); }
template<>
inline void Array<str>::write_to_file(FD& fd, uint a, uint e) noexcept(false)		{ while(a<e) fd.write_str(data[a++]); }


// read array from file
// which was written with writeToFile()
//
template<class T>
void Array<T>::readFromFile(FD& fd) noexcept(false)
{
	purge();
	uint n; fd.read(n);
	if(n>ArrayMAX/sizeof(T)) throw file_error(fd,limiterror);	// data corrupted?
	data = (T*) new int8[n*sizeof(T)];
	max = n;
	read_from_file(fd,0,n);				// throws file_error
	cnt = n;
}





// ___________________________________________________________
// array of strings:


class CstrArray : protected Array<cstr>
{
protected:
		void	kill		()                  { while(cnt) delete[] data[--cnt]; Array<cstr>::kill(); }

public:	virtual	~CstrArray	()					{ while(cnt) delete[] data[--cnt]; }
				CstrArray	()					{}
				CstrArray	(CstrArray const&q)	:Array<cstr>(q){}
	CstrArray&	operator=	(CstrArray const&q)	{ if(this!=&q) {kill();init(q);} return *this; }
				CstrArray	(cstr t)			{ append(t); }
				CstrArray	(cstr t1, cstr t2)	{ append(t1); append(t2); }

		cstr*	getData		() const			{ return data; }

		void	drop		()                  { if(cnt) delete[] data[--cnt]; }
//		cstr	pop			()                  { ASSERT(cnt); return data[--cnt]; }
		void	shrink		(uint n)            { while(cnt>n) delete[] data[--cnt]; Array<cstr>::shrink(n); }
		void	purge		()                  { kill(); init(); }

		void	append		(cstr s)				{ Array<cstr>::append(newcopy(s)); }
		void	append		(CstrArray const& q)	noexcept(false) { Array<cstr>::append(q); }
		void	insertat	(uint idx, cstr s)		noexcept(false) { Array<cstr>::insertat(idx,s); }

		void	remove		(uint idx)          { ASSERT(idx<cnt); delete[] data[idx]; Array<cstr>::remove(idx); }
		void	removerange	(uint , uint );     //TODO
		uint const&	count   () const			{ return cnt; }
		cstr const&	operator[]	(uint i) const	{ return Array<cstr>::operator[](i); }
		cstr&	operator[]	(uint i)			{ return Array<cstr>::operator[](i); }
		bool	operator==	(CstrArray const& q) const;
		bool	operator!=	(CstrArray const& q) const;

	CstrArray	copy		()                  { CstrArray z; for(uint i=0;i<cnt;i++) z.append(data[i]); return z; }

		void	revert		()						{ Array<cstr>::revert(); }
		void 	rol			()						{ Array<cstr>::rol(0,cnt); }
		void 	ror			()						{ Array<cstr>::ror(0,cnt); }
		void 	shuffle		()						{ Array<cstr>::shuffle(0,cnt); }
		void 	sort		()						{ Array<cstr>::sort(0,cnt); }
		void 	rsort		()						{ Array<cstr>::rsort(0,cnt); }
		void 	sort		(bool(*gt)(cstr&,cstr&)){ Array<cstr>::sort(0,cnt,gt); }

		void 	rol			(uint a, uint e)						{ Array<cstr>::rol(a,e); }
		void 	ror			(uint a, uint e)						{ Array<cstr>::ror(a,e); }
		void 	shuffle		(uint a, uint e)						{ Array<cstr>::shuffle(a,e); }
		void 	sort		(uint a, uint e)						{ Array<cstr>::sort(a,e); }
		void 	rsort		(uint a, uint e)						{ Array<cstr>::rsort(a,e); }
		void 	sort		(uint a, uint e, bool(*gt)(cstr&,cstr&)){ Array<cstr>::sort(a, e, gt); }

		void	insert_sorted(cstr s)           { uint i=cnt;grow(cnt+1); while(i--&&strcmp(data[i],s)>0) data[i+1]=data[i]; data[i+1]=newcopy(s);}

		void	writeToFile	(FD& fd) const      { fd.write_int32(cnt); for(uint i=0;i<cnt;i++) fd.write_nstr(data[i]); }
		void	readFromFile	(FD& fd)       	{ kill();max=cnt = fd.read_int32();data=new cstr[max];
											      for(uint i=0;i<cnt;i++)data[i] = fd.read_new_nstr(); }

		void	writefile	(FD& fd) const      { for(uint i=0;i<cnt;i++){fd.write_str(data[i]); fd.write_int8('\n');} }
		void	readfile	(FD& fd)            { kill();try{cstr s = fd.read_new_str(); append(s);}
                                                  catch(file_error& e){if(e.error!=endoffile) throw(e);}
                                                  for(uint i=0;i<cnt;i++) data[i] = fd.read_new_str(); }

		int		find		(cstr s) const      { for(int i=cnt;i--;) if(eq(s,data[i])) return i; return -1; }
		void	append_if_new(cstr s)           { for(int i=cnt;i--;) if(eq(s,data[i])) return; append(s); }

    	void	appendFromFile(FD& fd,int32 cnt) noexcept(false);  //TODO
};


inline bool CstrArray::operator== ( CstrArray const& q ) const
{
	if(cnt!=q.cnt) return no;
	for( uint i=cnt; i--; ) { if(ne(data[i],q.data[i])) return no; }
	return yes;
}


inline bool CstrArray::operator!= ( CstrArray const& q ) const
{
	if(cnt!=q.cnt) return yes;
	for( uint i=cnt; i--; ) { if(ne(data[i],q.data[i])) return yes; }
	return no;
}



// ___________________________________________________________
// array of strings:


class StrArray : protected Array<str>
{
protected:
		void	kill		()                  { while(cnt) delete[] data[--cnt]; Array<str>::kill(); }

public:	virtual	~StrArray	()					{ while(cnt) delete[] data[--cnt]; }
				StrArray	()					{}
				StrArray	(StrArray const&q)	:Array<str>(q){}
	StrArray&	operator=	(StrArray const&q)	{ if(this!=&q) {kill();init(q);} return *this; }
				StrArray	(cstr t)			{ append(t); }
				StrArray	(cstr t1, cstr t2)	{ append(t1); append(t2); }
				StrArray	(uint cnt, uint max=0) :Array<str>(cnt,max){}

		void	drop		()                  { if(cnt) delete[] data[--cnt]; }
//		str		pop			()                  { ASSERT(cnt); return data[--cnt]; }
		void	shrink		(uint n)            { while(cnt>n) delete[] data[--cnt]; Array<str>::shrink(n); }
		void	purge		()                  { kill(); init(); }

		void	append		(cstr s)            { Array<str>::append(newcopy(s)); }

		void	remove		(uint idx)          { ASSERT(idx<cnt); delete[] data[idx]; Array<str>::remove(idx); }
		void	removerange	(uint , uint );     //TODO
		uint const&	count   () const			{ return cnt; }
		str const&	operator[]	(uint i) const	{ return Array<str>::operator[](i); }
		str&	operator[]	(uint i)			{ return Array<str>::operator[](i); }
		bool	operator==	(StrArray const& q) const;
		bool	operator!=	(StrArray const& q) const;

	StrArray	copy		()                  { StrArray z; for(uint i=0;i<cnt;i++) z.append(data[i]); return z; }

		void	revert		()					{ Array<str>::revert(); }
		void 	rol			()						{ Array<str>::rol(0,cnt); }
		void 	ror			()						{ Array<str>::ror(0,cnt); }
		void 	shuffle		()						{ Array<str>::shuffle(0,cnt); }
		void 	sort		()						{ Array<str>::sort(0,cnt); }
		void 	rsort		()						{ Array<str>::rsort(0,cnt); }
		void 	sort		(bool(*gt)(str&,str&))	{ Array<str>::sort(0,cnt,gt); }

		void 	rol			(uint a, uint e)						{ Array<str>::rol(a,e); }
		void 	ror			(uint a, uint e)						{ Array<str>::ror(a,e); }
		void 	shuffle		(uint a, uint e)						{ Array<str>::shuffle(a,e); }
		void 	sort		(uint a, uint e)						{ Array<str>::sort(a,e); }
		void 	rsort		(uint a, uint e)						{ Array<str>::rsort(a,e); }
		void 	sort		(uint a, uint e, bool(*gt)(str&,str&))	{ Array<str>::sort(a, e, gt); }


		void	insert_sorted(cstr s)           { uint i=cnt;grow(cnt+1); while(i--&&strcmp(data[i],s)>0) data[i+1]=data[i]; data[i+1]=newcopy(s);}

		void	writeToFile	(FD& fd) const      { fd.write_int32(cnt); for(uint i=0;i<cnt;i++) fd.write_nstr(data[i]); }
		void	readFromFile	(FD& fd)       	{ kill();max=cnt = fd.read_int32();data=new str[max];
											      for(uint i=0;i<cnt;i++)data[i] = fd.read_new_nstr(); }

		void	writefile	(FD& fd) const      { for(uint i=0;i<cnt;i++){fd.write_str(data[i]); fd.write_int8('\n');} }
		void	readfile	(FD& fd)            { kill();try{cstr s = fd.read_new_str(); append(s);}
                                                  catch(file_error& e){if(e.error!=endoffile) throw(e);}
                                                  for(uint i=0;i<cnt;i++) data[i] = fd.read_new_str(); }

		int		find		(cstr s) const      { for(int i=cnt;i--;) if(eq(s,data[i])) return i; return -1; }
		void	append_if_new(cstr s)           { for(int i=cnt;i--;) if(eq(s,data[i])) return; append(s); }

    	void	appendFromFile(FD& fd,int32 cnt) noexcept(false);  //TODO
};


inline bool StrArray::operator== ( StrArray const& q ) const
{
	if(cnt!=q.cnt) return no;
	for( uint i=cnt; i--; ) { if(ne(data[i],q.data[i])) return no; }
	return yes;
}


inline bool StrArray::operator!= ( StrArray const& q ) const
{
	if(cnt!=q.cnt) return yes;
	for( uint i=cnt; i--; ) { if(ne(data[i],q.data[i])) return yes; }
	return no;
}



// ___________________________________________________________
// array of (dynamical) Objects pointers:
// passed objects get owned by the Array

template<class T>
class ObjArray : protected Array<T*>
{
protected:
	using Array<T*>::cnt;	// make cnt and data visible
	using Array<T*>::data;

	void		kill		()						{ while(cnt) delete data[--cnt]; Array<T*>::kill(); }
	void		init		()						{ Array<T*>::init(); }
	void		init		(ObjArray<T>const& q)	{ Array<T*>::init(q); }

public:			~ObjArray	()						{ while(cnt) delete data[--cnt]; }
				ObjArray	()						{}
				ObjArray	(ObjArray const&q)		:Array<T*>(q){}
	ObjArray&	operator=	(ObjArray const&q)		{ if(this!=&q) {kill();init(q);} return *this; }

	void		drop		()						{ if(cnt) delete data[--cnt]; }
	void		shrink		(uint n)				{ while(cnt>n) delete data[--cnt]; Array<T*>::shrink(n); }
	void		purge		()						{ kill(); init(); }
	void		append		(T* s)					{ Array<T*>::append(s); }
	void		append		(T& s)					{ Array<T*>::append(new T(s)); }
	void		remove		(uint idx)				{ ASSERT(idx<cnt); delete data[idx]; Array<T*>::remove(idx); }
	void		remove		(T* t)					{ for(uint i=cnt;i--;) if(data[i]==t){delete t; Array<T*>::remove(i); break;} }
	void		removerange	(uint a, uint e)		{ for(uint i=a;i<e;i++) delete data[i]; Array<T*>::removerange(a,e); }
	void		insertrange	(uint a, uint e)		{ Array<T*>::insertrange(a,e); }
	void		grow		(uint newcnt)			{ Array<T*>::grow(newcnt); }
	void		insertat	(uint idx, T* obj)		{ Array<T*>::insertat(idx,obj); }
	void		insertat	(uint idx, ObjArray& q)	{ Array<T*>::insertat(idx,q); }
	void		revert		()						{ Array<T*>::revert(); }

	ObjArray	copy		()						{ ObjArray z; for(uint i=0;i<cnt;i++) z.append(new T(*data[i])); return z; }

	uint		count		() const				{ return Array<T*>::cnt; }
	T**			getData		() const				{ return Array<T*>::data; }

	void		print		(FD& fd, cstr indent)	const;
	void		writeToFile	(FD& fd)				const;
	void		readFromFile(FD& fd)				noexcept(false);

    ObjArray&   operator << (T* t)					{ Array<T*>::append(t); return *this; }

//	bool		operator==	(ObjArray const& q) const	{ return Array<T*>::operator==(q); }		hmm, das testet auf Identität der Objekte!

    // note: returned object may be nullptr if grow() or auto-grow operator[] was used...
	T const&	operator[]	(uint i) const		{ return *Array<T*>::operator[](i); } // { ASSERT(i<count()); ASSERT(getData()[i]); /*if(!getData()[i]) getData()[i]=new T(); wg. zxsp class TzxBlock*/ return *getData()[i]; }
	T&			operator[]	(uint i)			{ return *Array<T*>::operator[](i); } // { if(i>=count()) Array<T*>::grow(i+1); if(!getData()[i]) getData()[i]=new T(); return *getData()[i]; }
	T const&	last		() const			{ return *Array<T*>::last(); }        // { ASSERT(count()); return operator[](count()-1); }
	T&			last		()					{ return *Array<T*>::last(); }        // { ASSERT(count()); return operator[](count()-1); }
	T const&	first		() const			{ return *Array<T*>::first(); }       // { ASSERT(count()); return operator[](0); }
	T&			first		()					{ return *Array<T*>::first(); }       // { ASSERT(count()); return operator[](0); }
	T*			pop			()					{ return *Array<T*>::pop(); }

	void		append		(ObjArray<T>& q);
	void		insertat	(ObjArray<T>& q, uint idx);
};


template<class T>
void ObjArray<T>::insertat(ObjArray<T>& q, uint idx)
{
	Array<T*>::insertrange(idx,idx+q.cnt);
	memcpy(data+idx,q.data,q.cnt*sizeof(T*));
	q.Array<T*>::purge();
}

template<class T>
void ObjArray<T>::append( ObjArray<T>& q )
{
	uint n=cnt, m=q.cnt;
	grow(n+m);
	memcpy(data+n, q.data, m*sizeof(T*));
	Array<T*>::purge();
}

template<class T>
void ObjArray<T>::print( FD& fd, cstr indent ) const
{
    for(uint i=0;i<count();i++)
    {
//        T::print(fd,indent,getData()[i]);     // note: if array can contain nullptr then static T::print(int,cstr,T*) must handle this!
		T* obj = data[i];
		if(obj) obj->print(fd,indent); // else ...
    }
}

template<class T>
void ObjArray<T>::writeToFile( FD& fd ) const
{
	fd.write(cnt);

    for(uint i=0; i<cnt; i++)
    {
		T* obj = data[i];
		uint8 f =obj!=nullptr;
		fd.write(f);
		if(f) obj->writeToFile(fd);
    }
}

template<class T>
void ObjArray<T>::readFromFile( FD& fd ) noexcept(false)
{
	purge();
	uint n; fd.read(n);
	if(n>ArrayMAX/sizeof(T)) throw file_error(fd,limiterror);
	data = (T**)new uint8[n*sizeof(T*)]; cnt = n;
	memset(data,0,n*sizeof(T));

    for(uint i=0; i<n; i++)
    {
		uint8 f; fd.read(f);
		if(f) (data[i] = new T())->readFromFile(fd);
    }
}

#undef CHECK_SIZE
#undef CHECK_IDX
#undef CHECK_END

#endif






















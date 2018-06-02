/*	Copyright  (c)	Günter Woigk 1991 - 2017
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

	2002-04-02 kio	fixed bug with rootidx()
	2005-06-09 kio	gcc4.0 complains about *((char*)p)++
*/

#define SAFE 3
#define LOG 1
#include "config.h"
#include "freeze.h"
#define	 RMASK(n)		(~(0xFFFFFFFF<<(n)))				// mask to select n bits from the right



// ====	Settings for Freeze() ==============================================================
// 		settings to adjust speed/compression ratio

/*	LTBITSMAX		size of link[] table: (1<<LTBITSMAX) entries, 4*(1<<LTBITSMAX) bytes.
					=> LTBITSMAX = 20  <=>  4 MByte !
					automatically scaled down by Freeze(); LTBITSMAX is a maximum value.
					recommended value: 		18 (fast, less mem) .. 20 (best)
					maximum values:			14 .. 21
					impact on compression: 	L
					impact on duration:    	XXL
*/
#define	LTBITSMAX	20

/*	LINKLIMIT		walking along the link[] list to find the best match for the current position
					may be a very long task. LINKLIMIT limits the number of steps.
					recommended value: 		100 (fast) ... 500 (better)
					maximum values:			50 .. 999999
					impact on compression: 	M
					impact on duration:    	XXL
*/
#define	LINKLIMIT	200

/* RTBITSMAX		size of root[] table: (1<<RTBITSMAX) entries, 4*(1<<RTBITSMAX) bytes.
					=> LTBITSMAX = 20  <=>  4 MByte !
					currently not scaled down automatically
					recommended value: 		16 (less mem), 18, or 20 (fast)
					supported values:		16, 18, or 20
					impact on compression: 	XXS
					impact on duration:    	L
*/
#define	RTBITSMAX	20


//	%00aaaaaa											6A + 0D
#define	HEAD_1		0x00
#define	ABITS_1		6
#define	DBITS_1		0
#define	DMIN_1		2
#define	DMAX_1		(DMIN_1-1+(1<<DBITS_1))
#define	AMAX_1		(1<<ABITS_1)

//	%010aaaaa  %aaaaaaaa								13A + 0D
#define	HEAD_20		0x40
#define	ABITS_20	13
#define	DBITS_20	0
#define	DMIN_2  	3
#define	DMAX_20		(DMIN_2-1+(1<<DBITS_20))
#define	AMAX_20		(1<<ABITS_20)

//	%011aaaaa  %aaaaaadd								11A + 2D
#define	HEAD_21		0x60
#define	ABITS_21	11
#define	DBITS_21	2
#define	DMIN_2 		3
#define	DMAX_21		(DMIN_2-1+(1<<DBITS_21))
#define	AMAX_21		(1<<ABITS_21)

//	%100aaaaa  %aaaaaaaa  %aaaaaadd						19A + 2D
#define	HEAD_30		0x80
#define	ABITS_30	19
#define	DBITS_30	2
#define	DMIN_3 		4
#define	DMAX_30		(DMIN_3-1+(1<<DBITS_30))
#define	AMAX_30		(1<<ABITS_30)

//	%101aaaaa  %aaaaaaaa  %aadddddd						15A + 6D
#define	HEAD_31		0xA0
#define	ABITS_31	15
#define	DBITS_31	6
#define	DMIN_3 		4
#define	DMAX_31		(DMIN_3-1+(1<<DBITS_31))
#define	AMAX_31		(1<<ABITS_31)

//	%1100aaaa  %aaaaaaaa  %aaaaaaaa  %addddddd			21A + 7D
#define	HEAD_40		0xC0
#define	ABITS_40	21
#define	DBITS_40	7
#define	DMIN_4 		5
#define	DMAX_40		(DMIN_4-1+(1<<DBITS_40))
#define	AMAX_40		(1<<ABITS_40)

//	%1101aaaa  %aaaaaaaa  %aaaaaddd  %dddddddd			17A + 11D
#define	HEAD_41		0xD0
#define	ABITS_41	18
#define	DBITS_41	10
#define	DMIN_4 		5
#define	DMAX_41		(DMIN_4-1+(1<<DBITS_41))
#define	AMAX_41		(1<<ABITS_41)

//	%1110dddd											4D
#define	VHEAD_1		0xE0
#define	VBITS_1		4
#define	VMIN_1		1
#define	VMAX_1		(VMIN_1-1+(1<<VBITS_1))

//	%1111dddd  %dddddddd								12D
#define	VHEAD_2		0xF0
#define	VBITS_2		(VBITS_1+8)
#define	VMIN_2		(VMAX_1+1)
#define	VMAX_2		(VMIN_2-1+(1<<VBITS_2))



// ----	table sizes ----------------------------------------------------
#define	RTBITS		RTBITSMAX		// root table size
#define	LTBITS						// link table size (calculated)
#define	R2BITS		8				// tupel root table size
#define	L2BITS		ABITS_1			// tupel link table size

#define	RTSIZE		(1<<RTBITS)		// the real sizes
#define	LTSIZE		ltsize			// (loc. var.)
#define	R2SIZE		(1<<R2BITS)
#define	L2SIZE		(1<<L2BITS)

#define	RTMASK		(RTSIZE-1)		// force-legal masks
#define	LTMASK		(LTSIZE-1)
#define	R2MASK		(R2SIZE-1)
#define	L2MASK		(L2SIZE-1)



/* ----	map 3 bytes to root table index --------------------------------------
		sort of a hash function
*/
#if RTBITS==16
	inline ushort rootidx ( const uchar* p )
	{
		return *(ushort*)p ^ *(ushort*)(p+1)*13;
	}
#else
	inline ulong rootidx ( const uchar* p )
	{
		return (((ulong)*(ushort*)p)<<(RTBITS-16)) ^ (*(ushort*)(p+1));		// kio 2002-04-02
	}
#endif


/* ----	compare two strings ----------------------------------------------
		return length of same bytes
		stop at p==e
*/
inline int compare ( const uchar* p, const uchar* q, const uchar* e )
{	e--;
	const uchar* p0 = p;
	while (p<e && *(short*)p == *(short*)q) { p+=2; q+=2; }
	return p-p0 +(p<=e && *p==*q);
}


/* ----	calculate minimum field size -------------------------------------
		return the number of bits required to hold the number
*/
inline int bits ( size_t n )
{
	int b=0;
	while (n>>8)	{ b+=8; n>>=8;  }
	if    (n>>4)	{ b+=4; n>>=4;  }
	if    (n>>2)	{ b+=2; n>>=2;  }
	if    (n>>1)	{ b+=1; n>>=1;  }
	return b + n;
}


/* ----	calculate size of compressed data --------------------------------
		how many bytes are required to store a reference
		with offset 'offset' to 'bytes' bytes ?
*/
inline uint comp_size ( size_t offset, size_t bytes )
{
	assert(offset >= 1);

	if ( offset <= AMAX_1  &&  bytes >= DMIN_1 &&  bytes <= DMAX_1  ) return 1;

	if ( offset <= AMAX_20 &&  bytes >= DMIN_2 &&  bytes <= DMAX_20 ) return 2;
	if ( offset <= AMAX_21 &&  bytes >= DMIN_2 &&  bytes <= DMAX_21 ) return 2;

	if ( offset <= AMAX_30 &&  bytes >= DMIN_3 &&  bytes <= DMAX_30 ) return 3;
	if ( offset <= AMAX_31 &&  bytes >= DMIN_3 &&  bytes <= DMAX_31 ) return 3;

	if ( offset <= AMAX_40 &&  bytes >= DMIN_4 &&  bytes <= DMAX_40 ) return 4;
	if ( offset <= AMAX_41 &&  bytes >= DMIN_4 &&  bytes <= DMAX_41 ) return 4;

	return 99;
}


/* ----	compress buffer --------------------------------------------------
		returns size of compressed data
		sets/clears errno
*/
size_t Freeze ( const uchar* sp, size_t ssz, uchar* dp, size_t dsz )
{
// first quick test:
	errno = ok;
	if (ssz==0) return 0;

// local vars:
	const uchar* se = sp+ssz;		// source buffer end
	uchar*       de = dp+dsz;		// destination buffer end
	const uchar* s0 = sp++;			// start of pending verbatim data

// the tables

// root[] contains a pointer to the last occurence of any byte triplet.
// since this would require 2^24 pointers, the size is reduced to 2^16 pointers
// by use of function rootidx(). NULL means: no match.
// note: therefore these pointers may point to 'wrong' places, which contain data
// which coincidentially has the same rootidx.

// link[] contains links from one occurance of a root[] entry to the previous occurance.
// ideally it has the same size as the source buffer. but because we may not get that much memory,
// we'll have to cope with a smaller array. every entry is a pointer from the current
// position to the previous occurance. NULL means: last match.
// note: this list links occurances of the same rootidx() which may be quite different!

	size_t LTSIZE;
	LTSIZE = 1 << min ( LTBITSMAX, bits(ssz-1) );
	const uchar**  	root = new const uchar*[RTSIZE];
	const uchar**  	link = new const uchar*[LTSIZE];
	const uchar*	r2[R2SIZE];
	const uchar*	l2[L2SIZE];

	while (!link && (LTSIZE<<=1)>=0x0100) link = new const uchar*[LTSIZE];
	if (!link||!root) { errno = outofmemory; goto x; }

	memset (root,0,RTSIZE*sizeof(*root));
	memset (link,0,LTSIZE*sizeof(*link));
	memset (r2  ,0,R2SIZE*sizeof(*r2)  );
	memset (l2  ,0,L2SIZE*sizeof(*l2)  );



// ---- do it ----

	while (true)
	{
	// template finder data
		size_t	bytes  = 1;		// template size
		size_t	offset = 0;		// distance
		size_t	size   = 0;		// compressed data size
		size_t	ratio  = 0;		// compression ratio *12

	// near end of source ?
	// too short to get the rootidx which requires 3 bytes ahead ?
		if (sp>se-3)
		{
			sp = se;			// store last verbatim data and exit
			goto sto;
		}

	// scan possible templates starting from root table and proceeding via link table
	// min. size=3; tupels are scanned later.
		{
			size_t	mybytes;		// my template size
			size_t	myoffset;		// my distance
			size_t	mysize;			// my compressed data size
			size_t	myratio;		// my compression ratio *12
			const uchar* p;			// temp
			int f=LINKLIMIT;
			for ( p=root[rootidx(sp)]; (myoffset=sp-p)<=LTSIZE&&--f; p=link[(long)p&LTMASK] )
			{
				assert(p >= se-ssz);
				assert(p < sp);

				mybytes  = compare(sp,p,se);
				mysize   = 2;

				if ( mybytes < DMIN_2 ) continue; else							// template too short for 2-byte code
				if ( (myratio=mybytes*(12/2))<ratio ) continue; else			// no compression improvement

				if ( myoffset <= AMAX_20 && mybytes <= DMAX_20 ) goto s2; else	// YEP!
				if ( myoffset <= AMAX_21 && mybytes <= DMAX_21 ) goto s2; else	// YEP!

				if ( mybytes < DMIN_3 ) continue; else							// template too short for 3-byte code
				if ( (myratio=mybytes*(12/3))<ratio ) continue; else			// no compression improvement

				if ( myoffset <= AMAX_30 && mybytes <= DMAX_30 ) goto s3; else	// YEP!
				if ( myoffset <= AMAX_31 && mybytes <= DMAX_31 ) goto s3; else	// YEP!

				if ( mybytes < DMIN_4  ) continue; else							// template too short for 4-byte code
				if ( (myratio=mybytes*(12/4))<ratio ) continue; else			// no compression improvement

				if ( myoffset <= AMAX_41 ) { if(mybytes>DMAX_41) {f=1;mybytes=DMAX_41;myratio=DMAX_41*12/4;} } else // YEP!
				if ( myoffset <= AMAX_40 ) { if(mybytes>DMAX_40) {f=1;mybytes=DMAX_40;myratio=DMAX_40*12/4;} } else // YEP!
				break;															// distance too far => exit loop

				mysize++;
			s3:	mysize++;
			s2:
				assert( mysize == comp_size(myoffset,mybytes) );

				ratio  = myratio;
				bytes  = mybytes;
				size   = mysize;
				offset = myoffset;
			}

			assert( ratio==0 || ratio == 12*bytes/size);

		// scan possible 2-byte templates starting from r2 table and proceeding via l2 table
		// compressed size = 1; ratio = 2 *12
			if (ratio < 2*12)
			{
				for ( p=r2[*sp]; sp-p<=L2SIZE; p=l2[(long)p&L2MASK] )
				{
					assert(p >= se-ssz);
					assert(p < sp);

					if (*(short*)sp != *(short*)p) continue;

					ratio  = 2*12;
					bytes  = 2;
					size   = 1;
					offset = sp-p;
					break;
				}
			}


		// update tables
			{	const uchar* e = sp+bytes;
				if (e>se-2) e=se-2;				// kio 2002-04-02

			// update root[] and link[]
				for ( p=sp; p<e; p++ )
				{
					int n = rootidx(p);
					link[(long)p&LTMASK] = root[n];
					root[n] = p;
				}

			// update r2[] and l2[]
				for ( p=sp; p<e; p++ )
				{
					l2[(long)p&L2MASK] = r2[*p];
					r2[*p] = p;
				}
			}


		// no template for compression found ?
			if (ratio < 1*12 +(uint)(sp>s0)) { sp++; assert(bytes==1); continue; }
	}

	// *** template found ***

	// store pending verbatim data (if any)
		if (sp>s0)
		{
sto:		int n;
			while ((n=sp-s0) > VMAX_1)
			{
				n = min(n,VMAX_2);
				if (dp+n+2>de) goto bts;		// buffer too small
				*dp++ = VHEAD_2 | ((n-VMIN_2)>>8);
				*dp++ =             n-VMIN_2;
				memcpy ( dp, s0, n );
				s0 += n;
				dp += n;
				xxlog("V%i",n);
			}

			if (n)
			{
				if (dp+n+1>de) goto bts;		// buffer too small
				*dp++ = VHEAD_1 | (n-VMIN_1);
				memcpy ( dp, s0, n );
				dp += n;
				xxlog("V%i",n);
			}

			if (sp>=se) goto x;					// ok
		}

	// update source pointers
		sp += bytes;
		s0  = sp;


	// store template reference:
		{
			assert(size == comp_size(offset,bytes));
			if (dp+size>de) goto bts;			// buffer too small

			offset-= 1;							// value to store
		//	bytes -= 1+size;					// value to store	assumes: DMIN_<n> = n+1
		//	XXTRAP(DMIN_1!=2);
		//	XXTRAP(DMIN_2!=3);
		//	XXTRAP(DMIN_3!=4);
		//	XXTRAP(DMIN_4!=5);
			xxlog("C%lu",bytes);

			switch(size)
			{
			case 1:
				*dp++ = HEAD_1 | offset;
				continue;
			case 2:
				if (offset<AMAX_21)	bytes += (offset<<DBITS_21) -DMIN_2 +(HEAD_21<<8);
				else				bytes += (offset<<DBITS_20) -DMIN_2 +(HEAD_20<<8);
				*dp++ = bytes>>8;
				*dp++ = bytes;
				continue;
			case 3:
				if (offset<AMAX_31)	bytes += (offset<<DBITS_31) -DMIN_3 +(HEAD_31<<16);
				else				bytes += (offset<<DBITS_30) -DMIN_3 +(HEAD_30<<16);
				*dp++ = bytes>>16;
				*dp++ = bytes>>8;
				*dp++ = bytes;
				continue;
			case 4:
				if (offset<AMAX_41)	bytes += (offset<<DBITS_41) -DMIN_4 +(HEAD_41<<24);
				else				bytes += (offset<<DBITS_40) -DMIN_4 +(HEAD_40<<24);
				*dp++ = bytes>>24;
				*dp++ = bytes>>16;
				*dp++ = bytes>>8;
				*dp++ = bytes;
				continue;
			default:
				IERR();
			}
		}
	}


bts:
	errno = buffertoosmall;

x:	delete[] link;
	delete[] root;

	return dp-(de-dsz);
}



// ----	shift memory to higher address ---------------------------------
//		if overlapping, the just copied data is taken as source
inline void shift ( uchar* z, const uchar* q, long n )
{	uchar* e = z + n;

	switch ( z-q )
	{
	case 0:
		return;

	case 1:
	{	uchar c = *q;
		memset ( z, c, n );
	}	return;

	case 2:
	{	if (n&1)	{ *(char* )z = *(char*)q; z+=1; q+=1; }		// 2005-06-09: gcc4.0 meckerte neuerdings => geändert. mumpfl.
		short s  =    *(short*)q;								//
		while (z<e)	{ *(short*)z = s; z+=2; }					// 2005-06-09: gcc4.0 meckerte neuerdings
	}	return;

	case 3:
	{	if (n>3)
		{
			long l = *(long*)q;
			while (z<e-3)
			{
				*(long*)z = l;
				z += 3;
			}
		}
		while (z<e)	{ *(char*)z = *(char*)q; z+=1; q+=1; }		// 2005-06-09: gcc4.0 meckerte neuerdings
	}	return;

	default:
	{	if (n&1)	{ *(char* )z = *(char *)q; z+=1; q+=1; }	// 2005-06-09: gcc4.0 meckerte neuerdings
		if (n&2)	{ *(short*)z = *(short*)q; z+=2; q+=2; }	// 2005-06-09: gcc4.0 meckerte neuerdings
		if (n&4)	{ *(long* )z = *(long *)q; z+=4; q+=4; }	// 2005-06-09: gcc4.0 meckerte neuerdings
		while (z<e)
		{	((long*)z)[0] = ((long*)q)[0];
			((long*)z)[1] = ((long*)q)[1];
			q += 8;												// 2005-06-09: gcc4.0 meckerte neuerdings
			z += 2;												// 2005-06-09: gcc4.0 meckerte neuerdings
		}
	}	return;
	}
}


/* ----	expand buffer --------------------------------------------------
		returns size of expanded data
		sets/clears errno
*/
size_t Melt ( const uchar* sp, size_t ssz, uchar* dp, size_t dsz )
{
	const uchar* se = sp+ssz;
	const uchar* de = dp+dsz;

	assert(DBITS_1 == 0);
	assert(DBITS_20 == 0);

	errno = endoffile;

	while (sp<se)
	{
		uchar c1 = *sp++;
		ulong n;
		ulong a;

		switch(c1>>5)
		{
		case  HEAD_1>>5:
		case (HEAD_1>>5)+1:
			a = c1;//&RMASK(ABITS_1);
			n = DMIN_1;
			break;

		case HEAD_20>>5:
			if (sp==se) goto x;
			a = ((c1-HEAD_20)*256+*sp++)&RMASK(ABITS_20);
			n = DMIN_2;
			break;

		case HEAD_21>>5:
			if (sp==se) goto x;
			n = (c1-HEAD_21)*256 + *sp++;
			a = (n>>DBITS_21) & RMASK(ABITS_21);
			n = DMIN_2 + (n&RMASK(DBITS_21));
			break;

		case HEAD_30>>5:
			if (sp>se-2) goto x;
			n = (c1-HEAD_30)*256 + *sp++;
			n = n *256 + *sp++;
			a = (n>>DBITS_30) & RMASK(ABITS_30);
			n = DMIN_3 + (n&RMASK(DBITS_30));
			break;

		case HEAD_31>>5:
			if (sp>se-2) goto x;
			n = (c1-HEAD_31)*256 + *sp++;
			n = n *256 + *sp++;
			a = (n>>DBITS_31) & RMASK(ABITS_31);
			n = DMIN_3 + (n&RMASK(DBITS_31));
			break;

		case HEAD_40>>5:
	//	case HEAD_41>>5:
			if (sp>se-3) goto x;
			n = (c1&0x0F)*256 + *sp++;
			n = n *256 + *sp++;
			n = n *256 + *sp++;

			if (c1&0x10)
			{	a = (n>>DBITS_41) & RMASK(ABITS_41);
				n = DMIN_4 + (n&RMASK(DBITS_41));
			}
			else
			{	a = (n>>DBITS_40) & RMASK(ABITS_40);
				n = DMIN_4 + (n&RMASK(DBITS_40));
			}
			break;

		default:	// verbatim text
			if (sp==se) goto x;
			if (c1&0x10) n = VMIN_2 + ((c1*256+*sp++)&RMASK(VBITS_2));
			else		 n = VMIN_1 + ( c1           &RMASK(VBITS_1));
			xxlog("V%lu",n);
			if (sp<=se-n && dp<=de-n)
			{
				memcpy ( dp, sp, n );
				sp+=n;
				dp+=n;
				continue;
			}
			if (sp+n>se) errno = corrupteddata;
			else		 errno = buffertoosmall;
			goto x;
		}

		a += 1;
		xxlog("C%lu",n);

		if ( dp-a>=de-dsz && dp+n<=de )
		{
			shift ( dp, dp-a, n );
			dp += n;
			continue;
		}
		if (dp-a<de-dsz) errno = corrupteddata;
		else			 errno = buffertoosmall;
		goto x;
	}

	errno = ok;

x:	return dp-(de-dsz);
}


/* ----	calculate expanded buffer size --------------------------------------------------
		returns size of expanded data
		sets/clears errno
*/
size_t MeltSize ( const uchar* sp, size_t ssz )
{
	const uchar* se = sp+ssz;
	size_t dp = 0;
	ulong n;
	uchar c1;

	while (sp<se)
	{
		switch ( (c1=*sp++)>>5 )
		{
		case  HEAD_1>>5:		//	%00aaaaaa
		case (HEAD_1>>5)+1:		//	%00aaaaaa
			dp += DMIN_1;
			continue;

		case HEAD_20>>5:		//	%010aaaaa  %aaaaaaaa
		//	if (sp==se) goto x;
			sp++;
			dp += DMIN_2;
			continue;

		case HEAD_21>>5:		// %011aaaaa  %aaaaaadd
			if (sp==se) goto x;
			dp += DMIN_2 + ((*sp++)&RMASK(DBITS_21));
			continue;

		case HEAD_30>>5:		// %100aaaaa  %aaaaaaaa  %aaaaaadd
			if (sp>se-2) goto x;
			sp++;
			dp += DMIN_3 + ((*sp++)&RMASK(DBITS_30));
			continue;

		case HEAD_31>>5:		// %101aaaaa  %aaaaaaaa  %aadddddd
			if (sp>se-2) goto x;
			sp++;
			dp += DMIN_3 + ((*sp++)&RMASK(DBITS_31));
			continue;

		case HEAD_40>>5:		//	%1100aaaa  %aaaaaaaa  %aaaaaaaa  %addddddd
	//	case HEAD_41>>5:		//	%1101aaaa  %aaaaaaaa  %aaaaaddd  %dddddddd
			if (sp>se-3) goto x;
			sp++;
			n = *sp++;
			n = n*256 + *sp++;

			if (c1&0x10) dp += DMIN_4 + (n&RMASK(DBITS_41));
			else		 dp += DMIN_4 + (n&RMASK(DBITS_40));
			continue;

		default:	// verbatim text
			if (sp==se) goto x;
			if (c1&0x10) n = VMIN_2 + ((c1*256+*sp++)&RMASK(VBITS_2));	//	%1111dddd  %dddddddd
			else		 n = VMIN_1 + ( c1           &RMASK(VBITS_1));	//	%1110dddd
			sp+=n;
			dp+=n;
			continue;
		}
	}

	if (sp==se)
	{
		errno = ok;
		return dp;
	}

x:	errno = corrupteddata;
	return 0;
}
















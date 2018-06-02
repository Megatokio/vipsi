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


	used in zxsp
*/


template<class T>		// auto-destruct array
class ADA
{
	T*		p;
	
			ADA			(ADA const&);	// prohibit
	ADA&	operator=	(ADA const&);	// prohibit

public:		ADA			()				{ p = NULL; }
			ADA			(T* q)			{ p = q; }
			ADA			(uint32 n)		: p(NULL) { p = new T[n]; }
			~ADA		()				{ delete[] p; }
	void	operator=	(T* q)			{ delete[] p; p = q; }
	T&		operator[]	(uint32 i)		{ return p[i]; }
	T*		operator+	(uint32 n)		{ return p+n; }
	T*		operator-	(uint32 n)		{ return p-n; }
};

// typedef ADA<int8>	ADi8ptr;
// typedef ADA<uint8>	ADu8ptr;
//	typedef ADA<int16>	ADi16ptr;
//	typedef ADA<uint16>	ADu16ptr;
//	typedef ADA<int32>	ADi32ptr;
//	typedef ADA<uint32>	ADu32ptr;



















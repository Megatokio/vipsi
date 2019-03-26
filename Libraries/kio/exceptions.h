/*	Copyright  (c)	Günter Woigk 1999 - 2019
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


	error handling

this file provides:

	exception classes
		bad_alloc		alias for std::bad_alloc
		any_error		base class for below error classes
		file_error		"unix/file_utilities.h"
		index_error		"kio/abort.h"		macro INDEX()
		limit_error		"kio/abort.h"		macro LIMIT()
		data_error							for data parsers, e.g. for input data from file


	raising, reading and clearing of errors.
	uses the global errno.

	cstr ErrorStr	(int errorcode)			returns error text for an OS or application defined error
	cstr ErrorStr	()						returns the user-readable text for the current error
	void SetError	(cstr custommessage)	sets an error with an arbitrary error text
	void SetError	(int errorcode)			sets an OS or application defined error
	int  GetError	()						gets the current error  ((same as errno))
	void ClearError ()						clears the current error


	extending the error code list:

	1:	simply use SetError("custommessage") to set arbitrary errors.
		disadvantage: you can't use errno to detect which error exactly happened.
	2:	define custom errors in "custom_errors.h"
	3:	use SetError(err,msg) to set errno to known error number and msg to arbitrary text.
*/



#ifndef	_exceptions_h_
#define	_exceptions_h_

#include <new>
#include "kio/kio.h"
extern str usingstr( cstr, ... );		// #include "cstrings/cstrings.h"

#ifndef __cplusplus
	#error	C und nicht C++  !!
#endif



// ------------------------------------------------------------------------
//
//			exception classes
//
// ------------------------------------------------------------------------


/*
	class std::exception			// #include <exception>
	{
		public:			exception	()			throw(){}
		virtual			~exception	()			throw();
		virtual cstr	what		() const	throw();
	};

hierarchy:
	std::exception					// #include <exception>		base class
		bad_alloc					// #include <new>			out of memory
		std::bad_typeid				// #include <typeinfo>		Falscher Objekttyp
		std::bad_cast				// #include <typeinfo>		Falscher Objekttyp bei Typumwandlung
		std::bad_exception			// #include <exception>		unexpected()

		any_error;					//							base class for own errors: includes error code
			internal_error			// #include "kio/abort.h"	ABORT, TRAP, ASSERT, IERR, TODO
				index_error;		// "kio/abort.h"			macro INDEX()	index outside array
				limit_error;		// "kio/abort.h"			macro LIMIT()	array too large
			data_error;				//							data parsers, e.g. for input data from file
			file_error;				// "unix/file_utilities.h"

		std::logic_error			// #include <stdexcept>		theoretisch vermeidbare Laufzeitfehler
			std::invalid_argument	// #include <stdexcept>		stdc++ lib ((general function argument error))
			std::length_error		// #include <stdexcept>		stdc++ lib ((data exceeding allowed size))
			std::out_of_range		// #include <stdexcept>		stdc++ lib ((e.g. index))
			std::domain_error		// #include <stdexcept>		stdc++ lib ((domain of math. function))

		std::runtime_error			// #include <stdexcept>		fehlerhafte Daten zur Laufzeit
			std::range_error		// #include <stdexcept>		arith. Bereichsüberschreitung
			std::overflow_error		// #include <stdexcept>		arith. Überlauf
			std::underflow_error	// #include <stdexcept>		arith. Unterlauf
*/


// ---------------------------------------------
//			bad_alloc
// ---------------------------------------------

typedef	std::bad_alloc	bad_alloc;



// ---------------------------------------------
//			any_error
// ---------------------------------------------

class any_error : public std::exception
{
public:
	int		error;			// errno
	cstr	text;			// custom error message: const, temp or nullptr

public:		any_error		(cstr msg)					throw()	:error(customerror),text(msg){}
			any_error		(int error)					throw()	:error(error), text(nullptr){}
			any_error		(int error, cstr msg)		throw()	:error(error), text(msg){}
VIR			~any_error		()							throw()	{}

VIR	cstr	what			() const					throw();
};



// ---------------------------------------------
//			internal_error
// ---------------------------------------------

class internal_error : public any_error
{
public:
	cstr  file;			// source line: const or temp
	uint line;			// source line number

public:	 internal_error	(cstr file, uint line)			 throw() :any_error(internalerror),file(file),line(line){}
		 internal_error	(cstr file, uint line, int err)	 throw() :any_error(err),file(file),line(line){}
		 internal_error	(cstr file, uint line, cstr msg) throw() :any_error(internalerror,msg),file(file),line(line){}
	cstr what			() const						 throw();
};



// ---------------------------------------------
//			limit_error
// ---------------------------------------------

class limit_error : public internal_error
{
	uint 	sz,max;

public:	 limit_error (cstr file, uint line, uint sz, uint max)	throw() :internal_error(file,line,limiterror),sz(sz),max(max){}

	cstr what		 () const									throw();
};



// ---------------------------------------------
//			index_error
// ---------------------------------------------

class index_error : public internal_error
{
	uint idx,max;

public:	 index_error (cstr file, uint line, uint idx, uint max)	throw() :internal_error(file,line,indexerror),idx(idx),max(max){}

	cstr what		 () const									throw();
};



// ---------------------------------------------
//			data_error								--> md5, backup_daemon.client
// ---------------------------------------------

class data_error : public any_error
{
public:
		 data_error	()								throw()	:any_error(dataerror){}
		 data_error	(cstr msg)						throw()	:any_error(dataerror,msg){}
		 data_error	(int error, cstr msg)			throw()	:any_error(error,msg){}
};



// ---------------------------------------------
//			file_error
// ---------------------------------------------

class file_error : public any_error
{
public:
	cstr filepath;
	int	 fd;

public:	 file_error(class FD&, int error)					throw();
		 file_error(class FD&, int error, cstr msg)			throw();
		 file_error(cstr path, int fd, int error)			throw()	: any_error(error),filepath(path),fd(fd){}
		 file_error(cstr path, int fd, int error, cstr msg)	throw()	: any_error(error,msg),filepath(path),fd(fd){}
//		 file_error(class FD&, cstr msg)					throw();
//		 file_error(cstr fpath, int fd)						throw()	: any_error(errno),filepath(fpath),fd(fd){}
//		 file_error(cstr fpath, int fd, cstr msg)			throw()	: any_error(errno,msg),filepath(fpath),fd(fd){}

	cstr what() const										throw();
};



#endif





























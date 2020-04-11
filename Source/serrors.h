#pragma once
/*	Copyright  (c)	Günter Woigk 2019 - 2020
					mailto:kio@little-bat.de

	This file is free software.

	Permission to use, copy, modify, distribute, and sell this software
	and its documentation for any purpose is hereby granted without fee,
	provided that the above copyright notice appears in all copies and
	that both that copyright notice, this permission notice and the
	following disclaimer appear in supporting documentation.

	THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT ANY WARRANTY, NOT EVEN THE
	IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE
	AND IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY DAMAGES
	ARISING FROM THE USE OF THIS SOFTWARE,
	TO THE EXTENT PERMITTED BY APPLICABLE LAW.
*/

#include "VString/String.h"

extern String ErrorString (int err);
inline String ErrorString ()					{ return ErrorString(errno); }

inline void ForceError (int err)				{ errno = err; }
inline void SetError (int err)					{ if (errno==ok) errno = err; }

extern void ForceError (int err, cString& msg);
inline void ForceError (cString& msg)			{ ForceError(customerror,msg); }

inline void SetError (int err, cString& msg)	{ if (errno==ok) ForceError(err,msg); }
inline void SetError (cString& msg)				{ if (errno==ok) ForceError(msg); }

extern void ForceError (int err, cstr msg);
inline void ForceError (cstr msg)				{ ForceError(customerror, msg); }

inline void SetError (int error, cstr msg)		{ if (errno==ok) ForceError(error,msg); }
inline void SetError (cstr msg)					{ if (errno==ok) ForceError(msg); }

extern void AppendToError  (cString& msg);
extern void PrependToError (cString& msg);



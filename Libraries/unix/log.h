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

	log to logfile or stderr
	2015-02 kio branched from kio/log.cpp


	this file is included from kio/kio.h

*/



#ifndef _log_h_
#define _log_h_

#include "kio/kio.h"


// get current time:
extern double	now();



//	–––––––––––––––––––––––––––––––––––––––––––––––––––
//	Logging:
//	–––––––––––––––––––––––––––––––––––––––––––––––––––

enum LogRotation { NEVER,DAILY,WEEKLY,MONTHLY };	// value for uint logrotate in openLogfile(..)
extern bool log2console;                            // write access allowed. default: log to console = true.
extern bool timestamp_with_date;					// typically set by openLogfile(…); default = no
extern bool timestamp_with_msec;					// typically set by openLogfile(…); default = no

// open logfile,
// define log rotation,
// define max. number of logfiles to keep,
// switch on|off logging to console:
extern void openLogfile(cstr dirpath, LogRotation, uint max_logfiles, bool log2console, bool with_date, bool with_msec);
extern void openLogfile(cstr dirpath, LogRotation, uint max_logfiles, bool log2console);

// log to stderr and/or logfile:
//
extern void LogLine ( cstr format, ... );			// log line with timestamp etc.
extern void Log 	( cstr format, ... );			// incrementally build log string, requires a final LogNL()
extern void LogNL 	( );							// finalize incremental log string or log an empty line



// indented logging for the lifetime of a function:
//
struct LogIndent
{
	LogIndent	(cstr format, ...);
	~LogIndent	();
};

#define LogIn	LogIndent _z_log_ident				// usage:  LogIn("format/message", ...)



//	–––––––––––––––––––––––––––––––––––––––––––––––––––
//	Conditional Versions:
//	–––––––––––––––––––––––––––––––––––––––––––––––––––

#define	XLOG		(LOG>=1)
#define	XXLOG		(LOG>=2)
#define	XXXLOG		(LOG>=3)
#define	XXXXLOG		(LOG>=4)

#define	XLogNL		if( LOG<1 )	{} else LogNL
#define	XXLogNL		if( LOG<2 )	{} else LogNL
#define	XXXLogNL	if( LOG<3 )	{} else LogNL
#define	XXXXLogNL	if( LOG<4 )	{} else LogNL

#define	XLog		if( LOG<1 )	{} else Log
#define	xlog		if( LOG<1 )	{} else Log
#define	XXLog		if( LOG<2 )	{} else Log
#define	xxlog		if( LOG<2 )	{} else Log
#define	XXXLog		if( LOG<3 )	{} else Log
#define	XXXXLog		if( LOG<4 )	{} else Log

#define	XLogLine    if( LOG<1 )	{} else LogLine
#define	xlogline    if( LOG<1 )	{} else LogLine
#define	XXLogLine	if( LOG<2 )	{} else LogLine
#define	xxlogline	if( LOG<2 )	{} else LogLine
#define	XXXLogLine	if( LOG<3 )	{} else LogLine
#define	XXXXLogLine	if( LOG<4 )	{} else LogLine


#if XLOG
#define XLogIn		LogIn
#define xlogIn		LogIn
#else
#define XLogIn		XLog
#define xlogIn		XLog
#endif

#if XXLOG
#define XXLogIn		LogIn
#define xxlogIn		LogIn
#else
#define XXLogIn		XXLog
#define xxlogIn		XXLog
#endif

#if XXXLOG
#define XXXLogIn	LogIn
#else
#define XXXLogIn	XXXLog
#endif

#if XXXXLOG
#define XXXXLogIn	LogIn
#else
#define XXXXLogIn	XXXXLog
#endif




#endif







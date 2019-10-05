/*	Copyright  (c)	Günter Woigk   2001 - 2019
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


#ifndef vscript_h
#define vscript_h

#include "Var/Var.h"
#include "Compiler.h"


class Stream;
class Prefs;

enum rt { rt_none, rt_shell, rt_script, rt_cgi };

#ifdef OPCODE_PROFILING
extern uint32		opcodeProfile[256];
extern uint32		opcodeTupels[256][256];
#endif


typedef int32 displacement;

extern	Var*		root;

class VScript
{
friend class Thread;

	Var*			rsrc;
	Var*			threads;
	Var*			appl;
	Var*			shell;
	Var*			globals;		// note: during call() Thread->globals != VScript.globals
	Var*			env;
	Var*				user;
	Var*				group;
	Var*				hostname;
	Var*			job;
	Var*				script;
	Var*				runtype;
	Var*				verbose;
	Var*			prefs;
//	Var*				stdin_attr;
//	Var*				stdout_attr;
//	Var*				stderr_attr;
//	Var*				prompt;
	Var*			stdio;
	Var*				stdin0;		// original stream
	Var*				stdout0;	// original stream
	Var*				stderr0;	// original stream
	Var*			stats;
#ifdef MALLOC_STATISTICS
	Var*				malloc_requests;
	Var*				malloc_xrequests;
	Var*				malloc_usedblocks;
#endif
#ifdef OPCODE_PROFILING
	Var*				opcode_profile;
#endif


	Compiler	compiler;


	void		InitThreadError		( );				// error = { 0, "", {} }
	void		SetThreadError		( );				// error = { errno, errorstr, {} }
	void		StartCompileError	( cstr msg );		// error = { errno, errorstr, { c.row,c.col,c.file,msg} }
	void		AddToThreadError	( uint row, uint col, cString& file, cString& info );
	void		AddToThreadError	( uptr ip, cString& msg );

	void		get_cgi_arguments	( Var*, cstr );
	cstr 		tostr				( uchar*, Var** );

				VScript				( const VScript& );		// prohibit
	VScript&	operator=			( const VScript& );		// prohibit

public:
				VScript 			( rt runtype,			// rt_shell if invoced without scriptfile
									  cString& applname, 	// argv[0]
									  cString& filename, 	// scriptfile to execute
									  Var* argv, 			// arguments to scriptfile
									  Stream*i, Stream*o, Stream*e );
				~VScript			( );

// Run the VIP
	Var*		Execute				( );
	int			RunType				( )						{ return runtype->LongValue(); }
};










#endif


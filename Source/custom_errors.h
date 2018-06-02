/*	Copyright  (c)	Günter Woigk   1999-2015
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


/*	this file is included from "errors.h"
	it defines additional error codes/messages
*/

// compile:

	EMAC( syntaxerror,			"syntax error" ),					// compile
	EMAC( errKEND,				"unexpected end of block comment" ),// compile::tokenize
	EMAC( errKANF,				"unterminated block comment" ),		// compile::tokenize
	EMAC( separatormissing,		"separator missing" ),				// compile::tokenize
	EMAC( nestingerror,			"flow control nesting error" ),		// compile::checkloops
	EMAC( branchtoofar,			"branch too far" ),					// compile::assemble
	EMAC( expectedvalue,		"expected value" ),					// compile::value

// execute:

	EMAC( indexoorange,			"index out of range" ),
	EMAC( corrupteddata,		"corrupted data" ),				// MeltString()
	EMAC( varexists,			"variable exists" ),
//	EMAC( varinuse,				"variable in use" ),
	EMAC( toofewarguments,		"too few arguments for procedure" ),
	EMAC( toomanyarguments,		"too many arguments for procedure" ),
	EMAC( noresult,				"procedure returned no result" ),
	EMAC( termiroot,			"cannot terminate root thread (--> use end)" ),
	EMAC( tokennotfound,		"word not found" ),				// tINFO
	EMAC( variablenotfound,		"variable not found" ),			// tIDF, tAT
	EMAC( procnoparent,			"not inside a procedure" ),		// tTHIS
	EMAC( topnoparent,			"top no parent" ),				// tDEL

	EMAC( varreq,				"variable required" ),
	EMAC( numreq,				"numeric value required" ),
	EMAC( stringreq,			"text required" ),
	EMAC( stringorlistreq,		"text or list required" ),
	EMAC( listreq,				"list required" ),
	EMAC( tidreq,				"thread id required" ),
	EMAC( irptreq,				"interrupt cell required" ),
	EMAC( semareq,				"semaphore required" ),
	EMAC( procreq,				"procedure required" ),
	EMAC( streamreq,			"stream handle required" ),
	EMAC( ttyreq,				"tty (terminal) required" ),

// execute: i/o error:
	EMAC( csz1req,				"binary (8-bit clean) string required" ),
	EMAC( timeoutexpired,		"timeout expired" ),
// Stream:
	EMAC( asynciopending,		"async i/o pending" ),
	EMAC( nostopctlfound,		"line length exceeded limit (stopctl)" ),
	EMAC( ttynoresponse,		"tty did not respond to query sequence" ),
	EMAC( inputnotpossible,		"file not open for reading" ),
	EMAC( outputnotpossible,	"file not open for writing" ),
// SystemIO:
	EMAC( unsuitedfiletype,	"unsuited file type" ),
//	EMAC( wrongfiletype,	"wrong file type" ),


#include "Var/var_errors.h"
#include "Unicode/unicode_errors.h"
#include "VString/String_errors.h"
#include "compress_errors.h"




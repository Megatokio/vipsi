#pragma once

/* ===============================================
			compile-time options
=============================================== */


#define CUSTOM_ERRORS "custom_errors.h"
#define APPL_NAME "vipsi"
#define LOGFILE 1


//  version info:
//
#define applName        APPL_NAME
#define applLongName    "vipsi - VIP script interpreter"
#define applVersion     "0.9.a0"
#define applCopyright   "(c) 1994-2019 Günter Woigk"
#define applEmail       "vipsi@little-bat.de"
#define buildDate       __DATE__ " " __TIME__
#define buildUser       "Kio"
#define buildTarget     _PLATFORM
#define applWebsite     "https://k1.spdns.de/Develop/Projects/vipsi/"


//	settings:
//
//#define INCLUDE_STRING_TEST_SUITE	class String
//#define INCLUDE_VAR_TEST_SUITE	class Var
//#define VAR_LONG_DOUBLE			class Var
//#define TABWIDTH					tokenize() -> keep track of real col for error messages et.al.
//#define OPCODE_PROFILING

#define TABWIDTH	4

#ifdef DEBUG
#define INCLUDE_STRING_TEST_SUITE
//#define INCLUDE_VAR_TEST_SUITE
#define OPCODE_PROFILING
#endif

#ifdef RELEASE
#endif















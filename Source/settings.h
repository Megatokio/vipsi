
/* ===============================================
			compile-time options
=============================================== */


#define CUSTOM_ERRORS "custom_errors.h"
#define APPL_NAME "vipsi"
#define LOGFILE 1


#if 0
------- switches: --------------------------------

#define	INCLUDE_STRING_TEST_SUITE	class String
#define	INCLUDE_VAR_TEST_SUITE		class Var

#define VAR_LONG_DOUBLE				class Var
#define TABWIDTH					tokenize() -> keep track of real col for error messages et.al.

#define	OPCODE_PROFILING

--------------------------------------------------
#endif


#define TABWIDTH	4

#ifdef DEBUG
//#  define	USE_KIOS_MALLOC
//#  define	MALLOC_STATISTICS
//#  define	INCLUDE_STRING_TEST_SUITE
//#  define	INCLUDE_VAR_TEST_SUITE
#  define	OPCODE_PROFILING
#endif

#ifdef DEVELOP
//#  define	INCLUDE_STRING_TEST_SUITE
//#  define	INCLUDE_VAR_TEST_SUITE
#  define	OPCODE_PROFILING
#endif

#ifdef RELEASE
#endif















#!/usr/local/bin/vipsi

/* ---- vipsi test script ----------------------------

        test a lot of vipsi functionality
        add more tests as you like ...
        demonstrates syntax and capabilities as well

        kio 2001-08-24
        kio 2002-03-09
        kio 2002-03-31
		kio 2003-08-14
		kio 2004-06-21 split into include files
*/


put nl,nl,"test script for vipsi",nl

if count globals
    log nl, count globals, " global variable", "s"[count globals>1],
		" already defined:", nl
    {
        var i=1;
        do
            while i<count globals // globals[count globals] = local { … }
            if name globals[i]!="" log name globals[i], " = "
            else log "unnamed variable: " then
            log globals[i], nl
            i=i+1
        loop
    }
then

del *
var a,b,c,d
var err = 0;
var errors = 0
var e,f,g,h,n
err=0

// ----------------------------------------------------------
put nl,"basic test requirements "
put ".", 47+2; 			// just execute it
put ".", "123";

put "." n=1 if 0                              log "test ",n," failed" end then
put "." n=2 a=4 if a!=4                       log "test ",n," failed" end then
put "." n=3 a++ if a!=5                       log "test ",n," failed" end then
put "." n=4 a-- if a!=4                       log "test ",n," failed" end then
put "." n=5 a=1 b=2 a=b   if a!=2||b!=2       log "test ",n," failed" end then
put "." n=6 a=3 var z=a   if a!=3||z!=3       log "test ",n," failed" end then

put "." n=7 if 0 log "test ",n," failed" end else ; then
put "." n=8 if 0 log "test ",n," failed" end        then
put "." n=9 if 1 ; else log "test ",n," failed" end then

put "." n=11 if err!=0 log "test ",n," failed" end then
put "." n=12 err++; if err!=1 log "test ",n," failed" end then
put "." n=13 err--; if err!=0 log "test ",n," failed" end then
put "." n=14 proc trap(f) { if f err++ then }
put "." n=15 trap(0) if err!=0 log "test ",n," failed" end then
put "." n=16 trap(1) if err!=1 log "test ",n," failed" end then err--


/*	trap()
*/
put "." n=17 proc trap(f) { put "."; if f err++ log "#",n," failed" then }
		n=18 trap(0) if err!=0 end then


/*	assert()
*/
put "." n=19 proc assert(f) { put ".";  if !f;  log "#",n," failed"; err++;  then; }
		n=20 assert(1) if err end then

/*	NaN
*/
put "." n=21 var nan = 0/0
		n=22 proc IsNan(n){return n!=n}

/*	inf
*/
put "." n=24 var inf = 1e99999 /* > 18 bit exponent */
		n=25 proc IsInf(n){return n+1==n-1 }

put " passed",nl


put nl,"inf, nan, +0, -0 "
try
	n=1 assert(IsNan(nan))
	n=2 assert(IsInf(inf))
	n=3 assert(inf==inf+1) if err end then
	n=4 assert(inf==inf-1) if err end then
	n=5 assert(inf*-1==-inf)
	n=6 assert(IsNan(inf/inf))

	n=10 put"." try a=1e777777777 else log"#10 failed" err++ then
	n=11 put"." try a=-1e777777777 else log"#11 failed" err++ then
	n=12 put"." try a=1e-777777777 else log"#12 failed" err++ then
	n=13 put"." try a=-1e-777777777 else log"#13 failed" err++ then
	n=14 assert( 1e-77777777 == -1e-77777777 )		// +0 == -0
else
	log "test#",n," bummered: ", error.message
	err++
then
if !err put " passed" else errors+=err err=0 then



//cd (fullpath(".") # "00_test.vs")[to rfind(fullpath(".") # "00_test.vs","/")]
//var d = env._
//cd d[to rfind(d,"/")]

cd job.script[to rfind(job.script,"/")]		// own includes

include "each_and_every.vs"
a=0
include "operators_compare.vs"
b=0
include "operators_numeric.vs"
include "operators_text.vs"
include "operators_pruning.vs"
include "functions_text.vs"
include "functions_numeric.vs"
include "functions_list.vs"
include "names.vs"
include "instructions_branching.vs"
include "procedures.vs"
include	"variable_protection.vs"

var wdir = fullpath "."
include "include_sys_call.vs"
cd wdir
include "files_etc.vs"
cd wdir
try
	include "old_bugs.vs"
else
	log nl,«include "old_bugs.vs" bummered: »,error.message err++
then





// ----------------------------------------------------------
if errors
    put nl," ",nl
	log "--- ",errors," errors total ---"
	put nl
else
    put nl," ",nl,"+++ all tests passed +++",nl,nl
then



end














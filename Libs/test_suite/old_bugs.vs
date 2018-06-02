
// ----------------------------------------------------------

put nl,"Old bugs: "

put nl, "pruning did not skip upto '}' after '{' --> skript ended after executing '}' "
include "old_bugs_pruning_lists.vs"
n=5 assert(prune_completed)
if !err put " passed " else errors+=err err=0 then

put nl, "<list>[<var> to <value>] --> overwrote <var> with <value> "
var l={8,7,6,5,4,3,2,1}, a=5
put "." if l[3 to 4]!={6,5} log " failed "  err++ then
put "." if l[3 to a]!={6,5,4} log " failed "  err++ then
put "." if l[a to]!={4,3,2,1} log " failed "  err++ then
put "." l=l[a to 6] if a!=5 log "failed"  err++ then
put "." if l!={4,3} log " failed "  err++ then
if !err put " passed " else errors+=err err=0 then


put nl, "exists file <path/file> --> set error if <path> did not exist "
var fname = "/tmp/vipsi-temp-zzz-348576387"
put "." file fname
put "." if !exists file fname log " failed "  err++ then
put "." del file fname
put "." if exists file fname log " failed "  err++ then
try
  put "." if exists file (fname#"/543856") log " failed " err++ then
else
  log "[failed]" err++
then
if !err put " passed" else errors+=err err=0 then


put nl, "pruning could not skip over prefix operators and hogged cpu "
put "." if 1?0:-1 log " failed "  err++ then
put "." if 0?+1:0 log " failed "  err++ then
//var a=0  --> ++a / --a is skipped and a is not accessed
put "." if 1?0:--a log " failed "  err++ then
put "." if 0?++a:0 log " failed "  err++ then
if !err put " passed" else errors+=err err=0 then


put nl, «operator @: @"globals" and @"locals" were not found »
put "." assert(exists @"globals")
put "." var ödäuszg=345 assert( exists( @"globals".@"ödäuszg" ) )
put "." try assert( @"globals".@"ödäuszg" == 345 ) then
put "." proc TestAt(*){ try return @"locals" else log"failed" err++ then return @"locals" }
		assert( TestAt()=={} )
		assert( TestAt(1,3,5)=={1,3,5} )
if !err put " passed" else errors+=err err=0 then


put nl, «env[i] crashed in 0.8.8pre »
put "." var z = env[1]
put "." var z = name env[1]
		assert( z == name env[1] )
if !err put " passed" else errors+=err err=0 then


put nl, «new file "zz" and new link "zz" failed »
if CreateTempDir()==0
	try
		put "." cd "vipsi_"
		put "." new file "ff" = "4711"
		put "." new link "ll" = "ff"
		assert(file "ll" == "4711")
		put "." del file "ll"
		put "." del link "ll"
		assert(dir":"=={})
	else
		log "failed"
	then
then
if !err put " passed" else errors+=err err=0 then

put nl, «file "zz" truncated existing files»
if CreateTempDir()==0
	try
		n=1 put "." cd "vipsi_"
		n=2 put "." file "f0"
		n=3 assert(file("f0")=="")
		n=4 put "." file "ff" = "4711"
		n=5 assert(file("ff")=="4711")
		n=6 put "." file "ff"
		n=7 assert(file("ff")=="4711")
		n=8 put "." file "ff" #= "affe"
		n=9 assert(file("ff")=="4711affe")
	else
		log "[#",n," bummered: ",error.message,"]" err++
	then
then
if !err put " passed" else errors+=err err=0 then


put nl,«exists env.XYZ didn't work»
put "." var e = env
if exists e.user assert(exists env.user ) assert(exists(env.user)) else log "." then
if !exists e.GHJK assert(!exists(env.GHJK)) assert(!exists env.GHJK ) else log "." then
var n = name e[1]
assert( exists(env.@n) )
assert( exists env.@n )
assert( exists(env.@(name e[1])) )
if !err put " passed" else errors+=err err=0 then


put nl,«evaluation of '\\'' didn't work»
try
	n=1 put"." if eval"'\"'" != 34 log " t1 failed" err++ then
	n=2 put"." if eval"'\\''" != 39 log " t2 failed" err++ then
else
	log "[#",n," bummered: ",error.message,"]" err++
then

put nl,«evaluation of "\\"" didn't work»
try
	n=1 put"." if eval«"\'"» != charstr(39) log " t1 failed" err++ then
	n=2 put"." if eval«"\\""» != charstr(34) log " t2 failed" err++ then
else
	log "[#",n," bummered: ",error.message,"]" err++
then




if !err put nl,"Old bugs passed" else errors+=err err=0 then



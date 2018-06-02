

// ----------------------------------------------------------
put nl,"branching:"

put " t1" if 0                         log " failed"   err++ then
put " t2" if 1 else                    log " failed"   err++ then
put " t3" if 1 a=1 b=2 else            log " failed"   err++ then
put " t4" if 1 if 0 else then else     log " failed"   err++ then
put " t5" if 1 if 1 else then else     log " failed"   err++ then

put " t6" err++ if 0 if 0 log" failed(1)" else log" failed(2)" then
				else err-- then
put "."   err++ if 0 if 1 log" failed(1)" else log" failed(2)" then
				else err-- then

put " t7" a=10 do a-=1 while a loop                 trap(a)
put "."   a=10 do a-=1 until !a loop                trap(a)
put "."   a=10 do a-=1 if a next then exit loop     trap(a)
put "."   a=10 do a-=1 if !a exit then loop         trap(a)
put "."   a=10 if 1 do a-=1 while a loop then       trap(a)
put "."   a=10 if 0 else do a-=1 while a loop then  trap(a)




put " t1"
var i=0
var j={} j[6]=0 j[6]+=1 if j[6]!=1 log " failed" err++ then
put " t2"
do
    until ++i==9
    if i==2 j[1]=1 next then
    if i==2 j[2]=2 next then
    if i==9 j[3]=3 next then
    if i==5
        if 0 j[4]=4 exit else j[5]=5 exit then
    else
        j[6]+=1
    then
loop
if string(j) != "{ 1, 0, 0, 0, 5, 4 }"
	log " failed", " string(j)=",string(j) err++
then

put " t3"
var i=0
var j={} j[4]=0
var k=0
do
    while ++i!=9
    if i==2 j[1]=1 next then
    if i==2 j[2]=2 next then
    if i==9 j[3]=3 next
    else
        if i!=5
            if ++k==10 log "abort " end then
            j[4]+=1
            next
        else
            if 1 j[5]=5 exit else j[6]=6 exit then
            j[7]=7
        then
    then
    j[8]=8
loop
if string(j) != "{ 1, 0, 0, 3, 5 }" log " failed" err++ then


if !err put " passed" else errors+=err err=0 then

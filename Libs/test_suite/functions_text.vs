
// ----------------------------------------------------------
put nl,"text functions: "
put"." if string(0)!="0"                   log " t1 failed" err++ then
put"." if string(0.001)!="0.001"           log " t2 failed" err++ then
put"." if string(100.11+0.001)!="100.111"  log " t3 failed" err++ then
put"." if find("hasta la vista","la")!=7   log " t4 failed" err++ then
put"." if find("hasta la vista","zz")!=0   log " t5 failed" err++ then
put"." if find("hasta la vista","ha")!=1   log " t6 failed" err++ then
put"." if find("hasta la vista","")!=1     log " t7 failed" err++ then
put"." if find("la","hasta la vista")!=0   log " t8 failed" err++ then
put"." if find("xyz","xyz")!=1             log " t9 failed" err++ then
put"." if find("xyz","xy")!=1              log " t10 failed" err++ then
put"." if find("xyz","yz")!=2              log " t11 failed" err++ then
put"." if find("","")!=1                   log " t12 failed" err++ then
put"." if charstr(65)!="A"                 log " t13 failed" err++ then
put"." b="B"
	   if charstr(65)#b#charstr(67)!="ABC" log " t14 failed" err++ then
	   if b!="B" 						   log " t14.1 failed" err++ then
put"." if hexstr(-1)!="F"                  log " t15 failed" err++ then

if !err put " passed" else errors+=err err=0 then



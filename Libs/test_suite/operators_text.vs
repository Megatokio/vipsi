
// ----------------------------------------------------------
put nl,«text: »
put"."    if "abcde"#"fghi"!="abcdefghi" log " t1 failed" err++ then
put"."    if "abcde"[2]!="b"          log " t2 failed" err++ then
put"."    if "abcde"[to 2]!="ab"      log " t3 failed" err++ then
put"."    if "abcde"[4 to]!="de"      log " t4 failed" err++ then
put"."    if "abcde"[2 to 4]!="bcd"   log " t5 failed" err++ then
put"."    if "abcde"[-2 to 2]!="ab"   log " t6 failed" err++ then
put"."    if "abcde"[-2 to]!="abcde"  log " t7 failed" err++ then
put"."    if "abcde"[to 99]!="abcde"  log " t8 failed" err++ then
put"."    if "abcde"[to -5]!=""       log " t9 failed" err++ then

put"."    if "abcde"<"abcde"  log " t10 failed" err++ then
put"."    if "abcde">"abcde"  log " t11 failed" err++ then
put"."    if "abcde"!="abcde" log " t12 failed" err++ then
put"."    if "abcde"<"abcd"   log " t13 failed" err++ then
put"."    if "abcde"<"abcdE"  log " t14 failed" err++ then
put"."    if upperstr"AnToN MüLLer öäüßÖÄÜ"!="ANTON MÜLLER ÖÄÜßÖÄÜ"
			 log " t15 failed" err++ then
put"."    if lowerstr"AnToN MüLLer öäüßÖÄÜ"!="anton müller öäüßöäü"
			 log " t16 failed" err++ then

if !err put " passed" else errors+=err err=0 then




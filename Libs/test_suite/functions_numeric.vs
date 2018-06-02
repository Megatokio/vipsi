
// ----------------------------------------------------------
put nl,"numeric functions: "

put"t1"   a=1000 do while--a b=random 10000 c=random b until c<0||c>=b loop
put"."    if a log " t1 failed" err++ then
put"."    if round 7.1!=7     log " t19 failed" err++ then
put"."    if round 8.5!=9     log " t20 failed" err++ then
put"."    if round -8.5!=-8   log " t21 failed" err++ then
put"."    if round -8.4!=-8   log " t22 failed" err++ then
put"t2"   if sign 3434.1!=1   log " t23 failed" err++ then
put"."    if sign 0 != 0      log " t24 failed" err++ then
put"."    if sign -0.001!=-1  log " t25 failed" err++ then
put"t3"   if abs +124.4!=124.4 log " t26 failed" err++ then
put"."    if abs 0.0!=0       log " t27 failed" err++ then
put"."    if abs -17.3!=17.3  log " t28 failed" err++ then
put"t4"   if int 99.9!=99     log " t29 failed" err++ then
put"."    if int -99.9!=-99   log " t30 failed" err++ then
put"."    if round(1000*fract 99.9)!=900 log " t31 failed" err++ then
put"."    if round(1000*fract -99.9)!=-900 log " t32 failed" err++ then

put"t5"   if min(2,5)!=2            log " t33 failed" err++ then
put"."    if min(2,5,-55,0,17)!=-55 log " t34 failed" err++ then
put"."    if min(5)!=5              log " t35 failed" err++ then
put"."    a=44 b=33 c=66 min(a,b,c)=99 if b!=99 log " t36 failed" err++ then
put"."    if max(2,5)!=5            log " t37 failed" err++ then
put"."    if max(2,5,-55,17,0)!=17  log " t38 failed" err++ then
put"."    if max(5)!=5              log " t39 failed" err++ then
put"t6"   a=44 b=33 c=66 max(a,b,c)=99 if c!=99 log " t40 failed" err++ then
put"."    a=44 b=55 c=66 if max(a,b,c)!=66 log " t41 failed" err++ then
put"."    if (a!=44) log " t42 failed" err++ then
put"."    if (b!=55) log " t43 failed" err++ then
put"."    if (c!=66) log " t44 failed" err++ then
put":"    a=44 b=55 c=66 if max(c,b,a)!=66 log " t45 failed" err++ then
put"."    if (a!=44) log " t46 failed" err++ then
put"."    if (b!=55) log " t47 failed" err++ then
put"."    if (c!=66) log " t48 failed" err++ then
put":"    d=99 d=max(c,b,a) if d!=66 log " t49 failed" err++ then
put"."    if (a!=44) log " t50 failed" err++ then
put"."    if (b!=55) log " t51 failed" err++ then
put"."    if (c!=66) log " t52 failed" err++ then
put"."    if (d!=66) log " t53 failed" err++ then

put" t11" //try trap( stdin!=0  ) else log " t11.1 failed" err++ then		// note: stdin/out/err sind jetzt vom Datentyp "Stream",  
put"."    //try trap( stdout!=1 ) else log " t11.2 failed" err++ then		//		 und keine Zahlen mehr.
put"."    //try trap( stderr!=2 ) else log " t11.3 failed" err++ then
put"."    trap( pi!=3.141592653589793 )
put"."    trap( ec!=2.718281828459045 )

if !err put " passed" else errors+=err err=0 then




put nl,"exp, exp2, expe, exp10 "

try
	n=10.1 assert(exp(0,77.77)==0)		// 0^n = 0    für n>0
	n=10.2 assert(exp(0,77e20)==0)
	n=10.3 assert(exp(0,77e-20)==0)

	n=11 assert(IsNan(exp(0,0)))		// 0^n = nan  für n==0

	n=12.1 assert(exp(0,-77.77)==inf)	// 0^n = inf  für n<0
	n=12.2 assert(exp(0,-77e20)==inf)
	n=12.3 assert(exp(0,-77e-20)==inf)

	n=13.1 assert(exp(77e-20,0)==1)		// n^0 = 1    für n>0
	n=13.2 assert(exp(77,0)==1)
	n=13.3 assert(exp(77e20,0)==1)

	var i=0
	do									// -n^x = nan
		while ++i<=4
		a = -0.357<<i
		n=15.1 assert(IsNan(exp(a,-9.9)))
		n=15.2 assert(IsNan(exp(a,-1)))
		n=15.3 assert(IsNan(exp(a,-0.1)))
		n=15.4 assert(IsNan(exp(a,0)))
		n=15.5 assert(IsNan(exp(a,0.1)))
		n=15.6 assert(IsNan(exp(a,1)))
		n=15.7 assert(IsNan(exp(a,9.9)))
	loop

	n=18 put "." var i=100 do i++ until exp(random(2),i)==inf loop
	n=19 put "." var i=100 do i++ until exp(random(2),-i)==0 loop

	n=20 assert(exp2(0)==1)
	n=21 assert(exp2(7777777777.7)==inf)
	n=22 assert(exp2(-7777777777.7)==0)
	n=23 put "." var i=100 do i++ until exp2(i)==inf loop
	n=24 put "." var i=100 do i++ until exp2(-i)==0 loop
	n=25 assert(exp2(1)==2)

	n=30 assert(expe(0)==1)
	n=31 assert(expe(7777777777.7)==inf)
	n=32 assert(expe(-7777777777.7)==0)
	n=33 put "." var i=100 do i++ until expe(i)==inf loop
	n=34 put "." var i=100 do i++ until expe(-i)==0 loop
	n=35 assert(expe(1)==ec)

	n=40 assert(exp10(0)==1)
	n=41 assert(exp10(7777777777.7)==inf)
	n=42 assert(exp10(-7777777777.7)==0)
	n=43 put "." var i=100 do i++ until exp10(i)==inf loop
	n=44 put "." var i=100 do i++ until exp10(-i)==0 loop
	n=45 assert(abs(exp10(1)/10)>1e-8)
else
	log "test#",n," bummered: ", error.message
	err++
then
if !err put " passed" else errors+=err err=0 then




put nl,"log, log2, loge, log10 "
try
	n=1.1 assert(		 log(   0, 81) == -0 )
	n=1.2 assert( 		 log( 1/3, 81) == -4 )		// fails: ppc: rounding
	n=1.3 assert( IsNan( log(   1, 81) ) )			// fails: i386: inf			fixed.
	n=1.4 assert(		 log(   3, 81) == +4 )		// fails: ppc: rounding

	n=2.1 assert( 		 log(  -0, 81) == -0 )
	n=2.2 assert( IsNan( log(-1/3, 81) ) )
	n=2.3 assert( IsNan( log(  -1, 81) ) )
	n=2.4 assert( IsNan( log(  -3, 81) ) )

	n=3.1 assert( 		 log(   0, 1/81) == +0 )
	n=3.2 assert(		 log( 1/3, 1/81) == +4 )	// fails: ppc: rounding
	n=3.3 assert( IsNan( log(   1, 1/81) ) )		// fails: i386: -inf		fixed.
	n=3.4 assert(		 log(   3, 1/81) == -4 )	// fails: ppc: rounding

	n=4.1 assert( 		 log(  -0, 1/81) == +0 )
	n=4.2 assert( IsNan( log(-1/3, 1/81) ) )
	n=4.3 assert( IsNan( log(  -1, 1/81) ) )
	n=4.4 assert( IsNan( log(  -3, 1/81) ) )

	n=10 assert(IsNan(log(0,0)))
	n=11 assert(IsNan(log(-1e-100,0)))
	n=12 assert(IsNan(log(77,-1)))

	n=21 assert(log(0.1,0)==inf)
	n=22 assert(log(0.9,0)==inf)
	n=23 var i=0 do i+=7.13 while i<=100 assert(log(i,0)==-inf) loop


	n=31 assert(loge(0)==-inf)
	n=32 assert(log2(0)==-inf)
	n=33 assert(log10(0)==-inf)

	n=41 assert(IsNan(log2(-1)))
	n=42 assert(IsNan(loge(-1)))
	n=43 assert(IsNan(log10(-1)))
else
	log "test#",n," bummered: ", error.message
	err++
then
if !err put " passed" else errors+=err err=0 then




put nl,"sinh, cosh, tanh "
try
	n=1 put "." var i=10 do i++ until sinh(i)==inf loop
	n=2 put "." var i=10 do i++ until cosh(i)==inf loop
	n=3 assert(tanh(i)==1) assert(tanh(0)==0) assert(tanh(-i)==-1)
	n=4 put "." var i=-10 do i-- until sinh(i)==-inf loop
	n=5 put "." var i=-10 do i-- until cosh(i)==inf loop
else
	log "test#",n," bummered: ", error.message
	err++
then
if !err put " passed" else errors+=err err=0 then



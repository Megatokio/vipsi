#!/usr/local/bin/vipsi


var argc; argc = count globals -1

var testloops = 1
var quiet     = 0
var testzahl  = 0
var bits_a    = 15		// a<b
var bits_b    = 25		// und a+b <= 53


var i=0
do
	while++i<=argc
	var arg = globals[i]

	if arg=="-h" || arg=="--help"
		var msg = «
Zerlege Testzahl in zwei (Prim-)faktoren
Erzeugt entweder eine Testzahl aus zwei generierten Testzahlen
oder zerlegt die übergebene Testzahl in 2 Faktoren.
	-q			--quiet		Ohne Zwischenmeldungen
	-n <zahl>	--loops		Anzahl Testläufe (dflt: 1)
	-a <bits>	--bits_a	Größenordnung für erste Primzahl:  2..51
	-b <bits>	--bits_b	Größenordnung für zweite Primzahl: 2..51
	<zahl>					Eigene statt generierter Testzahl
»		convert msg from tab4
		put msg
		end 0
	then

	if arg=="-q" || arg=="--quiet"	quiet=1 next then

	if arg=="-n" || arg=="--tests" || arg=="--loops"
		if ++i>argc next then
		testloops=globals[i]
		if !isnumber testloops
			try testloops=eval testloops else testloops=1 then
		then
		next
	then

	if arg=="-a" || arg=="--bits_a"
		if ++i>argc next then
		arg=globals[i]
		if !isnumber arg
			try arg=eval arg bits_a=arg then
		else
			bits_a=arg
		then
		if bits_a<2||bits_a>51 bits_a=40-bits_b then
		if bits_a>bits_b { var z=bits_a bits_a=bits_b bits_b=z } then
		next
	then

	if arg=="-b" || arg=="--bits_b"
		if ++i>argc next then
		arg=globals[i]
		if !isnumber arg
			try arg=eval arg bits_b=arg then
		else
			bits_b=arg
		then
		if bits_b<2||bits_b>51 bits_b=40-bits_a then
		if bits_a>bits_b { var z=bits_a bits_a=bits_b bits_b=z } then
		next
	then

	if testloops!=1 || testzahl!=0
		put nl,"Unerwartete Testzahl wird ignoriert."
		next
	then

	try
		testzahl = arg
		if !isnumber testzahl testzahl = eval testzahl then
		var z = testloops
		testloops = 0

		if testzahl<3 || fract(testzahl>>1)!=0.5 || testzahl+1==testzahl || testzahl-1==testzahl
			put nl,"Testzahl außerhalb der Rechengenauigkeit.",
				nl,"Starte ersatzweise Test.", nl
			testloops = z
			testzahl = 0
		then
	else
		put nl,"Parameter war nicht numerisch.",
			nl,"Starte ersatzweise Test.", nl
		testloops = z
		testzahl = 0
	then
loop




/* ==== TOOLS ===================================================
*/
proc NewPrime(bits)
{
	do
		var n = (0.5+random(0.5)) << bits
			n = int(n>>1) << 1 +1
		var e = int sqrt n
		var q = 1
		do
			if (q+=2)>e return n then
			until fract(n/q)==0
		loop
	loop
}


proc PrüfeRechengenauigkeit( prime_a, prime_b )
{
	var n = prime_a * prime_b
	if n / prime_a != prime_b	return "failed (1)"  then
	if n / prime_b != prime_a	return "failed (1a)" then
	if n-1+1 != n				return "failed (2)"  then
	if n-1   == n				return "failed (2a)" then
	if n+1   == n				return "failed (2b)" then

	var a0 = int(sqrt(n))		var da = 2 	a0 -= fract(a0>>1)==0
	var b0 = int(n/a0)			var db = 2	b0 -= fract(b0>>1)==0
	var r0 = n - a0 * b0
	var z0 = 2*(b0-a0+da)		var dz = 2*(da+db)

	if a0*b0+r0 != n 			return "failed (3)" then

	return "ok"
}


proc PrüfeAlgorithmus ( prime_a, prime_b )
{
	var n = prime_a * prime_b

	var a0 = int(sqrt(n))	var da = 2 	a0 -= fract(a0>>1)==0
	var b0 = int(n/a0)		var db = 2	b0 -= fract(b0>>1)==0
	var r0 = n - a0 * b0
	var z0 = 2*(b0-a0+da)	var dz = 2*(da+db)

	var a = a0-da
	var b = b0+db
	var r = r0+z0
	var z = z0+dz

	if a*b+r != n	return "failed (4)" then

	a-=da	b+=db	r+=z	z+=dz

	if a*b+r != n	return "failed (5)" then

	a-=da	b+=db	r+=z	z+=dz

	if a*b+r != n	return "failed (6)" then

	return "ok"
}


/* ==========================================================
	a_prime * b_prime = n

	a * b + r = n			a < b, a--, b++
	a' = a-da;		da = 2
	b' = b+db;		db = 2; wird erhöht, wenn dr>=2*a

	r  = n -a*b
	r' = n -a'*b'	= n-(a-da)*(b+db)
	   				= n -a*b -a*db +b*da +da*db
				 	= r      -a*db +b*da +da*db
	dr = r' - r		= r      -a*db +b*da +da*db -r
					=        -a*db +b*da +da*db

	r" = n -a"*b" 	= n-(a-2da)*(b+2db)
					= n -a*b -2*a*db +2*b*da +4*da*db
					= r' -a*db +b*da +3*da*db
	dr' = r" - r'	= r' -a*db +b*da +3*da*db -r'
					=    -a*db +b*da +3*da*db
					= dr             +2*da*db

	ddr = dr' - dr	= dr             +2*da*db - dr
					= 2*da*db

	Zur Steigung db/da:
	b     = f(a)  = n/a  = n * a⁻¹
	db/da = f'(a) = n/a² = n * a⁻²
	db muss auf 4 erhöht werden, bei:
		f'(a) = n/a² = 4/2
		<=>		n = 2*a²
				a = sqrt(n/2)
				a = sqrt(n) / sqrt(2)
				wobei sqrt(n) ja der startwert für a ist.
	allgemein: db muss immer auf das nächste db erhöht werden bei
				a = sqrt(n) / sqrt(db/da)
	bei a = sqrt(n)/2 wird db/da also gerade auf 8/2 erhöht.
				Dann ist der halbe Wertebereich für a abgearbeitet!

	Wann muss db zum ersten Mal um mehr als 2 (also 4) erhöht werden?
				wenn dr' = dr+ddr >= 4*a
	Wann muss b zum ersten Mal um mehr als 2 (also 4) zusätzlich gesteppt werden?
				wenn r' = r+dr >= 4*a

*/
proc FindPrimes(n)
{
	var a = int(sqrt(n))	a -= fract(a>>1)==0
	var b = int(n/a)		b -= fract(b>>1)==0
	var r = n - a * b

	var da  = 2
	var db  = 2
	var dr  = -a*db +b*da +da*db
	var ddr = 2*da*db

	var f
	if r==0 return locals then

	do
	//	while a*b+r == n	// check

		if r>=2*a
			if dr>=2*a
				f = int(dr/(2*a))*2
			//	if (f>2) log " dr/2a=",f," " then
				db  += f
				dr  -= (a-da)*f		//if(dr!=-a*db +b*da +da*db)  log" (1)" exit then
				ddr += f*2*da		//if(ddr!=2*da*db) log" (2)" exit then
			//	put nl, locals, "  db incremented"
				if r>=4*a
					f = int(r/(2*a))*2
					b += f
					r -= f*a		//if(r!=n-a*b) log"(3)" end then
					dr += f*2		//if(dr!=-a*db +b*da +da*db) log"(4)" end then
					until r==0		// found  ((if n was prime then this triggers when a==1))
					next
				then
			then
			f = int(r/(2*a))*2
			if (f>2) log " r/2a=",f," " then
			b += f
			r -= f*a				//if(r!=n-a*b) log"(5)" end then
			dr += f*2				//if(dr!=-a*db +b*da +da*db) log"(6)" end then
			until r==0				// found  ((if n was prime then this triggers when a==1))
		//	until a<2				// no prime factor found!  ((no need: a==1 is always found))
		//	while a*b+r == n		// check
		then
		a-=da
		b+=db
		r+=dr
		dr+=ddr
	loop

	return locals
}


proc ZerlegeZahl ( n )
{
	if !quiet
		put nl, "---- start ----"
	then

	var zeit = now
	var r = FindPrimes(n)
	zeit = now-zeit

	if quiet
		put nl, n, " = ", r.a, " * ", r.b, " + ", r.r
	else
		put nl, n, " = ", r.a, " * ", r.b, " + ", r.r
		put nl,"Zeit     = ", int(zeit*1000)/1000
		put nl,"Tests/s  = ", int((int(sqrt(n))-r.a+2)/2/zeit+0.5)
		put nl,"--- finished ----",nl
	then
}



proc RunTest ( prime_a, prime_b )
{
	if !quiet
		put nl, "---- start ----"
	then

	var n = prime_a*prime_b
	var zeit = now
	var r = FindPrimes(n)
	zeit = now-zeit

	if quiet
		put nl, n, " = ", r.a, " * ", r.b, " + ", r.r
	else
		put nl,"faktor a = ", r.a,		{" (ok)"," ("#prime_a  #")"}[1+(r.a!=prime_a)]
		put nl,"faktor b = ", r.b,		{" (ok)"," ("#prime_b  #")"}[1+(r.b!=prime_b)]
		put nl,"rest   r = ", r.r,		{" (ok)"," ("#n-r.a*r.b#")"}[1+(r.r!=n-r.a*r.b)]
		put nl,"produkt  = ", r.a*r.b,	{" (ok)"," ("#n        #")"}[1+(r.a*r.b!=n)]

		put nl,"Zeit     = ", int(zeit*1000)/1000
		put nl,"Tests/s  = ", int((int(sqrt(n))-r.a+2)/2/zeit+0.5)

		put nl,"--- finished ----",nl
	then
}











if !quiet put nl, "Primzahlzerlegung" then


if testloops==0
	ZerlegeZahl(testzahl);
	end 0
then

do
	while testloops--

	do	// erzeuge zwei Primzahlen
		var prime_a = NewPrime( bits_a )
		var prime_b = NewPrime( bits_b )
		until prime_a < prime_b
	loop

	if !quiet
		var n = prime_a * prime_b
		put nl, "Prime    a = ",prime_a, " (", int(log2(prime_a+1))+1, " Bit)"
		put nl, "Prime    b = ",prime_b, " (", int(log2(prime_b+1))+1, " Bit)"
		put nl, "Produkt  n = ",n,       " (", int(log2(n+1))+1, " Bit)"
		put nl, "Wurzel   n = ",sqrt n
		put nl, "Faktor b/a = ",int(prime_b/prime_a*100)/100

		put nl, "Prüfe Rechengenauigkeit ... "
		var t1 = PrüfeRechengenauigkeit(prime_a,prime_b)
		put t1
		put nl, "Prüfe Algorithmus ... "
		var t2 = PrüfeAlgorithmus(prime_a,prime_b)
		put t2
		if t1#t2 != "okok" end 1 then
	then

	RunTest(prime_a,prime_b);
loop




end 




















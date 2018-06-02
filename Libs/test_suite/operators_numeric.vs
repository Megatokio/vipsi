

// ----------------------------------------------------------
put nl,"numerics:"
put" t1" a=4 trap( a!=4.0 )
put" t2" trap( a+4!=8 )
put"."   trap( 17%5!=2 )
put"."   trap( -17%5!=-2 )
put"."   trap( 17%2.5!=2 )
put"."   trap( -17%2.5!=-2 )
put"."   trap( 17%-2.5!=2 )
put"."   trap( -17%-2.5!=-2 )
put" t3" trap( 4-3.5!=0.5 )
put" t4" trap( -a!=+3-7.0 )
put"."   trap( -0!=0 )
put"."   trap( !0!=1 || !55!=0 )
put"."   trap( ~0!=-1 )
put" t5" trap( +a!=4 )
put"."   trap( a*3.5!=14 )
put"."   trap( a/0.5!=+0.0+8 )
put"."   trap( 123.456!=((((123.0))+(0.456))) )

put" t6" trap( (3+4*50)!=203 )
		 trap( (3-4/5)!=2.2 )
		 trap( (3*(4+5))!=27 )
		 trap( ((3+4)*5)!=35 )
		 trap( (3+-4)!=-1 )
		 trap( (15&26!=10)!=0 )

put" t7" a = 123.456+385.12*46.1/46/(1.02*(0.03+164.44))
		 b = 1.02*0.03+164.44*1.02		trap(b!=167.7594);
		 c = 385.12*46.1				trap(c!=17754.032)
		 d = c/23/2.000					trap( a!=d/b+123+0.456 )

put" t8" a=44 trap( ++a!=45 || ++a!=46 )
put"."   a=44 trap( --a!=43 || --a!=42 )
put"."   a=44 trap( a++!=44 || a++!=45 )
put"."   a=44 trap( a--!=44 || a--!=43 )

put" t10" a=123; a-=20+3;       trap( a-100 )
put"."    a=123; a+=-46/2;      trap( a-100 )
put"."    a=123; a*=3+2*10;     trap( a!=2829 )
put"."    a=1604.928 a/=123.456 trap( a!=13 )

put " t12" a=3 a=a+a+a trap( a!=9 )
put "."    trap( 33<<2 != 132 )
put "."    trap( 132>>2 != 33 )
put "."    trap( $1234!=4660 )
put "."    trap( $9Affe54f!=2600461647 )

if !err put " passed" else errors+=err err=0 then


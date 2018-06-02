



put nl,"each and every:"

var token
proc chk(a) { put "." if a<0.999999 || a>1.000001 log " ",token," failed " err++ then }

token="tNUM0"	chk( 0 == 2-2 )
token="tNUM1"	chk( 1 == 3-2 )
token="tSTR"	chk( "abc" == "a"#"b"#"c" )

token="tNUM"
	chk( 32.5 == 30+2.5 )
	chk( %100000.1 == 30+2.5 )
	chk( $20.8 == 30+2.5 )
	chk( -32 == 32 * -1 )
	chk( -$20.8 == -32.5 )
	chk( -'AA' == -257*charcode "A" )

token="tPOSTINCR"
	var a = 5
	chk( a++ == 5 )
	chk( a==6 ) a++
	chk( a==7 )

token="tPOSTDECR"
	var a = 5
	chk( a-- == 5 )
	chk( a==4 ) a--
	chk( a==3 )

token="tPREINCR"
	var a = 5
	chk( ++a == 6 )
	chk( a==6 )

token="tPREDECR"
	var a = 5
	chk( --a == 4 )
	chk( a==4 )


put"\nassignment:"

token="="
	var a = 66  	chk( a==66 )
		a = -7  	chk( a==-7 )
		a = "c" 	chk( a=="c" )
		a = {1,0,2}	chk( a=={1,0,2} )
		a = 66-7	chk( a==59 )
	var a = 6-7		chk( a==-1 )

token="+="
	var a=5, b=10
		a += 6		chk( a==11 )
		a += b		chk( a==21 )	chk( b==10 )
		a += 6+7	chk( a==34 )
		a += {1,2}	chk( a=={35,36} )

token="-="
	var a=34, b=10
		a -= 7		chk( a==27 )
		a -= 2*4	chk( a==19 )
		a -= b		chk( a==9 )	chk( b==10 )
		a -= {1,2}	chk( a=={8,7} )

token="*="
	var a=5, b=10
		a *= 3		chk( a==15 )
		a *= b		chk( a==150 )	chk( b==10 )
		a *= {3,4}	chk( a=={450,600} )

token="/="
	var a=600, b=10
		a /= 3		chk( a==200 )
		a /= b		chk( a==20 )	chk( b==10 )
		a /= {5,4}	chk( a=={4,5} )
		a /= 0.1	chk(a=={40,50})

token="%="
	var a=62, b=10
	a=62	a %= 3		chk( a==2 )
	a=107	a %= b		chk( a==7 )	chk( b==10 )
	a=9		a %= {5,4}	chk( a=={4,1} )

token="#="
	var a="A", b="BC"
	a="A"	a #= "3"		chk( a=="A3" )
	a="A"	a #= b			chk( a=="ABC" )	chk( b=="BC" )
	a="A"	a #= {"5","6"}	chk( a=={"A5","A6"} )
	a={"5","6"} a#="A"		chk( a=={"5A","6A"} )

token=">>="
	var a=600, b=3
	a=600	a >>= 3		chk( a==75 )
	a=600	a >>= b		chk( a==75 )	chk( b==3 )
	a=600	a >>= {5,4}	chk( a=={600/32,600/16} )

token="<<="
	var a=6, b=3
	a=6	a <<= 3		chk( a==48 )
	a=6	a <<= b		chk( a==48 )	chk( b==3 )
	a=6	a <<= {5,4}	chk( a=={192,96} )


token="^="
	var a=6, b=3
	a=6	a ^= 3		chk( a==5 )
	a=6	a ^= b		chk( a==5 )	chk( b==3 )

token="|="
	var a=6, b=3
	a=6	a |= 3		chk( a==7 )
	a=6	a |= b		chk( a==7 )	chk( b==3 )
	a=6	a |= {5,4}	chk( a=={7,6} )

token="&="
	var a=6, b=3
	a=6	a &= 3		chk( a==2 )
	a=6	a &= b		chk( a==2 )	chk( b==3 )
	a=6	a &= {5,4}	chk( a=={4,4} )

token="##="
	var a="A", b="BC"
	a="A"	a ##= "3"		chk( a=={"A","3"} )
	a="A"	a ##= b			chk( a=={"A","BC"} )	chk( b=="BC" )
	a="A"	a ##= {"5","6"}	chk( a=={"A","5","6"} )
	a={"5","6"} a##="A"		chk( a=={"5","6","A"} )
	a={"A","5"}	a ##= {"3"}	chk( a=={"A","5","3"} )
	b={"b","c"} a ##= b		chk( a=={"A","5","3","b","c"} )	chk( b=={"b","c"} )

put "\ndiadic:"

token="+"
	chk(+2+14==16)
	a=10 b=22 chk(2+a==12)
	chk(a+2==12)
	chk(+a+b==32) chk(a==10) chk(b==22)
	chk(2+{3,4}=={5,6})

token="-"
	chk(-12-4==-16)
	a=10 b=22 chk(2-a==-8)
	chk(a-2==8)
	chk(a-b==-12) chk(a==10) chk(b==22)
	chk(8-{3,4}=={5,4})

token="*"
	chk(2*5==10)
	a=10 b=22 chk(2*a==20)
	chk(a*2==20)
	chk(a*b==220) chk(a==10) chk(b==22)
	chk(2*{3,4}=={6,8})

token="/"
	chk(20/5==4)
	a=200 b=10 chk(200/b==20)
	chk(a/2==100)
	chk(a/b==20) chk(a==200) chk(b==10)
	chk(6/{3,2}=={2,3})

token="%"
	chk(19%5==4)
	a=198 b=10 chk(198%b==8)
	chk(a%10==8)
	chk(a%b==8) chk(a==198) chk(b==10)
	chk(7%{3,4}=={1,3})

token="|"
	chk(20|5==21)
	a=20 b=5 chk(20|b==21)
	chk(a|5==21)
	chk(a|b==21) chk(a==20) chk(b==5)
	chk(2|{3,4}=={3,6})
	a=6		chk( a|-1==-1 )
			chk( a|-7==-1 )
			chk( a|-11==-9 )
			chk( -11|a==-9 )
	a=~6	chk( a|~12==~4 )

token="&"
	chk(20&5==4)
	a=20 b=5 chk(20&b==4)
	chk(a&5==4)
	chk(a&b==4) chk(a==20) chk(b==5)
	chk(2&{3,4}=={2,0})
	a=6		chk( a&-1==6 )
			chk( a&-3==4 )
			chk( a&-5==2 )
			chk( -3&a==4 )
	a=~4	chk( a&~3==~7 )

token="^"
	chk(20^5==17)
	a=20 b=5 chk(20^b==17)
	chk(a^5==17)
	chk(a^b==17) chk(a==20) chk(b==5)
	chk(2^{3,4}=={1,6})
	a=6		chk( a^-1==~6 )
			chk( a^-7==-1 )
			chk( a^-11==-13 )
			chk( -11^a==-13 )
	a=~6	chk( a^~12==10 )

token="<<"
	chk(20<<3==160)
	a=20 b=3 chk(0.5<<b==4)
	chk(a<<3==160)
	chk(a<<b==160) chk(a==20) chk(b==3)
	chk(2<<{3,4}=={16,32})
	chk(-6>>1==-3)

token=">>"
	chk(20>>3==2.5)
	a=20 b=3 chk(20>>b==2.5)
	chk(a>>3==2.5)
	chk(a>>b==2.5) chk(a==20) chk(b==3)
	chk(8>>{3,4}=={1,0.5})
	chk(-3<<1==-6)


put "\ncompare:"
proc nck(n){chk(!n)}

token="=="
	var a=10,b=20,c=10
	chk(3==3)
	chk(!(3==4))
	chk(a==10)
	chk(10==a)
	chk(a==c)
	chk(!(a==11))
	chk(!(11==a))
	chk(!(a==b))
	chk(a==10) chk(b==20) chk(c==10)

	var a="10",b="20",c="10"
	chk("3"=="3")
	chk(!("3"=="4"))
	chk(a=="10")
	chk("10"==a)
	chk(a==c)
	chk(!(a=="11"))
	chk(!("11"==a))
	chk(!(a==b))
	chk(a=="10") chk(b=="20") chk(c=="10")

	var a={10,1},b={10,2},c={10,1}
	chk({3}=={3})
	chk(!({3}=={4}))
	chk(a=={10,1})
	chk({10,1}==a)
	chk(a==c)
	chk(!(a=={11,1}))
	chk(!({11}==a))
	chk(!(a==b))
	chk(a=={10,1}) chk(b=={10,2}) chk(c=={10,1})

token="!="
put ":"
	var a=10,b=20,c=10
	nck(3!=3)
	chk(3!=4)
	nck(a!=10)
	nck(10!=a)
	nck(a!=c)
	chk(a!=11)
	chk(11!=a)
	chk(a!=b)
	chk(a==10) chk(b==20) chk(c==10)

	var a="10",b="20",c="10"
	nck("3"!="3")
	chk("3"!="4")
	nck(a!="10")
	nck("10"!=a)
	nck(a!=c)
	chk(a!="11")
	chk("11"!=a)
	chk(a!=b)
	chk(a=="10") chk(b=="20") chk(c=="10")

	var a={10,1},b={10,2},c={10,1}
	nck({3}!={3})
	chk({3}!={4})
	nck(a!={10,1})
	nck({10,1}!=a)
	nck(a!=c)
	chk(a!={11,1})
	chk({11}!=a)
	chk(a!=b)
	chk(a=={10,1}) chk(b=={10,2}) chk(c=={10,1})

token=">"
put ":"

	var a=10,b=20,c=10
	nck(3>3)
	nck(3>4)
	chk(4>3)
	nck(a>10)
	nck(10>a)
	nck(a>c)
	nck(a>11)
	chk(11>a)
	nck(a>b)
	chk(a==10) chk(b==20) chk(c==10)

	var a="10",b="20",c="10"
	nck("3">"3")
	nck("3">"4")
	chk("4">"3")
	nck(a>"10")
	nck("10">a)
	nck(a>c)
	nck(a>"11")
	chk("11">a)
	nck(a>b)
	chk(a=="10") chk(b=="20") chk(c=="10")

	var a={10,1},b={10,2},c={10,1}
	nck({3}>{3})
	nck({3}>{4})
	chk({4}>{3})
	nck(a>{10,1})
	nck({10,1}>a)
	nck(a>c)
	nck(a>{11,0})
	chk({11}>a)
	nck(a>b)
	chk(a=={10,1}) chk(b=={10,2}) chk(c=={10,1})

token="<"
put ":"

	var a=10,c=11
	nck(3<3)
	chk(3<4)
	nck(4<3)
	nck(a<10)
	nck(10<a)
	chk(a<c)
	chk(a==10) chk(c==11)

	var a="10", c="11"
	nck("3"<"3")
	chk("3"<"4")
	nck("4"<"3")
	nck(a<"10")
	nck("10"<a)
	chk(a<c)
	chk(a=="10") chk(c=="11")

	var a={10,1},c={10,0}
	nck({3}<{3})
	chk({3}<{4})
	nck({4}<{3})
	nck(a<{10,1})
	nck({10,1}<a)
	nck(a<c)
	chk(a=={10,1}) chk(c=={10,0})

token=">="
put ":"

	var a=10,b=20,c=10
	chk(3>=3)
	nck(3>=4)
	chk(4>=3)
	chk(a>=10)
	chk(10>=a)
	chk(a>=c)
	nck(a>=11)
	chk(11>=a)
	nck(a>=b)
	chk(a==10) chk(b==20) chk(c==10)

	var a="10",b="20",c="10"
	chk("3">="3")
	nck("3">="4")
	chk("4">="3")
	chk(a>="10")
	chk("10">=a)
	chk(a>=c)
	nck(a>="11")
	chk("11">=a)
	nck(a>=b)
	chk(a=="10") chk(b=="20") chk(c=="10")

	var a={10,1},b={10,2},c={10,1}
	chk({3}>={3})
	nck({3}>={4})
	chk({4}>={3})
	chk(a>={10,1})
	chk({10,1}>=a)
	chk(a>=c)
	nck(a>={11,0})
	chk({11}>=a)
	nck(a>=b)
	chk(a=={10,1}) chk(b=={10,2}) chk(c=={10,1})

token="<="
put ":"

	var a=10,c=11
	chk(3<=3)
	chk(3<=4)
	nck(4<=3)
	chk(a<=10)
	chk(10<=a)
	chk(a<=c)
	chk(a==10) chk(c==11)

	var a="10", c="11"
	chk("3"<="3")
	chk("3"<="4")
	nck("4"<="3")
	chk(a<="10")
	chk("10"<=a)
	chk(a<=c)
	chk(a=="10") chk(c=="11")

	var a={10,1},c={10,0}
	chk({3}<={3})
	chk({3}<={4})
	nck({4}<={3})
	chk(a<={10,1})
	chk({10,1}<=a)
	nck(a<=c)
	chk(a=={10,1}) chk(c=={10,0})


put "\nother:"

token="#"
	chk("2"#"5"=="25")
	a="10" b="22" chk("2"#a=="210")
	chk(a#"2"=="102")
	chk(a#b=="1022") chk(a=="10") chk(b=="22")
	chk("2"#{"3","4"}=={"23","24"})
	chk({"2","1"}#{"3","4"}=={"23","14"})
	chk({"2","1"}#"3"=={"23","13"})

token="##"
	chk({"2"}##{"5"}=={"2","5"})
	a={"10"} b={"22"} chk({"2"}##a=={"2","10"})
	chk(a##{"2"}=={"10","2"})
	chk(a##"2"=={"10","2"})
	chk(a##b=={"10","22"}) chk(a=={"10"}) chk(b=={"22"})
	chk("2"##{"3","4"}=={"2","3","4"})
	chk({"2","1"}##{"3","4"}=={"2","1","3","4"})
	chk({"2","1"}##"3"=={"2","1","3"})

token="-"
	var a=-10
	chk(-10==a)
	chk(-a==10)
	chk(10+-a==20)
	chk(-10 + - -a==-(20))
	chk(a==-10)

token="!"
	var a=10
	nck(!1)
	chk(!0)
	nck(!(2+2))
	chk(!(2-2))
	nck(!a)
	chk(!!a)
	chk(a==10)

token="~"
	var a=10
	chk(~5==-6)
	chk(~(2+3)==-6)
	chk(~a==-11)
	chk(~~a==a)
	chk(a==10)


put "\nfunctions:"

token="abs"
	var a=10, b=-20
	chk(abs 5 == 5)
	chk(abs(-5) == 5)
	chk(abs-5 == 5)
	chk(abs 0 == 0)
	chk(abs a == 10)
	chk(abs b == 20)
	chk(abs (a+b) == 10)
	chk(a==10&&b==-20)

token="sign"
	var a=10, b=-20
	chk(sign 5 == 1)
	chk(sign(-5) == -1)
	chk(sign-5 == -1)
	chk(sign 0 == 0)
	chk(sign a == 1)
	chk(sign b == -1)
	chk(sign (a+b) == -1)
	chk(a==10&&b==-20)

token="round"			// x.5 rounded upwards
	var a=10.3, b=10.5
	chk(round 10.3==10)
	chk(round 10.5==11)
	chk(round 10.6==11)
	chk(round -10.3==-10)
	chk(round -10.5==-10)
	chk(round -10.6==-11)
	chk(round a==10)
	chk(round(a)==10)
	chk(round(a+b)==21)
	chk(a==10.3&&b==10.5)

token="fract"
	var a=10.3, b=10.5
	chk(fract 10.3 / 0.3)
	chk(fract 10.5 / 0.5)
	chk(fract 10.6 / 0.6)
	chk(fract -10.3 / -0.3)
	chk(fract -10.5 / -0.5)
	chk(fract -10.6 / -0.6)
	chk(fract a / 0.3)
	chk(fract(a) / 0.3)
	chk(fract(a+b) / 0.8)
	chk(a==10.3&&b==10.5)

token="int"			// rounded to 0
	var a=10.3, b=10.5
	chk(int 10.3==10)
	chk(int 10.5==10)
	chk(int 10.6==10)
	chk(int -10.3==-10)
	chk(int -10.5==-10)
	chk(int -10.6==-10)
	chk(int a==10)
	chk(int(a)==10)
	chk(int(a+b)==20)
	chk(a==10.3&&b==10.5)

token="->"
	a=1 b=2 var p->a 
	chk(p==a)
	a=3 chk(p==a)
	p->b
	chk(p==2)
	del b
	chk(p==2)
	var b
	a={a=1,b=2,c=3}
	p->a
	try a.a->p chk(0) else chk(1) then
	a.b->a.a
	chk(a=={1,1,3})
	a[2]->a[3]
	chk(a=={1,3,3})
	a=0









































if !err put " passed" else errors+=err err=0 then





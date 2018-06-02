


put nl,"Operator &&= ||= "
try
	var a
	n=1	 a=0 a&&=0 assert( a==0 )
	n=2	 a=0 a&&=1 assert( a==0 )
	n=3	 a=1 a&&=0 assert( a==0 )
	n=4	 a=1 a&&=1 assert( a==1 )
	n=5	 a=1e33 a&&=1e33 assert( a==1 )
	n=6	 a=1e33 a&&=0    assert( a==0 )
	n=7	 a=0    a&&=1e33 assert( a==0 )
	n=8	 a=0    a&&=0    assert( a==0 )

	n=10 a=0 a||=0 assert( a==0 )
	n=11 a=0 a||=1 assert( a==1 )
	n=12 a=1 a||=0 assert( a==1 )
	n=13 a=1 a||=1 assert( a==1 )
	n=14 a=1e33 a||=1e33 assert( a==1 )
	n=15 a=1e33 a||=0    assert( a==1 )
	n=16 a=0    a||=1e33 assert( a==1 )
	n=17 a=0    a||=0    assert( a==0 )

	n=20 a=0 put "." try a &&= Murks*Not+Exist else log n," failed" err++ then
	n=21 a=1 put "." try a ||= Murks*Not+Exist else log n," failed" err++ then
else
	log "test#",n," bummered "
	err++
then
if !err put " passed" else put " ",err," errors!" errors+=err err=0 then



put nl,"Operator ?: "
try
	n=1  assert( ( 0 ? 0?2:3 : 0?4:5 ) == 5 )
	n=2	 assert( ( 0 ? 0?2:3 : 1?4:5 ) == 4 )
	n=3	 assert( ( 0 ? 1?2:3 : 0?4:5 ) == 5 )
	n=4	 assert( ( 0 ? 1?2:3 : 1?4:5 ) == 4 )
	n=5	 assert( ( 1 ? 0?2:3 : 0?4:5 ) == 3 )
	n=6	 assert( ( 1 ? 0?2:3 : 1?4:5 ) == 3 )
	n=7	 assert( ( 1 ? 1?2:3 : 0?4:5 ) == 2 )
	n=8	 assert( ( 1 ? 1?2:3 : 1?4:5 ) == 2 )
	n=11 assert( ( 0 ? 1 : 0 ? 0?2:3 : 0?4:5 ) == 5 )
	n=12 assert( ( 0 ? 1 : 0 ? 0?2:3 : 1?4:5 ) == 4 )
	n=13 assert( ( 0 ? 1 : 0 ? 1?2:3 : 0?4:5 ) == 5 )
	n=14 assert( ( 0 ? 1 : 0 ? 1?2:3 : 1?4:5 ) == 4 )
	n=15 assert( ( 1 ? 1 : 0 ? 0?2:3 : 0?4:5 ) == 1 )
	n=16 assert( ( 1 ? 1 : 0 ? 0?2:3 : 1?4:5 ) == 1 )
	n=17 assert( ( 1 ? 1 : 0 ? 1?2:3 : 0?4:5 ) == 1 )
	n=18 assert( ( 1 ? 1 : 0 ? 1?2:3 : 1?4:5 ) == 1 )
	n=21 assert( ( 1 ? 0 : 1 ? 0?2:3 : 0?4:5 ) == 0 )
	n=22 assert( ( 1 ? 0 : 1 ? 0?2:3 : 1?4:5 ) == 0 )
	n=23 assert( ( 1 ? 0 : 1 ? 1?2:3 : 0?4:5 ) == 0 )
	n=24 assert( ( 1 ? 0 : 1 ? 1?2:3 : 1?4:5 ) == 0 )
	n=25 assert( ( 0 ? 0 : 1 ? 0?2:3 : 0?4:5 ) == 3 )
	n=26 assert( ( 0 ? 0 : 1 ? 0?2:3 : 1?4:5 ) == 3 )
	n=27 assert( ( 0 ? 0 : 1 ? 1?2:3 : 0?4:5 ) == 2 )
	n=28 assert( ( 0 ? 0 : 1 ? 1?2:3 : 1?4:5 ) == 2 )

	n=30 assert ( (3>4?(2*3):"B")=="B" )
	n=31 assert ( (!0?"A"#"B":0)=="AB" )
	n=32 assert ( (0?5:0?6:1?7:1?8:0)==7 )
	n=33 assert ( (0?5:0?6:1?7:0?8:1)==7 )

else
	log "test#",n," bummered: ", error.message, " "
	err++
then
if !err put " passed" else put " ",err," errors!" errors+=err err=0 then



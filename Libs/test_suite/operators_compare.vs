
a=0

// ----------------------------------------------------------
put nl,"comparing:"
put " t0" trap(0)
put "."   assert(1)

put " t1" trap( 1==2 )
put "."   trap( 2!=2 )
put "."   trap( 1>=2 )
put "."   trap( 2<=1 )
put "."   trap( 2>2 || 1>2 )
put "."   trap( 2<2 || 2<1 )

put " t2" var e=1 var z=2
put "."   trap( e==z )
put "."   trap( z!=z )
put "."   trap( e>=z )
put "."   trap( z<=e )
put "."   trap( z >z )
put "."   trap( z< z )
put "."   trap( e!=1 || z!=2 )

	n=2  assert(nan!=nan)
	n=3  assert(!(nan==nan))
	n=4  assert(!(nan>=nan))
	n=5  assert(!(nan<=nan))
	n=6  assert(!(nan>nan))
	n=7  assert(!(nan<nan))

if !err put " passed" else errors+=err err=0 then

c=0
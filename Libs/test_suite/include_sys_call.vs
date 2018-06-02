

// ----------------------------------------------------------

put nl,"include: "
var fname = "/tmp/zzz.vs"
var n

proc failed()
{
	put "."
	log " test #",n," failed "; err++;
}
proc ok()
{
	put "."
}

try
		n=1 if exists zzz del zzz then
		file fname = " if 1 var zzz=711 then"
		include(fname)
		assert(exists zzz && zzz==711)

		n=2 file fname = " if 1 then end; failed() "
		n=2.5 include(fname) ok()

		n=3 file fname = " if 1 then end 0 failed()"
		include(fname) ok()

		n=4 file fname = " if 1 then end 1 failed()"
		include(fname) ok()

		n=5 file fname = " if 1 then proc z(){end} z() failed()"
		include(fname) ok()

		n=6 file fname = " if 1 then "
		assert(include(fname) == 0 )

		n=7 if exists zzz del zzz then
		file fname = «var zzz = "qwertz"\n»
		assert(include(fname)==0 )
		assert(exists zzz && zzz=="qwertz" )

		n=9 file fname = «del zzz\n»
		assert(include(fname)==0 )
		assert(!exists zzz )

		n=10 file fname = «var zzz = "qwertz" end\n»
		assert(include(fname)==0 )

		n=11 file fname = «var zzz = "qwertz" end 0\n»
		assert(include(fname)==0 )

		n=12 file fname = «var zzz = "qwertz" end 11\n»
		assert(include(fname)==11 )

		n=13 file fname = «proc z(){end} z() end 1»
		assert(include(fname)==0 )

		n=14 file fname = «proc z(){end 0} z() end 1»
		assert(include(fname)==0 )

		n=15 file fname = «proc z(){end 22} z() end»
		assert(include(fname)==22 )

		n=16 file fname = «proc z(){if 1 end then} z() end 1»
		assert(include(fname)==0 )

		n=17 file fname = «proc z(){if 1 end 0 then} z() end 1»
		assert(include(fname)==0 )

		n=18 file fname = «proc z(){if 1 end 22 then} z() end»
		assert(include(fname)==22 )

		n=19 file fname = «proc z(){do if 1 do end loop then loop} z() end 1»
		assert(include(fname)==0 )

		n=20 file fname = «proc z(){do if 1 do end 0 loop then loop} z() end 1»
		assert(include(fname)==0 )

		n=21 file fname = «proc z(){do if 1 do end 22 loop then loop} z() end»
		assert(include(fname)==22 )

		n=22 file fname = «proc z() {{end 22}} z() end 0»
		assert(include(fname)==22 )

		n=23 file fname = «proc z() {end 33} {z()} end 0»
		assert(include(fname)==33 )

		n=24 file fname = «proc z() {{ var z=0 do if 1 {do {while 33}{end {22}}
						   loop} then loop}} {if 1 z() failed(25) end then}»
		assert(include(fname)=={22} )

else
	log " test #",n," bummered: ", error.message; err++; xxx()
then

if !err put " passed" else errors+=err err=0 then



















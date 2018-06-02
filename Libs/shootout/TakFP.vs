#!/usr/local/bin/vipsi

var Start, Duration

proc Tak(x,y,z)
{
	if y<x	return Tak(Tak(x-1.0,y,z),Tak(y-1.0,z,x),Tak(z-1.0,x,y))
	else	return  z
	then
}

if Tak(7*3,7*2,7*1) != 14
	log "failed!"
	end 1
then

Start = now
put nl, "Tak(10*3,10*2,10*1) = ", Tak(10*3.0,10*2.0,10*1.0)
Duration = now-Start
put "  --  Total time = ", Duration, " sec."



put nl
end 0








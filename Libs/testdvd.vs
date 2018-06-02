#!/usr/local/bin/vipsi


openin #4, "/dev/cdrom1"

var n = 1<<20
var z = " " do while count(z) < n z#=z loop


var total=0
do
	var t0 = now
	put total,"          "[count(string(total)) to]
	var j=0
	do
		put "#"
		read#4,z
		total += n
		while count(z) == n
		while ++j<44
	loop
	put " ", int((now-t0)*1000)/1000, "sec", nl
	while count(z) == n

loop

put "finished",nl
close#4
end

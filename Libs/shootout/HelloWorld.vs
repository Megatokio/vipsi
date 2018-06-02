#!/usr/local/bin/vipsi

var Start, Duration


file "/tmp/test.vs" = «#!/usr/local/bin/vipsi\nput"hello world\n"\n»


Start = now
var i=0
do
	while ++i <= 200
	sys job.invocation, "/tmp/test.vs"
loop
Duration = now-Start
put "  --  Total time = ", Duration, " sec."



put nl
end 0








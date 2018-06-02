#!/usr/local/bin/vipsi

var Start, Duration


proc Doit(fname)
{
	var f
	openin #f,fname
	var last_char = " "
	
	do
		var z = spacestr(4*1024)
		try read #f,z then
		until count z==0
		c += count z
		l += count split(z) -1
		replace z,"\n"," "
		replace z,"\r"," "
		replace z,"\t"," "
		do while find(z,"   ") replace z,"   "," " loop
							   replace z,"  "," "  
		w += count split(z," ") -1
		w -= last_char==" " && z[1]==" "
		l -= last_char=="\r" && z[1]=="\n"
		l -= last_char=="\n" && z[1]=="\r"
		last_char = z[count z]
	loop
	w += last_char!=" " 
	
	close #f
}




Start = now		

	var i=0
	var c=0, w=0, l=0
	do
		while ++i <= 2500
		Doit( job.script[to rfind(job.script,"/")] # "wc-input.txt" )
	loop
	put nl, l, " ", w, " ", c

Duration = now-Start
put "  --  Total time = ", Duration, " sec."





put nl
end 0
		
		
		
		
		
		
		
		
		
		
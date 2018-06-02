#!/usr/local/bin/vipsi

var Start, Duration


proc Mandelbrot(w,h)
{
	var f openout#f, job.script[to rfind(job.script,"/")] # "Mandelbrot_" # string(w) # "x" # string(h) # ".pbm"
	put #f, "P4",nl, w, " ", h, nl
	
	var bit_num  = 0
    var byte_acc = 0
    var iter  = 50
    var limit = 2.0, limit_limit = limit*limit
    var Zr, Zi, Cr, Ci, Tr, Ti

	var y = -1
	do
		while ++y<h
		
		var x = -1
		do
			while ++x<w

            Zr = 0.0; Zi = 0.0;
            Cr = 2*x/w - 1.5; Ci=2*y/h -1;

			var i=-1
			do
				while ++i < iter
                Tr = Zr*Zr - Zi*Zi + Cr;
                Ti = 2*Zr*Zi + Ci;
                Zr = Tr; Zi = Ti;
                until Zr*Zr+Zi*Zi > limit_limit 
            loop

			byte_acc = (byte_acc << 1) | (Zr*Zr + Zi*Zi <= limit_limit)
			
            bit_num++

            if(bit_num == 8)
                write #f, charstr(byte_acc)
                byte_acc = 0;
                bit_num = 0;
            elif(x == w-1)
                byte_acc = byte_acc << (8-w%8);
                write #f, charstr(byte_acc)
                byte_acc = 0;
                bit_num = 0;
            then
		loop
	loop
}






Start = now
put nl, "Mandelbrot(64,64) "
Mandelbrot(64,64)
Duration = now-Start
put "  --  Total time = ", Duration, " sec."

Start = now
put nl, "Mandelbrot(200,200) "
Mandelbrot(200,200)
Duration = now-Start
put "  --  Total time = ", Duration, " sec."


put nl
end 0








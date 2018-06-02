#!/usr/local/bin/vipsi

var Start, Duration

proc Ackermann(x,y)
{
  if   x == 0     return y+1
  elif y == 0     return Ackermann(x-1,1)
  else            return Ackermann(x-1, Ackermann(x,y-1))
  then
}

if Ackermann(3,4) != 125 
	log "failed!"
	end 1
then

Start = now
put nl, "Ackermann(3,7) = ", Ackermann(3,7)
Duration = now-Start
put "  --  Total time = ", Duration, " sec."

Start = now
put nl, "Ackermann(3,8) = ", Ackermann(3,8)
Duration = now-Start
put "  --  Total time = ", Duration, " sec."

Start = now
put nl, "Ackermann(3,9) = ", Ackermann(3,9)
Duration = now-Start
put "  --  Total time = ", Duration, " sec."


put nl
end 0








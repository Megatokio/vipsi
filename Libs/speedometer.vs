#!/usr/local/bin/vipsi


// speed test
// calibrated to yield approx. 1.00 on my P4




put nl, "hi, i'm vipsi!"
put nl, "and i'm incredibly fast! ..."

wait 1

put nl
put nl, "let's make the test."
put nl, "calculating... "


del *
var ta = now
var dt = 10
var te = now + dt
var z,l,i,j
var s = "|/-\\", si = 1, st=ta

proc fak(n) { if n<=1 return 1 else return fak(n-1)*n then }

i=0
do
	while now < te
	i++
	if now>st st+=0.100 put s[1+si++%4],"\b" then
	fak(100)
	l = {0,1,2,3,4,5,6,7,8,9}
	l ##= l+10
	l ##= l+20
	l ##= l+40
	l ##= l+80 l=l[to 100]
	if min(l)!=0  log "booboo1",nl end then
	if max(l)!=99 log "booboo2",nl end then
	z = l;
	j=0 do  while ++j<=100  swap z[1+random 100], z[1+random 100]  loop
	sort z
	if z!=l log "booboo3",nl log z,nl end then
	z = ""
	j=0 do while ++j<=100 z#=l[j] loop
	if z[to 20]!="01234567891011121314" log "booboo4",nl end then
loop


put " ",nl
put "yeah! i made ",i," loops in ",dt," seconds!",nl
put "that's nearly ",i/dt/710," megaboogies!",nl
put "unbelievable...",nl


return i/dt/710 # " megaboogies"
end









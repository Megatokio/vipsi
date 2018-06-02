
// ----------------------------------------------------------
put nl,"procedures:"

put " t0" proc test1 ( ) { a=77 } if !isproc test1 log " failed"  err++ then
put "."   proc test2 (a,b,c) {return a*b*c}
								  if !isproc test2 log " failed"  err++ then
proc drei ( ) { return 3 }
proc vier(){return 4}
put " t1" a=10 test1() if a!=77         log " failed"  err++ then
put "." a=10 if test2(2,3,4)!=24        log " failed"  err++ then
put "." if a!=10                        log " failed"  err++ then
put "." a=10 if a+drei()!=13            log " failed"  err++ then
put "." if a!=10                        log " failed"  err++ then
put "." a=10 if drei()+a!=13            log " failed"  err++ then
put "." if a!=10                        log " failed"  err++ then
put "." if drei()*vier()!=12            log " failed"  err++ then
proc zwei(a){var b return count locals}
put " t2" if zwei(0)!=2                 log " failed"  err++ then
proc das(a){return locals[1]}
put " t3" if das(55)!=55                log " failed"  err++ then
proc mult(a,b){return locals[1]*locals[2]}
put " t4" if mult(5,6)!=30              log " failed"  err++ then
put "." a={} a[1]=drei if !isproc a[1]	log " failed"  err++ then
put "." a=a[1]() if a!=3                log " failed"  err++ then
proc t(*){return count locals}
put " t5" if t()!=0                     log " failed"  err++ then
put "." if t(4)!=1                      log " failed"  err++ then
put "." if t(6,a)!=2                    log " failed"  err++ then
put "." if t(a,5+6,0)!=3                log " failed"  err++ then
proc t1(*){return count locals}
put " t6" if t1(4)!=1                   log " failed"  err++ then
put "." if t1(6,a)!=2                   log " failed"  err++ then
put "." if t1(a,5+6,0)!=3               log " failed"  err++ then
proc mult(*)
{ var n; n=count locals; var r=1; do while --n r*=locals[n] loop return r }
put " t7" if mult()!=1                  log " failed"  err++ then
put "." if mult(4,5.5)!=22              log " failed"  err++ then
put "." if mult(pi)!=pi                 log " failed"  err++ then
var a=10,b=20
put " t8" if mult(a,2,b)!=400           log " failed"  err++ then
put "." if a!=10||b!=20                 log " failed"  err++ then
proc fak(n){ if n<=1 return n then return n*fak(n-1)}
put " t9" if fak(11)!=39916800          log " failed"  err++ then

var a={} rename a[1]="wert" rename a[2]="SetWert" rename a[3]="GetWert"
	a.wert=55

proc gw() {return wert}     a.GetWert = gw
proc sw(n){wert=n}          a.SetWert = sw

put " t10" a.SetWert(66) if a.wert!=66  log " failed"  err++ then
put "."    if a.GetWert()!=66           log " failed"  err++ then
a.SetWert(77)
put " t11" { if a.GetWert()!=77         log " failed"  err++ then }

if !err put " passed" else errors+=err err=0 then



put nl,"Variable protection "

proc bummer(){del bummer}
n=1.1 try bummer() 				else log "bummer" err++ then 	trap(exists bummer)

proc bummer(){del bummer}
proc booboo(){bummer()}
n=1.2 try booboo() 				else log "bummer" err++ then 	trap(exists bummer)

proc bummer(){del booboo}
n=1.3 try booboo() 				else log "bummer" err++ then 	trap(exists booboo)


do while exists a del a loop
proc bummer(){del a; return 1}

n=2	  try var a = bummer() 				else log "bummer",n err++ then trap(exists a)
n=3.1 try var a,c = a + bummer() 		else log "bummer",n err++ then trap(exists a)
n=3.2 try var a,b=0,c = bummer() + a 	else b=1 then   assert(b==1)   trap(exists a)
n=4.1 try var a a += bummer() 			else log "bummer",n err++ then trap(exists a)
n=4.2 try var a a -= bummer() 			else log "bummer",n err++ then trap(exists a)
n=4.3 try var a = sin(bummer()*pi) 		else log "bummer",n err++ then trap(exists a)
n=4.4 try var a = max(1,bummer()) 		else log "bummer",n err++ then trap(exists a)
n=4.5 try var a = min(bummer(),1) 		else log "bummer",n err++ then trap(exists a)
n=4.7 try var a = 1 && bummer() 		else log "bummer",n err++ then trap(exists a)
n=4.8 try var a = 0 || bummer() 		else log "bummer",n err++ then trap(exists a)
n=4.9 try var a = bummer() || 0			else log "bummer",n err++ then trap(exists a)
n=5.1 try var a = bummer() ? 2:3 		else log "bummer",n err++ then trap(exists a)
n=5.2 try var a=9,c = max(a,bummer()) 	else log "bummer",n err++ then trap(exists a) assert(c==9)
n=5.3 try var a=9,b=0,c=max(bummer(),a) else b=1 then n=5.31 assert(b==1)   n=5.32 trap(exists a)

n=6.1 try b=0 globals=0					else b=1 then 	assert(b)
n=6.2 try b=0 del globals				else b=1 then 	assert(b)
n=7.1 try b=0 locals=0					else b=1 then 	assert(b)
n=7.2 try b=0 del locals				else b=1 then 	assert(b)
n=8.1 try b=0 root=0					else b=1 then 	assert(b)
n=8.2 try b=0 del root					else b=1 then 	assert(b)
n=9.1 try b=0 swap root,locals			else b=1 then 	assert(b)
n=9.2 try b=0 swap root,globals			else b=1 then 	assert(b)
n=10  try b=0 var a={b=0} swap a,a[1]	else b=1 then 	assert(b)
n=11  try b=0 var a={b=0} swap a[1],a	else b=1 then 	assert(b)
n=12  try b=0 var a={b=0} swap a,a.b	else b=1 then 	assert(b)
n=13 assert(exists a)

n=13.1 try b=0 a=proc(){var i del i return 0}()	else b=1 then 	assert(b==0)
n=13.2 try b=0 a=proc(){del locals return 0}()	else b=1 then 	assert(b==1)


if !err put " passed" else errors+=err err=0 then

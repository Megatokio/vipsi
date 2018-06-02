

// ----------------------------------------------------------
put nl,"variable names:"
put " t1" a=0 if !isnumber a            log " failed" err++ then
put "."   a=0 if istext a               log " failed" err++ then
put "."   a=0 if islist a               log " failed" err++ then
put "."   a=0 if !exists a              log " failed" err++ then
put "."   a=0 if exists a.foo           log " failed" err++ then
put "."   a=0 if exists foo             log " failed" err++ then
put " t2" var foo if !exists foo        log " failed" err++ then
put "."   foo={} foo[1]=1 foo[2]=2 foo[3]=3 foo[4]=4
put "."   if !exists foo[2]             log " failed" err++ then
put "."   foo[2]={} foo[2,5]=0
put "."   if !exists foo[2,5]           log " failed" err++ then
put "."   if !isnumber foo[2,5]         log " failed" err++ then
put "."   if count foo[2]!=5            log " failed" err++ then
put " t3" del foo[2]
put "."	  if foo[2]!=3		            log " failed" err++ then
put "."   if count foo[2]!=0            log " failed" err++ then
put "."   if !isnumber foo[2]           log " failed" err++ then
put " t4" foo={1,2,3,4,5}
put "."   if count foo!=5               log " failed" err++ then
put "."   del foo[5] if count foo!=4    log " failed" err++ then
put "."   del foo[3] if count foo!=3    log " failed" err++ then
put "."   if foo!={1,2,4}               log " failed" err++ then

if !err put " passed" else errors+=err err=0 then



// ----------------------------------------------------------
put nl,"list item names:"
put "." var punkt
put "." punkt={}
put " t0" if name punkt[1]!="" log "failed " err++ then
put "."   rename punkt[1]="x" put"." punkt.x=0
put "."   rename punkt[2]="y" put"." punkt.y=0

put " t1" if name punkt[1]!="x" log "failed " err++ then
put "."   if name punkt[2]!="y" log "failed " err++ then
put "."   punkt[1]=77; if punkt.x!=77  log "failed " err++ then
put "."   punkt[2]=88; if punkt.y!=88  log "failed " err++ then
put "."   punkt.x=34;  if punkt[1]!=34 log "failed " err++ then if punkt.x!=34 log "failed! " err++ then
put "."   punkt.y=45;  if punkt[2]!=45 log "failed " err++ then if punkt.y!=45 log "failed! " err++ then

put " t2" var v={} v[3]=punkt rename v[3]="pt"
put "."   if v.pt !={34,45} log "failed " err++ then
put "."   if v.pt.x !=34 log "failed " err++ then
put "."   if v.pt.y !=45 log "failed " err++ then

put "."   if string v != "{ 0, 0, pt={ x=" # string punkt.x # ", y="
					# string punkt.y # " } }" log "failed " put nl,string v err++ then
put "."   if !exists v.pt    log "failed " err++ then
put ":"   if string v != "{ 0, 0, pt={ x=" # string punkt.x # ", y="
					# string punkt.y # " } }" log "failed " put nl,string v err++ then
put "."   if v[3,1]!=punkt.x log "failed " err++ then
put "."   if v[3,2]!=punkt.y log "failed " err++ then
put "."   if v[3].x!=punkt.x log "failed " err++ then
put "."   if v[3].y!=punkt.y log "failed " err++ then
put "."   if v.pt.x!=punkt.x log "failed " err++ then
put "."   if v.pt.y!=punkt.y log "failed " err++ then
put ":"	  if count v[3]!=2   log "failed " err++ then
put "."   if !exists punkt.y log "failed " err++ then
put "."   if  exists punkt.z log "failed " err++ then
put "."   if !exists v.pt.y  log "failed " err++ then
put "."   if  exists v.pt.z  log "failed " err++ then
put "."   if !exists v[3].y  log "failed " err++ then
put "."   if  exists v[3].z  log "failed " err++ then

put " t3"
put "." var a = 0;			if exists a.foo.boo.z   log " failed" err++ then
put "." a = {};				if exists a.foo.boo.z   log " failed" err++ then
put "." var a.foo.boo.z = 1	if !exists a.foo.boo.z	log " failed" err++ then
put "." 					if a.foo.boo.z!=1       log " failed" err++ then
put "."						if exists a.foo.boo.zz	log " failed" err++ then
put "."						if exists a.foo.boo.zz	log " failed" err++ then
put "."						if exists a.foo.boo[3]	log " failed" err++ then
put "."						if exists a.foo.boo[3]	log " failed" err++ then
put "."						if !exists a.foo.boo[1]	log " failed" err++ then
put "." b="boo" 			if !exists a.foo.@b.z 	log " failed" err++ then
put "."	b="boo.z" 			if exists a.foo.@b 		log " failed" err++ then

if !err put " passed" else errors+=err err=0 then
























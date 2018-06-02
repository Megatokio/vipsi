
// ----------------------------------------------------------
put nl,"list functions:"
put "t1 " a={} if count(a)!=0 log "failed " err++ then
put "t2 " a[1]=1 if count(a)!=1 log "failed " err++ then
put "t3 " a[55]="" if count(a)!=55 log "failed " err++ then
put "t4 " if !islist a log "failed " err++ then
put "t5 " if !isnumber a[5] log "failed " err++ then
put "."   if a[5]!=0 log "failed " err++ then
put "."   a[2]="<AnToN MüLLer öäüßÖÄÜ>" convert a to html
put "t6 " if !isnumber a[1] log "failed " err++ then
put "t7 " if !istext a[55] log "failed " err++ then
put "t8 " if a[2]!="&lt;AnToN MüLLer öäüßÖÄÜ&gt;" log "failed " err++ then
put "t9 " if !isnumber a[6] log "failed " err++ then
put "."   if a[6]!=0 log "failed " err++ then
put "t10 " if a[2,7]!="T" log "failed " err++ then
			trap( {} != {} )
			trap( {1}!={1} )
			trap( {"gg"} != {"gg"} )
			trap( {1,"gg"} != {1,"gg"} )
			trap( {"gg",1} != {"gg"} ## {1} )
			trap( {}##{1,"gg"} != {1,"gg"} )
			trap( {1,"gg"}##{} != {1,"gg"} )
			trap( {"gg",1} != {"gg"} ## 1 )
			trap( {}##1 != {1} )
			trap( 1##"1" != {1,"1"} )
			trap( {1,"1"} != {}##1##"1" )
			trap( {}##1##"1" != {1,"1"} )
put "t11 " if {}##1##"1" != {1,"1"}             log "failed " err++ then
put "t12 " a={1,2} a[3]=a if a!={1,2,{1,2,0}}   log "failed " err++ then
put "t13 " a={2,3} a=a##a if a!={2,3,2,3}       log "failed " err++ then

// find(), rfind()
put "t14 "	var l = { 1,3,5,"7",{9},0,10,{1,2},99 }
put "."		if( find(l,3)      != 2 )		log "failed " err++ then
put "."		if( find(l,4)      != 0 )		log "failed " err++ then
put "."		if( find(l,{})     != 0 )		log "failed " err++ then
put "."		if( find(l,{9})    != 5 )		log "failed " err++ then
put "."		if( find(l,{1,2})  != 8 )		log "failed " err++ then
put "."		if( find(l,{1,2,3})!= 0 )		log "failed " err++ then
put "."		if( find(l,99)     != 9 )		log "failed " err++ then
put "."		if( find(l,"7")    != 4 )		log "failed " err++ then
put "."		if( find(l,"7",4)  != 4 )		log "failed " err++ then
put "."		if( find(l,"7",5)  != 0 )		log "failed " err++ then
put "."		if( rfind(l,3)     != 2 )		log "failed " err++ then
put "."		if( rfind(l,"7")   != 4 )		log "failed " err++ then
put "."		if( rfind(l,"7r")  != 0 )		log "failed " err++ then
put "."		if( rfind(l,{9})   != 5 )		log "failed " err++ then
put "."		if( rfind(l,{9},5) != 5 )		log "failed " err++ then
put "."		if( rfind(l,{9},4) != 0 )		log "failed " err++ then

if !err put " passed" else errors+=err err=0 then


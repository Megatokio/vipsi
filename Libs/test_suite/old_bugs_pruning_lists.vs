


// test for pruning bug in vs.0.9.0:		
// does not skip to "}" after "{" 
// => ruft prune_bummer() auf und beendet das aktuelle skript, wenn es dann auf "}" aufläuft

var prune_completed = 0
proc prune_bummer(){ put "."; err++ log "#",n," failed! " return 0 }

	n=1 var a = 1 ? {1} : {prune_bummer()}  n=2 assert(a=={1}) 
	n=3	 a = 0 ? {prune_bummer()} : {2}		n=4 assert(a=={2})

prune_completed = 1


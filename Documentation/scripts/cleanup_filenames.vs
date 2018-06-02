#!/usr/local/bin/vipsi

if count globals==0
	put "dieses script bereinigt dateinamen von non-printables usw.",nl
	put "bitte mit datei/verzeichnis aufrufen",nl
	end
then

var argc = count globals -1



proc doit ( path, fname )
{
	if fname=="" || rightstr(fname,1)=="/" ? !exists dir(path#fname) : !exists file(path#fname)
		log "broken name: ", convert( path#fname to quoted ), nl
	then

	var v,c,i=0,uci=0,j
	var nname=fname
	do
		while++i<=count nname
		c=nname[i]

		if c>=" " && c<="z" next then

		if c<" "
			if !uci uci = i then
			log "_$"#hexstr(charcode(c),2)#"_"
			nname = nname[to i-1]#"_$"#hexstr(charcode(c),2)#"_"#nname[i+1 to]
			next
		then

		if charcode(c)==$308	// diaresis ((umlaut))
			if i>1
				v = nname[i-1]
				j = find("aeiouAEIOU",v)
				if j
					if !uci uci = i then
					nname = nname[to i-2] # "äëïöüÄËÏÖÜ"[j] # nname[i+1 to]
					next
				then
			then
		then

		if charcode(c)==$300	// accent grave "`"
			if i>1
				v = nname[i-1]
				j = find("aeiouAEIOU",v)
				if j
					if !uci uci = i then
					nname = nname[to i-2] # "àèìòùÀÈÌÒÙ"[j] # nname[i+1 to]
					next
				then
			then
		then

		if charcode(c)==$301	// accent acute "'"
			if i>1
				v = nname[i-1]
				j = find("aeiouAEIOU",v)
				if j
					if !uci uci = i then
					nname = nname[to i-2] # "áéíóúÁÉÍÓÚ"[j] # nname[i+1 to]
					next
				then
			then
		then

		if charcode(c)==$302	// accent circumflex "^"
			if i>1
				v = nname[i-1]
				j = find("aeiouAEIOU",v)
				if j
					if !uci uci = i then
					nname = nname[to i-2] # "âêîôûÂÊÎÔÛ"[j] # nname[i+1 to]
					next
				then
			then
		then

		if charcode(c)==$303	// tilde "~"
			if i>1
				v = nname[i-1]
				j = find("na",v)
				if j
					if !uci uci = i then
					nname = nname[to i-2] # "ñã"[j] # nname[i+1 to]
					next
				then
			then
		then

		if charcode(c)==$327	// cedilla
			if i>1
				v = nname[i-1]
				j = find("cC",v)
				if j
					if !uci uci = i then
					nname = nname[to i-2] # "çÇ"[j] # nname[i+1 to]
					next
				then
			then
		then

		if charcode(c)==$30A	// ring above "°"
			if i>1
				v = nname[i-1]
				j = find("aA",v)
				if j
					if !uci uci = i then
					nname = nname[to i-2] # "åÅ"[j] # nname[i+1 to]
					next
				then
			then
		then

		if charcode(c)>=$300 && charcode(c)<=$340  // non-spacing character  ***TODO***
			if i>1 log nname[i-1],hexstr(charcode(c),3)," " then
			if !uci uci = i then
		then
	loop

	if uci
		put "rename: ", convert( path#fname to quoted ), " to ", convert(nname to quoted), nl
	//	rename file(path#fname) = path#nname
		fname = nname
	then

	if rightstr(fname,1)=="/" || fname==""
		path #= fname
		if fullpath(path)!=path return then		// symbolic link: skip to avoid infinite recursion

		var files = dir (path)
		var i=0
		do
			while ++i <= count files
			if files[i].flags[1]=="?" next then			// broken link
			if files[i].flags[1]=="d"
				if files[i].flags[2]!="r" next then		// directory not readable
			then
			doit(path,files[i].fname)
		loop
	then
}




var j,i=0
var fname,path
do
	while ++i <= argc
	path = fullpath(globals[i])
	j = rfind(path,"/")
	if j==0 log "no slash found in ", convert(path to quoted) put nl end then
	fname = path[j+1 to]
	path  = path[to j]
	doit(path,fname)
loop



put "--- done ---",nl
end

#!/usr/local/bin/vipsi


var argc    = count globals -1
var pretend = 0

if argc>1 && globals[1]=="-p"
	pretend = 1
	del globals[1]
	argc--
then

if argc==0
	put "dieses script löscht alle bekannten trash files",nl
	put "bitte mit datei/verzeichnis aufrufen",nl
	end 1
then





proc doit ( path )
{
	if fullpath(path) != path return then		// symlink?

// directory:
	if isdir(path)
		if rightstr(path,1)!="/" path #= "/" then
		var i = rfind(path,"/",count path-1)
		var dname = path[i+1 to]
		if dname=="resource.frk/"
		|| dname=="RESOURCE.FRK/"
		|| dname=="TheVolumeSettingsFolder/"
		|| dname==".xvpics/"
			put "del: ", convert(path to quoted), nl
			if !pretend sys "rm", "-fR", path then
			return
		then

		var files = dir (path)
		var i=0
		do
			while ++i <= count files
			if files[i].flags[1]=="?" next then		// broken link
			if files[i].flags[1]=="l" next then		// link
			if files[i].flags[1]=="d"
				if files[i].flags[2]!="r" next then	// directory not readable
			then
			doit(path#files[i].fname)
		loop
		return
	then

// file:
	var i = rfind(path,"/")
	var fname = path[i+1 to]

	if fname=="Icon\r"
	|| fname=="Icon_"
	|| fname=="FINDER.DAT"
	|| fname=="._Icon\r"
	|| fname==".DS_Store"
	|| fname=="finder.dat"
	|| fname[to 2] == "._"
	|| rightstr(fname,5) == ".xSYM"
	|| ( rightstr(fname,4) == ".cat" && find(path,"/Develop/projects/MacOS Classic/") )
		put "del: ", convert(path to quoted), nl
		if !pretend del file path then
		return
	then
}




var i=0
do
	while ++i <= argc
	doit ( fullpath(globals[i]) )
loop



put "--- done ---",nl
end 0



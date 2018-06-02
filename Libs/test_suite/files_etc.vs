

// ----------------------------------------------------------

put nl,"paths: "

put " t1" if rightstr(fullpath("."),1)!="/"			log " failed"  err++ then
put "."   if rightstr(fullpath("./"),1)!="/"		log " failed"  err++ then
put "."   if rightstr(fullpath("./"),2)=="//"		log " failed"  err++ then
put " t2" if rightstr(fullpath(".."),1)!="/"		log " failed"  err++ then
put "."   if rightstr(fullpath("../"),1)!="/"		log " failed"  err++ then
put "."   if rightstr(fullpath("../"),2)=="//"		log " failed"  err++ then
put " t3" if rightstr(fullpath("~"),1)!="/"			log " failed"  err++ then
put "."   if rightstr(fullpath("~/"),1)!="/"		log " failed"  err++ then
put "."   if rightstr(fullpath("~/"),2)=="//"		log " failed"  err++ then
put " t4" if fullpath("/")!="/"						log " failed"  err++ then
put "."   if fullpath("/./")!="/"					log " failed"  err++ then
put "."   if fullpath("/../")!="/"					log " failed"  err++ then

proc CreateTempDir()
{
	cd "/tmp"
	if exists dir "vipsi_" || exists file "vipsi_" || exists link "vipsi_"
		try
			if islink "vipsi_" del link "vipsi_"
			else sys "rm", "-Rf", "vipsi_" then
		then
		if exists dir "vipsi_" || exists file "vipsi_" || exists link "vipsi_"
			log nl,
			«failed to remove old scratch dir "/tmp/vipsi_"», nl,
			«subsequent errors may happen», nl
			return 1	// error
		then
	then
	try
		new dir "vipsi_"
		return 0	/*no error*/
	else
		log nl,
		«failed to create test dir "/tmp/vipsi_"», nl,
		«subsequent errors may happen», nl
		return 1	/*errror*/
	then
}

if CreateTempDir()==0
	put " t5" var tmp = fullpath(".")
	try
		put "." if fullpath("vipsi_")!=tmp#"vipsi_/"	log " failed"  err++ then
		cd "vipsi_"
		put "." if fullpath(".")!=tmp#"vipsi_/"		 	log " failed"  err++ then
		new dir "dir_"
		put "." if fullpath("dir_")!=tmp#"vipsi_/dir_/" log " failed"  err++ then
		cd ".."
		put "." del dir(tmp#"vipsi_/dir_")  del dir(tmp#"vipsi_")
	else
		log "[failed]"  err++
	then
then

if !err put " passed" else errors+=err err=0 then

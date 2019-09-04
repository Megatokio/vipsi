#!./vipsi


put«
  *********************************************
  *                hello vipsi                *
  *********************************************

        I'm about to install myself.
        Root privileges are required for a
        standard installation in "/usr/bin/".
        Prefs are installed in "~/.vipsi/".
»


var d = dir "/usr/bin*"
if d=={}
	log nl,«  ?!? "/usr/bin/" not found !?!»
	log nl,«  aborting. please fix me.»
	end 1
then
var is_root = d[1].flags[3] == "w"

if !is_root
	put nl, «        »
	log «not root: installing in "~/bin/"»
	put nl
then


proc ReplaceFile ( Path, Source, Owner, Mod )
{
	put nl, «  create file », Path
	try
		var f = exists file Path
		file Path = file Source
		if !f
			sys "chown",Owner,Path
			sys "chmod", Mod, Path
		then
	else
		log nl, «  create file »#Path#« failed:»
		log nl, "  ", errorstr
		return 1
	then
	return 0
}


proc CreateFile ( Path, Source, Owner, Mod )
{
	if exists file(Path) return 0 then
	return ReplaceFile(Path,Source,Owner,Mod)
}


proc CreateDir ( Path, Owner, Mod )
{
	if !exists dir(Path)
		put nl, «  create dir », Path
		try
			new dir Path
			sys "chown",Owner,Path
			sys "chmod", Mod, Path
		else
			log nl, «  create dir »#Path#« failed:»
			log nl, "  ", errorstr
			return 1
		then
	then
	return 0
}


proc CreateLink ( Path, Target, Owner )
{
	if !exists link(Path)
		put nl, «  create symlink », Path
		try
			link Path = Target
			sys "chown",Owner,Path
		else
			log nl, «  create symlink »#Path#« failed:»
			log nl, "  ", errorstr
			return 1
		then
	then
	return 0
}


var Root = sys("whoami")	replace Root, nl, ""
var User = sys("logname")	replace User, nl, ""

put nl,"creating executables and global stuff"

if is_root
	ReplaceFile("/usr/local/bin/vipsi", "./vipsi", Root, "a+rx")
	CreateLink ("/usr/bin/calc","vipsi",Root)
	CreateFile ("/etc/vipsi.cgi.conf", "../examples/vipsi.cgi.conf", Root, "a+r")
	CreateDir  ("/usr/local/lib/vipsi/", Root, "a+rx")
	ReplaceFile("/usr/local/lib/vipsi/test_suite.vs","../examples/test_suite.vs", Root, "a+rx")
	CreateDir  ("/usr/share/apps/", Root, "g-w")
	CreateDir  ("/usr/share/apps/katepart/", Root, "go-w")
	CreateDir  ("/usr/share/apps/katepart/syntax/", Root, "go-w")
	ReplaceFile("/usr/share/apps/katepart/syntax/vip.xml", "../examples/vip.xml", Root, "a+r")
else
	var bindir = fullpath "~/bin/"
	CreateDir  ( bindir, User, "o-rwx" )
	var bindir = fullpath bindir
	ReplaceFile( bindir#"vipsi", "./vipsi", User, "a+rx" )
	CreateLink ( bindir#"calc", "vipsi", User )
	ReplaceFile( bindir#"test_suite.vs", "../examples/test_suite.vs", User, "a+rx" )
then


do
	put nl,"creating preferences for ",User

	var p,i,j
	p = nl # file "/etc/passwd"
	convert p from utf8

	i = find(p,nl#User#":")

	if i==0
	// MacOSX has the user somewhere else, but not in /etc/passwd
	// give it a 2nd chance:
		var home_dir = fullpath("~/")

		if rightstr(home_dir,count User+2) != "/"#User#"/"
			log nl, "  couldn't find ",User," in /etc/passwd."
			log nl, "  skipped."
			var errorstr
			until User==Root
			User=Root next
		then

	else
		i = find(p,"/",i)
		j = find(p,":",i)
		var home_dir = p[i to j-1]
	then

	if !exists dir home_dir
		log nl, «  home directory »,homedir,« for user »,User,« does not exist.»
		log nl, "  skipped."
		var errorstr
		until User==Root
		User=Root next
	then

	var prefs_dir = fullpath( home_dir#"/" # ".vipsi/" )
	CreateDir  ( prefs_dir, User, "o-rwx" )
	var prefs_dir = fullpath( prefs_dir )
	CreateFile ( prefs_dir#"script.prefs", "../examples/script.prefs", User,"o-rwx" )
	CreateFile ( prefs_dir#"shell.prefs",  "../examples/shell.prefs",  User,"o-rwx" )
	CreateFile ( prefs_dir#"calc.prefs",   "../examples/calc.prefs",   User,"o-rwx" )

	until User==Root
	User=Root next
loop


if exists errorstr
	log nl,nl, "some errors occured. check output and try to fix it.",nl,nl
	end 1
else
	put nl,nl, "all seemed to install fine. you are done.",nl,nl
	end 0
then










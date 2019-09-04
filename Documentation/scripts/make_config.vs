#!/usr/local/bin/vipsi


/*	vip script interpreter:
	http://vipsi.sourceforge.net
*/


/*	generic script to create Makefile.in etc. for source and binary distributions.
	customized via $PROJECT/unix/automate.sourcefiles and $PROJECT/unix/automate.tarball.binary
	reads & updates $PROJECT/version.h

	this script is included in multiple projects - don't break it for other projects!

	this script is for configuring the make process
	this script does not yet create $PROJECT/unix/configure --> run automake on your own!
	this script is not required for compiling the configured source


howto:
	directory "MYPROJECT/" contains the source and anything else
	directory "MYPROJECT/unix/" contains all the unix make stuff
	directory "MYPROJECT/libraries/" must exist ((or link))

	#include "..." paths in sources must be relative to
			 "MYPROJECT/", "./", "MYPROJECT/unix/" or "MYPROJECT/libraries/"
		e.g. #include "kio/kio.h" if this file is in "MYPROJECT/libraries/kio/kio.h"

run:
	cd $PROJECT/unix
	autoconf				<-- somehow create MYPROJECT/unix/configure.
	./make_config.vs
	make
	make source-dist
	make binary-dist


	minimal "version.h"
	----------------------------------------------------------------------
	#define applName        "helloworld"
	#define applVersion     "0.0.1"
	#define releaseDate     "unknown"
	#define releaseHost     "unknown"
	#define releaseUser     "unknown"
	#define buildDate       "unknown"
	#define buildHost       "unknown"
	#define buildUser       "unknown"
	#define buildTarget     "unknown"
	----------------------------------------------------------------------


	minimal "unix/automate.sourcefiles"
	----------------------------------------------------------------------
	var source_files = { "main.cp" }
	var non_includes = { }							// if any
	var source_tarball_files = { "README.txt" }		// if any
	----------------------------------------------------------------------


	example "unix/automate.tarball.binary"
	----------------------------------------------------------------------
	helloworld/unix/helloworld
	helloworld/README.txt
	helloworld/doc/index.html
	----------------------------------------------------------------------
*/


/*	Copyright  (c)	Günter Woigk  2004
  					mailto:kio@little-bat.de

	This file is free software

 	This program is distributed in the hope that it will be useful,
 	but WITHOUT ANY WARRANTY; without even the implied warranty of
 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 	Permission to use, copy, modify, distribute, and sell this software and
 	its documentation for any purpose is hereby granted without fee, provided
 	that the above copyright notice appear in all copies and that both that
 	copyright notice and this permission notice appear in supporting
 	documentation, and that the name of the copyright holder not be used
 	in advertising or publicity pertaining to distribution of the software
 	without specific, written prior permission.  The copyright holder makes no
 	representations about the suitability of this software for any purpose.
 	It is provided "as is" without express or implied warranty.

 	THE COPYRIGHT HOLDER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 	INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 	EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 	CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 	DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 	TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 	PERFORMANCE OF THIS SOFTWARE.

	2004-04-01	started
	2004-05-03	preliminary version completed
*/



/* ----	display help? ------------------------------------------------
*/
if count globals == 1 && globals[1,1]=="-"	// assume help wanted
	put «\033[103m
make_config.vs copyright (c) 2004 Günter Woigk (kio@little-bat.de)
	this file is free software.
	see file itself for BSD-style license.\033[49m

make_config.vs im Projektverzeichnis oder unix-Unterverzeichnis aufrufen.
Optional auch Name des Projektverzeichnisses als Paramater übergeben.
Das Projektverzeichnis wird an der Datei "version.h" erkannt!

runs:
	autoheader
	autoconfig
	aktualisiert version.h
	scannt die #include-Struktur des Projektes
	-> erzeugt ein Makefile.in
	-> erzeugt automate.tarball.source \n\n»
	end 0
then



/* ----	"cd" to project dir ---------------------------
*/
if count globals == 1
	cd globals[1]
	del globals[1]
then

do
	while !exists file "version.h"
	cd ".."
	if fullpath(".")=="/"
		log «Wurzelverzeichnis des Projektes nicht gefunden!»,nl
		log «((Datei "$(PROJECT)/version.h" nicht gefunden))»,nl
		log «aborting.»,nl
		end 1
	then
loop



/* ----	get project dir ---------------------------
*/
var libdir		 = "libraries/"
var unixdir		 = "unix/"
var source_files = { }
var non_includes = { }
var source_tarball_files = { }


var prodir = "zasm/"
	prodir = fullpath(".")
	if rightstr(prodir,1) == "/" prodir = prodir[to count(prodir)-1] then
	prodir = prodir[rfind(prodir,"/")+1 to] # "/"


/* ----	include unix/automate.sourcefiles -----------
*/
if exists file( unixdir # "automate.sourcefiles" )
	include unixdir # "automate.sourcefiles"
else
	log «File "», unixdir#"automate.sourcefiles", «" not found.», nl
	log «aborting.»,nl
	end 1
then


/* ----	checks for required directories ---------------
*/
if !isdir(unixdir)
	log «Directory "», unixdir, «" not found.»,nl
	log «aborting.»,nl
	end 1
then
if !isdir(libdir)
	log «Directory "», libdir, «" not found.»,nl
	log «aborting.»,nl
	end 1
then
if !isfile(unixdir#"configure")
	log «Script "», unixdir#"configure", «" not found.»,nl
	log «You must do the autoconf stuff first!»,nl
	log «aborting.»,nl
	end 1
then




cd unixdir
put "---- running autoheader ----",nl
sys "autoheader"
put "---- running autoconf ----",nl
sys "autoconf"
cd ".."



put «---- adding standard files to file lists ----»,nl
// add "config.h" to ignore-files
// will be created by ./configure
non_includes ##= "config.h"

// add files to automate.tarball.source for ./configure:
source_tarball_files ##=
{	unixdir#"configure",
	unixdir#"Makefile.in",
	unixdir#"config.h.in",
	unixdir#"config.guess",
	unixdir#"config.sub",
	exists install_binary_script ? install_binary_script : unixdir#"install.sh",
 	unixdir#"shtool"
}

// add files to automate.tarball.source for make source-dist from the source dist itself:
// ((optional))
{
	source_tarball_files ##=
	{
		unixdir#"automate.sourcefiles",
		unixdir#"make_config.vs",
		unixdir#"configure.in"
	}
	proc add(f) { if exists file(f) source_tarball_files ##= f then }
	add(unixdir#"aclocal.m4")
	add(unixdir#"acsite.m4")
	add(unixdir#"acconfig.h")
	add(unixdir#"config.h.top")
	add(unixdir#"config.h.bot")
	add(unixdir#"automate.tarball.binary")
}




/* ----	update "version.h" --------------------------
		and get applVersion and applName
*/
put «---- updating "version.h" ----»,nl

var applVersion   = "9.9.9"
var applName      = "zasm"
var releaseDate   = datestr(now)[to 10]
var releaseUser   = sys("whoami")
var releaseHost   = sys("hostname")
if !find(releaseHost,".")
	try  var releaseHost   = sys("hostname", "-f")	// try -f: force domain completition ((linux only?))
	else var releaseHost   = sys("hostname")
	then
then
replace releaseHost, nl, ""
replace releaseUser, nl, ""

{
var version_file = file "version.h"
if version_file == ""
	log «could not open "version.h"»,nl
	log «aborting.»,nl
	end 1
then
convert version_file from utf8

var i,j
i = find(version_file,"applName");
if i==0
	log "no applName in version.h",nl
	log «aborting.»,nl
	end 1
then
i = find(version_file,«"»,i)
j = find(version_file,«"»,i+1)
applName = version_file[i+1 to j-1]
put "  applName: ",applName,nl

i = find(version_file,"applVersion");
if i==0
	log "no applVersion in version.h",nl
	log «aborting.»,nl
	end 1
then
i = find(version_file,«"»,i)
j = find(version_file,«"»,i+1)
applVersion = version_file[i+1 to j-1]
put "  applVersion: ",applVersion,nl

split version_file
do while version_file[count version_file]=="" del version_file[count version_file] loop

var i=0
do
	while ++i <= count version_file
	var zeile = version_file[i]

	// update release info:
	// who made the source distro:
	if match( zeile, "#define*release*" )
		if find( zeile, "releaseDate" )	zeile = «#define releaseDate\t\t"» # releaseDate # «"»	then
		if find( zeile, "releaseHost" )	zeile = «#define releaseHost\t\t"» # releaseHost # «"»	then
		if find( zeile, "releaseUser" )	zeile = «#define releaseUser\t\t"» # releaseUser # «"»	then
		version_file[i] = zeile
	then

	// clear build info:
	// who made the binary distro:
	if match( zeile, "#define*build*" )
		if find( zeile, "buildDate" )	zeile = «#define buildDate\t\t"unknown_date"»		then
		if find( zeile, "buildHost" )	zeile = «#define buildHost\t\t"unknown_host"»		then
		if find( zeile, "buildUser" )	zeile = «#define buildUser\t\t"unknown_user"»		then
		if find( zeile, "buildTarget" )	zeile = «#define buildTarget\t\t"unknown_target"»	then
		version_file[i] = zeile
	then
loop

convert version_file from tab4
openout#4,"version.h"
var i=0
do
	while ++i<=count version_file
	put#4,version_file[i],nl
loop
close#4
}



/* ----	define source files -----------------------
		source_files[i] := { path="path/rel/to/projectdir", headers={} }
*/
put «---- searching for files from unix/automate.sourcefiles ----»,nl
var i=0
do
	while ++i <= count source_files
	if !exists file (source_files[i])
		log source_files[i]," does not exist!",nl
		del source_files[i--]
		next
	then
	source_files[i] = { path = source_files[i], headers = {} }
loop

var num_sources = count source_files

/* ---- check source_tarball_files ----------------------
		list of additional source tarball files
		from "unix/automate.sourcefiles":
*/
var i=0
do
	while ++i <= count source_tarball_files
	if !exists file(source_tarball_files[i])
		log source_tarball_files[i], " does not exist!",nl
		del source_tarball_files[i--]
	then
loop




/* ----	utilities ----------------------------------
*/
proc ClipSpaces(s)
{
	var i=1, j=count s
	do while s[i]==" " || s[i]=="\t" i++ loop
	do while s[j]==" " || s[j]=="\t" j-- loop
	return s[i to j]
}

proc AddToSources(path)
{
	var i=0
	do
		while ++i <= count source_files
		if path == source_files[i].path return then
	loop
	source_files ##= { { path = path, headers={} } }
}




/* ----	scan file for #includes ----------------------
		add not-yet-seen headers to source_files[]
		return list of included headers
*/
proc FindHeaders(fpath)
{
	put nl,"  ",fpath
	var dpath = fpath[to rfind(fpath,"/")]

	try
		var f = file(fpath)
		convert f from utf8
		split f
	else
		put ": "
		log errorstr
		return {}
	then

// Schleife über alle Zeilen der Datei:
	var headers={}
	var i=0
	do
		while ++i <= count f
		var s = ClipSpaces(f[i])
		if s[to 8]!="#include" next then	// kein «#include»

		s = ClipSpaces(s[9 to])
		if s[1]!="\"" next then				// kein «#include "fname"»

		s = s[to find(s,"\"",2)]			// unquote include file name
		put nl,"    #include ",s
		convert s from quoted

		var j=0								// match against non-include file list
		do
			while ++j <= count non_includes
			until match(s,non_includes[j])
		loop
		if j <= count non_includes
			put "  (ignored)"
			next
		then

		if exists file(unixdir#s)			// search for header file in prodir/unix
			AddToSources(unixdir#s)
			headers ##=  unixdir#s
			next
		then

		if exists file(dpath#s)				// search in same dir as file
			AddToSources(dpath#s)
			headers ##=  dpath#s
			next
		then

		if exists file(s)					// search in prodir
			AddToSources(s)
			headers ##=  s
			next
		then

		if exists file(libdir#s)			// search in libdir
			AddToSources(libdir#s)
			headers ##=  libdir#s
			next
		then

		put ": " log "does not exist"
	loop

	return headers
}

put «---- scanning for #included header files ----»
var i=0
do
	while ++i <= count source_files
	source_files[i].headers ##= FindHeaders(source_files[i].path)
loop
put nl



/* ----	create "automate.tarball.source" -------------------------
*/
put «---- writing "automate.tarball.sources" ----»,nl

// add source_files and #included headers:
var i=0
do
	while ++i <= count source_files
	source_tarball_files ##= source_files[i].path
loop

// write list to file
openout #4, unixdir#"automate.tarball.source"
var i=0
do
	while ++i <= count source_tarball_files
	put#4, prodir, source_tarball_files[i], nl
loop
close#4









/* ========================================================
				write Makefile.in
======================================================== */

put «---- writing "Makefile.in" ----»,nl

/* ----	set flags -----------------------------------
*/
var ccFlagsStable   = "-fsigned-char -W -Wall -Wno-multichar -Wundef -Wcast-align -O3"
var ccFlagsBeta		= "-fsigned-char -W -Wall -Wno-multichar -Wundef -Wcast-align -O2"
var ccFlagsAlpha   	= "-fsigned-char -W -Wall -Wno-multichar -Wundef -Wcast-align -O1"
var ccFlagsDebug   	= "-fsigned-char -W -Wall -Wno-multichar -Wundef -Wcast-align -g"
var ldFlags    		= ""
var ccIncludes 		= "-I . -I .. -I ../" # libdir 
var ldLibs	   		= "-lstdc++"



openout #4, unixdir#"Makefile.in"

var text = «
#
# "Makefile.in" was created by script "make_config.vs" on »#releaseDate#« by »#releaseUser#«@»#releaseHost#«
#
BINNAME     	= »#applName#«
VERSION     	= »#applVersion#«
DATE			= `date +%Y-%m-%d`
TARGET			= `./config.guess`
USER			= `whoami`
HOST			= `hostname`

prefix      	= @prefix@
exec_prefix 	= @exec_prefix@
bindir      	= @bindir@
libdir      	= @libdir@
includedir  	= @includedir@
mandir      	= @mandir@

SHELL       	= @SH@
CC          	= @CC@
CCFLAGS_STABLE	= »#ccFlagsStable#«
CCFLAGS_BETA	= »#ccFlagsBeta#«
CCFLAGS_ALPHA	= »#ccFlagsAlpha#«
CCFLAGS_DEBUG	= »#ccFlagsDebug#«
CCINCLUDES		= »#ccIncludes#«
LDFLAGS     	= @LDFLAGS@ »#ldFlags#«
LIBS        	= @LIBS@ »#ldLibs#«
RM          	= @RM@ -f
TAR         	= @TAR@
STRIP       	= @STRIP@
TOUCH			= @TOUCH@

# APXS        	= @APXS@
# AP_OBJ      	= @APACHE_MODULE_OBJ@

INSTALL			= @INSTALL@
INSTALL_PROGRAM	= @INSTALL_PROGRAM@
INSTALL_DATA	= @INSTALL_DATA@

OBJECTS_STABLE	= %%OBJECTS_STABLE%%
OBJECTS_BETA	= %%OBJECTS_BETA%%
OBJECTS_ALPHA	= %%OBJECTS_ALPHA%%
OBJECTS_DEBUG	= %%OBJECTS_DEBUG%%
»

var o_stable="", o_alpha="", o_beta="", o_debug=""
var i=0
do
	while ++i <= num_sources
	var s = source_files[i].path
	s = s[rfind(s,"/")+1 to rfind(s,".")-1]
	o_debug  #= " " # s # ".d.o"
	o_alpha  #= " " # s # ".a.o"
//	if find(s,"test_suite") next then
	o_beta   #= " " # s # ".b.o"
	o_stable #= " " # s # ".s.o"
loop
replace text, "%%OBJECTS_STABLE%%",	o_stable[2 to]
replace text, "%%OBJECTS_BETA%%",	o_beta[2 to]
replace text, "%%OBJECTS_ALPHA%%",	o_alpha[2 to]
replace text, "%%OBJECTS_DEBUG%%",	o_debug[2 to]

convert text from tab4
put #4, text






put#4, «\n\n
# ====	Dependencies ===============================================
»

var dep_list={}
// create $(variable_name) for all dependencies of file "path":
proc dep_idf ( path )
{
	if !exists dep_list.@path
		dep_list ##= { @path = "DEP_" # count(dep_list) }
	then

	return dep_list.@path
}


// write $(variable_name) definitions for dependencies to Makefile:
var i=count source_files+1
do
	while --i >= 1

	var idf = dep_idf(source_files[i].path)
	put #4, idf," ="

	var j=0
	do
		while ++j <= count source_files[i].headers
		put#4," ../",source_files[i].headers[j]," $(",dep_idf(source_files[i].headers[j]),")"
	loop

	put#4,nl
loop




put#4,«\n\n
# =====	phony targets ========================================

# default make target: compile and link program
#
$(BINNAME): $(BINNAME)-beta
	cp $(BINNAME)-beta $(BINNAME)
	$(TOUCH) $(BINNAME)

$(BINNAME)-stable: Makefile $(OBJECTS_STABLE)
	$(CC) $(LDFLAGS) -o $(BINNAME)-stable $(LIBS) $(OBJECTS_STABLE)
	$(STRIP) $(BINNAME)-stable

$(BINNAME)-beta: Makefile $(OBJECTS_BETA)
	$(CC) $(LDFLAGS) -o $(BINNAME)-beta $(LIBS) $(OBJECTS_BETA)
	$(STRIP) $(BINNAME)-beta

$(BINNAME)-alpha: Makefile $(OBJECTS_ALPHA)
	$(CC) $(LDFLAGS) -o $(BINNAME)-alpha $(LIBS) $(OBJECTS_ALPHA)

$(BINNAME)-debug: Makefile $(OBJECTS_DEBUG)
	$(CC) $(LDFLAGS) -o $(BINNAME)-debug $(LIBS) $(OBJECTS_DEBUG)

mod_$(BINNAME).so: Makefile $(OBJECTS_BETA)
	$(CC) -c -Wl,"$(LDFLAGS)" -o mod_$(BINNAME).so $(LIBS) $(OBJECTS_BETA)


all: $(BINNAME)-stable $(BINNAME)-beta $(BINNAME)-alpha $(BINNAME)-debug $(BINNAME)
»




// ----	append dependencies ----------------------------------------

put#4, «\n\n
# ---- dependencies -------------------------------------\n\n»

var i=0
do
	while ++i <= count source_files
	var sfile = source_files[i].path
	var headers = source_files[i].headers

	if rightstr(sfile,3)==".cp"
		var dep = "../"#sfile # " $("#dep_idf(sfile)#")"

		var obj = sfile[rfind(sfile,"/")+1 to count sfile-3]

		put #4, obj#".s.o:", dep, nl
		put #4, "\t$(CC) -c -DSTABLE $(CCFLAGS_STABLE) $(CCINCLUDES) ../", sfile, " -o ",obj,".s.o\n\n"

		put #4, obj#".b.o:", dep, nl
		put #4, "\t$(CC) -c -DBETA $(CCFLAGS_BETA) $(CCINCLUDES) ../", sfile, " -o ",obj,".b.o\n\n"

		put #4, obj#".a.o:", dep, nl
		put #4, "\t$(CC) -c -DALPHA $(CCFLAGS_ALPHA) $(CCINCLUDES) ../", sfile, " -o ",obj,".a.o\n\n"

		put #4, obj#".d.o:", dep, nl
		put #4, "\t$(CC) -c -DDEBUG $(CCFLAGS_DEBUG) $(CCINCLUDES) ../", sfile, " -o ",obj,".d.o\n\n"

		next
	then

	if rightstr(sfile,2)==".h"
		next
	then

	put nl
	log "unknown extension: ", sfile
loop


/* ----	make version.h --------------------------------------------------------
		regexp rulez %-|
		wer an den regexp was ändert, testet bitte auch, wo das danach noch läuft!
*/
put#4,«\
../version.h: . ..
	cp ../version.h zzz
	sed -e "s/^\\(#define[ \\t]\\{1,\\}buildUser[ \\t]\\{1,\\}\\"\\)[-_a-zA-Z0-9]\\{1,\\}\\(\\".*\\)$$/\\1$(USER)\\2/" \\
		-e "s/^\\(#define[ \\t]\\{1,\\}buildHost[ \\t]\\{1,\\}\\"\\)[-_a-zA-Z0-9]\\{1,\\}\\(\\".*\\)$$/\\1$(HOST)\\2/" \\
		-e "s/^\\(#define[ \\t]\\{1,\\}buildDate[ \\t]\\{1,\\}\\"\\)[-_a-zA-Z0-9]\\{1,\\}\\(\\".*\\)$$/\\1$(DATE)\\2/" \\
		-e "s/^\\(#define[ \\t]\\{1,\\}buildTarget[ \\t]\\{1,\\}\\"\\)[-_a-zA-Z0-9]\\{1,\\}\\(\\".*\\)$$/\\1$(TARGET)\\2/" \\
	zzz > ../version.h
	rm zzz
\n»


/* ----	make source-dist ------------------------------------------------------
		tar ist auf fc1 (Fedora Core 1) ein gnutar
		tar ist auf macosx kein gnutar und kann kein -T filelistenfile
		das gnutar auf macosx beachtet -C ../.. anders als das (gnu)tar auf fc1
		make source-dist funktioniert wahrscheinlich nur auf linux-systemen
*/
put #4, «\n\n
# -----	create source distribution -----
#
source-dist: config
	mkdir  -p   ../distributions
	$(TAR) -chf ../distributions/$(BINNAME)-$(VERSION)-source-$(DATE).tar -T automate.tarball.source -C ../..
	bzip2  -f   ../distributions/$(BINNAME)-$(VERSION)-source-$(DATE).tar
»


/* ----	make binary-dist ------------------------------------------------------
		wir haben wieder das tar-problem.
		damit auch nicht-linux-binary-distributionen gebaut werden können,
		muss make binary-dist aber auf allen system laufen.
		-> wir hängen die file-liste auf der kommandozeile an
*/
if !exists file(unixdir#"automate.tarball.binary")
	log unixdir,"automate.tarball.binary does not exist",nl
	log «make target "binary-dist" omitted from Makefile.»,nl
else
	var f = file(unixdir#"automate.tarball.binary")
		split f
		convert f from utf8
		convert f to quoted
	var binary_dist_liste = ""
	var i=0
	do
		while ++i <= count f
		if f[i]!=«""» binary_dist_liste #= f[i]#" " then
	loop

	put#4, «\n\n
# ----- create binary distribution -----
binary-dist: $(BINNAME)
	mkdir  -p   ../distributions
	$(TAR) -chf ../distributions/$(BINNAME)-$(VERSION)-$(TARGET)-$(DATE).tar -C ../.. »#binary_dist_liste#«
	bzip2  -f   ../distributions/$(BINNAME)-$(VERSION)-$(TARGET)-$(DATE).tar
»
then


if exists install_binary_script
	install_binary_script = "../" # install_binary_script
else
	install_binary_script = "$(INSTALL_PROGRAM) -c -m 755 $(BINNAME) $(bindir)/$(BINNAME)"
then

put#4, «\n\n
# ----- install application ------------
# priviledged operation; requires su / sudo!
#
install: $(BINNAME)
	»#install_binary_script#«


# ----- clean-up -----------------------
#
clean:
	$(RM) *.o *~ ../*~ *.log $(BINNAME) $(BINNAME)-debug $(BINNAME)-alpha $(BINNAME)-beta $(BINNAME)-stable config.status


# ----- run test suites ----------------
# ((BROKEN!))
#
test: $(BINNAME)
	./$(BINNAME) -v -t
»


/*
	autoconf/Makefile dependencies.               kio 2002-05-07
	based on the following diagram:
	(MacOS X dev docs for autoconf)

	Files used in preparing a software package for distribution:

	your source files --> [autoscan*] --> [configure.scan] --> configure.in

	configure.in --.   .------> autoconf* -----> configure
	               +---+
	[aclocal.m4] --+   `---.
	[acsite.m4] ---'       |
	                       +--> [autoheader*] -> [config.h.in]
	[acconfig.h] ----.     |
	                 +-----'
	[config.h.top] --+
	[config.h.bot] --'

	Makefile.in -------------------------------> Makefile.in

	Files used in configuring a software package:

					       .-------------> config.cache
	configure* ------------+-------------> config.log
	                       |
	[config.h.in] -.       v            .-> [config.h] -.
	               +--> config.status* -+               +--> make*
	Makefile.in ---'                    `-> Makefile ---'
*/

put #4, «\n\n
# -----	create configuration files -----

# phony target:
config:
	./make_config.vs

# dependencies:
#configure: aclocal.m4 configure.in
#	autoconf
#
#config.h.in: aclocal.m4 configure.in
#	autoheader
#
#Makefile.in: make_config.vs automate.sourcefiles
#	./make_config.vs

config.status: configure config.h.in Makefile.in
	./configure

config.h: config.status
	./config.status
	$(TOUCH) config.h

Makefile: config.status
	./config.status
	$(TOUCH) Makefile
»



close #4


cd unixdir
put "---- running ./configure ----",nl
sys "./configure"

end 0




















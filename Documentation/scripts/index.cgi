#!/home/smile77/bin/vipsi


/*	show beautified directory contents

	url:	/				-> show DOCUMENT_ROOT/index.ht
			/Pub/PATH		-> show DOCUMENT_ROOT/PATH
			/PATH			-> show DOCUMENT_ROOT/PATH
			/PATH/PATTERN	-> show DOCUMENT_ROOT/PATH: only files matching pattern

	optionally includes "info.vh" to customize page
*/


/* ----	send http header ----------------------------------------------------
*/
put	"content-type: text/html; charset=utf-8\n\n"








/* ----	evaluate env data -------------------------------------------------
		http_host		=	eden.kio.lan
		root_dir		=	/www_root/
		request_uri		=	/Pub/RPG/Fantasy/
*/
var http_host	 = env.HTTP_HOST				//	"eden.kio.lan"


var root_dir	 = env.DOCUMENT_ROOT			//	"/www_root/"
	root_dir 	 = fullpath(root_dir) # "/"		//	resolve potential symlinks
	replace root_dir,"//","/"


var request_uri	 = env.REQUEST_URI # "/"		//	"/Pub/RPG/Fantasy/file?mask*pattern"
	replace request_uri,"//","/"


var dir_part_path = request_uri[2 to]					//	"RPG/Fantasy/"
	convert dir_part_path from url						//	"RPG/Fantasy/"

var dir_path = root_dir # dir_part_path					//	"/www_root/RPG/Fantasy/"




/* ==== show root: "/" ==================================================
		ignore pattern, show start page and quit
*/
if request_uri == "/"
	var d = dir( root_dir # "*index.ht*" )
	if count d && d[1].flags[1]=="-"
		var f = file ( root_dir # d[1].fname )
		convert f from utf
		put f
		end 0
	then
then



/* ====	default customization ===========================================
*/
if find(root_dir,"smile77")
// ----	LITTLE-BAT.DE ----------------
	var logo = «<img alt="little bat" style="background-color:#88CC88;" \
				width=128 height=128 src="/b/bat_sw_128_frei_v2.png">»

	var icon_dir_url = "/b/icon/"					// where are the generic file icons files?
	var temp_dir_url = "/tmp/"						// where to store the thumbnails?
else
// ----	EDEN @ HOME ----------------
	var logo = «<img alt="little bat" style="background-color:#88CC88;" \
				width=128 height=128 src="/www/b/bat_sw_128_frei_v2.png">»

	var icon_dir_url = "/www/b/icon/"					// where are the generic file icons files?
	var temp_dir_url = "/www/tmp/"						// where to store the thumbnails?
then




var	forbidden_folders = { "/cgi-bin/", "/tmp/", "/CVS/" }	// actually match "*/cgi-bin/*" etc.


var zip_icon = "tgz.png"		// archives
var avi_icon = "video.png"		// video
var mp3_icon = "sound.png"		// audio
var img_icon = "images.png"		// images			-> replaced by image preview
var gfx_icon = "vectorgfx.png"	// graphics
var src_icon = "source.gif"		// sources
var txt_icon = "txt.png"		// plain texts

var icon_info = {
	{ "/",		"folder.gif"},

	{ ".txt",	txt_icon	},
	{ ".doc",	txt_icon	},
	{ ".rtf",	txt_icon	},
	{ ".md5",	txt_icon	},
	{ ".log",	txt_icon	},
	{ ".h",		src_icon 	},
	{ ".c",		src_icon 	},
	{ ".cp",	src_icon 	},
	{ ".cpp",	src_icon 	},
	{ ".mm",	src_icon 	},
	{ ".asm",	src_icon 	},
	{ ".ass",	src_icon 	},
	{ ".vs",	"script.gif"},
	{ ".vh",	"script.gif"},
	{ ".cgi",	"script.gif"},
	{ ".sh",	"script.gif"},
	{ ".m4",	"script.gif"},
	{ ".htm",	"html.gif"	},
	{ ".html",	"html.gif"	},

	{ ".zip",	zip_icon	},
	{ ".sit",	zip_icon	},
	{ ".gz",	zip_icon	},
	{ ".bz2",	zip_icon	},
	{ ".tgz",	zip_icon	},
	{ ".rpm",	zip_icon	},
	{ ".arj",	zip_icon	},
	{ ".tar",	zip_icon	},
	{ ".arj",	zip_icon	},
	{ ".z",		zip_icon	},

	{ ".jpg",	img_icon	},
	{ ".jpeg",	img_icon	},
	{ ".gif",	img_icon	},
	{ ".png",	img_icon	},
	{ ".tif",	img_icon	},
	{ ".tiff",	img_icon	},

	{ ".mp3",	mp3_icon	},
	{ ".aif",	mp3_icon	},
	{ ".mod",	mp3_icon	},
	{ ".aiff",	mp3_icon	},
	{ ".wav",	mp3_icon	},
	{ ".mid",	"midi2.png"	},
	{ ".midi",	"midi2.png"	},

	{ ".avi",	avi_icon	},
	{ ".mov",	avi_icon	},
	{ ".mpeg",	avi_icon	},
	{ ".mpg",	avi_icon	},
	{ ".fli",	avi_icon	},

	{ ".exe",	"kwrite.png"},
	{ ".hex",	"binary.png"},
	{ ".bin",	"binary.png"},
	{ ".sys",	"binary.png"},

	{ ".pdf",	"pdf.png"	},
	{ ".eps",	gfx_icon 	},
	{ ".ps",	gfx_icon 	},
	{ ".ai",	gfx_icon 	},
	{ ".art",	gfx_icon 	},	// Freehand's version of ".ai"
	{ ".fh2",	gfx_icon	},
	{ ".fh3",	gfx_icon 	},
	{ ".fh5",	gfx_icon 	},
	{ ".fh7",	gfx_icon 	},
	{ ".fh8",	gfx_icon 	},
	{ ".fh9",	gfx_icon 	},
	{ ".fmz",	"make.png" 	},
	{ ".c4d",	"make.png"	},

	{ ".pm4",	"wordprocessing.png"	},
	{ ".pm5",	"wordprocessing.png"	},
	{ ".ppt",	"ppt.gif"	},
	{ ".pps",	"ppt.gif"	},

	{ ".sna",	"sna.gif"	},
	{ ".tap",	"tap.gif"	},
	{ ".tape",	"tap.gif"	},
	{ ".z80",	"sna.gif"	},
	{ ".scr",	"scr.gif"	},
	{ ".rom",	"rom.gif"	},

	{ "",    	"empty.png"	}		// catch all: generic file
}


/* ----	default header texts --------------------------------------
*/
var head_robots = "noindex,nofollow"	// noindex,index,nofollow,follow,noarchive,archive
var head_description = ""
var head_keywords = ""
var head_title = «Verzeichnis "»#convert(request_uri from url)#«"»

/* ----	default body texts --------------------------------------
*/
var body_data = «head\nmain\nfoot\n»


/* ----	include local customization --------------------------------
*/
if exists file(dir_path#"info.vh")
	try
		include dir_path#"info.vh"
	else
		convert errorstr to html
		body_data = «p "info.vh" yielded an error: » # errorstr # nl # body_data
	then
then


















var dname = convert(request_uri from url)		// url_decoded url

var icon_dir_path = root_dir # icon_dir_url[2 to]
var temp_dir_path = root_dir # temp_dir_url[2 to] # dir_path


/* create hidden mirror directory tree for thumbnails
*/
if !exists dir temp_dir_path
	var i=0
	do
		while i = find(temp_dir_path,"/",i+1)
		if exists dir temp_dir_path[to i] next then
		try
			new dir temp_dir_path[to i]
		else
			temp_dir_path = ""					// failed to create temp_dir_path
		then
	loop
then



/* ----	html header ----------------------------------------------------
*/
put
«<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html><head>
<META http-equiv=Content-Type content="text/html; charset=UTF-8">
<title>», convert(head_title to html), «</title>», nl

if head_robots!=""
	put «<meta name="robots" content="», head_robots, «">», nl
then

if head_description!=""
	convert head_description to html
	replace head_description, "\n", "<br>"
	put «<meta name="description" content="», head_description, «">», nl
then

if head_keywords!=""
	convert head_keywords to html
	put «<meta name="keywords" content="», head_keywords, «">», nl
then

put
«<style type="text/css"><!--
body,div,table,tr,th,td,h1,h2,h3,h4,h5,p,pre,img {padding:0px;margin:0px;font-size:12pt;}
h1,h2,h3,h4,h5,h6,p,pre {padding-top:0.2em; padding-bottom:0.2em;}
h1,h2,h3,h4 {text-align:center; padding-top:0.4em;}
h1 {font-size:20pt;}
h2 {font-size:18pt; padding-top:0.2em;}
h3 {font-size:16pt;}
h4 {font-size:14pt;}
body {background-color:#dddddd; font-family:sans-serif;}
pre {border-left-style:solid; border-left-width:2px; border-left-color:#000099; padding-left:8px;}
--></style>
</head><body>», nl




proc icon ( fname )
{
	var lower_fname = lowerstr(fname)
	var i=0
	do
		++i
		until rightstr(lower_fname,count icon_info[i,1])==icon_info[i,1]
	loop

	var icon = icon_info[i,2]

	if icon!=img_icon || temp_dir_path==""
		return «<img width=32 height=32 alt="" src="» # icon_dir_url # icon # «">»
	then

// create preview

	var pname = fname;
	if find(".jpg/jpeg"/*".jpg/jpeg/.gif/.png"*/,rightstr(lower_fname,4)) == 0 pname #= ".jpg" then

	if !exists file(temp_dir_path#pname)
		try
			var n = split( sys( "identify", dir_path#fname ) )
			n = count n>2 ? "-flatten" : {}
			var para = { "-size", "160x80", dir_path#fname } ## n
			        ## { "-resize", "160x80", "+profile", "*", temp_dir_path#pname }
			sys "convert", para
		then
	then

// convert seems to never fail -> check for thumbnail file created

	if !exists file(temp_dir_path#pname)
			sys "ln", icon_dir_path#icon, temp_dir_path#pname
	then

// display preview

	return «<img alt="" src="» # convert(temp_dir_url # dir_path[2 to] # pname to url) # «">»
}


proc show_file ( dpath, dirent )
{
	var fname        = dirent.fname
	var href_link    = dirent.fname
	var visible_link = dirent.fname
	var flags		 = dirent.flags
	var mtime  		 = dirent.mtime
	var length 		 = dirent.length

// deref symlink:
	if flags[1]=="l"
	{
		try
			var fpath = fullpath(dpath#fname)
			if isdir fpath && rightstr(fpath,1)!="/" fpath#="/" then
			if fpath[to count root_dir] != root_dir return then		// won't be accessible

			// get link target's parent dir:
			var i = rfind(fpath,"/",count(fpath)-1)
			fname = fpath[i+1 to]
			dpath = fpath[to i]

			// get deref'ed dirent:
			var ddir = dir dpath
			var i=0
			do
				while ++i<=count ddir
				if fname!=ddir[i].fname && fname!=ddir[i].fname#"/" next then
				dirent = ddir[i]
				exit
			loop

		//	fname        = dirent.fname
			href_link 	 = fpath[count root_dir to]
		//	visible_link = dirent.fname
			flags		 = dirent.flags
			mtime  		 = dirent.mtime
			length 		 = dirent.length

		else
			return
		then
	}
	then

// trap forbidden folders:
	if flags[1]=="d"
		if rightstr(fname,1)!="/" fname #= "/" then
		var i=0
		do
			while ++i <= count forbidden_folders
			if fname == forbidden_folders[i,2 to] return then
		loop
	then

// trap general don't-show files:
	if fname[1]=="." return then
	if rightstr(fname,1)=="~" return then
	if rightstr(fname,4)==".alt" return then
	if rightstr(fname,4)==".bak" return then
	if rightstr(fname,4)==".cgi" return then
	if fname=="info.vh" return then
	if fname[to 6]=="index." return then

// finally: print it:
	convert href_link to url
	convert href_link to quoted
	convert visible_link to escaped
	convert visible_link to html
	mtime = datestr(mtime)[to 16]
	put
	«<tr valign=middle>
	<td align=center style="width:32px;padding:1px;">» ,icon(fname), «</td>
	<td><a href=» ,href_link, «>» ,visible_link, «</a></td><td></td>
	<td align=center>» ,mtime, «</td><td></td>
	<td align=right>» ,length, «&nbsp;</td>
	</tr>\n»
}


/* ---------- Headline -------------
*/
proc put_backlinks ( url )
{	var text,i

	if rightstr(url,1)=="/" url = url[to count url -1] then

	if url==""
		text = http_host					// my.domain.net
		if text[1]<="9"	text = "eden.kio.lan" then
		url  = http_host					// my.domain.net
	else
		i = rfind(url,"/")
		put_backlinks ( url[to i-1] )
		text = url[i+1 to]					// path
		url  = http_host # url # "/"		// my.domain.net/my/partial/path/
	then

	put "<a href=\"http://",convert (url to url),"\">",text,"</a> / \n"
}



proc put_body_head()
{
	put
	«<table border=0 width="100%" cellpadding=0 cellspacing=0>
	<tr valign=top>
	<td style="background-color:#88CC88;width:128px;height:128px;" valign=top rowspan=2>» ,logo, «</td>
	<td style="background-color:#99ff99;height:64px;" valign=middle align=center><h2>» ,dname, «</h2></td>
	</tr>
	<tr valign=top>
	<td style="background-color:#ccff99;height=64px;" valign=middle align=center>»
	put_backlinks(dname)
	put «</td></tr></table>»,nl

	put «<div style="margin-left:10%; margin-right:10%;">»
}


proc put_body_main(dir_path,dir_part_path)
{
	put «</div>»,nl

	put
	«<div align=center><table border=0 cellpadding=0 cellspacing=0 style="background-color:#cccccc;">
	<tr valign=middle>
		<th width=40 height=32></th><th align=left>Name</th><th width=16></th>
		<th align=center>Letzte Änderung</th><th width=32></th><th align=right>Länge&nbsp;</th>
		</tr>», nl

	// read directory
	var directory = {}
	try directory = dir dir_path then
	sort directory

	// if this is a forbidden directory, then just clear the list
	var i=0
	do
		while ++i <= count forbidden_folders
		until find(dir_part_path,forbidden_folders[i])
	loop
	if i <= count forbidden_folders
		directory = {}
	then


	var i=0
	do
		while ++i <= count directory
		show_file( dir_path, directory[i] )
	loop

	put "</table></p>",nl

	put «<div style="margin-left:10%; margin-right:10%;">»
}


proc put_body_foot()
{
	put
	«<p style="margin-top:0.5em; text-align:center; font-size:9pt;">
	powered by <a href="http://vipsi.sourceforge.net">vipsi</a> - your friendly VIP Script Interpreter
	</p>»

	put «</div>»,nl
}


proc put_tagged(tag,text)
{
	convert text to html
	put "<",tag,">", text, "</",tag,">",nl
}









split body_data
var i=0
do
	while ++i <= count body_data
	var zeile = body_data[i]
	if zeile=="" next then
	if zeile[1]<=" " next then
	if zeile=="head" put nl,"<!--head-->",nl put_body_head() next then
	if zeile=="foot" put nl,"<!--foot-->",nl put_body_foot() next then
	if zeile=="main" put nl,"<!--main-->",nl put_body_main(dir_path,dir_part_path) next then

	var j=0 do while ++j<=count zeile && zeile[j]>" "  loop
	var k=j do while ++k<=count zeile && zeile[k]<=" " loop

	if k>count zeile next then
	put_tagged( zeile[to j-1], zeile[k to] )
loop


put"
</body></html>
"





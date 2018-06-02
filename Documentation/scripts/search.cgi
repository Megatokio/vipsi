#!/home/smile77/bin/vipsi


// "search site" script as used on my homepage
// -------------------------------------------

// please adjust paths ... B-)



// the search pattern passed to the script (must exist):
	if !exists(pattern) var pattern = "" then


// file system base expressed as http and file
	var file_root = env.DOCUMENT_ROOT # "/"
	var http_root = "http://" # env.SERVER_NAME # "/"


// little patch so i can use it at home:
	if env.SERVER_NAME=="_default_"
		http_root = "http://" # env.HTTP_HOST # "/"
	then
	if env.SERVER_ADDR=="127.0.0.1" || env.SERVER_ADDR=="10.0.0.4"
		file_root #= "www/little-bat.de/"
		http_root #= "www/little-bat.de/"
	then

// http and html header:
	put "content-type: text/html",nl,nl
	put "<!doctype html public \"-//w3c//dtd html 4.01 strict//en\"><html><head>"
	put "<META http-equiv=Content-Type content='text/html; charset=UTF-8'>"
	put "<title>search results for '"#pattern#"'</title></head>",nl
	put "<body background='"#http_root#"b/hdgr.jpg' bgcolor='#CCCCCC' ",
		"text=black link=black vlink=blue alink=black>",nl
		

// test for magic parameter to show my own source:
	if exists showsource
		var myfile = env.SCRIPT_FILENAME
		var mysource = file(myfile)
		convert mysource to html
		put "<pre>",nl, mysource, nl,"</pre></body></html>",nl
		end
	then


// Procedure: Print usage instructions
	proc DisplayHelp(errmsg)
	{
		put 
		"<p align=center>&nbsp;",
		"<br><b>",errmsg,"</b>",
		"<br>&nbsp;",
		"<table border=1 width='95%' align=center cellpadding=2>",
		"<tr><th align=center>",
			"Help for Search Site",
		"</th></tr>",nl,
		
		"<tr><td align=center>",
			"<table border=0><tr><td>",
				"<ul>",
				"<li>The search pattern must be at least 3 characters long.</li>",
				"<li>The search is not case sensitive.</li>",
				"<li>The search pattern must match exactly the text in a file.",
				"<br>If it contains multiple words, then they must appear 'as written'.</li>",
				"<li>Use the wildcards '?' and '*' to match any single character or",
				"<br>any sequence of non-significant characters.</li>",
				"</ul>",
			"</td></tr></table>",
		"</td></tr>",nl,

		"<tr><td align=center>",
			"e.g. to search for files which contain '<b>foo</b>' and '<b>bar</b>' search for '<b>foo*bar</b>'.",
		"</td></tr></table>",
		"<p>&nbsp;",nl
	}
	

// Procedure: search all files in directory "path"
// calls itself recursively for all subdirectories
	proc Search(path)
	{
		if rightstr(path,1)!="/" path#="/" then
		var folder = dir(file_root#path)
		var i = 0
		do
			while ++i<=count(folder)

			var item  = folder[i]
			var fname = item.fname
			if item.flags[1]=="d" && rightstr(fname,1)!="/" fname#="/" then


			if fname[1]=="." next then			// skip hidden files
			if fname=="b/" next then			// skip images folders
			if fname=="stats/" next then		// skip stats folder (requires password anyway)

			if item.flags[1]=="d"				// recursively handle folder
				Search(path#fname) next
			then
			
			var ext = fname[ rfind(fname,".") to ]
			if !find(".html.hh.cpp.txt",ext) next then	// no text file
			
			var datei = lowerstr(file(file_root#path#fname))
			files_scanned += 1
			bytes_scanned += count(datei)
			if !match(datei,pattern) next then			// not found

			var a=find(datei,"<title>")					// found: display file
			var e=find(datei,"</title>")
			var title = a&&e ? datei[a+7 to e-1] : "<i>no title</i>"
			Display(path,fname,title)
		loop
	}


// Procedure: build a link:
	proc Link(url,text)
	{
		return "<a href='" # url # "'>" # text # "</a>"
	}


// Procedure: print table row for file:
	proc Display(path,fname,title)
	{
		files_found++
		
		convert path to html
		convert fname to html
		var url = http_root # path # fname;

		put "<tr><td>", Link(url,path#fname), "</td><td>", title, "</td></tr>",nl
	}
	

// statistics counters
	var files_scanned = 0
	var bytes_scanned = 0
	var files_found   = 0

	
// print table start
	put 
	"<p>&nbsp;<table border=1 width='95%' align=center cellpadding=2>",
	"<tr><th bgcolor='#ffffcc' colspan=2 align=center style='font-size:150%;'>Search results for '", pattern, "'</th></tr>",
	"<tr><th bgcolor='#00CCCC'>file</th><th bgcolor='#00CCCC'>title</th></tr>\n"
	

// search & print table rows	
	if count(pattern)>=3
		pattern = "*" # lowerstr(pattern) # "*"
		Search("")
	then


// print table end
	put "</table>",nl


// if no files found print additional help
	if files_found
		put
		"<p align=center>&nbsp;",
		"<br>Scanned ",files_scanned,
		" files (",bytes_scanned,
		" bytes) and found <b>",files_found,
		"</b> file","s"[to files_found>1],".</p>\n"
	else
		if count(pattern)==0 DisplayHelp("Search failed. (no search pattern given)")   else
		if count(pattern)<3	 DisplayHelp("Search failed. (search pattern too short)")  else
							 DisplayHelp("No files found.");
		then then
	then


// print info on vipsi
	put
	"<p>&nbsp;",
	"<table border=1 width='95%' align=center cellpadding=2>",
	"<tr><td align=center>\n",
	Link( "http://" # env.HTTP_HOST # env.SCRIPT_NAME # "?showsource=yes", "view script" ),
	" &nbsp; powered by vipsi - vipsi shell and script language &nbsp; ",
	Link( "http://vipsi.sourceforge.net' target='_blank","about vipsi" ),
	"\n</td></tr></table>\n"


// print end of html page	 
	put "<p>&nbsp;</body></html>",nl	 
	end




















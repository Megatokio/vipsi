#!/home/smile77/bin/vipsi


/* --------------------------------------------------------------------------------------------
			ONLINE ZASM Z80 ASSEMBLER
-------------------------------------------------------------------------------------------- */

var argc = count globals -1		// for proc ShowCommandLineArguments()



/*	zasm.cgi				 			=> show main page w/ info
	zasm.cgi?what="assemble"&src={..}	=> assemble, show main page w/ links to results
	zasm.cgi?what="sdjhgsadfg.log"		=> send named file (text) == assembler listing
	zasm.cgi?what="sdjhgsadfg.bin"  	=> send named file (bina) == assembler output
	zasm.cgi?what="sdjhgsadfg.rom"  	=> send named file (bina) == assembler output
	zasm.cgi?what="sdjhgsadfg.tap"  	=> send named file (bina) == assembler output
	zasm.cgi?what="sdjhgsadfg.z80"  	=> send named file (bina) == assembler output
*/
if !exists what		var what = "main page"		then		// "main page", "assemble", <filename>



// ---- sanity check & enforce existance of form data ----

if !exists listing  			var listing = ""		then		// "", "-1", "-2"
if !exists verbose  			var verbose = ""		then		// "", "-v"
if !exists labels   			var labels  = ""		then		// "", "-w"
if !exists encoding				var encoding= "-b"		then		// "-b", "-x"
if listing=="" && labels!=""	listing="-2"  			then
if listing=="" && verbose!=""	listing="-2"  			then

if !exists src.filename || !exists src.data	var src = { filename="", data="" }	then		// source file
if !exists inc.filename || !exists inc.data	var inc = { filename="", data="" }	then		// optional include file


if exists dir "/pub/www/tmp/zasm/"			// eden@home
	cd "/pub/www/tmp/zasm/"
	var tempdir = "./"
	var htmltempdir = "/www/tmp/zasm/"
	var zasm = "/usr/bin/zasm"
	var htmlbdir = "/www/b/"
else										// little-bat
	cd "/home/smile77/htdocs/tmp/zasm/"
	var tempdir = "./"
	var htmltempdir = "/tmp/zasm/"
	var zasm = "/home/smile77/bin/zasm"
	var htmlbdir = "/b/"
then


var email_link =
	«<a href="mailto:kio@little-bat.de">mailto:kio</a>»

var disclaimer =
	«<a href="zasm.cgi?what=disclaimer">disclaimer</a>»

var vipsi_link =
	«powered by <a target="X" href="http://vipsi.sourceforge.net">vipsi</a> - your friendly VIP Script Interpreter»

var z80_links = «\
	<a target="X" href="http://little-bat.de/cpc/">Das Schneider CPC Systembuch</a> &nbsp;
	<a target="X" href="http://little-bat.de/cpc/index.html#/cpc/6.3_z80tab.html">Z80 instruction set</a> &nbsp; [German] [little-bat.de]»

var zasm_links = «\
	<a target="X" href="http://k1.spdns.de/Develop/projects/zasm/doc/">documentation</a> &nbsp;
	<a target="X" href="http://k1.spdns.de/Develop/projects/zasm/distributions/">download page</a> &nbsp; [k1.spdns.de]
	<br>
	<a target="X" href="http://sourceforge.net/projects/zasm/">zasm project page</a> &nbsp;
	<a target="X" href="http://sourceforge.net/project/showfiles.php?group_id=53490">download page</a> &nbsp; [sourceforge.net]
	<br>
	<a target="X" href="http://little-bat.de">little-bat.de</a> &nbsp;
	<a target="X" href="http://little-bat.de/prog">download page</a> &nbsp; [little-bat.de]
	»



var styles = «
<style type="text/css">
/*page body*/           body           { font-family:sans-serif; background-image:url(»#htmlbdir#«hdgr.jpg); }
/*main title*/          h1             { font-size:20pt; line-height:150%; padding:0pt; margin:0pt; margin-bottom:0.3em; }
/*push file form*/      table.pushfile { border:ridge #88cc88; padding:20px; background-color:#ccff99; }
/*kleingedrucktes*/     .small         { text-align:center; vertical-align:middle; font-size:9pt; }
</style>
»




/* ----	show table of command line arguments to script ----------------------------------------
*/
proc ShowCommandLineArguments()
{
	if argc==0
	//	put «
	//		<h4>command line arguments</h4>
	//		<p>--none--</p>
	//	»
		return
	then

	put «
		<h4>command line arguments</h4>
		<table border width="100%">
			<tr>
				<td width="10%"><b>index</b></td>
				<td width="20%"><b>name</b></td>
				<td><b>value</b></td>
			</tr>
	»

	var i = 0
	do
		while ++i<=argc
		var gname = name globals[i]
		var gdata = globals[i]
		if istext(gdata) convert gdata to quoted; then convert gdata to html
		convert gname to printable; convert gname to html
		if gname=="" gname = "<i>unnamed</i>" then
		put «
			<tr>
				<td>globals[»,i,«]</td>
				<td>», gname, «</td>
				<td>», gdata, «</td>
			</tr>
		»
	loop

	put «
		</table>
	»
}




/* ----	show table of environment variables -----------------------------------
*/
proc ShowEnvironmentVariables()
{
	put «
		<h4>environment variables</h4>
		<table border width="100%">
			<tr>
				<td width="10%"><b>index</b></td>
				<td width="20%"><b>name</b></td>
				<td><b>value</b></td>
			</tr>
	»

//	in order to sort by name we convert the named items of env
//	into sub-lists of form { name, data }

	var env_var = env
	var i = 0
	do
		while ++i <= count env_var
		env_var[i] = { name env_var[i], env_var[i] }
	loop

	sort    env_var						// sort the list

	var i = 0
	do
		while ++i <= count env_var
		var ename = env_var[i,1] convert ename to printable convert ename to html
		var edata = env_var[i,2] convert edata to quoted    convert edata to html
		if ename=="" ename = "<i>unnamed</i>" then
		put «
			<tr>
				<td>env[»,i,«]</td>
				<td>», ename, «</td>
				<td>», edata, «</td>
			</tr>
		»
	loop

	put «
		</table>
	»
}




/* ----	send push-file form --------------------------------------------------
*/
proc ShowPushFileForm()
{
	var checked0 = listing==""   ? "checked" : ""
	var checked1 = listing=="-1" ? "checked" : ""
	var checked2 = listing=="-2" ? "checked" : ""
	var checkedv = verbose==""   ? "" : "checked"
	var checkedw = labels==""    ? "" : "checked"
	var checked_e1 = encoding=="-b" ? "checked" : ""
	var checked_e2 = encoding=="-x" ? "checked" : ""

	put «
		<form action="zasm.cgi" enctype="multipart/form-data" method="post">
			<table class="pushfile">
				<tr>
					<td>Source:
					<td><input type="file" name="src" size="50" maxlength="200000" accept="text/*">
					<td><input type="submit" value="    Assemble    ">
						<!--input type="reset" value="Reset form"-->
						<input type="hidden" name="what" value="assemble">
				</tr>
				<tr valign=top>
					<td>Include:<span style="font-size:66%;"><br>(optional)</span>
					<td><input type="file" name="inc" size="50" maxlength="200000" accept="text/*">
					<td>&nbsp;
				</tr>
				<tr>
					<td>Output:
					<td><input type="radio" name="encoding" value="-b" »#checked_e1#«>Binary
						<input type="radio" name="encoding" value="-x" »#checked_e2#«>Intel Hex
					</td>
				</tr>
				<tr>
					<td>Listing:
					<td colspan=2><input type="radio" name="listing" value=""   »#checked0#«>None
						<input type="radio" name="listing" value="-1" »#checked1#«>Pass 1
						<input type="radio" name="listing" value="-2" »#checked2#«>Pass 2
					&nbsp; &nbsp;
					<input type="checkbox" name="verbose" value="-v" »#checkedv#«>Include generated code
					&nbsp; &nbsp;
					<input type="checkbox" name="labels" value="-w" »#checkedw#«>Append label listing
				</tr>
			</table>
		</form>
	»
}




/* ---- HEADER --------------------------------------------------
*/
proc PutHtmlHeader()
{
	put «content-type: text/html; charset=utf-8», nl, nl,
		«<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">», nl,
		«<html><head><META http-equiv="Content-Type" content="text/html; charset=UTF-8">», nl,
		«<title>zasm - z80 online assembler</title>», nl,
		styles, nl,
		«</head><body>», nl, nl

	// alles-umfassende Tabelle
	put «<table align="center"><tr><td>», nl

	put «<h1>zasm - z80 online assembler</h1>», nl
}




/* ----------- Footer -------------------
*/
proc PutHtmlFooter()
{
	put «
	<table width="100%" style="margin-top:1em; border-top:solid thin; padding-top:0.5em;">
		<tr><td class="small">»,
			disclaimer, « &nbsp; », email_link, «<br><br>»,
			z80_links, «<br><br>»,
			zasm_links, «<br><br>»,
			vipsi_link, «
		</td></tr>
	</table>»

	// ende der alles umfassenden tabelle:
	put nl, «</td></tr></table>», nl

	put «</body></html>», nl
}




/* ----	send initial page ----------------------------------------------------
		show main page w/ info
*/
if what=="main page"
	PutHtmlHeader()
	ShowPushFileForm();
	put «<p>welcome to the online version of zasm - the z80 assembler.</p>», nl
//	ShowCommandLineArguments();
//	ShowEnvironmentVariables();
	PutHtmlFooter()
	end 0
then


/* ----	send disclaimer ----------------------------------------------------
		show legal stuff
*/
if what=="disclaimer"
	PutHtmlHeader()
	ShowPushFileForm();
	put «
	<table><tr><td>

<h4>Copyright</h4>
	<p>Copyright (c) Günter Woigk 1994-2004<br>
	<a href="mailto:kio@little-bat.de">kio@little-bat.de</a></p>

<h4>No warranties</h4>
	<p>This program is free software.</p>

	<p>This program is distributed in the hope that it will be useful,
 	but WITHOUT ANY WARRANTY; without even the implied warranty of
 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.</p>

 	<p>Permission to use, copy, modify, distribute, and sell this software and
 	its documentation for any purpose is hereby granted without fee, provided
 	that the above copyright notice appear in all copies and that both that
 	copyright notice and this permission notice appear in supporting
 	documentation, and that the name of the copyright holder not be used
 	in advertising or publicity pertaining to distribution of the software
 	without specific, written prior permission.  The copyright holder makes no
 	representations about the suitability of this software for any purpose.
 	It is provided "as is" without express or implied warranty.</p>

 	<p>THE COPYRIGHT HOLDER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 	INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 	EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 	CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 	DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 	TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 	PERFORMANCE OF THIS SOFTWARE.</p>

<h4>Non-disclosure agreement</h4>
	<p>There is no agreement on non-disclosure for any files assembled online.</p>

	</td></tr></table>
	», nl
	PutHtmlFooter()
	end 0
then




/* ----	decode MacBinary --------------- [kio 2004-05-24]
		$0000 - $0080: viele Nullen
		"mBIN" und "TEXT" sollten vorkommen
		außerdem der Dateiname. naja.
		am Ende gibts dann auch Gestrüpp,
		vermutlich die Resource-Fork.
*/
proc DecodeMacBinary(Filename,Text)
{

	if !rfind(Text,charstr(0),$80) return Text then
	var n=0, i=0
	do while ++i<=$80 n += Text[i]==charstr(0) loop
	n = (n>=$20) + !!rfind(Text,"mBIN",$80) + !!rfind(Text,"TEXT",$80) + !!rfind(Text,Filename,$80)
	if n>=3	// crop MacBinary stuff
		Text = Text[$81 to]
		Text = Text[to find(Text,charstr(0))-1]
		var e1=rfind(Text,charstr(13))
		var e2=rfind(Text,charstr(10))
		Text = Text[to max(e1,e2)]
	then
	return Text
}



/* ----	assemble ---------------------
*/
if what=="assemble"
	if !exists(src.filename) || src.filename=="" || !exists(src.data) || src.data==""
		PutHtmlHeader()
		ShowPushFileForm();
		put «<p style="color:red;">Please select a source file before clicking on [Assemble].</p>», nl
//		ShowCommandLineArguments();
//		ShowEnvironmentVariables();
		PutHtmlFooter()
		end 0

	else
		PutHtmlHeader()
		ShowPushFileForm();

		var filebasename = src.filename[to rfind(src.filename,".")-1]
		var tempdirlist = dir(tempdir#filebasename#".*")
		var i=0
		do
			while ++i <= count tempdirlist
			del file(tempdir#tempdirlist[i].fname)
		loop
		file(tempdir#src.filename) = DecodeMacBinary(src.filename,src.data);

		if inc.filename!=""
			file (tempdir#inc.filename) = DecodeMacBinary(inc.filename,inc.data);
		then

		var fname_html = src.filename
		convert fname_html to quoted
		convert fname_html to html
		put «<h4>Assemble »,fname_html,«</h4>», nl

		var args = {}
		if listing!=""  args ##= listing then
		if verbose!=""  args ##= verbose then
		if labels!=""   args ##= labels then
		if encoding!="" args ##= encoding then
		args ##= { "-i", tempdir#src.filename }

		put "<pre>"			// the assembler may print some lines
		try
			var output = sys( zasm,args )
		else
			// no need to print error: zasm has already printed it to stderr
		then
		convert output to html
		put output
		put "</pre>"

		var tempdirlist = dir(tempdir#filebasename#".*")

		put «<h4>Your result files</h4>», nl

		put "<pre>  "
		var i=0
		do
			while ++i<=count tempdirlist
			put «<a href="»,htmltempdir,tempdirlist[i].fname,«">»,tempdirlist[i].fname,«</a> &nbsp; »
		loop
		put "</pre>"

		PutHtmlFooter()
		end 0
	then
then




/* ----	send result ------------------------------------------------------------
		((not used))
*/
var fname = what
var fname_html = fname
convert fname_html to printable
convert fname_html to html

if !exists file fname
	PutHtmlHeader()
	ShowPushFileForm();
	put «<p>the requested result file "»,fname_html,«" was not found.</p>», nl
	ShowCommandLineArguments();
	ShowEnvironmentVariables();
	PutHtmlFooter()
	end 0
else
	PutHtmlHeader()
	ShowPushFileForm();
	put «<p>the requested result file "»,fname_html,«" is present.</p>», nl
	ShowCommandLineArguments();
	ShowEnvironmentVariables();
	PutHtmlFooter()
	end 0
then


























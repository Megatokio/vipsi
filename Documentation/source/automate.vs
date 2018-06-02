#!/usr/local/bin/vipsi


/* ----	command line arguments ? -------------------------------
*/
var verbose = 0
var i = 0
do
	while ++i <= count globals && name globals[i]==""
	if globals[i]=="-v" del globals[i--] verbose=1 next then
loop



include "/usr/local/lib/vipsi/BOOK.vl"

SplitFileSize = 30000		// adjust
TargetDir     = "../"		// adjust


/* --------------------------------------------------------------------

	This file already contains some sophisticated customizations:

	Template.Page = «...»	replaced html page template
	Tags.include			added a tag for source file inclusion as <pre>
	Tags.shell 				added a tag for mocked-up vipsi shell examples

-------------------------------------------------------------------- */



/*	new Tag "shell":
  	input lines:
		shell
			i> what the user typed  o> output
			more output
			e> error message
			i> <>
			i> some words<>
			i> edit<x>line

	generated html:
		<pre class=out>
			vipsi> <span class=in>what the user typed</span> output
			more output
			<span class=err>error message</span>
			vipsi> <span class=crsr> </span>
			vipsi> <span class=in>some words</span><span class=crsr> </span>
			vipsi> <span class=in>edit<span class=crsr>x</span>line</span>
		</pre>

	note: Bei Aufruf von Tags.shell.Read(i) sind schon alle Zeilen verschmolzen
*/
proc r(i)
{
	rename Buch[i] = "pre"			// --> default write handler w0()
	Buch[i] ##= { Class="out" }		// --> dflt write handler will add css class
	var Text = nl#Buch[i].Text#nl, a,e,e2

// "<>"  -> cursor blob
	do 
		a = find(Text,"<>")			// <> anywhere: cursor
		while a				
		Text = Text[to a-1]#"<span class=crsr> </span>"#Text[a+2 to]
	loop

// "<X>"  -> cursor über Zeichen "X"
	a=0
	do 
		a = find(Text,"<",a+1)
		while a
		if Text[a+2]!=">" next then
		if find(Text,"</"#Text[a+1]#">",a+3) next then
		Text = Text[to a-1] # "<span class=crsr>" # Text[a+1]
							# "</span>" # Text[a+3 to]
		a+=20
	loop

// "e> " am Zeilenanfang: Error Message
	do
		a = find(Text,"\ne> ")
		while a
		e = find(Text,nl,a+3)
		Text = Text[to a]#"<span class=err>"#Text[a+4 to e-1]#"</span>"#Text[e to]
	loop

// "o> " am Zeilenanfang: text to stdout. no formatting ((whole <pre> is class=out))
	do a = find(Text,"\no> ") while a Text = Text[to a]#Text[a+4 to] loop
	do a = find(Text,"\no>")  while a Text = Text[to a]#Text[a+3 to] loop

// "i> " am Zeilenanfang: vipsi> prompt und text to stdin. Prüfe auf folgendes "o> "
	do 
		a = find(Text,"\ni> ")
		while a
		e  = find(Text,nl,a+3)
		e2 = find(Text,"o> ",a+3)
		if (e2&&e2<e)	// "o> " exists
			Text = Text[to a] # "vipsi> <span class=in>" # Text[a+4 to e2-1]
							  # "</span>" # Text[e2+3 to]
		else
			Text = Text[to a] # "vipsi> <span class=in>" # Text[a+4 to e-1]
							  # "</span>" # Text[e to]
		then
	loop

	Buch[i].Text = Text[2 to count Text -1]
	return i+1
}
var Tags.@"shell" = { Class='PRE', Read=r, Write=w0, Where=" h1 h2 h3 h4 h5 h6 " }


/*	new Tag "include":	include file and place it in <pre> tags

	since lots of those files are cgi scripts, we have the
	problem with the html tags in the source.
	therefore the text is converted to html in before.
	The html converter HTML.BuchNachHtmlKonvertieren() is smart enough
	to detect and preserve most html tags.
*/
proc r(i)
{
	try
		var Text = file( Buch[i].Text )
	else
		log nl, «include file », Buch[i].Text, « failed: ignored.»
		del Buch[i] return i
	then

	convert Text from utf8
	convert Text from @("tab"#TabWidth)
	convert Text to html
	Buch[i].Text = TEXT.CropSpaces(Text)
	rename Buch[i] = "pre"			// --> default write handler w0()
									// 	   will create a <pre> tag
	Buch[i] ##= { Class="src" }		// --> css

	return i+1
}
var Tags.@"include" = { Class='DEF', Read=r, Write=w0, Where=" h1 h2 h3 h4 h5 " }




/*	Template.Index		für die Startseite
	Template.Split		für die TOC-Seite von gesplitteten Kapiteln
	Template.Page 		für normale Seiten
*/
Template.Page =
«<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html><head>
<META http-equiv=Content-Type content="text/html; charset=UTF-8">
<title>%%TITLE%%</title>
<meta name="author"      content="kio@little-bat.de">
<meta name="copyright"   content="1998-2004 Kio (Günter Woigk)">
<meta name="robots"      content="index,follow">
<meta name="keywords"    content="%%KEYWORDS%%">
<meta name="description" content="%%TITLE%%  ((online book))">
<link rel="stylesheet" type="text/css" href="b/all.css">
<script type="text/javascript">
  var o="";
  function S(n)
  { H(); document.getElementById(n).style.visibility='visible'; o=n; }
  function H()
  { if(o!="") document.getElementById(o).style.visibility='hidden'; o=""; }
</script>
</head><body id="A">

%%HEADLINE0%%%%HEADLINE1%%
<p class="pglinks_o"><a href="%%PREV%%">[&lt;prev]</a> &nbsp; &nbsp;
<a href="%%NEXT%%">[next&gt;]</a></p>

<table width="100%"><tr><td width="10%"></td><td width=500>
%%HEADLINE2%%%%HEADLINE3%%%%HEADLINE4%%%%HEADLINE5%%%%HEADLINE6%%
%%TOC%%%%BODY%%
</td><td width="10%"></td></tr></table>

<p class="pglinks_u"><a href="%%PREV%%">[&lt;prev]</a> &nbsp;
<a href="#A">[top]</a> &nbsp; <a href="%%NEXT%%">[next&gt;]</a></p>
<p class="foot"><a href="http://validator.w3.org/check/referer">
<img border=0 src="b/html401.png" alt="Valid HTML" height=31 width=88></a>
&nbsp; <img border=0 src="b/css.png" alt="Valid CSS" height=31 width=88></p>
</body></html>
»


Doit()


put nl,nl,"--- finished ---",nl,nl


end 0









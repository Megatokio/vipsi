/*
	this file is included by index.cgi
*/


var TITLE = «vipsi - the friendly vip script interpreter»

var DESCRIPTION =
«vipsi - the friendly vip script interpreter - the developers most up-to-date archive.
vipsi is intended as an easy-to-use programming, shell and script language with a syntax similar to C/C++, Pascal or Basic. vipsi can be used interactively as a shell, as a pocket calculator, as a script interpreter e.g. for file synchronization or maybe to update a web site, or for CGI scripts.»

var KEYWORDS =
«vipsi, vip script, cgi, cgi script, shell, shell script, kio»

//var ROBOTS = «index,follow»



var MAIN = 
«
h3	Source of 'vipsi' - your friendly vip script interpreter.

p	Documentation is in <a href="Documentation">Documentation/</a> and tarballs are in <a href="Distributions">Distributions/</a>. Examples can be found in <a href="Libs">Libs/</a> and <a href="Documentation/scripts">Documentation/scripts/</a>.

p	The tarballs are for the old, interpreting version of vipsi. 


h4	About

p	<i>vipsi</i> is an interpreter for the vip script language. The vip script language is both, easy to use and powerful. vip scripts are easily readable and similar to C/C++, Pascal or Basic.

p	<i>vipsi</i> is a command line application and can be invoked as an interactive shell, run shell scripts or process CGI scripts for a web server. <i>vipsi</i> is one language for all.

p	<i>vipsi</i> uses unicode for text, double float for numbers and handles arrays and lists alike. It comes with lots of predefined functions, mathematical and for text manipulation and text conversion, has powerful list operations and of course all the basic stuff.

h4	Latest Version

p	The current state of development, which is a bytecode interpreter, can be found in <a href="build/">build</a>. They probably run on Mac OS X only, eventually also on other Intel-BSD environments. If you like, you can try to port the source to your platform. I'll appreciate it. 

p	You will also need to install some vipsi files from <a href="Libs/">Libs</a>, especially <a href="Libs/shell.vs">shell.vs</a>, which implements the interactive vipsi shell, and the "<tt>.vl</tt>" files in "/usr/local/lib/vipsi/". You will have to create this directory and you'll need to use 'sudo' for this.

p	This directory listing is served to you by vipsi itself.
»

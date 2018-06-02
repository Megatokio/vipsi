#!/usr/local/bin/vipsi

// http header:
put "content-type: text/html; charset=utf-8\n\n"

// html page, header etc.:
put «<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN">
<html><head><title>vipsi cgi test script</title></head>
<body style="font-family:sans-serif;">
<h2>passed cgi parameters</h2>»

var argc = count globals-1

put "<table><tr><td><b>name</b></td><td><b>value</b></td></tr>\n"

var i = 0
do
	while ++i <= argc
	var Name = name globals[i]
		convert Name to printable
		convert Name to html
	var Value = globals[i]
		convert Value to quoted
		convert Value to html
	put "<tr><td>", Name, "</td><td>", Value, "</td></tr>"
loop

put "</table></body></html>\n"
end


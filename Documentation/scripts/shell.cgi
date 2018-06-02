#!/home/smile77/bin/vipsi


//  print all the headers:
//  http header
//  head section of html page and
//  start body section

    put «\
content-type: text/html; charset=utf-8

<!doctype html public \"-//w3c//dtd html 4.01 transitional//en\">
<html>
<head>
	<META http-equiv=Content-Type content=\"text/html; charset=UTF-8\">
	<title>vipsi online shell</title>
	</head>

<body style="font-family:sans-serif;">
	<h2>vipsi online shell</h2>

	<form>
		<input name="input_line" id="theinput" size=80 maxlength=240>
	</form>
»

	var hist_file = "/home/smile77/htdocs/cgi-bin/shell.zzz"

	try
		include hist_file
	then

	if exists input_line
		put "<pre>"
		put "<b>",input_line,"</b>",nl
		split input_line," "

		if count input_line==1
			if input_line[1]!=""
				sys input_line[1]
			then
		else
			if count input_line==2
				if input_line[1]=="cd"
					cd input_line[2]
				else
					sys input_line[1],input_line[2]
				then
			else
				if count input_line==3
					sys input_line[1],input_line[2],input_line[3]
				else
					if count input_line==4
						sys input_line[1],input_line[2],input_line[3],input_line[4]
					then
				then
			then
		then

		try
			openout#4,hist_file
				put#4, "cd ",convert(fullpath"." to quoted),nl
				put#4, "end",nl
			close#4
		then
	then


	put
«
	</pre>
	<script type="text/javascript">
		document.getElementById("theinput").focus();
	</script>
	</body></html>
»


	end











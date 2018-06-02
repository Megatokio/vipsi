#!/home/smile77/bin/vipsi


// chat script as used on my homepage
// ----------------------------------

// very basic and buggy
// please adjust paths!



var argc	 = count globals-1

var Title	 = "Chat at little-bat"
var H1_Login = "Welcome to chat at little-bat!"
var H1_Input = "Chat at little-bat!"

put "content-type: text/html; charset=utf-8\n\n"

/*	Kio's chat script
	the chat consists of a frameset with an input frame and an output frame.
	the frameset is written if the script is called with no arguments
	the output frame is written if  Action == "Output"
	the input  frame is written for Action == "Input"

*/

if !exists Action     var Action    = "Frameset"    then
if !exists SessionID  var SessionID = env.UNIQUE_ID then
if !exists Name		  var Name	    = "" then
if !exists Password	  var Password  = "" then
if !exists TextInput  var TextInput = "" then

var Sessions  = {}	  // from file
var History   = {}	  // from file

var Http  = {}
var File  = {}
var Style = {}
var Img   = {}

var ScriptFilename = env.SCRIPT_FILENAME		//	"/home...cgi-bin/chat.cgi"
var Script         = env.SCRIPT_NAME			//	"/bin-cgi/chat.cgi"
var File.Root      = env.DOCUMENT_ROOT # "/"	//	"/home/pub/www/"
var File.CgiDir    = ScriptFilename[to rfind(ScriptFilename,"/")]
var File.History   = File.CgiDir # "chat.history"
var File.Sessions  = File.CgiDir # "chat.sessions"

var Http.Host      = env.HTTP_HOST				//	"10.0.0.4"
var Http.Root      = "/"

if Http.Host=="10.0.0.4" || lowerstr(Http.Host[to 4])=="eden"
	File.Root     #= "little-bat.de/"
	Http.Root     #= "little-bat.de/"
then

var Http.Images   = Http.Root   # "b/"
var Http.Hdgr     = Http.Images # "hdgr.jpg"
var Http.Bat128   = Http.Images # "bat_sw_128_frei.png"
var Http.Space    = Http.Images # "_.gif"

var Style.NoGap   = " style='margin:0px;padding:0px;' "
var Style.Body    = " style='margin:0px;padding:8px; \
					  background-color:#cccccc; \
					  background-image:url(" #Http.Hdgr# ")' "
var Style.Self    = "style='font-weight:bold;'"
var Style.Kio     = "style='color:#000099;'"
var Style.User    = "style=''"
var Style.Anon    = "style='color:#666666;'"

var  Img.Bat128   = "<img width=128 height=128 src='" #Http.Bat128# "'>"
proc Img.Space(w,h)
{ return "<img width='" #w# "' height='" #h# "' src='"#Http.Space#"'>" }



/* ----------------------------------------------------------------------
	in: Action == Frameset
		=> send frameset
*/
if Action=="Frameset"
	put«
	<!doctype html public "-//w3c//dtd html 4.01 frameset//en"
	<html>
	<head>
   	<title>» #Title# «</title>
	<meta http-equiv='content-type' content='text/html; charset=utf-8'>
	<meta name='author' content='kio@little-bat.de'>
	<meta name='robots' content='noindex'>
	</head>

	<frameset rows='256,*'>
	<frame src='» ,Script, «?Action=Login' name='chat_input' scrolling=no>
	<frame src='» ,Script, «?Action=Empty' name='chat_output'>
	</frameset>
	<body><h1>Frames required!!</h1></body></html>
»
	end
then



/* -------------------------------------------------------------------
	Action != Frameset
	=> send standard frame header
*/
put «\
	<!doctype html public "-//w3c//dtd html 4.01 transitional//en"
	<html>
	<head>
	<title>» #Title# «</title>
	<meta http-equiv='content-type' content='text/html; charset=utf-8'>
	<meta name='author' content='kio@little-bat.de'>
»

if Action=="Output"
	put «<meta http-equiv="refresh" content=\"20; URL=»
		# Script
		# "?Action=Output&amp;SessionID="
		# SessionID
		# «">\n»
then

put	«<style type='text/css'><!--
	table,tr,td,p,h1,h2,form,input {padding:0px;margin:0px;}
	--></style>
	</head>
»



/* --------------------------------------------------------------------
	Action == ShowSource
	show my own source:
*/
if Action == "ShowSource"
	var mysource = file(ScriptFilename)
	convert mysource from utf8
	convert mysource to html
	put "<body", Style.Body, ">\n"
	put "<pre>",nl, mysource, nl,"</pre></body></html>",nl
	end
then



/* ----	Utilities -----------------------------------------------------
*/
proc Styled ( style, block )
{
	var i = find(block,">")
	return block[to i-1] # " " # style # block[i to]
}

proc Block ( tag, content )
{
	var r1 = "<" # tag # "\n>"
	var r3 = "</" # tag # "\n>"

	if islist content
		var r2 = ""
		var i=0 do while ++i<=count content r2 #= content[i] loop
		return r1 # r2 # r3
	else
		return r1 # content # r3
	then
}

proc H1 ( text ) { return Block("h1",text) }
proc H2 ( text ) { return Block("h2",text) }
proc P  ( text ) { return Block("p", text) }

proc Table ( rows )
{
	return Styled(
		"cellpadding=0 cellspacing=0 border=0",
		Block("table", rows)
	)
}

proc TR ( cells )
{
	return Styled( "valign=top", Block("tr",cells) )
}

proc TD ( content )
{
	if content=="" content="&nbsp;" then
	return Block( "td", content )
}

proc Input ( )
{
	return "<input type='text'" # Style.NoGap # "\n>"
}

proc Button ( url, text )
{
	return
		"<input type='button' value='" #text# "' " #
		Style.NoGap #
		« onClick="location.href='» #url# «'">»
}


proc Form ( content )
{
	return Styled(
		«action="» #Script# «" method="post"»,
		Block( "form", content )
	)
}


proc JS ( jscript )
{
	return
		"<script type=\"text/javascript\">" # jscript #
		"</script>\n"
}



proc WriteSessions()
{
	/* ***TODO*** blocking */
	file File.Sessions = string Sessions
}

proc ReadSessions()
{
	/* ***TODO*** blocking */
	if Sessions=={} Sessions = eval file File.Sessions then
}

proc WriteHistory()
{
	/* ***TODO*** blocking */
	file File.History = string History
}

proc ReadHistory()
{
	/* ***TODO*** blocking */
	if History=={} History = eval file File.History then
}

proc FindName()
{
	var i = count Sessions +1
	do
		while --i
		until Name==Sessions[i,1]
	loop
	return i
}





/* ----	Check Login --------------------------------------------------
	in:	Action  == "CheckLogin"
		Name     = name
		Password = password
*/
if Action == "CheckLogin"
	ReadSessions()
	ReadHistory()

	proc PW_OK()
	{
		if Name[1]=="(" return 0 then
		if count Name<3 return 0 then
		var i = FindName()
		if i==0 return 0 then
		if Password!=Sessions[i,2] return 0 then
		Sessions[i,3] = SessionID
		return 1
	}

	if Password=="" && count Name >= 3
		Action = "Input"
		Name   = "(" # Name # ")"
		Sessions ##= {{ Name, "", SessionID, now }}
		History ##= {{ Name, SessionID, "+++ logged in +++" }}

	else if !PW_OK()
		Action = "RetryLogin"
		History ##= {{ Name, SessionID, "+++ failed to log in +++" }}

	else
		Action = "Input"
		History ##= {{ Name, SessionID, "+++ logged in +++" }}
		Sessions[FindName(),4] = now
	then
	then

	if Action!="RetryLogin" WriteSessions() then
	WriteHistory()
then



/* ----	Login ---------------------------------------------------------
	in:	Action == "Login" / "RetryLogin"
		=> present the login screen
*/
if Action=="Login" || Action=="RetryLogin"
	proc Login()
	{
		var text1 =
		H1(H1_Login) #
		"<p>This chat system is for help and discussions related to \
			software and projects on this site."
		"<p>Please log in." #
		"<br>&nbsp;"

		var t1 =
			TD( "Name" ) #
			TD( "<input type='text' name='Name' \
				size=20 style='width:100%'>" )

		var t2 =
			TD( "Password" ) #
			TD( "<input type='password' name='Password' \
				size=20 style='width:100%'>" )

		var t3 =
			TD("<input type='hidden' name='Action' value='CheckLogin'>") #
			Styled( "width='70%' align=right", TD("<input type='submit'
					value='Enter' style='width:50%'>") )


		var t = TR(t1) ## TR(t2) ## TR(t3)

		var text2 = ""
		if Action=="RetryLogin"
			text2 = "<p style='color:#cc0000'>Your login failed!"
		then

		var text3 =
		"<p>If you are new, enter your name and leave the password blank." #
		"<br>Send e-mail to Kio to register a user name and password."

		return text1 # Styled("name='login_form'",Form(Table(t))) #
			   text2 # text3
	}

	var td={}
	td ##= Styled( "width=128 height=128", TD(Img.Bat128) )
	td ##= Styled( "width=16",             TD(Space(16,2)) )
	td ##= Styled( "align=center",         TD(Login()) )

	put "<body", Style.Body, ">\n"
	put Styled( "width='100%'", Table(TR(td)) )
	put JS
	(	// on load: focus for name input
		"document.login_form.Name.focus();"
	)
	end
then




/* ----	Input --------------------------------------------------------------
	in:	Action   == "Input"
		SessionID = Session ID

	this runs in the upper frame is never reloaded again
	submitting the form reloads the lower frame with the new history
*/
if Action == "Input"
	ReadSessions()
	var i=0
	do
		while ++i<=count Sessions
		until SessionID==Sessions[i,3]
	loop
	if i>count Sessions put "<p>internal error #306#",nl end then
	Name = Sessions[i,1]
	Password = Sessions[i,2]

	proc Enter()
	{
		var text = H1(H1_Input) # P("please be patient!")

		var f1 = "<textarea onKeyUp='CheckForSend()' name='TextInput' \
					cols=50 rows=4></textarea>"
		var f2 = "<input type='submit' value='Send'>"
		var f3 = "<input type='hidden' name='Action' value='Output'>"
		var f4 = "<input type='hidden' name='SessionID' value='"#SessionID#"'>"
		var f5 = "<input type='hidden' name='Name' value='"#Name#"'>"
		var f6 = Styled( "href='"#Script#
					"?Action=ShowSource' target='chat_output'",
					Block("a","show source") )

		var form = Styled( "name='input_form' target='chat_output'",
					Form(f1#"<p>"#f6#f3#f4#f5#" &nbsp; "#f2) )
		return text # form
	}

	var td={}
	td ##= Styled( "width=128 height=128", TD(Img.Bat128) )
	td ##= Styled( "width=16",             TD(Space(16,2)) )
	td ##= Styled( "align=center",         TD(Enter()) )

	put "<body onUnload=\"Logout();\" ", Style.Body, ">\n"

	put JS
	(	// onKeyUp event: works only for few browsers
		"function CheckForSend(){var d=document.input_form, \
			t=d.TextInput, v=t.value;" #
		"if(v[v.length-1]<\" \") { d.submit(); t.value=\"\"; \
			t.focus(); } }" #
		// onUnload event: seems to work when new url loaded
		// but not when window closed
		"function Logout(){var d=document.input_form, t=d.TextInput; \
			t.value=\"--- logged out ---\";d.submit();}"
	)
	put Styled( "width='100%'", Table(TR(td)) )
	put JS
	(	// on load: focus for textarea
		"document.input_form.TextInput.focus();" #
		"parent.chat_output.location.href=\""#Script#
		"?Action=Output&SessionID="#SessionID#"\";"
	)

	var i = count Sessions +1
	do
		while --i
		until Sessions[i,3]!="" && Sessions[i,3]!=SessionID
	loop
	if !i && lowerstr(Name)!="kio"
		put JS("alert(\"You are alone. Kio is not online.\n\
			You may leave the chat window hanging around and wait.\")")
	then

	end
then




/* ----	Output ----------------------------------------------------------
	in:	Action   == "Output"
		TextInput = new text or empty
		SessionID = session ID
*/
if Action == "Output"
	ReadHistory()
	ReadSessions()
	var i=count TextInput
	do
		while i>=1
		while TextInput[i]<=" "
		var z = TextInput[to --i]
		TextInput = z
	loop

	put "<body", Style.Body, ">\n"

	do while count History > 20 del History[1] loop
	if TextInput!=""
		History ##= {{ Name, SessionID, TextInput }}
		WriteHistory()
		put JS("var t=parent.chat_input.document.input_form.TextInput; \
			t.value=\"\";t.focus();")
	then

	var i = count Sessions +1
	do
		while --i
		if Sessions[i,3]==SessionID Sessions[i,4] = now then
		if Sessions[i,3]!="" && now - Sessions[i,4] > 60
			History ##= {{ Sessions[i,1], Sessions[i,3], "--- vanished ---" }}
			WriteHistory()
			Sessions[i,3] = ""
			if Sessions[i,1,1]=="(" del Sessions[i] then
		then
	loop
	WriteSessions()

	var i=count History
	do
		while i
		var                              style = Style.User
		if History[i,1,1]=="("           style = Style.Anon then
		if lowerstr(History[i,1])=="kio" style = Style.Kio  then
		if History[i,2]==SessionID       style=Style.Self   then
		convert History[i,1] to html
		convert History[i,3] to html
		put Styled( style, "<p>" # History[i,1] # ": " # History[i,3] # nl )
		i--
	loop
	end
then




/* -----------------------------------------------------------------------
	Action == Empty  or  bogus
	=> empty frame
*/
put	"<body", Style.Body, ">\n"
if Action!="Empty"
	var i=0
	do
		while ++i<=argc
		put "<p>", name globals[i], " = ", globals[i]
	loop
then
end

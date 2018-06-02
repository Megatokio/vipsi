#!/home/smile77/bin/vipsi


/* --------------------------------------------------------------------------------------------

    cgi scripts:

    method "GET":   
        arguments are passed in the environment variable "QUERY_STRING".

    method "POST":
        arguments are passed via stdin. size of input data is passed in "CONTENT_LENGTH".
        
    common:
        the argument string consists of multiple fields which are separated by ampersands "&".
        each field has (should have) the form: "name=value".
        the data is encoded according to mime type "application/x-www-form-urlencoded".
        this means: spaces " " are replaced by plus signs "+" and special characters are 
        represented by their hex code a la "%AF".

    the cgi script must write a file to stdout. 
    in the most common case this is a html file:
    
    step 1: http-header:
            "content-type: text/html\n\n"
    or
            "content-type: text/html; charset=utf-8\n\n"

    then the html page itself. it has the following basic layout:
            <!doctype html public "-//w3c//dtd html 4.01 transitional//en">
            <html>

            <head>
               <title>Titel der Seite ist Pflicht</title>
               <META http-equiv=Content-Type content="text/html; charset=UTF-8">        ((opt.))
            </head>

            <body>
                <!-- Contents of page here -->
            </body>
        
            </html>

--------------------------------------------------------------------------------------------- */



//  print all the headers:
//  http header
//  head section of html page and
//  start body section

    put 
    "content-type: text/html; charset=utf-8\n\n",
    "<!doctype html public \"-//w3c//dtd html 4.01 transitional//en\">\n",
    "<html>\n",
    "<head>\n",
    "<META http-equiv=Content-Type content=\"text/html; charset=UTF-8\">\n",
    "<title>vipsi test script for cgi'</title>\n",
    "</head>\n",
    "<body style='font-family:sans-serif;'>\n",
    "<h2>*** vipsi *** cgi test script</h2>\n"



//  check whether the "magic parameter" is passed to show my own source:
//  this magic parameter is see_source=yes
//  if so, show source as preformatted text and quit

    if exists see_source
        if see_source[1]=="y" || see_source[1]=="1"
            if exists env.SCRIPT_FILENAME
                var myfile = env.SCRIPT_FILENAME
                var mysource = file(myfile)
            else
                var mysource = "erm ... error error ... SCRIPT_FILENAME is not set in the process environment ..."
            then
            convert mysource to html
            put "<pre>\n", mysource, "\n</pre></body></html>\n"
            end
        then
    then



//  print notification on what the script does
//  how to call it and
//  how to call it so that it print's it's own source

    if exists env.SERVER_NAME && exists env.SCRIPT_NAME
    {       
        var mydomain = env.SERVER_NAME
        var myurlpath = env.SCRIPT_NAME
        var myurl = "http://" # mydomain # myurlpath
        put 
        "<p>this script lists environment variables, command line arguments and get/post cgi parameters.",
        "<p>to see the source <a href='test.cgi?see_source=yes'>click here</a>.",
        "<p>you can call it directly: ", myurl,
        "<br>or with some arguments: ", myurl, "?anton=123&amp;berta=wetter",
        "<br>or you can write an own page with a form which calls this script ;-)."
    }
    then
    


//  print global variables
//  the script can access all global variables unknowing their names by accessing the list variable "globals".
//  command line arguments passed to the script come first and are unnamed
//  (a cgi script is normally not passed any command line arguments)
//  data from the CGI GET/POST request come next and are named and already unencoded
//  if bogus data is passed the names may be weird or unnamed
//  typically a cgi script should already know the names of arguments passed to it
//  but this cgi test script does not and must examine the variable names 'by hand'.

    var argc = count globals-1

    if argc
    {
        put 
        "<h4>argv[] arguments</h4>\n",
        "<table border width='100%'><tr><td width='10%'><b>index</b></td>", 
        "<td width='20%'><b>name</b></td><td><b>value</b></td></tr>\n"
        
        var i = 0
        do
            while ++i<=argc
            var gname = name globals[i]   
            var gdata = globals[i]        
            convert gname to printable; convert gname to html
            convert gdata to printable; convert gdata to html
            put "<tr>"
            put "<td>globals[",i,"]</td>"
            put "<td>", gname=="" ? "<i>unnamed variable</i>" : gname, "</td>"
            put "<td>", gdata, "</td>"
            put "</tr>"
        loop
        
        put "</table>\n"
    }
    then



//  print environment variables
//  the script can request the process environment variables with the built-in function env.
//  the returned list contains one named item per environment variable
//  the CGI GET arguments can be found in the variable "QUERY_STRING"

    {   
        put 
        "<h4>env environment variables</h4>\n",
        "<table border width='100%'><tr><td width='10%'><b>index</b></td>",
        "<td width='20%'><b>name</b></td><td><b>value</b></td></tr>\n"
    
    //  in order to sort by name we convert the named items of env 
    //  into sub-lists of form { name, data }

        var env_var = env
        var i = 0
        do
            while ++i <= count env_var
            env_var[i] = { name env_var[i], env_var[i] }
        loop

        sort    env_var                     // sort the list
        convert env_var to printable        // escape control characters et.al.
        convert env_var to html             // convert '<' to '&lt;' etc.
    
        var i = 0
        do
            while ++i <= count env_var
            var ename = env_var[i,1]
            var edata = env_var[i,2]
            put "<tr>"
            put "<td>env[",i,"]</td>"
            put "<td>", ename=="" ? "<i>unnamed variable</i>" : ename, "</td>"
            put "<td>", edata, "</td>"
            put "</tr>\n"
        loop
        
        put "</table>\n"
    }
    


// print misc. info

	put "<h4>/bin/*</h4>\n"
	put "<pre>"
	sys "ls","-l","/bin"

	put "<h4>/usr/bin/*</h4>\n"
	put "<pre>"
	sys "ls","-l","/usr/bin"

	put "<h4>misc. tests</h4>"
	put "<pre>"
	sys "which","gcc"
	sys "which","make"



// the final closing tags
// be nice and terminate the last line with a line ending character

    put "</body></html>\n"
    end
    










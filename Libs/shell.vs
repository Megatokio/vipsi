#!/usr/local/bin/vipsi

/*
12345678901234567890123456789012345678901234567890123456789012345678901234567890
*/

globals ##= { y = proc(t) { t = ("    "#t#spacestr(80))[to 79]; put "\033[103m", t, "\033[49m", nl } }
	put nl
	y( interpreter.@"name" # " " # interpreter.version # " " # interpreter.copyright )
	y( "development release. please set your terminal to utf-8 and vt100." )
	y( "see <http://k1.spdns.de/vipsi/> for latest documentation." )
	put nl
	y(«        vipsi is waiting for your commands.»)
	y(«        type an expression to calculate the result.»)
	y(«        type "words" for a list of words.»)
	y(«        type "info foo" for info on word "foo".»)
	y(«        use "pd_up" and "pd_dn" for history.»)
	y(«        type ctrl-D or "quit" to quit.»)
	put nl
del globals.y


/*	alle Daten für den Editor werden in root.shell angelegt.
	dadurch sind sie normalerweise unsichtbar.
	außerdem wird die thread-variable 'error' vor und nach dem Ausführen
	eines Eingabekommandos mit root.shell.err geswappt. Dies hat nur den Vorteil, 
	dass der User nach einem Fehler noch ein bischen in 'error' rumpeeken kann.
*/
root.shell ##= 
{ 
	cmd="", col=0, z=0,
	err=
	{	number = 0, 
		message= "", 
		stack  = {},
		Print  = proc()
		{
			put nl; log "*****  ", message
			del stack[1]			// <-- error position in shell.vs
			del stack[1]			// <-- error position in eval(..)
			var ns=0,nc=0,nr=0,i=0
			do while ++i<=count stack; ns = max(ns,count stack[i].info); nc = max(nc,stack[i].col); nr = max(nr,stack[i].row); loop
			nc=count string nc; nr=count string nr;
			i=count stack; 
			do
				while i
				var e = stack[i]
				log nl," "
				if(ns)
					if(count e.info) log " in ", e.info, spacestr(ns-count e.info)
					else			 log spacestr(ns+5); then
				then
				log " at "
				if(e.row!=1) log "row=",e.row,", ",spacestr(nr-count string e.row) then
				log "col=",e.col,spacestr(nc-count string e.col) 
				if(e.row==1) log spacestr(nr+4+2) then
				if(e.file!="") log " in file ", convert(e.file to quoted) then
				i--
			loop
		}
	},
	SwapErrors = proc() { swap err.number,error.number; swap err.message,error.message; swap err.stack,error.stack; }
}


do
// ====	input command line =================================================================

	if(count root.shell.cmd)									// cmd line is not cleared => error happened
		root.shell.col = root.shell.err.stack[2].col  
		root.shell.err.Print()
		putback spacestr( count root.shell.cmd - (root.shell.col-1), "\e[D" )	// n * cursor_left
	then
	put nl,"vipsi> "
	edit root.shell.cmd		


// ====	test for shell-only commands =====================================================

	until root.shell.cmd=="exit" 
	until root.shell.cmd=="quit" 
	until root.shell.cmd=="end"

	if root.shell.cmd=="pwd"
		put "  = ", fullpath "."
		root.shell.cmd=""
		next

	elif root.shell.cmd=="words"
		root.shell.z = words
		put "  = ",nl,root.shell.z,nl
		root.shell.cmd=""
		next

	elif root.shell.cmd=="globals"
		root.shell.z = globals
		put "  = ",nl,root.shell.z,nl
		root.shell.cmd=""
		next

	elif root.shell.cmd=="env"
		put " env... = ", env		// TODO
		root.shell.cmd=""
		next

	elif 0&&root.shell.cmd[to 3]=="cd "
		put " cd..."
		root.shell.cmd=""
		next

	elif 0&&root.shell.cmd[to 4]=="dir "
		put " dir..."
		root.shell.cmd=""
		next

	elif root.shell.cmd[to 5]=="info "
	{	
		var z = root.shell.cmd[6 to]
		var i = count z
		do while i && z[i]==" " z=z[to i-1] i-=1 loop
		do while i && z[1]==" " z=z[2 to] i-=1 loop
		if i>=3 && z[1]==«"» && z[i]==«"» z=z[2 to i-1] then
		try
			put nl,info z
			root.shell.cmd=""
			next
		else
			root.shell.cmd = «info "» # z # «"»
		then
	}	
	then


// ====	compile and execute ==========================================================

	try
		// test compile for instruction syntax
		root.shell.z = eval( "proc(){" # root.shell.cmd # "}" )

		//	syntax for instruction ok
		try
			// 	test compile for value syntax
			root.shell.z = eval( "1?0:(" # root.shell.cmd # ")" )

			// both compiled
			//	<proc>(<arguments>)		-> try value, discard error "proc returned no value"
			//	{ a=b }					-> default to list literal: value
			root.shell.SwapErrors()
			try
				put "  "
				root.shell.z = eval root.shell.cmd
				put "= ", root.shell.z
				root.shell.cmd = ""
			else
				if find(error.message,"no result") root.shell.cmd = "" then		// -> no error
			then
			root.shell.SwapErrors()

		else
			// instruction ok, value bummer => instruction
			root.shell.SwapErrors()
			try
				put "  "
				eval root.shell.cmd
				root.shell.cmd = ""
			then
			root.shell.SwapErrors()
		then

	// test compile for instruction syntax failed!
	else
		root.shell.col = error.stack[2].col

		try
			// 	test compile for value syntax
			root.shell.z = eval( "1?0:(" # root.shell.cmd # ")" )

			// syntax ok for value => do it!
			root.shell.SwapErrors()
			try
				put " "
				root.shell.z = eval root.shell.cmd
				put " = ", root.shell.z
				root.shell.cmd = ""
			then
			root.shell.SwapErrors()

		else
			// both instruction and value failed: bummer.
			if(error.stack[2].col-5 < root.shell.col-7)
				root.shell.SwapErrors()
				try eval root.shell.cmd then					// will bummer
				root.shell.SwapErrors()
			else
				root.shell.SwapErrors()
				try root.shell.z = eval(root.shell.cmd) then	// will bummer
				root.shell.SwapErrors()
			then
		then
	then
loop


put	nl,"       vipsi says: bye bye!",nl,nl
end


















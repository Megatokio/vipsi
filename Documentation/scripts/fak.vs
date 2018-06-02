#!/usr/local/bin/vipsi

/*
	example script for sys() and call()

	if called with  «call("fak.vs",number)»      it returns a numeric value
	if called with  «sys("fak.vs",numberstring)» it prints the result

	both cases are distinguished by examining the type of the passed argument:
	if it is numeric then it must be call()
	if it is a text  then we assume it was sys()
*/

	if count globals != 1
		log "error: script 'fak.vs' takes exactly one argument",nl
		var i = 1
		do
			while i<count globals
			log "arg[",i,"] = ",globals[i],nl
			i++
		loop
		end 1
	then

	rename globals[1] = "n"
	var is_num  = isnumber(n)
	var is_text = istext(n)		if is_text n=eval(n) then
	var result  = 1;

	if (!is_num && !is_text)
		log "error: script 'fak.vs' requires a numeric or text argument",nl
		end 1
	then

	do
		while n>1
		result *= n--
	loop

	if (is_num)				// if called with "call"
		return result		// then we can return real data
	else
		put result,nl		// if called with "sys"
		end 0				// we can only print to stdout
	then					// and return "ok"















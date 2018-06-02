#!/usr/local/bin/vipsi

var argc = count globals -1

proc show_help ( )
{
	put "  dieses script konvertiert alle textfiles nach UTF-8",nl
	put "  bitte mit datei/verzeichnis aufrufen",nl
	put "  option -v / --verbose => verbose. multiple -v possible",nl
	put "  option -b / --backup  => kopiert datei vorher nach *~",nl
	put "  option -t / --test    => test only",nl
	put "  option -p / --pretend => test only",nl
	put "  option -h / --help    => show help",nl
	put "  option -  / --        => end of options",nl
}

var verbose = 0
var pretend = 0 
var backup  = 0
do
	while argc && globals[1,1]=="-"
	if globals[1]=="-h" || globals[1]=="--help"		show_help() 		            end	0 then
	if globals[1]=="-v" || globals[1]=="--verbose"	argc-- del globals[1] verbose++ next  then
	if globals[1]=="-b" || globals[1]=="--backup"	argc-- del globals[1] backup++  next  then
	if globals[1]=="-t" || globals[1]=="--test"		argc-- del globals[1] pretend++ next  then
	if globals[1]=="-p" || globals[1]=="--pretend"	argc-- del globals[1] pretend++ next  then
	if globals[1]=="-"  || globals[1]=="--"			argc-- del globals[1]           exit  then

	log "unknown option: ", convert(globals[1] to quoted)
	put nl
	return 1
loop

if argc==0
	show_help()
	return 1
then


var good_words =
{
	@"•••"=0, @"{"=0, @"}"=0, @"["=0, @"]"=0, @"|"=0, @"\\"=0, @"~"=0,
	@"…"=0, @"•"=0, @"€"=0, @"π"=0, @"•••todo•••"=0, @"••buggy••"=0,
	@"[daily"=0, @"[apache"=0, @"[stl"=0, @"[netscape"=0, @"«info"=0,
	@"[documentation"=0, @"[japaneese"=0, @"[inhalt"=0, @"[desktop"=0,
	für=0, muß=0, stück=0, daß=0, zufällig=0, öffnen=0, läuft=0, coupé=0, spaß=0,
	höher=0, günter=0, länger=0, günstig=0, fräulein=0, mußte=0, märchen=0,
	straßenstrich=0, führer=0, lästern=0, wählten=0, empfänger=0, tafelgeschäft=0,
	gehört=0, über=0, übung=0, köln=0, überzeugen=0, blöden=0, darüber=0, groß=0,
	gerührt=0, gehören=0, drücken=0, müller=0, müssen=0, beschwörung=0, möbius=0,
	übertragungsprotokoll=0, übersicht=0, würfen=0, überleben=0, schluß=0, mühe=0,
	anfänger=0, zähle=0, stärke=0, verbündete=0, ganzkörpersymbionten=0, lösche=0,
	kälteschutz=0, löschsalz=0, muräne=0, erhält=0, größe=0, scheinrealität=0,
	auslöst=0, übersetzung=0, füllen=0, längs=0, ausdrücklich=0, suchausdrücke=0,
	prüfen=0, bézier=0, öfter=0, gruß=0, wählen=0, sprüche=0, broschüren=0, wörk=0,
	müsste=0, näher=0, entführt=0, großmutter=0, kämpfen=0, komödie=0, führt=0,
	wächst=0, nürnberg=0, werbeflächen=0, menü=0, paßwort=0, spät=0, hinzufügen=0,
	inkompatibilität=0, können=0, außerdem=0, grün=0, ändern=0, außer=0, tötet=0,
	überfall=0, kräfte=0, sprüht=0, verlässt=0, großer=0, gründen=0, tödlich=0,
	zurück=0, kämpft=0, später=0, schön=0, künftige=0, lässt=0, schläft=0, fürth=0,
	lösung=0, drängarna=0, müll=0, characterbögen=0, fläche=0, blümchen=0, löscht=0,
	märz=0, möglich=0, dürfen=0, nächsten=0, nächste=0, länge=0, näherungsformel=0, fußtext=0,
	abstände=0, dämon=0, biß=0, städte=0, kultstätte=0, rückseite=0, hauptmenü=0,
	nestlé=0, mäßig=0, müßig=0, bär=0, müde=0, sürül=0, fuß=0, präsentieren=0,
	stört=0, läßt=0, könig=0, Täter=0, serienmörder=0, plötzlich=0, mörder=0, fähig=0,
	gefühl=0, auswärtige=0, geglückt=0, bevölkerung=0, öko=0, änderungen=0, halbzüge=0,
	@"[entire"=0, @"[binhexed"=0, @"[any"=0, bürger=0, elitären=0, schlägt=0, überhaupt=0,
	genießt=0, mysteriöse=0, zunächst=0, stößt=0, glück=0, glückliche=0, übel=0, fällt=0,
	außen=0, @"[netscape]"=0, fälle=0, küsten=0, zusätzlich=0, wölfe=0, würfel=0, jörg=0,
	@"µP"=0, @"«"=0, @"»"=0, @"•••missing•••"=0, @"•••important•••"=0, @"•••not"=0, @"•••decoded"=0,
	unterlänge=0, gelöschter=0, überschrieben=0, @"[config"=0, würfeln=0, weiß=0, gültig=0,
	@"\\pkio"=0, @"xyz[\\]"=0, @"•••••"=0, @"•"=0, übersetzt=0, durchgeführt=0, menüs=0,
	@"[options]"=0
}



/* ---- handle file|directory <fname> in directory <path> ----
		<path> must be a fullpath(path)
		<fname> may be a symbolic link
*/
proc doit ( path, fname )
{
	if path#fname != fullpath(path#fname) 	// symolic link:
		return								// dir: avoid recursion
	then									// file: renaming / creation of bak file probs


// ----	handle DIRECTORIES ----

//	if rightstr(fname,1)=="/" || fname==""
	if fname=="" || isdir(path#fname)
		if fname!="" && rightstr(fname,1)!="/" fname #= "/" then

		if fname=="cinebank video cover/" return then
		if fname=="CVS/" return then
		if fname=="ftp:www.unicode.org:Public/" return then
		if fname=="cache/" return then
		if fname=="Cache/" return then
		if fname==".kde/" return then
		if fname==".mozilla/" return then
		if fname=="Mail/" return then
		if fname==".mcop/" return then
		if fname==".gnome/" return then
		if fname==".gnome2/" return then
		if fname==".gimp-1.2/" return then
		if fname=="Mülleimer/" return then
		if fname=="Schriften Mac/" return then
		if fname=="MacSOUP.BAK/" return then
		if fname=="Eudora kio/" return then
		if fname=="macos_classic/" return then
		if rightstr(fname,7)==".cache/" return then
		if rightstr(fname,8)==".pbproj/" return then

		path #= fname
		if verbose put "DIRECTORY:  ", convert(path to quoted), nl then

		var files = dir (path)
		var i=0
		do
			while ++i <= count files
			var files_i = convert(files[i].fname to quoted)
			var flags = files[i].flags
			if flags[1]=="?" next then					// broken link
			if flags[1]=="l" next then					// link
			if flags[1]=="d"							// directory
				if flags[2]!="r" if verbose>=2 log "skipped:    ",files_i," (not readable)"   put nl then next then
				if flags[3]!="w" if verbose>=2 log "skipped:    ",files_i," (not writable)"   put nl then next then
				if flags[4]!="x" if verbose>=2 log "skipped:    ",files_i," (not searchable)" put nl then next then
			else										// file
				if files[i].length>500000 next then		// too long
				if flags[2]!="r" if verbose>=2 log "skipped:    ",files_i," (not readable)" put nl then next then
				if flags[3]!="w" if verbose>=2 log "skipped:    ",files_i," (not writable)" put nl then next then
			then
			doit(path,files[i].fname)
		loop
		return
	then


// ----	handle FILES ----

// exclude file by type

	if rightstr(fname,1)=="~"
	|| leftstr(fname,2)=="._"
		if verbose>=2 put "skipped:    ", convert(path#fname to quoted), nl then
		return
	then

	var ext=""
	var i = rfind(fname,".")
	var j = rfind(fname," ")
	if i>j
		ext   = fname[i to]
		fname = fname[to i-1]
	then

	if ext!="" && find
		( ".jpg.mp3.zip.gz.tar.madh.wav.au.aiff.mid.mod.midi.mpg.avi.mpeg.mov.asf.tiff.art.ai.sit.o" #
		  ".fli.gif.jpeg.png.bmp.xpm.xbm.jpe.scr.z80.sna.tap.psd.tif.fh3.fh5.fh7.fh8.fh9.c4d.pdf.eps.rom" #
		  ".directory.ttf.pfm.pfb.cache.x-face.fmz.bz2.pict.iff.ppm.xsym.cat.pct.",
		  lowerstr(ext) # "."
		)
		if verbose>=2 put "skipped:    ", convert(path#fname#ext to quoted), " (by extension)", nl then
		return
	then

// load file

	var f = file(path#fname#ext)
	if count f==0 || count f>500000
		if verbose>=2 put "skipped:    ", convert(path#fname#ext to quoted), " (by size)", nl then
		return
	then


// calculate ASCII percentage

	var a=0,i=0
	do
		while ++i <= count f
		var c = charcode(f[i])
		a += ( c>=32 && c<127 ) || c==10 || c==13 || c==9
	loop

	if a/count f < 0.85
		if verbose
			put "non-ascii:  ", convert(path#fname#ext to quoted)
			put " --> (", int(100*(count f-a)/count f), "%)", nl
		then
		return
	then


// split into words

	var word_db  = {}
	var word,a,e = 0

	do
		a = e
		do
			while ++a<=count f
			var c = f[a]
			until (c>="a"&&c<="~") || (c>="A"&&c<="]") || c>=charstr(128)
		loop

		while a<=count f

		e = a
		do
			while ++e<=count f
			var c = f[e]
			while (c>="a"&&c<="~") || (c>="A"&&c<="]") || c>=charstr(128)
		loop

		word  = f[a to e-1]

		if exists word_db.@word
			word_db.@word += 1
		else
			word_db ##= { @word=1 }
		then
	loop


// remove 7-bit-only ascii words

	var i=0
	do
		while ++i<=count word_db
		word = name word_db[i]
		var j=0
		do
			while ++j<=count word
			c = word[j]
			until (c>="{"&&c<="~") || (c>="["&&c<="]") || c>=charstr(128)
		loop
		if j>count word del word_db[i--] then
	loop

	if count word_db==0
		if verbose>=2 put "ascii:      ", convert(path#fname#ext to quoted),nl then
		return
	then

	var i=0
	do
		while ++i <= count word_db
		word_db[i] = name word_db[i]	// text := name
	loop


// test possible conversions:

	var conv = { utf8=99, rtos=99, mac_roman=99, atari=99, latin_1=99, dos=99 }
	var db2 = word_db
	convert db2 from asciius
	if db2==word_db
		conv = { asciius=99, german=99 } 		// 7-bit only
	else
		db2 = word_db
		convert db2 from utf8
		convert db2 to utf8
		if db2!=word_db del conv.utf8 then
	then

	var i=0
	do
		while ++i<=count conv
		var db2 = word_db
		word = name conv[i]
		convert db2 from @word
		convert db2 to lower

		var j=0
		do
			while ++j <= count db2
			if exists good_words.@(db2[j]) del db2[j] j-- then
		loop

		conv[i] = count db2
		until conv[i]==0
	loop

	sort conv
	do while conv[count conv]>conv[1] del conv[count conv] loop


	if count conv > 1
		var f1=f
		convert f1 from @(name conv[1])
		do
			while count conv > 1
			var f2 = f
			convert f2 from @(name conv[2])
			if f1==f2 del conv[2] else exit then
		loop
	then


	if count conv == 1
		word = name conv[1]
		if word=="asciius" || word=="utf8"		// nothing to do?
			if verbose>=2 put word,":", "          "[count word to], convert(path#fname#ext to quoted),nl then
			return
		then
		if verbose put word,":", "          "[count word to], convert(path#fname#ext to quoted),nl then
		if pretend return then

		if backup file (path#fname#ext#"~") = f then

	// doit: choosing open..put..close to fix line breaks as well
		convert f from @word
		split f
		if f[count f]=="" del f[count f] then
		openout #4,path#fname#ext
		var i=0
		do
			while ++i<=count f
			put #4, f[i],nl
		loop
		close #4
		return
	then

	log "unknown:    ", convert(path#fname#ext to quoted), nl

	if verbose
		log "candidates: "
		var i=0
		do
			while ++i <= count conv
			log " ", name conv[i]
		loop
		put nl

		log "words:       "
		var i=0
		do
			while ++i<=count word_db
			word = name word_db[i]
			var j=0
			do while ++j<=count word
				if charcode(word[j])>=127 || charcode(word[j])<32
					word = word[to j-1] # "_$" # hexstr(charcode(word[j]),2) # "_" # word[j+1 to]
				then
			loop
			log word," "
		loop
		put nl
 	then
}




var fname,path,j,i=0
do
	while ++i <= argc
	path = fullpath(globals[i])
	j = rfind(path,"/")
	if j==0 log "no slash found in ", convert(path to quoted) put nl end 1 then
	fname = path[j+1 to]
	path  = path[to j]
	doit(path,fname)
loop


put "--- done ---",nl
end 0


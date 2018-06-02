// utf-8
/*	Copyright  (c)	Günter Woigk  2000-2003
  					mailto:kio@little-bat.de

 	This program is distributed in the hope that it will be useful,
 	but WITHOUT ANY WARRANTY; without even the implied warranty of
 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 	Permission to use, copy, modify, distribute, and sell this software and
 	its documentation for any purpose is hereby granted without fee, provided
 	that the above copyright notice appear in all copies and that both that
 	copyright notice and this permission notice appear in supporting
 	documentation, and that the name of the copyright holder not be used
 	in advertising or publicity pertaining to distribution of the software
 	without specific, written prior permission.  The copyright holder makes no
 	representations about the suitability of this software for any purpose.
 	It is provided "as is" without express or implied warranty.

 	THE COPYRIGHT HOLDER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 	INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 	EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 	CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 	DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 	TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 	PERFORMANCE OF THIS SOFTWARE.
*/

/* ==========================================

		INITIALISIERUNG

		wird im frameset ((top)) durchlaufen!

========================================== */


/* ---- Global Data ----
*/
var aKap = 0;
var Ihvz = new Array(500);	/* inhaltsverzeichnis = liste aller kapitel			*/
var nKap = 0;				/* anzahl kapitel									*/


/* ---- Creator für Kapitel ----
*/
function Kapitel(Level,Name,Link)
{
	this.Level = Level;		/* Verzeichnisebene; [0 .. 2]						*/
	this.Name  = Name;		/* Angezeigter Text									*/
	this.Link  = Link;		/* Zugehöriger Link									*/
	this.Offen = 0;			/* Aufgeklappt? (falls untergeordnete Ebenen ex.)	*/
	this.Idx   = 0;			/* Index in Ihvz[] (rückreferenz)					*/
}


/* ---- Inhaltsverzeichnis erstellen ----
*/
Ihvz[nKap++] = new Kapitel(	0, "introduction",							"00-about_vipsi.html");
Ihvz[nKap++] = new Kapitel(		1, "about vipsi",						"00-about_vipsi.html");
Ihvz[nKap++] = new Kapitel(		1, "about this guide",					"00-about_guide.html");
Ihvz[nKap++] = new Kapitel(		1, "copyright &amp; license",			"00-copyright&amp;license.html");

Ihvz[nKap++] = new Kapitel(   1, "links",								"00-links.html");
Ihvz[nKap++] = new Kapitel(     2, "your personal page",				"http://sourceforge.net/my/");
//Ihvz[nKap++] = new Kapitel(     	3, "login",							"http://sourceforge.net/account/login.php");
//Ihvz[nKap++] = new Kapitel(     	3, "your personal page",			"http://sourceforge.net/my/");
//Ihvz[nKap++] = new Kapitel(     	3, "logout",						"http://sourceforge.net/account/logout.php");
Ihvz[nKap++] = new Kapitel(     2, "project summary page",				"http://sourceforge.net/projects/vipsi/");
Ihvz[nKap++] = new Kapitel(     	3, "admin",							"http://sourceforge.net/project/admin/?group_id=51240");
Ihvz[nKap++] = new Kapitel(     	3, "forums",						"http://sourceforge.net/forum/?group_id=51240");
Ihvz[nKap++] = new Kapitel(     	3, "tracker",						"http://sourceforge.net/tracker/?group_id=51240");
Ihvz[nKap++] = new Kapitel(     	3, "bugs",							"http://sourceforge.net/tracker/?group_id=51240");
Ihvz[nKap++] = new Kapitel(     	3, "support",						"http://sourceforge.net/tracker/?group_id=51240");
//Ihvz[nKap++] = new Kapitel(     	3, "patches",						"http://sourceforge.net/tracker/?group_id=51240");
Ihvz[nKap++] = new Kapitel(     	3, "mailing lists",					"http://sourceforge.net/mail/?group_id=51240");
Ihvz[nKap++] = new Kapitel(     	3, "tasks",							"http://sourceforge.net/pm/?group_id=51240");
//Ihvz[nKap++] = new Kapitel(     	3, "docs",							"http://sourceforge.net/docman/?group_id=51240");
//Ihvz[nKap++] = new Kapitel(     	3, "surveys",						"http://sourceforge.net/survey/?group_id=51240");
Ihvz[nKap++] = new Kapitel(     	3, "news",							"http://sourceforge.net/news/?group_id=51240");
Ihvz[nKap++] = new Kapitel(     	3, "CVS",							"http://sourceforge.net/cvs/?group_id=51240");
Ihvz[nKap++] = new Kapitel(     	3, "downloads",						"http://sourceforge.net/project/showfiles.php?group_id=51240");
Ihvz[nKap++] = new Kapitel(     2, "browse CVS repository",				"http://cvs.sourceforge.net/cgi-bin/viewcvs.cgi/vipsi/");
Ihvz[nKap++] = new Kapitel(     2, "mailto project admin",				"mailto:kio&#64;little-bat.de");
Ihvz[nKap++] = new Kapitel(     2, "sourceforge.net",					"http://sourceforge.net' target='_blank");
Ihvz[nKap++] = new Kapitel(     2, "sourceforge documentation",			"http://sourceforge.net/docman/?group_id=1");
Ihvz[nKap++] = new Kapitel(     2, "little-bat.de",						"http://little-bat.de' target='little_bat");
Ihvz[nKap++] = new Kapitel(     	3, "downloads",						"http://little-bat.de#prog' target='little_bat");
Ihvz[nKap++] = new Kapitel(     	3, "play Sokoban",					"http://little-bat.de/sokoban/sokoban.html");
Ihvz[nKap++] = new Kapitel(     	3, "play GunShi",					"http://little-bat.de/gunshi/gunshi.html");


Ihvz[nKap++] = new Kapitel( 0, "1: installation",						"01-install.html");
Ihvz[nKap++] = new Kapitel(   1, "MacOS X",								"01-install.html#1");
Ihvz[nKap++] = new Kapitel(     2, "installing the application",		"01-install.html#11");
Ihvz[nKap++] = new Kapitel(     2, "preparations for cgi",				"01-install.html#12");
Ihvz[nKap++] = new Kapitel(     2, "notes for developers",				"01-install.html#13");
Ihvz[nKap++] = new Kapitel(     2, "compiling the source",				"01-install.html#14");


Ihvz[nKap++] = new Kapitel( 0, "2: vipsi basics",						"02-basics.html");
Ihvz[nKap++] = new Kapitel(   1, "what's it all about?",				"02-basics.html#1");
Ihvz[nKap++] = new Kapitel(   1, "getting help",						"02-basics.html#1a");
Ihvz[nKap++] = new Kapitel(   1, "a smidgen of vipsi code",				"02-basics.html#2");
Ihvz[nKap++] = new Kapitel(   1, "vipsi command syntax",				"02-basics.html#3");
Ihvz[nKap++] = new Kapitel(     2, "instructions",						"02-basics.html#31");
Ihvz[nKap++] = new Kapitel(     2, "expressions",						"02-basics.html#32");
Ihvz[nKap++] = new Kapitel(     2, "operators",							"02-basics.html#33");
Ihvz[nKap++] = new Kapitel(     2, "functions",							"02-basics.html#34");
Ihvz[nKap++] = new Kapitel(     2, "flow control",						"02-basics.html#35");
Ihvz[nKap++] = new Kapitel(     2, "identifiers (names)",				"02-basics.html#36");
Ihvz[nKap++] = new Kapitel(   1, "sequence of evaluation",				"02-basics.html#5");
Ihvz[nKap++] = new Kapitel(   1, "modes of operation",					"02-basics.html#4");
Ihvz[nKap++] = new Kapitel(     2, "interactive mode",					"02-basics.html#41");
Ihvz[nKap++] = new Kapitel(     2, "running scripts",					"02-basics.html#42");
Ihvz[nKap++] = new Kapitel(     2, "running cgi scripts",				"02-basics.html#43");
Ihvz[nKap++] = new Kapitel(     2, "quick calculation",					"02-basics.html#44");


Ihvz[nKap++] = new Kapitel( 0, "3: data types",							"03-types.html");
Ihvz[nKap++] = new Kapitel(   1, "numbers",								"03-types.html#1");
Ihvz[nKap++] = new Kapitel(   1, "texts and strings",					"03-types.html#2");
Ihvz[nKap++] = new Kapitel(   1, "lists and arrays",					"03-types.html#3");
Ihvz[nKap++] = new Kapitel(   1, "procedures",							"03-types.html#4");


Ihvz[nKap++] = new Kapitel( 0, "4: the built-in words",					"04-words.html");
Ihvz[nKap++] = new Kapitel(   1, "separators",							"04-sep.html");
Ihvz[nKap++] = new Kapitel(     2, "\",\"",								"04-sep.html#komma");
Ihvz[nKap++] = new Kapitel(     2, "\";\"",								"04-sep.html#semikolon");
Ihvz[nKap++] = new Kapitel(     2, "\"( ... )\"",						"04-sep.html#rk");
Ihvz[nKap++] = new Kapitel(     2, "\"[ ... ]\"",						"04-sep.html#ek");
Ihvz[nKap++] = new Kapitel(     2, "\"{ ... }\"",						"04-sep.html#gk");

Ihvz[nKap++] = new Kapitel(   1, "constants",							"04-const.html");
Ihvz[nKap++] = new Kapitel(     2, "ec",								"04-const.html#1");
Ihvz[nKap++] = new Kapitel(     2, "pi",								"04-const.html#2");
Ihvz[nKap++] = new Kapitel(     2, "nl",								"04-const.html#3");
Ihvz[nKap++] = new Kapitel(     2, "tab",								"04-const.html#4");
Ihvz[nKap++] = new Kapitel(     2, "stdin",								"04-const.html#5");
Ihvz[nKap++] = new Kapitel(     2, "stdout",							"04-const.html#6");
Ihvz[nKap++] = new Kapitel(     2, "stderr",							"04-const.html#7");

Ihvz[nKap++] = new Kapitel(   1, "operators",							"04-op.html");
Ihvz[nKap++] = new Kapitel(     2, "operator precedence",				"04-op.html#1");
Ihvz[nKap++] = new Kapitel(     2, "automatic type casting",			"04-op.html#1a");
Ihvz[nKap++] = new Kapitel(     2, "\"++\" increment",					"04-op.html#2");
Ihvz[nKap++] = new Kapitel(     2, "\"--\" decrement",					"04-op.html#3");
Ihvz[nKap++] = new Kapitel(     2, "\"!\" boolean not",					"04-op.html#4");
Ihvz[nKap++] = new Kapitel(     2, "\"~\" bitwise not",					"04-op.html#5");
Ihvz[nKap++] = new Kapitel(     2, "\"@\" make identifier",				"04-op.html#6");
Ihvz[nKap++] = new Kapitel(     2, "\"=\" assignment",					"04-op.html#7");
Ihvz[nKap++] = new Kapitel(     2, "\"+=\" add",						"04-op.html#8");
Ihvz[nKap++] = new Kapitel(     2, "\"-=\" subtract",					"04-op.html#9");
Ihvz[nKap++] = new Kapitel(     2, "\"*=\" multiply",					"04-op.html#10");
Ihvz[nKap++] = new Kapitel(     2, "\"/=\" divide",						"04-op.html#11");
Ihvz[nKap++] = new Kapitel(     2, "\"%=\" remainder",					"04-op.html#12");
Ihvz[nKap++] = new Kapitel(     2, "\"#=\" concatenate texts",			"04-op.html#13");
Ihvz[nKap++] = new Kapitel(     2, "\"##=\" concatenate lists",			"04-op.html#35");
Ihvz[nKap++] = new Kapitel(     2, "\"&gt;&gt;=\" shift right",			"04-op.html#36");
Ihvz[nKap++] = new Kapitel(     2, "\"&lt;&lt;=\" shift left",			"04-op.html#37");
Ihvz[nKap++] = new Kapitel(     2, "\"&amp;&amp;=\" boolean and",		"04-op.html#38");
Ihvz[nKap++] = new Kapitel(     2, "\"||=\" boolean or",				"04-op.html#39");
Ihvz[nKap++] = new Kapitel(     2, "\"&amp;=\" bitwise and",			"04-op.html#40");
Ihvz[nKap++] = new Kapitel(     2, "\"|=\" bitwise or",					"04-op.html#41");
Ihvz[nKap++] = new Kapitel(     2, "\"^=\" bitwise exclusive or",		"04-op.html#42");
Ihvz[nKap++] = new Kapitel(     2, "\"+\" add",							"04-op.html#14");
Ihvz[nKap++] = new Kapitel(     2, "\"-\" subtract",					"04-op.html#15");
Ihvz[nKap++] = new Kapitel(     2, "\"*\" multiply",					"04-op.html#16");
Ihvz[nKap++] = new Kapitel(     2, "\"/\" divide",						"04-op.html#17");
Ihvz[nKap++] = new Kapitel(     2, "\"%\" remainder",					"04-op.html#18");
Ihvz[nKap++] = new Kapitel(     2, "\"#\" concatenate texts",			"04-op.html#19");
Ihvz[nKap++] = new Kapitel(     2, "\"##\" concatenate lists",			"04-op.html#19a");
Ihvz[nKap++] = new Kapitel(     2, "\"&gt;\" compare",					"04-op.html#20");
Ihvz[nKap++] = new Kapitel(     2, "\"&lt;\" compare",					"04-op.html#21");
Ihvz[nKap++] = new Kapitel(     2, "\"&gt;=\" compare",					"04-op.html#22");
Ihvz[nKap++] = new Kapitel(     2, "\"&lt;=\" compare",					"04-op.html#23");
Ihvz[nKap++] = new Kapitel(     2, "\"==\" compare",					"04-op.html#24");
Ihvz[nKap++] = new Kapitel(     2, "\"!=\" compare",					"04-op.html#25");
Ihvz[nKap++] = new Kapitel(     2, "\"&lt;&lt;\" shift left",			"04-op.html#26");
Ihvz[nKap++] = new Kapitel(     2, "\"&gt;&gt;\" shift right",			"04-op.html#27");
Ihvz[nKap++] = new Kapitel(     2, "\"&amp;&amp;\" boolean and",		"04-op.html#28");
Ihvz[nKap++] = new Kapitel(     2, "\"||\" boolean or",					"04-op.html#29");
Ihvz[nKap++] = new Kapitel(     2, "\"&amp;\" bitwise and",				"04-op.html#30");
Ihvz[nKap++] = new Kapitel(     2, "\"|\" bitwise or",					"04-op.html#31");
Ihvz[nKap++] = new Kapitel(     2, "\"^\" bitwise exclusive or",		"04-op.html#32");
Ihvz[nKap++] = new Kapitel(     2, "\".\" named item selector",			"04-op.html#33");
Ihvz[nKap++] = new Kapitel(     2, "\"? :\" select",					"04-op.html#34");

Ihvz[nKap++] = new Kapitel(   1, "instructions",						"04-instr.html");
	var file = "04-instr.html";
	var lvl  = 2;
	function add(idf) { Ihvz[nKap++] = new Kapitel(lvl,idf,file+"#"+idf); }
	add("new");		add("del");		add("swap");	add("var");		add("freeze");	add("melt");
	add("put");		add("log");		add("get");		add("edit");	add("getchar");	add("write");
	add("read");	add("ioctl");	add("eval");	add("call");	add("sys");
	add("include");	add("dir");		add("wait");	add("info");	add("sort");
	add("rsort");	add("split");	add("convert");	add("replace"), add("open");	add("openin");
	add("openout");	add("close");
	add("rename");	add("cd");		add("pwd");		add("words");

Ihvz[nKap++] = new Kapitel(   1, "numeric functions",					"04-fu-num.html");
	var file = "04-fu-num.html";
	var lvl  = 2;
	add("mdate"); add("count"); add("index"); add("random"); add("abs"); add("sign"); add("int"); add("fract");
	add("round"); add("cos"); add("sin"); add("tan"); add("acos"); add("asin"); add("atan"); add("cosh");
	add("sinh"); add("tanh"); add("logn"); add("log2"); add("loge"); add("log10"); add("exp"); add("exp2");
	add("expe"); add("exp10"); add("square"); add("sqrt"); add("charcode"); add("now"); add("dateval"); add("find");
	add("rfind"); add("istext"); add("isnumber"); add("islist"); add("isproc"); add("isfile");
	add("isdir"); add("exists"); add("match"); add("min"); add("max");

Ihvz[nKap++] = new Kapitel(   1, "text (string) functions",				"04-fu-str.html");
	var file = "04-fu-str.html";
	var lvl  = 2;
	add("name"); add("string"); add("upperstr"); add("lowerstr"); add("charstr"); add("hexstr"); add("binstr");
	add("substr"); add("midstr"); add("leftstr"); add("rightstr"); add("datestr"); add("fullpath");
	add("file"); 

Ihvz[nKap++] = new Kapitel(   1, "list returning functions",						"04-fu-other.html");
	var file = "04-fu-other.html";
	var lvl  = 2;
	add("env"); add("dir");

Ihvz[nKap++] = new Kapitel(   1, "flow control (loops, conditions)",	"04-flow.html");
	var file = "04-flow.html";
	var lvl  = 2;
	add("proc"); add("return"); add("end"); add("do"); add("loop"); add("while"); add("until"); add("exit");
	add("next"); add("if"); add("then"); add("else"); add("try");

Ihvz[nKap++] = new Kapitel( 0, "5: example scripts",					"05-scripts.html");
Ihvz[nKap++] = new Kapitel(   1, "show my source",						"scripts/show_my_source.html");
Ihvz[nKap++] = new Kapitel(   1, "count words",							"scripts/count_words.html");
Ihvz[nKap++] = new Kapitel(   1, "show command line arguments",			"scripts/show_command_line_arguments.html");
Ihvz[nKap++] = new Kapitel(   1, "language test suite",					"scripts/test.html");
Ihvz[nKap++] = new Kapitel(   1, "cgi: show environment",				"scripts/show_environment.html");
Ihvz[nKap++] = new Kapitel(   1, "cgi: show querry arguments",			"scripts/show_querry_arguments.html");
Ihvz[nKap++] = new Kapitel(   1, "cgi: mine sweeper",					"scripts/mine_sweeper.html");


Ihvz[nKap++] = new Kapitel( 0, "6: bugs, errors, et.al.",				"06-bugs.html");
Ihvz[nKap++] = new Kapitel(   1, "known bugs",							"06-bugs.html");
Ihvz[nKap++] = new Kapitel(   1, "syntax cave-ats",						"06-caveat.html");
Ihvz[nKap++] = new Kapitel(   1, "todo list",							"06-todo.html");
Ihvz[nKap++] = new Kapitel(   1, "done list",							"06-done.html");


Ihvz[nKap++] = new Kapitel( 0, "7: developer documentation",			"07-docs.html");
Ihvz[nKap++] = new Kapitel(   1, "the beauty within",					"07-docs.html#1");
Ihvz[nKap++] = new Kapitel(   1, "class Var",							"http://little-bat.de/prog/download/library/var.htm");
Ihvz[nKap++] = new Kapitel(   1, "class String",						"http://little-bat.de/prog/download/library/string.htm");
Ihvz[nKap++] = new Kapitel(   1, "predefined data types",				"07-docs.html#3");
Ihvz[nKap++] = new Kapitel(   1, "VScript::Value ( ... )",				"07-docs.html#4");
Ihvz[nKap++] = new Kapitel(   1, "project files",						"07-docs.html#5");
Ihvz[nKap++] = new Kapitel(   1, "how to use the CVS",					"08-cvs.html");


//Ihvz[nKap++] = new Kapitel( 0, "Glossary",							"99-glossary.html");


//Ihvz[nKap++] = new Kapitel( 0, "Index",								"99-index.html");



for (var i=0;i<nKap;i++) Ihvz[i].Idx = i;		/* kapitelindizees nachbessern */
Ihvz[0].Offen = 1;								/* introduction anfangs immer ausklappen */



/* =======================================

		FUNKTIONEN

	werden alle von nav aus aufgerufen!

	Beobachtungen:	
	Variablenkontext = top
	write()-Kontext  = nav

======================================= */



/* ---- Ein Kapitel vollformatiert ausgeben ----
*/
function PrintKapitel(idx)
{
	with(nav.document)
	{
		var kap = Ihvz[idx];
		var lvl = Ihvz[idx].Level;

		write("<table width='100%'>");

		do
		{
			write("<tr valign=top>");
			var ukap= idx+1<nKap && Ihvz[idx+1].Level>lvl;		// unterkapitel vorhanden ?

			if (ukap)
			{
				var src = kap.Offen ? "b/dn.gif" : "b/rt.gif";
				write( "<td width=11><a href='nav.html' onClick='javascript:top.Zoom("+idx+");return 0;'>",
					   "<img src='"+src+"' border=0 width=11 height=9></a></td>" );
			}

			else	
			{
				write( "<td width=11><img src='b/o.gif' width=11 height=9></td>" );
			}

			if(lvl==0)	write( "<td><b><a href='", kap.Link, "' target='main'>", kap.Name, "</a></b></td>" );
			else		write( "<td><a href='", kap.Link,    "' target='main'>", kap.Name, "</a></td>" );

			if (ukap)
			{
				if(kap.Offen)
				{
					write("</tr><tr><td></td><td>");
					idx = PrintKapitel(idx+1);
					write("</td>");
				}
				else
				{
					do { idx++; } while(idx<nKap&&Ihvz[idx].Level>lvl);
				}
			}
			else
			{
				idx++;
			}

			write("</tr>");

			if(idx>=nKap) break;
			kap = Ihvz[idx];
		}
		while (kap.Level==lvl);

		writeln("</table>");
	}

	return idx;
}


/* ---- Inhaltsverzeichnis vollformatiert ausgeben ----
		Achtung: die Verwendung von <base target='main'>
		resultierte bei IE5.1, Mozilla6.0, Chimera0.22 in Zoom() in einer teilweisen Ausgabeumleitung in den 'main' Frame...
*/
function PrintInhaltsverzeichnis()
{	
	nav.document.writeln
	("	<html><head><TITLE>vipsi - manual - table of contents</TITLE>\
		<link rel=stylesheet type='text/css' href='nav.css'>\
	</head>\
	<body bgcolor='#CCCCCC' text='#000000' link='#000099' vlink='#003399' alink='#990000'>\
	<h3>vipsi</h3>");
	
	PrintKapitel(0);
	
	nav.document.writeln
	("</body></html>");
}


/* ---- Mausklick auf Einklapp/Ausklapp-Icon behandeln ----
*/
function Zoom(idx)
{
	aKap=idx;
	Ihvz[idx].Offen = !Ihvz[idx].Offen;

	nav.location.href="nav.html";
}









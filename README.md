## vipsi

This is a scripting language which i developed for personal use and to run the cgi scripts on the web server on my desktop PC at [https://k1.spdns.de].

It should compile on unix-style OSes like BSD, macos and Linux, though not strictly tested.

This project requires my [Libraries](https://github.com/Megatokio/Libraries.git) project side-by-side with this project in the same folder. 

When started with no argument, vipsi tries to load the script "shell.vs" and run in interactive mode. It searches for directory "Libs" in the current working directory or "/usr/local/lib/vipsi" and some other places to contain it.
You can also start it with this or any other script file explicitely. 

Most important command may be "words" and "info <any_word>".


TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += c++11
CONFIG += precompiled_header

QMAKE_CXXFLAGS += -Wno-deprecated-declarations

#unix:!macx: LIBS += -L/usr/lib/x86_64-linux-gnu/ -lcurl
linux-g++ {
LIBS += -pthread
}

# http://qt-project.org/faq/answer/what_does_the_syntax_configdebugdebugrelease_mean_what_does_the_1st_argumen
CONFIG(release,debug|release) { DEFINES += NDEBUG RELEASE } # ATTN: curly brace must start in same line!
CONFIG(debug,debug|release) { DEFINES += DEBUG } # ATTN: curly brace must start in same line!


INCLUDEPATH +=	\
	.			\
	Source		\
	Libraries	\


SOURCES +=  \
	Source/BPObj.cpp \
	Source/Compiler.cpp \
	Source/Execute.cpp \
	Source/file_utilities.cpp \
	Source/Irpt.cpp \
	Source/main.cpp \
	Source/Sema.cpp \
	Source/Stream.cpp \
	Source/SystemIO.cpp \
	Source/Thread.cpp \
	Source/VScript.cpp \
	Source/freeze.cpp \
	Source/serrors.cpp \
	Libraries/kio/kio.cpp \
	Libraries/kio/exceptions.cpp \
	Libraries/cstrings/cstrings.cpp \
	Libraries/cstrings/utf8.cpp \
	Libraries/cstrings/ucs1.cpp \
	Libraries/cstrings/ucs2.cpp \
	Libraries/cstrings/ucs4.cpp \
	Source/Var/NameHandles.cpp \
	Source/Var/Var.cpp \
	Source/VString/String.cpp \
	Source/VString/String_test_suite.cpp \
	Libraries/unix/log.cpp \
	Libraries/unix/tempmem.cpp \
	Libraries/unix/os_utilities.cpp \
	Libraries/cstrings/legacy_charsets.cpp \

HEADERS += \
	Libraries/kio/auto_config.h \
	Libraries/kio/detect_configuration.h \
	Libraries/kio/errors.h \
	Source/BPObj.h \
	Source/CObj.h \
	Source/Compiler.h \
	Source/custom_errors.h \
	Source/Execute.h \
	Source/globals.h \
	Source/Irpt.h \
	Source/Sema.h \
	Source/serrors.h \
	Source/settings.h \
	Source/Stream.h \
	Source/SystemIO.h \
	Source/Thread.h \
	Source/token.h \
	Source/VScript.h \
	Source/compress_errors.h \
	Source/freeze.h \
	config.h \
	Libraries/kio/error_emacs.h \
	Libraries/kio/exceptions.h \
	Libraries/kio/kio.h \
	Libraries/kio/peekpoke.h \
	Libraries/cstrings/cstrings.h \
	Source/Var/NameHandles.h \
	Source/Var/var_errors.h \
	Source/Var/Var.h \
	Source/VString/String_errors.h \
	Source/VString/String.h \
	Libraries/unix/log.h \
	Libraries/unix/tempmem.h \
	Libraries/unix/os_utilities.h \
	Libraries/kio/standard_types.h \
	Libraries/cstrings/legacy_charsets.h \
	Libraries/cstrings/legacy_charsets/ascii_ger.h \
	Libraries/cstrings/legacy_charsets/ascii_us.h \
	Libraries/cstrings/legacy_charsets/atari_st.h \
	Libraries/cstrings/legacy_charsets/cp_437.h \
	Libraries/cstrings/legacy_charsets/html.h \
	Libraries/cstrings/legacy_charsets/mac_roman.h \
	Libraries/cstrings/legacy_charsets/rtos.h \
	Libraries/Templates/Stack.h \
	Libraries/Templates/sort.h \
	Libraries/Templates/Array.h \
	Libraries/unix/s_type.h \
	Libraries/unix/FD.h \
	Libraries/Templates/template_helpers.h \
	Libraries/Templates/relational_operators.h \
	Libraries/Templates/sorter.h \
	Libraries/unix/MyFileInfo.h \
	Libraries/cstrings/ucs1.h \
	Libraries/cstrings/ucs4.h \
	Libraries/cstrings/utf8.h

DISTFILES += \
	Libs/test_suite/brokenNames.tar.gz \
	Libs/vip.xml \
	Libs/shootout/Mandelbrot_200x200.pbm \
	Libs/shootout/Mandelbrot_64x64.pbm \
	Libs/shootout/Ackermann.vs \
	Libs/shootout/BinaryTrees.vs \
	Libs/shootout/CountWords.vs \
	Libs/shootout/HelloWorld.vs \
	Libs/shootout/Mandelbrot.vs \
	Libs/shootout/TakFP.vs \
	Libs/shootout/wc-input.txt \
	Libs/test_suite/00_test.vs \
	Libs/test_suite/each_and_every.vs \
	Libs/test_suite/files_etc.vs \
	Libs/test_suite/functions_list.vs \
	Libs/test_suite/functions_numeric.vs \
	Libs/test_suite/functions_text.vs \
	Libs/test_suite/include_sys_call.vs \
	Libs/test_suite/instructions_branching.vs \
	Libs/test_suite/names.vs \
	Libs/test_suite/old_bugs.vs \
	Libs/test_suite/old_bugs_pruning_lists.vs \
	Libs/test_suite/operators_compare.vs \
	Libs/test_suite/operators_numeric.vs \
	Libs/test_suite/operators_pruning.vs \
	Libs/test_suite/operators_text.vs \
	Libs/test_suite/procedures.vs \
	Libs/test_suite/variable_protection.vs \
	Libs/BOOK.vl \
	Libs/DIR.vl \
	Libs/HTML.vl \
	Libs/script.prefs \
	Libs/shell.prefs \
	Libs/shell.vs \
	Libs/speedometer.vs \
	Libs/testdvd.vs \
	Libs/TEXT.vl \
	Libs/touch_version.vs \
	Libs/vipsi.cgi.conf



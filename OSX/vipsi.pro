TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += c++11
CONFIG += precompiled_header

#LIBS += -pthread

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
    Libraries/kio/exceptions.cpp \
    Libraries/cstrings/cstrings.cpp \
    Libraries/Var/NameHandles.cpp \
    Libraries/Var/Var.cpp \
    Libraries/VString/String.cpp \
    Libraries/unix/log.cpp \
    Libraries/unix/tempmem.cpp \
    Libraries/Unicode/Unicode.cpp \
    Libraries/Unicode/UnicodeLegacyCharsets.cpp \
    Libraries/Unicode/UTF-8.cpp \
    Libraries/unix/os_utilities.cpp \
    Libraries/VString/serrors.cpp

HEADERS += \
    Source/BPObj.h \
    Source/CObj.h \
    Source/Compiler.h \
    Source/custom_errors.h \
    Source/Execute.h \
    Source/file_utilities.h \
    Source/globals.h \
    Source/Irpt.h \
    Source/Sema.h \
    Source/settings.h \
    Source/Stream.h \
    Source/SystemIO.h \
    Source/Thread.h \
    Source/token.h \
    Source/version.h \
    Source/VScript.h \
    Source/compress_errors.h \
    Source/freeze.h \
    Source/errors.h \
    config.h \
    Libraries/kio/error_emacs.h \
    Libraries/kio/exceptions.h \
    Libraries/kio/kio.h \
    Libraries/kio/peekpoke.h \
    Libraries/cstrings/cstrings.h \
    Libraries/Var/NameHandles.h \
    Libraries/Var/var_errors.h \
    Libraries/Var/Var.h \
    Libraries/VString/String_errors.h \
    Libraries/VString/String.h \
    Libraries/unix/log.h \
    Libraries/unix/tempmem.h \
    Libraries/Unicode/unicode_errors.h \
    Libraries/Unicode/Unicode.h \
    Libraries/Unicode/UTF-8.h \
    Libraries/unix/os_utilities.h \



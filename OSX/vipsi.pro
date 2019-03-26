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
    Libraries/kio/standard_types.h \
    Libraries/kio/errors.h \
    Libraries/Unicode/Includes/Blocks.h \
    Libraries/Unicode/Includes/CanonicalCombiningClass.h \
    Libraries/Unicode/Includes/CanonicalCombiningClassUCS4.h \
    Libraries/Unicode/Includes/CodePointRanges.h \
    Libraries/Unicode/Includes/CodePointRanges_UCS4.h \
    Libraries/Unicode/Includes/EastAsianWidth.h \
    Libraries/Unicode/Includes/EastAsianWidth_UCS4.h \
    Libraries/Unicode/Includes/GeneralCategory.h \
    Libraries/Unicode/Includes/GeneralCategory_UCS4.h \
    Libraries/Unicode/Includes/GlobalConstants.h \
    Libraries/Unicode/Includes/Names.h \
    Libraries/Unicode/Includes/NumericValue.h \
    Libraries/Unicode/Includes/NumericValue_UCS4.h \
    Libraries/Unicode/Includes/Property_Enum.h \
    Libraries/Unicode/Includes/Property_Groups.h \
    Libraries/Unicode/Includes/Property_LongNames.h \
    Libraries/Unicode/Includes/Property_ShortNames.h \
    Libraries/Unicode/Includes/PropertyGroup_Enum.h \
    Libraries/Unicode/Includes/PropertyGroup_Names.h \
    Libraries/Unicode/Includes/PropertyValue_Enum.h \
    Libraries/Unicode/Includes/PropertyValue_LongNames.h \
    Libraries/Unicode/Includes/PropertyValue_ShortNames.h \
    Libraries/Unicode/Includes/Scripts.h \
    Libraries/Unicode/Includes/Scripts_UCS4.h \
    Libraries/Unicode/Includes/SimpleLowercase_Even.h \
    Libraries/Unicode/Includes/SimpleLowercase_Odd.h \
    Libraries/Unicode/Includes/SimpleLowercase_UCS4.h \
    Libraries/Unicode/Includes/SimpleTitlecase.h \
    Libraries/Unicode/Includes/SimpleUppercase_Even.h \
    Libraries/Unicode/Includes/SimpleUppercase_Odd.h \
    Libraries/Unicode/Includes/SimpleUppercase_UCS4.h \
    Libraries/Unicode/Includes/SpecialCasing.h \
    Libraries/Unicode/Includes/UnicodeData_Index.h \
    Libraries/Unicode/Includes/UnicodeData_Names.h \
    Libraries/Unicode/Headers/8859-1.h \
    Libraries/Unicode/Headers/8859-2.h \
    Libraries/Unicode/Headers/ascii_ger.h \
    Libraries/Unicode/Headers/ascii_us.h \
    Libraries/Unicode/Headers/atari_st.h \
    Libraries/Unicode/Headers/cp_437.h \
    Libraries/Unicode/Headers/html.h \
    Libraries/Unicode/Headers/mac_roman.h \
    Libraries/Unicode/Headers/rtos.h \
    Libraries/Templates/Stack.h \
    Libraries/Templates/sort.h \
    Libraries/Templates/Array.h \
    Libraries/unix/s_type.h \
    Libraries/unix/FD.h \
    Libraries/unix/files.h

DISTFILES += \
    ../Libs/test_suite/brokenNames.tar.gz \
    ../Libs/vip.xml \
    ../Libs/shootout/Mandelbrot_200x200.pbm \
    ../Libs/shootout/Mandelbrot_64x64.pbm \
    ../Libs/shootout/Ackermann.vs \
    ../Libs/shootout/BinaryTrees.vs \
    ../Libs/shootout/CountWords.vs \
    ../Libs/shootout/HelloWorld.vs \
    ../Libs/shootout/Mandelbrot.vs \
    ../Libs/shootout/TakFP.vs \
    ../Libs/shootout/wc-input.txt \
    ../Libs/test_suite/00_test.vs \
    ../Libs/test_suite/each_and_every.vs \
    ../Libs/test_suite/files_etc.vs \
    ../Libs/test_suite/functions_list.vs \
    ../Libs/test_suite/functions_numeric.vs \
    ../Libs/test_suite/functions_text.vs \
    ../Libs/test_suite/include_sys_call.vs \
    ../Libs/test_suite/instructions_branching.vs \
    ../Libs/test_suite/names.vs \
    ../Libs/test_suite/old_bugs.vs \
    ../Libs/test_suite/old_bugs_pruning_lists.vs \
    ../Libs/test_suite/operators_compare.vs \
    ../Libs/test_suite/operators_numeric.vs \
    ../Libs/test_suite/operators_pruning.vs \
    ../Libs/test_suite/operators_text.vs \
    ../Libs/test_suite/procedures.vs \
    ../Libs/test_suite/variable_protection.vs \
    ../Libs/BOOK.vl \
    ../Libs/DIR.vl \
    ../Libs/HTML.vl \
    ../Libs/script.prefs \
    ../Libs/shell.prefs \
    ../Libs/shell.vs \
    ../Libs/speedometer.vs \
    ../Libs/testdvd.vs \
    ../Libs/TEXT.vl \
    ../Libs/touch_version.vs \
    ../Libs/vipsi.cgi.conf



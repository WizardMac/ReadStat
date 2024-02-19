# Update this with each release. Be sure to change VS17/PropertySheet.props too
AC_INIT([ReadStat],[1.1.9],[https://github.com/WizardMac/ReadStat/issues],[readstat],[https://github.com/WizardMac/ReadStat])
AM_INIT_AUTOMAKE([foreign subdir-objects])
AM_SILENT_RULES([yes])

# Don't forget to update these too
READSTAT_MAJOR_VERSION=1
READSTAT_MINOR_VERSION=1
READSTAT_MICRO_VERSION=9

# libtool stuff, see https://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html#Updating-version-info
READSTAT_VERSION=$READSTAT_MAJOR_VERSION.$READSTAT_MINOR_VERSION.$READSTAT_MICRO_VERSION
# TODO the following logic is broken. Fix it during the next minor update.
# See https://github.com/libxls/libxls/commit/cf5b0500e78e77a70361c6de12b08230590857dc
READSTAT_VERSION_INFO=`expr $READSTAT_MAJOR_VERSION + $READSTAT_MINOR_VERSION`:$READSTAT_MICRO_VERSION:$READSTAT_MINOR_VERSION

AC_SUBST(READSTAT_VERSION)
AC_SUBST(READSTAT_VERSION_INFO)

LT_INIT([disable-static])

AC_PROG_CC
AC_PROG_CXX

AC_ARG_ENABLE([code-coverage], AS_HELP_STRING([--enable-code-coverage], [Enable code coverage profiling]), [code_coverage=yes], [code_coverage=no])
AC_ARG_ENABLE([fuzz-testing], AS_HELP_STRING([--enable-fuzz-testing], ["Enable fuzz testing (requires Clang 6 or later)"]), [
   AC_MSG_CHECKING([whether $CC accepts -fsanitize=fuzzer])
   tmp_saved_flags=$[]_AC_LANG_PREFIX[]FLAGS
   _AC_LANG_PREFIX[]FLAGS="$[]_AC_LANG_PREFIX[]FLAGS -fsanitize=fuzzer"
   AC_LINK_IFELSE([AC_LANG_PROGRAM()],
       [
        AC_MSG_RESULT(yes)
        SANITIZERS="-fsanitize=fuzzer"
        fuzzer=yes],
        AC_MSG_RESULT(no)
       AC_MSG_FAILURE([-fsanitize=fuzzer not supported (Required with --enable-fuzz-testing)]))
   _AC_LANG_PREFIX[]FLAGS=$tmp_saved_flags
], [SANITIZERS=""
    fuzzer=no])
AM_CONDITIONAL([FUZZER_ENABLED], test "x$fuzzer" = "xyes")
AC_SUBST([SANITIZERS])

AM_ICONV

AC_CANONICAL_HOST
AS_CASE([$host],
	[*linux*|*bsd*|*mingw*|*cygwin*|*gnu*], [EXTRA_LIBS="-lm"],
	[EXTRA_LIBS=""])
AC_SUBST([EXTRA_LIBS])

AS_CASE([$host],
	[*mingw*], [EXTRA_WARNINGS="-Wno-pedantic-ms-format -Wno-stringop-truncation"],
    [*cygwin*], [EXTRA_WARNINGS="-Wno-unused-const-variable"],
    [EXTRA_WARNINGS=""])
AC_SUBST([EXTRA_WARNINGS])

AS_CASE([$host],
	[*mingw*|*cygwin*], [EXTRA_LDFLAGS="$LTLIBICONV -no-undefined"],
	[EXTRA_LDFLAGS="$LTLIBICONV"])
AC_SUBST([EXTRA_LDFLAGS])

AS_CASE([$host],
	[*mingw*], 
        [
        #There is no real way to determine whether UCRT is used vs normal MINGW but one way is by checking a define in <windows.h>
        #If we are in UCRT we do not want to set the spawnv define because it breaks the build
        IS_UCRT=`echo '#include <windows.h>'| gcc -E -dM -  | grep -i _UCRT | xargs`
        AS_IF([test "$IS_UCRT" == ""], [
            CFLAGS="$CFLAGS -D_spawnv=spawnv"
        ])
    ],
	[CFLAGS="$CFLAGS"])

AC_ARG_VAR([LIB_FUZZING_ENGINE], [Location of prebuilt fuzzing engine library])
AC_SUBST([LIB_FUZZING_ENGINE])

AC_ARG_VAR([RAGEL], [Ragel generator command])
AC_ARG_VAR([RAGELFLAGS], [Ragel generator flags])
AC_PATH_PROG([RAGEL], [ragel], [true])
AM_CONDITIONAL([HAVE_RAGEL], test "$RAGEL" != "true")

AC_CHECK_LIB([xlsxwriter], [workbook_new], [true], [false])
AM_CONDITIONAL([HAVE_XLSXWRITER], test "$ac_cv_lib_xlsxwriter_workbook_new" = yes)

AC_CHECK_LIB([csv], [csv_parse], [true], [false])
AM_CONDITIONAL([HAVE_CSVREADER], test "$ac_cv_lib_csv_csv_parse" = yes)

AC_CHECK_LIB([z], [deflate], [true], [false])
AM_CONDITIONAL([HAVE_ZLIB], test "$ac_cv_lib_z_deflate" = yes)

AM_CONDITIONAL([CODE_COVERAGE_ENABLED], test "x$code_coverage" = "xyes")

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

AC_MSG_RESULT([
Configuration:

C compiler: $CC
CFLAGS: $CFLAGS
LD: $LD

C++ compiler: $CXX
CXXFLAGS: $CXXFLAGS
CXXLD: $CXXLD

Host: $host
Extra warnings: $EXTRA_WARNINGS
Extra libs: $EXTRA_LIBS
Extra ld flags: $EXTRA_LDFLAGS

Fuzzing engine: $LIB_FUZZING_ENGINE

Ragel: $RAGEL
Ragel flags: $RAGELFLAGS])

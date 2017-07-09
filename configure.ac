AC_INIT([readstat], [0.1.1])
AM_INIT_AUTOMAKE([foreign subdir-objects])
AM_SILENT_RULES([yes])

LT_INIT([disable-static])

AC_PROG_CC

AC_ARG_ENABLE([static-iconv], AS_HELP_STRING([--enable-static-iconv], [Link iconv statically]), [static_iconv=yes], [static_iconv=no])
AC_ARG_ENABLE([code-coverage], AS_HELP_STRING([--enable-code-coverage], [Enable code coverage profiling]), [code_coverage=yes], [code_coverage=no])

AC_CANONICAL_HOST
AS_CASE([$host],
	[*darwin*], [EXTRA_LIBS="-liconv"],
	[*linux*], [EXTRA_LIBS="-lm"],
	[*mingw*|*cygwin*], [AS_IF([test "x$static_iconv" = "xyes"],
                               [EXTRA_LIBS="-lm"],
                               [EXTRA_LIBS="-liconv -lm"])],
	[EXTRA_LIBS=""])
AC_SUBST([EXTRA_LIBS])

AS_CASE([$host],
	[*mingw*], [EXTRA_WARNINGS="-Wno-pedantic-ms-format"],
    [EXTRA_WARNINGS=""])
AC_SUBST([EXTRA_WARNINGS])

AS_CASE([$host],
	[*mingw*|*cygwin*], [AS_IF([test "x$static_iconv" = "xyes"],
                               [EXTRA_LDFLAGS="-Wl,-Bstatic,-liconv -no-undefined"],
                               [EXTRA_LDFLAGS="-no-undefined"])],
	[EXTRA_LDFLAGS=""])
AC_SUBST([EXTRA_LDFLAGS])

AC_ARG_VAR([RAGEL], [Ragel generator command])
AC_ARG_VAR([RAGELFLAGS], [Ragel generator flags])
AC_PATH_PROG([RAGEL], [ragel], [true])
AM_CONDITIONAL([HAVE_RAGEL], test "$RAGEL" != "true")
AC_CHECK_LIB([xlsxwriter], [workbook_new], [true], [false])
AM_CONDITIONAL([HAVE_XLSXWRITER], test "$ac_cv_lib_xlsxwriter_workbook_new" = yes)
AC_CHECK_LIB([csv], [csv_parse], [true], [false])
AM_CONDITIONAL([HAVE_CSVREADER], test "$ac_cv_lib_csv_csv_parse" = yes)
AM_CONDITIONAL([CODE_COVERAGE_ENABLED], test "x$code_coverage" = "xyes")

AC_OUTPUT([Makefile])

AC_MSG_RESULT([
Configuration:

C compiler: $CC
CFLAGS: $CFLAGS

Host: $host
Extra warnings: $EXTRA_WARNINGS
Extra libs: $EXTRA_LIBS
Extra ld flags: $EXTRA_LDFLAGS

Ragel: $RAGEL
Ragel flags: $RAGELFLAGS])

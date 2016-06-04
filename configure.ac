AC_INIT([readstat], [20160511])
AM_INIT_AUTOMAKE([foreign subdir-objects])
AM_SILENT_RULES([yes])
LT_INIT([disable-static])
AC_PROG_CC
AC_CANONICAL_HOST
AS_CASE([$host],
	[*darwin*], [EXTRA_LIBS="-liconv"],
	[*linux*], [EXTRA_LIBS="-lm"],
	[EXTRA_LIBS=""]
)
AC_SUBST([EXTRA_LIBS])
AC_ARG_VAR([RAGEL], [Ragel generator command])
AC_ARG_VAR([RAGELFLAGS], [Ragel generator flags])
AC_PATH_PROG([RAGEL], [ragel], [true])
AM_CONDITIONAL([HAVE_RAGEL], test "$RAGEL" != "true")
AC_CHECK_LIB([xlsxwriter], [workbook_new], [true], [false])
AM_CONDITIONAL([HAVE_XLSXWRITER], test "$ac_cv_lib_xlsxwriter_workbook_new" = yes)
AC_OUTPUT([Makefile])

AC_MSG_RESULT([
Configuration:

C compiler: $CC
CFLAGS: $CFLAGS

Host: $host
Host specific libs: $EXTRA_LIBS

Ragel: $RAGEL
Ragel flags: $RAGELFLAGS])

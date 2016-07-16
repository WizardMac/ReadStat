AC_INIT([ReadStat], [0.1.1], [emmiller@gmail.com], [ReadStat], [https://github.com/WizardMac/ReadStat])
AC_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE([foreign subdir-objects])
AM_SILENT_RULES([yes])

LT_INIT([disable-static])

AC_PROG_CC

AC_ARG_ENABLE([static-iconv], AS_HELP_STRING([--enable-static-iconv], [Link iconv statically]))

AC_CHECK_LIB([iconv], [iconv_open], [true], [false])
AM_CONDITIONAL([HAVE_ICONV], test x"$ac_cv_lib_iconv_iconv_open" = "xyes")

AC_CHECK_LIB([m], [pow], [true], [false])
AM_CONDITIONAL([HAVE_MATH], test x"$ac_cv_lib_m_pow" = "xyes")

AC_CHECK_LIB([lzma], [lzma_stream_decoder], [true], [AC_MSG_ERROR([lzma not found])])
AM_CONDITIONAL([HAVE_LZMA], test x"$ac_cv_lib_lzma_lzma_stream_decoder" = "xyes")

AC_CHECK_LIB([z], [zlibVersion], [true], [AC_MSG_ERROR([zlib not found])])
AM_CONDITIONAL([HAVE_ZLIB], test x"$ac_cv_lib_z_zlibVersion" = "xyes")

AC_CHECK_LIB([xlsxwriter], [workbook_new], [true], [false])
AM_CONDITIONAL([HAVE_XLSXWRITER], test x"$ac_cv_lib_xlsxwriter_workbook_new" = "xyes")

AC_CANONICAL_HOST
AS_CASE([$host],
	[*darwin*], [EXTRA_LDFLAGS=""],
	[*linux*], [EXTRA_LDFLAGS=""],
	[*mingw*|*cygwin*], [AS_IF([test "x$enable_static_iconv" = "xyes"], [EXTRA_LDFLAGS="-Wl,-Bstatic,-liconv -no-undefined"], [EXTRA_LDFLAGS="-no-undefined"])],
	[EXTRA_LDFLAGS=""]
)
AC_SUBST([EXTRA_LDFLAGS])

AC_ARG_VAR([RAGEL], [Ragel generator command])
AC_ARG_VAR([RAGELFLAGS], [Ragel generator flags])

AC_PATH_PROG([RAGEL], [ragel], [true])
AM_CONDITIONAL([HAVE_RAGEL], test "$RAGEL" != "true")

AC_OUTPUT([Makefile])

AC_MSG_RESULT([
Configuration:

C compiler: $CC
CFLAGS: $CFLAGS

Host: $host
Extra ld flags: $EXTRA_LDFLAGS

Ragel: $RAGEL
Ragel flags: $RAGELFLAGS])

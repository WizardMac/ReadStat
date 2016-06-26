lib_LTLIBRARIES = libreadstat.la

libreadstat_la_SOURCES = \
	src/CKHashTable.c \
	src/readstat_bits.c \
	src/readstat_convert.c \
	src/readstat_dta.c \
	src/readstat_dta_parse_timestamp.c \
	src/readstat_dta_read.c \
	src/readstat_dta_write.c \
	src/readstat_error.c \
	src/readstat_io_unistd.c \
	src/readstat_parser.c \
	src/readstat_por.c \
	src/readstat_por_parse.c \
	src/readstat_por_read.c \
	src/readstat_rdata.c \
	src/readstat_sas.c \
	src/readstat_sas_catalog.c \
	src/readstat_sas_data.c \
	src/readstat_sav.c \
	src/readstat_sav_parse.c \
	src/readstat_sav_parse_timestamp.c \
	src/readstat_sav_read.c \
	src/readstat_sav_write.c \
	src/readstat_spss.c \
	src/readstat_spss_parse.c \
	src/readstat_value.c \
	src/readstat_variable.c \
	src/readstat_writer.c

if HAVE_RAGEL
.rl.c:
	$(AM_V_GEN)$(RAGEL) $(RAGELFLAGS) -C $< -o $@
endif

libreadstat_la_CFLAGS = -Wall
libreadstat_la_LIBADD = -llzma -lz @EXTRA_LIBS@
libreadstat_la_LDFLAGS = -no-undefined

include_HEADERS = src/readstat.h

bin_PROGRAMS = \
   readstat

readstat_SOURCES = \
	src/bin/readstat.c \
	src/bin/module_util.c \
	src/bin/modules/mod_csv.c \
	src/bin/modules/mod_readstat.c

readstat_LDADD = libreadstat.la
readstat_CFLAGS =

if HAVE_XLSXWRITER
readstat_SOURCES += src/bin/modules/mod_xlsx.c
readstat_LDADD += -lxlsxwriter
readstat_CFLAGS += -DHAVE_XLSXWRITER=1
endif

check_PROGRAMS = \
	test_readstat

test_readstat_SOURCES = \
	src/test/test_buffer.c \
	src/test/test_dta.c \
	src/test/test_error.c \
	src/test/test_read.c \
	src/test/test_readstat.c \
	src/test/test_write.c

test_readstat_LDADD = libreadstat.la
test_readstat_CFLAGS = -g

TESTS = test_readstat

install-exec-hook:
	@(cd $(DESTDIR)$(libdir) && $(RM) $(lib_LTLIBRARIES))

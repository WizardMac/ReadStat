lib_LTLIBRARIES = libreadstat.la

libreadstat_la_SOURCES = \
	src/CKHashTable.c \
	src/readstat_bits.c \
	src/readstat_convert.c \
	src/readstat_error.c \
	src/readstat_io_unistd.c \
	src/readstat_parser.c \
	src/readstat_value.c \
	src/readstat_variable.c \
	src/readstat_writer.c \
	src/sas/ieee.c \
	src/sas/readstat_sas.c \
	src/sas/readstat_sas7bcat_read.c \
	src/sas/readstat_sas7bcat_write.c \
	src/sas/readstat_sas7bdat_read.c \
	src/sas/readstat_sas7bdat_write.c \
	src/sas/readstat_sas_rle.c \
	src/sas/readstat_xport.c \
	src/sas/readstat_xport_read.c \
	src/sas/readstat_xport_write.c \
	src/spss/readstat_por.c \
	src/spss/readstat_por_parse.c \
	src/spss/readstat_por_read.c \
	src/spss/readstat_por_write.c \
	src/spss/readstat_sav.c \
	src/spss/readstat_sav_parse.c \
	src/spss/readstat_sav_parse_timestamp.c \
	src/spss/readstat_sav_read.c \
	src/spss/readstat_sav_write.c \
	src/spss/readstat_spss.c \
	src/spss/readstat_spss_parse.c \
	src/stata/readstat_dta.c \
	src/stata/readstat_dta_parse_timestamp.c \
	src/stata/readstat_dta_read.c \
	src/stata/readstat_dta_write.c

if HAVE_RAGEL
.rl.c:
	$(AM_V_GEN)$(RAGEL) $(RAGELFLAGS) -C $< -o $@
endif

libreadstat_la_CFLAGS = -Wall @EXTRA_WARNINGS@ -Werror -pedantic-errors -std=c99
libreadstat_la_LIBADD = @EXTRA_LIBS@
libreadstat_la_LDFLAGS = @EXTRA_LDFLAGS@

if CODE_COVERAGE_ENABLED
libreadstat_la_CFLAGS += -O0 -fprofile-arcs -ftest-coverage
endif

include_HEADERS = src/readstat.h

noinst_HEADERS = \
       src/CKHashTable.h \
       src/readstat_bits.h \
       src/readstat_convert.h \
       src/readstat_iconv.h \
       src/readstat_io_unistd.h \
       src/readstat_writer.h \
       src/sas/ieee.h \
       src/sas/readstat_sas.h \
       src/sas/readstat_sas_rle.h \
       src/sas/readstat_xport.h \
       src/spss/readstat_por.h \
       src/spss/readstat_por_parse.h \
       src/spss/readstat_sav.h \
       src/spss/readstat_sav_parse.h \
       src/spss/readstat_sav_parse_timestamp.h \
       src/spss/readstat_spss.h \
       src/spss/readstat_spss_parse.h \
       src/stata/readstat_dta.h \
       src/stata/readstat_dta_parse_timestamp.h \
       src/bin/module.h \
       src/bin/format.h \
       src/bin/extract_metadata.h \
       src/bin/extract_label_set.h \
       src/bin/write_missing_values.h \
       src/bin/write_value_labels.h \
       src/bin/module_util.h \
       src/bin/modules/double_decimals.h \
       src/bin/modules/mod_csv.h \
       src/bin/modules/jsmn.h \
       src/bin/modules/json_metadata.h \
       src/bin/modules/produce_csv_value_dta.h \
       src/bin/modules/produce_csv_value_sav.h \
       src/bin/modules/produce_csv_value_csv.h \
       src/bin/modules/produce_csv_value.h \
       src/bin/modules/produce_csv_column_header.h \
       src/bin/modules/produce_value_label.h \
       src/bin/modules/produce_value_label_dta.h \
       src/bin/modules/produce_value_label_sav.h \
       src/bin/modules/produce_missingness.h \
       src/bin/modules/produce_missingness_dta.h \
       src/bin/modules/produce_missingness_sav.h \
       src/bin/modules/mod_readstat.h \
       src/bin/modules/mod_xlsx.h \
       src/test/test_buffer.h \
       src/test/test_dta.h \
       src/test/test_error.h \
       src/test/test_read.h \
       src/test/test_readstat.h \
       src/test/test_types.h \
       src/test/test_write.h

bin_PROGRAMS = \
   readstat \
   extract_metadata

readstat_SOURCES = \
	src/bin/readstat.c \
	src/bin/format.c \
	src/bin/module_util.c \
	src/bin/modules/double_decimals.c \
	src/bin/modules/mod_csv.c \
	src/bin/modules/jsmn.c \
	src/bin/modules/json_metadata.c \
	src/bin/modules/produce_csv_value_dta.c \
	src/bin/modules/produce_csv_value_sav.c \
	src/bin/modules/produce_csv_value_csv.c \
	src/bin/modules/produce_csv_value.c \
	src/bin/modules/produce_value_label.c \
	src/bin/modules/produce_value_label_dta.c \
	src/bin/modules/produce_value_label_sav.c \
	src/bin/modules/produce_missingness.c \
	src/bin/modules/produce_missingness_dta.c \
	src/bin/modules/produce_missingness_sav.c \
	src/bin/modules/produce_csv_column_header.c \
	src/bin/modules/mod_readstat.c \
	src/bin/util/readstat_dta_days.c \
	src/bin/util/readstat_sav_date.c

extract_metadata_SOURCES = \
	src/bin/format.c \
	src/bin/extract_label_set.c \
	src/bin/write_missing_values.c \
	src/bin/write_value_labels.c \
	src/bin/extract_metadata.c \
	src/bin/util/readstat_dta_days.c \
	src/bin/util/readstat_sav_date.c

readstat_LDADD = libreadstat.la
readstat_CFLAGS = -Wall -Werror -pedantic-errors -std=c99

extract_metadata_LDADD = libreadstat.la
extract_metadata_CFLAGS = -Wall -Werror -pedantic-errors -std=c99

if HAVE_XLSXWRITER
readstat_SOURCES += src/bin/modules/mod_xlsx.c
readstat_LDADD += -lxlsxwriter
readstat_CFLAGS += -DHAVE_XLSXWRITER=1
endif

if HAVE_CSVREADER
readstat_SOURCES += src/bin/modules/mod_csv_reader.c
readstat_LDADD += -lcsv
readstat_CFLAGS += -DHAVE_CSVREADER=1
endif

check_PROGRAMS = \
	test_readstat \
	test_dta_days \
	test_sav_date \
	test_double_decimals

test_readstat_SOURCES = \
	src/test/test_buffer.c \
	src/test/test_dta.c \
	src/test/test_error.c \
	src/test/test_read.c \
	src/test/test_readstat.c \
	src/test/test_sas.c \
	src/test/test_write.c

test_readstat_LDADD = libreadstat.la
test_readstat_CFLAGS = -g -Wall @EXTRA_WARNINGS@ -Werror -pedantic-errors -std=c99

test_dta_days_SOURCES = \
	src/bin/util/readstat_dta_days.c \
	src/test/test_dta_days.c

test_dta_days_LDADD = libreadstat.la
test_dta_days_CFLAGS = -g -Wall @EXTRA_WARNINGS@ -Werror -pedantic-errors -std=c99

test_sav_date_SOURCES = \
	src/bin/util/readstat_sav_date.c \
	src/test/test_sav_date.c

test_sav_date_LDADD = libreadstat.la
test_sav_date_CFLAGS = -g -Wall @EXTRA_WARNINGS@ -Werror -pedantic-errors -std=c99

test_double_decimals_SOURCES = \
	src/bin/modules/double_decimals.c \
	src/test/test_double_decimals.c

test_double_decimals_CFLAGS = -g -Wall @EXTRA_WARNINGS@ -Werror -pedantic-errors -std=c99


TESTS = test_readstat test_dta_days test_sav_date test_double_decimals

install-exec-hook:
	@(cd $(DESTDIR)$(libdir) && $(RM) $(lib_LTLIBRARIES))

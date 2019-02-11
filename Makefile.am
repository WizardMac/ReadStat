AUTOMAKE_OPTIONS = foreign dist-zip

lib_LTLIBRARIES = libreadstat.la

libreadstat_la_SOURCES = \
	src/CKHashTable.c \
	src/readstat_bits.c \
	src/readstat_convert.c \
	src/readstat_error.c \
	src/readstat_io_unistd.c \
	src/readstat_malloc.c \
	src/readstat_metadata.c \
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
	src/spss/readstat_sav_compress.c \
	src/spss/readstat_sav_parse.c \
	src/spss/readstat_sav_parse_timestamp.c \
	src/spss/readstat_sav_read.c \
	src/spss/readstat_sav_write.c \
	src/spss/readstat_spss.c \
	src/spss/readstat_spss_parse.c \
	src/stata/readstat_dta.c \
	src/stata/readstat_dta_parse_timestamp.c \
	src/stata/readstat_dta_read.c \
	src/stata/readstat_dta_write.c \
	src/txt/commands_util.c \
	src/txt/readstat_copy.c \
	src/txt/readstat_sas_commands_read.c \
	src/txt/readstat_spss_commands_read.c \
	src/txt/readstat_schema.c \
	src/txt/readstat_stata_dictionary_read.c \
	src/txt/readstat_txt_read.c

if HAVE_ZLIB
libreadstat_la_SOURCES += \
	src/spss/readstat_zsav_compress.c \
	src/spss/readstat_zsav_read.c \
	src/spss/readstat_zsav_write.c
endif

if HAVE_RAGEL
.rl.c:
	$(AM_V_GEN)$(RAGEL) $(RAGELFLAGS) -C $< -o $@
endif

libreadstat_la_CFLAGS = -g -Wall @EXTRA_WARNINGS@ -Werror -pedantic-errors -std=c99
libreadstat_la_LIBADD = @EXTRA_LIBS@
libreadstat_la_LDFLAGS = @EXTRA_LDFLAGS@ -version-info @READSTAT_VERSION_INFO@

if HAVE_ZLIB
libreadstat_la_LIBADD += -lz
libreadstat_la_CFLAGS += -DHAVE_ZLIB=1
endif

if CODE_COVERAGE_ENABLED
libreadstat_la_CFLAGS += -O0 -fprofile-arcs -ftest-coverage
endif

if FUZZER_ENABLED
libreadstat_la_CFLAGS += -fsanitize=fuzzer-no-link
libreadstat_la_LDFLAGS += -fsanitize=fuzzer
endif

dist_man1_MANS = man/readstat.man man/extract_metadata.man

include_HEADERS = src/readstat.h

noinst_HEADERS = \
       src/CKHashTable.h \
       src/readstat_bits.h \
       src/readstat_convert.h \
       src/readstat_iconv.h \
       src/readstat_io_unistd.h \
       src/readstat_malloc.h \
       src/readstat_writer.h \
       src/sas/ieee.h \
       src/sas/readstat_sas.h \
       src/sas/readstat_sas_rle.h \
       src/sas/readstat_xport.h \
       src/spss/readstat_por.h \
       src/spss/readstat_por_parse.h \
       src/spss/readstat_sav.h \
       src/spss/readstat_sav_compress.h \
       src/spss/readstat_sav_parse.h \
       src/spss/readstat_sav_parse_timestamp.h \
       src/spss/readstat_spss.h \
       src/spss/readstat_spss_parse.h \
       src/spss/readstat_zsav_compress.h \
       src/spss/readstat_zsav_read.h \
       src/spss/readstat_zsav_write.h \
       src/stata/readstat_dta.h \
       src/stata/readstat_dta_parse_timestamp.h \
       src/txt/commands_util.h \
       src/txt/readstat_copy.h \
       src/txt/readstat_schema.h \
       src/bin/extract_metadata.h \
       src/bin/read_csv/csv_metadata.h \
       src/bin/read_csv/jsmn.h \
       src/bin/read_csv/json_metadata.h \
       src/bin/read_csv/mod_csv.h \
       src/bin/read_csv/mod_dta.h \
       src/bin/read_csv/mod_sav.h \
       src/bin/read_csv/read_csv.h \
       src/bin/read_csv/read_module.h \
       src/bin/read_csv/value.h \
       src/bin/write/double_decimals.h \
       src/bin/write/json/write_missing_values.h \
       src/bin/write/json/write_value_labels.h \
       src/bin/write/mod_csv.h \
       src/bin/write/mod_readstat.h \
       src/bin/write/mod_xlsx.h \
       src/bin/write/module.h \
       src/bin/write/module_util.h \
       src/bin/util/file_format.h \
       src/bin/util/quote_and_escape.h \
       src/bin/util/readstat_dta_days.h \
       src/bin/util/readstat_sav_date.h \
       src/fuzz/fuzz_format.h \
       src/test/test_buffer.h \
       src/test/test_buffer_io.h \
       src/test/test_dta.h \
       src/test/test_error.h \
       src/test/test_list.h \
       src/test/test_read.h \
       src/test/test_readstat.h \
       src/test/test_sas.h \
       src/test/test_sav.h \
       src/test/test_types.h \
       src/test/test_write.h

bin_PROGRAMS = \
    extract_metadata \
	readstat

readstat_SOURCES = \
	src/bin/readstat.c \
	src/bin/read_csv/jsmn.c \
	src/bin/read_csv/json_metadata.c \
	src/bin/read_csv/mod_csv.c \
	src/bin/read_csv/mod_dta.c \
	src/bin/read_csv/mod_sav.c \
	src/bin/read_csv/value.c \
	src/bin/write/double_decimals.c \
	src/bin/write/mod_csv.c \
	src/bin/write/mod_readstat.c \
	src/bin/write/module_util.c \
	src/bin/util/file_format.c \
	src/bin/util/quote_and_escape.c \
	src/bin/util/readstat_dta_days.c \
	src/bin/util/readstat_sav_date.c

readstat_LDADD = libreadstat.la
readstat_CFLAGS = -DREADSTAT_VERSION=\"@READSTAT_VERSION@\" -Wall -Werror -pedantic-errors -std=c99
if HAVE_ZLIB
readstat_CFLAGS += -DHAVE_ZLIB=1
endif

extract_metadata_SOURCES = \
	src/bin/extract_metadata.c \
	src/bin/write/json/write_missing_values.c \
	src/bin/write/json/write_value_labels.c \
	src/bin/util/file_format.c \
	src/bin/util/quote_and_escape.c \
	src/bin/util/readstat_dta_days.c \
	src/bin/util/readstat_sav_date.c

extract_metadata_LDADD = libreadstat.la
extract_metadata_CFLAGS = -Wall -Werror -pedantic-errors -std=c99

if HAVE_XLSXWRITER
readstat_SOURCES += src/bin/write/mod_xlsx.c
readstat_LDADD += -lxlsxwriter
readstat_CFLAGS += -DHAVE_XLSXWRITER=1
endif

if HAVE_CSVREADER
readstat_SOURCES += src/bin/read_csv/read_csv.c
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
	src/test/test_buffer_io.c \
	src/test/test_dta.c \
	src/test/test_error.c \
	src/test/test_read.c \
	src/test/test_readstat.c \
	src/test/test_sas.c \
	src/test/test_sav.c \
	src/test/test_write.c

test_readstat_LDADD = libreadstat.la
test_readstat_CFLAGS = -Wall @EXTRA_WARNINGS@ -Werror -pedantic-errors -std=c99 -DDEBUG=1

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
	src/bin/write/double_decimals.c \
	src/test/test_double_decimals.c

test_double_decimals_CFLAGS = -g -Wall @EXTRA_WARNINGS@ -Werror -pedantic-errors -std=c99


TESTS = test_readstat test_dta_days test_sav_date test_double_decimals

EXTRA_PROGRAMS = \
    generate_corpus

generate_corpus_SOURCES = \
	src/fuzz/generate_corpus.c \
	src/test/test_buffer.c \
	src/test/test_write.c \
	src/test/test_read.c \
	src/test/test_buffer_io.c \
	src/test/test_error.c \
	src/test/test_dta.c \
	src/test/test_sas.c \
	src/test/test_sav.c

generate_corpus_LDADD = libreadstat.la
generate_corpus_CFLAGS = -g -Wall @EXTRA_WARNINGS@ -Werror -pedantic-errors -std=c99

EXTRA_PROGRAMS += \
	fuzz_compression_sas_rle \
	fuzz_compression_sav \
	fuzz_format_dta \
	fuzz_format_por \
	fuzz_format_sav \
	fuzz_format_sas7bcat \
	fuzz_format_sas7bdat \
	fuzz_format_xport \
	fuzz_format_sas_commands \
	fuzz_format_spss_commands \
	fuzz_format_stata_dictionary \
	fuzz_grammar_dta_timestamp \
	fuzz_grammar_por_double \
	fuzz_grammar_sav_date \
	fuzz_grammar_sav_time \
	fuzz_grammar_spss_format

# Force C++ linking for fuzz targets
nodist_EXTRA_fuzz_compression_sas_rle_SOURCES = dummy.cxx
nodist_EXTRA_fuzz_compression_sav_SOURCES = dummy.cxx
nodist_EXTRA_fuzz_format_dta_SOURCES = dummy.cxx
nodist_EXTRA_fuzz_format_por_SOURCES = dummy.cxx
nodist_EXTRA_fuzz_format_sav_SOURCES = dummy.cxx
nodist_EXTRA_fuzz_format_sas7bcat_SOURCES = dummy.cxx
nodist_EXTRA_fuzz_format_sas7bdat_SOURCES = dummy.cxx
nodist_EXTRA_fuzz_format_xport_SOURCES = dummy.cxx
nodist_EXTRA_fuzz_format_sas_commands_SOURCES = dummy.cxx
nodist_EXTRA_fuzz_format_spss_commands_SOURCES = dummy.cxx
nodist_EXTRA_fuzz_format_stata_dictionary_SOURCES = dummy.cxx
nodist_EXTRA_fuzz_grammar_dta_timestamp_SOURCES = dummy.cxx
nodist_EXTRA_fuzz_grammar_por_double_SOURCES = dummy.cxx
nodist_EXTRA_fuzz_grammar_sav_date_SOURCES = dummy.cxx
nodist_EXTRA_fuzz_grammar_sav_time_SOURCES = dummy.cxx
nodist_EXTRA_fuzz_grammar_spss_format_SOURCES = dummy.cxx

fuzz_grammar_dta_timestamp_SOURCES = \
	src/fuzz/fuzz_grammar_dta_timestamp.c

fuzz_grammar_dta_timestamp_LDADD = libreadstat.la @LIB_FUZZING_ENGINE@
fuzz_grammar_dta_timestamp_LDFLAGS = -static
fuzz_grammar_dta_timestamp_CFLAGS = -g -Wall @EXTRA_WARNINGS@ -Werror -pedantic-errors -std=c99 @SANITIZERS@

fuzz_grammar_por_double_SOURCES = \
	src/fuzz/fuzz_grammar_por_double.c

fuzz_grammar_por_double_LDADD = libreadstat.la @LIB_FUZZING_ENGINE@
fuzz_grammar_por_double_LDFLAGS = -static
fuzz_grammar_por_double_CFLAGS = -g -Wall @EXTRA_WARNINGS@ -Werror -pedantic-errors -std=c99 @SANITIZERS@

fuzz_grammar_sav_date_SOURCES = \
	src/fuzz/fuzz_grammar_sav_date.c

fuzz_grammar_sav_date_LDADD = libreadstat.la @LIB_FUZZING_ENGINE@
fuzz_grammar_sav_date_LDFLAGS = -static
fuzz_grammar_sav_date_CFLAGS = -g -Wall @EXTRA_WARNINGS@ -Werror -pedantic-errors -std=c99 @SANITIZERS@

fuzz_grammar_sav_time_SOURCES = \
	src/fuzz/fuzz_grammar_sav_time.c

fuzz_grammar_sav_time_LDADD = libreadstat.la @LIB_FUZZING_ENGINE@
fuzz_grammar_sav_time_LDFLAGS = -static
fuzz_grammar_sav_time_CFLAGS = -g -Wall @EXTRA_WARNINGS@ -Werror -pedantic-errors -std=c99 @SANITIZERS@

fuzz_grammar_spss_format_SOURCES = \
	src/fuzz/fuzz_grammar_spss_format.c

fuzz_grammar_spss_format_LDADD = libreadstat.la @LIB_FUZZING_ENGINE@
fuzz_grammar_spss_format_LDFLAGS = -static
fuzz_grammar_spss_format_CFLAGS = -g -Wall @EXTRA_WARNINGS@ -Werror -pedantic-errors -std=c99 @SANITIZERS@

fuzz_format_dta_SOURCES = \
	src/fuzz/fuzz_format.c \
	src/fuzz/fuzz_format_dta.c \
	src/test/test_buffer_io.c

fuzz_format_dta_LDADD = libreadstat.la @LIB_FUZZING_ENGINE@
fuzz_format_dta_LDFLAGS = -static
fuzz_format_dta_CFLAGS = -g -Wall @EXTRA_WARNINGS@ -Werror -pedantic-errors -std=c99 @SANITIZERS@

fuzz_format_por_SOURCES = \
	src/fuzz/fuzz_format.c \
	src/fuzz/fuzz_format_por.c \
	src/test/test_buffer_io.c

fuzz_format_por_LDADD = libreadstat.la @LIB_FUZZING_ENGINE@
fuzz_format_por_LDFLAGS = -static
fuzz_format_por_CFLAGS = -g -Wall @EXTRA_WARNINGS@ -Werror -pedantic-errors -std=c99 @SANITIZERS@

fuzz_format_sas7bcat_SOURCES = \
	src/fuzz/fuzz_format.c \
	src/fuzz/fuzz_format_sas7bcat.c \
	src/test/test_buffer_io.c

fuzz_format_sas7bcat_LDADD = libreadstat.la @LIB_FUZZING_ENGINE@
fuzz_format_sas7bcat_LDFLAGS = -static
fuzz_format_sas7bcat_CFLAGS = -g -Wall @EXTRA_WARNINGS@ -Werror -pedantic-errors -std=c99 @SANITIZERS@

fuzz_format_sas7bdat_SOURCES = \
	src/fuzz/fuzz_format.c \
	src/fuzz/fuzz_format_sas7bdat.c \
	src/test/test_buffer_io.c

fuzz_format_sas7bdat_LDADD = libreadstat.la @LIB_FUZZING_ENGINE@
fuzz_format_sas7bdat_LDFLAGS = -static
fuzz_format_sas7bdat_CFLAGS = -g -Wall @EXTRA_WARNINGS@ -Werror -pedantic-errors -std=c99 @SANITIZERS@

fuzz_format_sav_SOURCES = \
	src/fuzz/fuzz_format.c \
	src/fuzz/fuzz_format_sav.c \
	src/test/test_buffer_io.c

fuzz_format_sav_LDADD = libreadstat.la @LIB_FUZZING_ENGINE@
fuzz_format_sav_LDFLAGS = -static
fuzz_format_sav_CFLAGS = -g -Wall @EXTRA_WARNINGS@ -Werror -pedantic-errors -std=c99 @SANITIZERS@

fuzz_format_xport_SOURCES = \
	src/fuzz/fuzz_format.c \
	src/fuzz/fuzz_format_xport.c \
	src/test/test_buffer_io.c

fuzz_format_xport_LDADD = libreadstat.la @LIB_FUZZING_ENGINE@
fuzz_format_xport_LDFLAGS = -static
fuzz_format_xport_CFLAGS = -g -Wall @EXTRA_WARNINGS@ -Werror -pedantic-errors -std=c99 @SANITIZERS@

fuzz_format_sas_commands_SOURCES = \
	src/fuzz/fuzz_format.c \
	src/fuzz/fuzz_format_sas_commands.c \
	src/test/test_buffer_io.c

fuzz_format_sas_commands_LDADD = libreadstat.la @LIB_FUZZING_ENGINE@
fuzz_format_sas_commands_LDFLAGS = -static
fuzz_format_sas_commands_CFLAGS = -g -Wall @EXTRA_WARNINGS@ -Werror -pedantic-errors -std=c99 @SANITIZERS@

fuzz_format_spss_commands_SOURCES = \
	src/fuzz/fuzz_format.c \
	src/fuzz/fuzz_format_spss_commands.c \
	src/test/test_buffer_io.c

fuzz_format_spss_commands_LDADD = libreadstat.la @LIB_FUZZING_ENGINE@
fuzz_format_spss_commands_LDFLAGS = -static
fuzz_format_spss_commands_CFLAGS = -g -Wall @EXTRA_WARNINGS@ -Werror -pedantic-errors -std=c99 @SANITIZERS@

fuzz_format_stata_dictionary_SOURCES = \
	src/fuzz/fuzz_format.c \
	src/fuzz/fuzz_format_stata_dictionary.c \
	src/test/test_buffer_io.c

fuzz_format_stata_dictionary_LDADD = libreadstat.la @LIB_FUZZING_ENGINE@
fuzz_format_stata_dictionary_LDFLAGS = -static
fuzz_format_stata_dictionary_CFLAGS = -g -Wall @EXTRA_WARNINGS@ -Werror -pedantic-errors -std=c99 @SANITIZERS@

fuzz_compression_sas_rle_SOURCES = \
	src/fuzz/fuzz_compression_sas_rle.c

fuzz_compression_sas_rle_LDADD = libreadstat.la @LIB_FUZZING_ENGINE@
fuzz_compression_sas_rle_LDFLAGS = -static
fuzz_compression_sas_rle_CFLAGS = -g -Wall @EXTRA_WARNINGS@ -Werror -pedantic-errors -std=c99 @SANITIZERS@

fuzz_compression_sav_SOURCES = \
	src/fuzz/fuzz_compression_sav.c

fuzz_compression_sav_LDADD = libreadstat.la @LIB_FUZZING_ENGINE@
fuzz_compression_sav_LDFLAGS = -static
fuzz_compression_sav_CFLAGS = -g -Wall @EXTRA_WARNINGS@ -Werror -pedantic-errors -std=c99 @SANITIZERS@

lib_LTLIBRARIES = libreadstat.la

libreadstat_la_SOURCES = \
	src/CKHashTable.c \
	src/readstat_bits.c \
	src/readstat_convert.c \
	src/readstat_dta.c \
	src/readstat_dta_read.c \
	src/readstat_dta_write.c \
	src/readstat_error.c \
	src/readstat_io_unistd.c \
	src/readstat_parser.c \
	src/readstat_por.c \
	src/readstat_por_parse.c \
	src/readstat_rdata.c \
	src/readstat_sas.c \
	src/readstat_sas_catalog.c \
	src/readstat_sas_data.c \
	src/readstat_sav.c \
	src/readstat_sav_parse.c \
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

libreadstat_la_LIBADD = -llzma -lz
libreadstat_la_LDFLAGS =

include_HEADERS = src/readstat.h

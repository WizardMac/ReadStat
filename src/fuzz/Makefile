# The main file-format fuzzers are built by the top-level directoroy's 
# Makefile.am. This Makefile contains a few extra fuzzers for specific
# grammars that appear within each file (e.g. timestamps, compression)
#
# Copy libFuzzer.a into this directory and then tweak below to your liking.
#
# Usage:
#
# 	make grammars # build the small string-fuzzing binaries
# 	./fuzz_grammar_por_double # etc
# 	./fuzz_grammar_sav_time   # etc
# 	make rle      # build the SAS row-compression fuzzers
# 	./fuzz_rle_sas_compress
# 	./fuzz_rle_sas_decompress
#
# For best results ensure that ReadStat was built with --enable-sanitizers

CC=clang

GRAMMARS = por_double sav_time sav_date spss_format dta_timestamp
RLE = sas

.PHONY: grammars
grammars:
	$(foreach format,$(GRAMMARS), $(CC) -lstdc++ -lreadstat -fsanitize-coverage=trace-pc-guard -fsanitize=address fuzz_grammar_$(format).c $$format libFuzzer.a -o fuzz_grammar_$(format);)
	$(foreach format,$(GRAMMARS), install_name_tool -change /usr/local/lib/libreadstat.0.dylib $(shell pwd)/.libs/libreadstat.0.dylib fuzz_grammar_$(format);)

.PHONY: rle
rle:
	$(foreach format,$(RLE), $(CC) -lstdc++ -L../../.libs -lreadstat -fsanitize-coverage=trace-pc-guard -fsanitize=address fuzz_rle_$(format)_compress.c $$format libFuzzer.a -o fuzz_rle_$(format)_compress;)
	$(foreach format,$(RLE), $(CC) -lstdc++ -L../../.libs -lreadstat -fsanitize-coverage=trace-pc-guard -fsanitize=address fuzz_rle_$(format)_decompress.c $$format libFuzzer.a -o fuzz_rle_$(format)_decompress;)
	$(foreach format,$(RLE), install_name_tool -change /usr/local/lib/libreadstat.0.dylib $(shell pwd)/../../.libs/libreadstat.0.dylib fuzz_rle_$(format)_compress;)
	$(foreach format,$(RLE), install_name_tool -change /usr/local/lib/libreadstat.0.dylib $(shell pwd)/../../.libs/libreadstat.0.dylib fuzz_rle_$(format)_decompress;)

.PHONY: clean
clean:
	$(foreach format,$(GRAMMARS), rm fuzz_grammar_$(format);)
	$(foreach format,$(RLE), rm fuzz_rle_$(format)_compress fuzz_rle_$(format)_decompress;)

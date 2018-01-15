# Copy libFuzzer.a into this directory and then tweak below to your liking.
#
# Usage:
#
# 	make corpus   # generates a corpus of files using the test suite
# 	make formats  # build the big file-fuzzing binaries
# 	make grammars # build the small string-fuzzing binaries
# 	./fuzz_format_dta corpus/dta110 # or whatever
# 	./fuzz_grammar_sav_date 	    # or whatever

CC=clang

corpus:
	$(CC) -fsanitize=address -lreadstat generate_corpus.c ../test/test_buffer.c ../test/test_write.c ../test/test_read.c ../test/test_buffer_io.c ../test/test_error.c ../test/test_dta.c ../test/test_sas.c -o generate_corpus
	mkdir -p corpus
	./generate_corpus

FORMATS = dta por sav sas7bdat sas7bcat xport
GRAMMARS = por_double sav_time sav_date spss_format dta_timestamp
RLE = sas

.PHONY: formats
formats:
	$(foreach format,$(FORMATS), $(CC) -lstdc++ -lreadstat -fsanitize-coverage=trace-pc-guard -fsanitize=address fuzz_format_$(format).c $$format fuzzer.c ../test/test_buffer_io.c libFuzzer.a -o fuzz_format_$(format);)
	$(foreach format,$(FORMATS), install_name_tool -change /usr/local/lib/libreadstat.0.dylib /Users/emiller/Code/ReadStat/.libs/libreadstat.0.dylib fuzz_format_$(format);)

.PHONY: grammars
grammars:
	$(foreach format,$(GRAMMARS), $(CC) -lstdc++ -lreadstat -fsanitize-coverage=trace-pc-guard -fsanitize=address fuzz_grammar_$(format).c $$format libFuzzer.a -o fuzz_grammar_$(format);)
	$(foreach format,$(GRAMMARS), install_name_tool -change /usr/local/lib/libreadstat.0.dylib /Users/emiller/Code/ReadStat/.libs/libreadstat.0.dylib fuzz_grammar_$(format);)

.PHONY: rle
rle:
	$(foreach format,$(RLE), $(CC) -lstdc++ -L../../.libs -lreadstat -fsanitize-coverage=trace-pc-guard -fsanitize=address fuzz_rle_$(format)_compress.c $$format libFuzzer.a -o fuzz_rle_$(format)_compress;)
	$(foreach format,$(RLE), $(CC) -lstdc++ -L../../.libs -lreadstat -fsanitize-coverage=trace-pc-guard -fsanitize=address fuzz_rle_$(format)_decompress.c $$format libFuzzer.a -o fuzz_rle_$(format)_decompress;)
	$(foreach format,$(RLE), install_name_tool -change /usr/local/lib/libreadstat.0.dylib /Users/emiller/Code/ReadStat/.libs/libreadstat.0.dylib fuzz_rle_$(format)_compress;)
	$(foreach format,$(RLE), install_name_tool -change /usr/local/lib/libreadstat.0.dylib /Users/emiller/Code/ReadStat/.libs/libreadstat.0.dylib fuzz_rle_$(format)_decompress;)

.PHONY: clean
clean:
	rm fuzzer_dta
	rm fuzzer_sas7bdat
	rm generate_corpus
	rm -r corpus/*

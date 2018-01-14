# Copy libFuzzer.a into this directory and then tweak below to your liking.
#
# Usage:
#
# 	make corpus  # generates a corpus of files using the test suite
# 	make fuzzers # build the fuzzing binaries
# 	./fuzzer_dta corpus/dta110 # or whatever

CC=clang

corpus:
	$(CC) -fsanitize=address -lreadstat generate_corpus.c ../test/test_buffer.c ../test/test_write.c ../test/test_read.c ../test/test_buffer_io.c ../test/test_error.c ../test/test_dta.c ../test/test_sas.c -o generate_corpus
	mkdir -p corpus
	./generate_corpus

FORMATS = dta por sav sas7bdat sas7bcat xport

.PHONY: fuzzers
fuzzers:
	$(foreach format,$(FORMATS), $(CC) -lstdc++ -lreadstat -fsanitize-coverage=trace-pc-guard -fsanitize=address fuzzer_$(format).c $$format fuzzer.c ../test/test_buffer_io.c libFuzzer.a -o fuzzer_$(format);)
	$(foreach format,$(FORMATS), install_name_tool -change /usr/local/lib/libreadstat.0.dylib /Users/emiller/Code/ReadStat/.libs/libreadstat.0.dylib fuzzer_$(format);)

.PHONY: clean
clean:
	rm fuzzer_dta
	rm fuzzer_sas7bdat
	rm generate_corpus
	rm -r corpus/*

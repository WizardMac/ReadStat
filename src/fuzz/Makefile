# Copy libFuzzer.a into this directory and then tweak below to your liking.
#
# Usage:
#
# 	make corpus # generates a corpus of files using the test suite
# 	make fuzzer # build a fuzzing binary
# 	./fuzzer corpus/dta110 # or whatever
#
# As of now you'll also need to tweak fuzzer.c to parse the file type you're
# fuzzing

CC=clang

corpus:
	$(CC) -fsanitize=address -lreadstat generate_corpus.c ../test/test_buffer.c ../test/test_write.c ../test/test_read.c ../test/test_buffer_io.c ../test/test_error.c ../test/test_dta.c ../test/test_sas.c -o generate_corpus
	mkdir -p corpus
	./generate_corpus

fuzzer:
	$(CC) -lstdc++ -lreadstat -fsanitize-coverage=trace-pc-guard -fsanitize=address fuzzer.c ../test/test_buffer_io.c libFuzzer.a -o fuzzer
	install_name_tool -change /usr/local/lib/libreadstat.0.dylib /Users/emiller/Code/ReadStat/.libs/libreadstat.0.dylib fuzzer

clean:
	rm fuzzer
	rm generate_corpus
	rm -r corpus/*

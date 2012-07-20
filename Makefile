
CC=gcc
RAGEL=/usr/local/bin/ragel

all:
	@mkdir -p obj
	$(RAGEL) src/readstat_por_parse.rl -G2
	$(RAGEL) src/readstat_sav_parse.rl -G2
	$(CC) src/*.c -dynamiclib -o obj/libreadstat.dylib -Wall -Werror

clean:
	rm -f src/readstat_por_parse.c
	rm -f src/readstat_sav_parse.c
	rm -rf obj

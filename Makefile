
CC=clang
PREFIX=/usr/local
RAGEL=/usr/local/bin/ragel
DYLIB=obj/libreadstat.dylib

all:
	@mkdir -p obj
	$(RAGEL) src/readstat_por_parse.rl -G2
	$(RAGEL) src/readstat_sav_parse.rl -G2
	$(CC) -Os src/*.c -dynamiclib -o $(DYLIB) -llzma -Wall -Wno-multichar -Werror

install: all
	@mkdir -p $(PREFIX)/lib
	@cp $(DYLIB) $(PREFIX)/lib/

clean:
	rm -f src/readstat_por_parse.c
	rm -f src/readstat_sav_parse.c
	rm -rf obj

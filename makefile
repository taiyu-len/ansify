CFLAGS=-std=c11 -Wall -Wextra
LDLIBS=-lm
PREFIX=/usr/local

ansify: ansify.c

clean:
	rm ansify

install: ansify
	@echo :: installing 'ansify' in directory $(PREFIX)/bin/ansify
	sudo install -m 755 ansify $(PREFIX)/bin/ansify

.PHONY: clean
.PHONY: install

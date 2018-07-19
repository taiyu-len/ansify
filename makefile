CFLAGS=-std=c11 -Wall -Wextra -g
LDLIBS=-lm
PREFIX=/usr/local

ansify: main.o ansify.o

clean:
	rm ansify

install: ansify
	@echo :: installing 'ansify' in directory $(PREFIX)/bin/ansify
	sudo install -m 755 ansify $(PREFIX)/bin/ansify

.PHONY: clean
.PHONY: install

CC=cc
PREFIX=/usr/local
CFLAGS=-O2 -Wall -Wextra -pipe

.PHONY: echfs-utils clean install

echfs-utils:
	@$(CC) $(CFLAGS) echfs-utils.c -o echfs-utils

clean:
	@rm -f echfs-utils

install:
	@mkdir -p $(PREFIX)/bin
	@cp echfs-utils $(PREFIX)/bin

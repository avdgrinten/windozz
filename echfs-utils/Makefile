CC=clang
PREFIX=/usr/local
CFLAGS=-O2 -w -pipe

echfs-utils:
	@echo "\e[0m\e[1m\e[32m[ CC ]\e[0m echfs-utils"
	@$(CC) $(CFLAGS) echfs-utils.c -o echfs-utils

clean:
	@rm -f echfs-utils

install:
	@mkdir -p $(PREFIX)/bin
	@cp echfs-utils $(PREFIX)/bin

CC=gcc
CFLAGS=-std=c99 -Wall -Wextra -Werror -pedantic

ifj21: src/*.c src/*.h
	$(CC) $(CFLAGS) src/main.c -o ifj21

.PHONY: doxygen

doxygen:
	doxygen
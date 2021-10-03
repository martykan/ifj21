CC=gcc
CFLAGS=-std=c99 -Wall -Wextra -Werror -pedantic

ifj21: src/*.c src/*.h
	$(CC) $(CFLAGS) src/main.c -o ifj21

test: src/*.c src/*.h tests/unit/*.c
	$(CC) $(CFLAGS) tests/unit/main.c -o tests/unit/run
	tests/unit/run

.PHONY: doxygen

doxygen:
	doxygen
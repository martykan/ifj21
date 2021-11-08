CC=gcc
CFLAGS=-std=c99 -Wall -Wextra -Werror -pedantic
TEST_CFLAGS=$(CFLAGS) -ftest-coverage -fprofile-arcs

TEST_SOURCES=tests/unit/*.c

.PHONY: doxygen test test_cov test_cov_run test_cov_gen clean_tests

ifj21: src/*.c src/*.h
	$(CC) $(CFLAGS) src/main.c -o ifj21

test: $(TEST_SOURCES)
	$(CC) $(CFLAGS) $^ -o tests/unit/run
	tests/unit/run

test_cov_run: $(TEST_SOURCES)
	rm -f tests/unit/run
	$(CC) $(TEST_CFLAGS) $^ -o tests/unit/run
	tests/unit/run

test_cov_gen:
	lcov -c --directory . --output-file main_coverage.info
	lcov --remove main_coverage.info "*tests/*" -o main_coverage.info
	genhtml main_coverage.info --output-directory ./tests/unit/coverage
	make clean_tests

test_cov: test_cov_run test_cov_gen

clean_tests:
	( cd tests/unit && rm -rf *.gc* *.info )
	rm -rf *.gc* *.info

doxygen:
	doxygen

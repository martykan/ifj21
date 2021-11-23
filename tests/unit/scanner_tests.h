#ifndef __SCANNER_TESTS_H__
#define __SCANNER_TESTS_H__

#include <stdbool.h>
#include <stdio.h>

#define REWRITE_BUFF_FILE(str) \
  ASSERT_EQm("file write failed", rewrite_buffer_file((str)), true);

#define SET_INPUT(str)                                                     \
  do {                                                                     \
    ASSERT_EQm("file write failed", rewrite_buffer_file((str)), true);     \
    freopen("tests/unit/scanner_input_files/buffer_file.txt", "r", stdin); \
  } while (0)
#endif

bool rewrite_buffer_file(char *str);
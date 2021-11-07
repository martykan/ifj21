#include "../../lib/greatest.h"
#include "../../src/dynstr.h"

#include <string.h>
/* TODO(filip): this may be needed only for clangd? If so, can be fixed */



TEST dynstr_init_test() {
  dynstr_t dstr;
  dynstr_t *res = NULL;

  res = dynstr_init(&dstr);

  ASSERT_EQm("dynstr_init returns the pointer to the original.", res, &dstr);

  ASSERT_EQm("sets len.", dstr.len, 0);
  ASSERT_EQm("sets alloced_bytes.", dstr.alloced_bytes, 32);
  ASSERT_NEQm("allocates str buffer.", dstr.str, NULL);
  ASSERT_EQm("creates an empty string.", strlen(dstr.str), 0);


  free(dstr.str);
  PASS();
}

TEST dynstr_init_error_test() {
  void *tmp = dynstr_init(NULL);

  ASSERT_EQm("returns NULL when not given a valid pointer", tmp, NULL);
  PASS();
}

TEST dynstr_free_buffer_test() {
  dynstr_t dstr;
  dynstr_t *res = dynstr_init(&dstr);
  ASSERT_NEQ(res, NULL);

  res = dynstr_free_buffer(&dstr);
  ASSERT_NEQ(res, NULL);
  ASSERT_NEQ(&dstr, NULL);

  ASSERT_EQm("frees string buffer", dstr.str, NULL);
  ASSERT_EQ(dstr.len, 0);
  ASSERT_EQ(dstr.alloced_bytes, 0);


  PASS();
}

TEST dynstr_append_test() {
  dynstr_t dstr;
  dynstr_t *res = dynstr_init(&dstr);
  ASSERT_NEQ(res, NULL);

  ASSERT_EQ(strlen(dstr.str), 0);
  res = dynstr_append(&dstr, 'a');
  ASSERT_NEQ(res, NULL);
  ASSERT_EQm("append to empty gives lenght 1", strlen(dstr.str), 1);
  ASSERT_EQm("len property is set correctly", dstr.len, 1);
  ASSERT_EQm("correct char is appended", dstr.str[0], 'a');

  res = dynstr_append(&dstr, '3');
  ASSERT_NEQ(res, NULL);
  ASSERT_EQm("append to len 1 gives lenght 2", strlen(dstr.str), 2);
  ASSERT_EQm("len property is set correctly", dstr.len, 2);
  ASSERT_EQm("previous char is unchanged", dstr.str[0], 'a');
  ASSERT_EQm("correct char is appended", dstr.str[1], '3');

  ASSERT_EQm("alloced_bytes is set to default", dstr.alloced_bytes, 32);
  int num_added = 0;
  for (int i = 0; i < 30; i++) {
    res = dynstr_append(&dstr, 'A'+i);
    ASSERT_NEQ(res, NULL);
    num_added++;
  }
  // if looped 30 times, last char is ^
  ASSERT_EQm("looped 30 times", num_added, 30);

  ASSERT_EQm("alloced_bytes is double the default", dstr.alloced_bytes, 64);
  ASSERT_NEQm("str buffer was realloced correctly", dstr.str, NULL);

  ASSERT_EQm("2+30 appends gives len 32", strlen(dstr.str), 32);
  ASSERT_EQm("len property is set correctly", dstr.len, 32);
  ASSERT_EQm("first char is unchanged", dstr.str[0], 'a');
  ASSERT_EQm("correct char is appended last", dstr.str[31], '^');

  /* printf("res dynstr: %s\n", dstr.str); */

  res = dynstr_free_buffer(&dstr);
  ASSERT_NEQ(res, NULL);
  PASS();
}

TEST dynstr_clear_test() {
  dynstr_t dstr;
  dynstr_t *res = dynstr_init(&dstr);
  ASSERT_NEQ(res, NULL);

  int num_added = 0;
  for (int i = 0; i < 30; i++) {
    res = dynstr_append(&dstr, 'A'+i);
    ASSERT_NEQ(res, NULL);
    num_added++;
  }
  ASSERT_EQ(num_added, 30);

  ASSERT_NEQm("dstr is not empty", strlen(dstr.str), 0);

  res = dynstr_clear(&dstr);
  ASSERT_NEQ(res, NULL);

  ASSERT_EQm("dstr is empty", strlen(dstr.str), 0);
  ASSERT_EQm("dstr len is zero", dstr.len, 0);
  ASSERT_EQm("dstr alloced_bytes is default", dstr.alloced_bytes, 32);

  res = dynstr_free_buffer(&dstr);
  ASSERT_NEQ(res, NULL);
  PASS();
}

TEST dynstr_clear_append_test() {
  dynstr_t dstr;
  dynstr_t *res = dynstr_init(&dstr);
  ASSERT_NEQ(res, NULL);

  int num_added = 0;
  for (int i = 0; i < 30; i++) {
    res = dynstr_append(&dstr, 'A'+i);
    ASSERT_NEQ(res, NULL);
    num_added++;
  }
  ASSERT_EQ(num_added, 30);

  ASSERT_NEQm("dstr is not empty", strlen(dstr.str), 0);

  res = dynstr_clear(&dstr);
  ASSERT_NEQ(res, NULL);

  ASSERT_EQm("dstr is empty", strlen(dstr.str), 0);
  ASSERT_EQm("dstr alloced_bytes is default", dstr.alloced_bytes, 32);

  res = dynstr_append(&dstr, '.');
  ASSERT_NEQ(res, NULL);
  res = dynstr_append(&dstr, '/');
  ASSERT_NEQ(res, NULL);
  res = dynstr_append(&dstr, '+');
  ASSERT_NEQ(res, NULL);

  ASSERT_EQ(strlen(dstr.str), 3);
  ASSERT_EQ(dstr.len, 3);
  ASSERT_EQm("dstr alloced_bytes is default", dstr.alloced_bytes, 32);

  res = dynstr_free_buffer(&dstr);
  ASSERT_NEQ(res, NULL);
  PASS();
}


TEST dynstr_copy_to_static_test() {
  dynstr_t dstr;
  dynstr_t *res = dynstr_init(&dstr);
  ASSERT_NEQ(res, NULL);

  int num_added = 0;
  for (int i = 0; i < 35; i++) {
    res = dynstr_append(&dstr, 'A'+i);
    ASSERT_NEQ(res, NULL);
    num_added++;
  }
  ASSERT_EQ(num_added, 35);
  ASSERT_EQ(dstr.alloced_bytes, 64);

  ASSERT_NEQm("dstr is not empty", strlen(dstr.str), 0);

  char *dest = dynstr_copy_to_static(&dstr);
  ASSERT_NEQ(dest, NULL);

  ASSERT_EQm("original and copy have same length", strlen(dest), strlen(dstr.str));
  ASSERT_EQm("original and copy are the same string", strcmp(dest, dstr.str), 0);

  res = dynstr_append(&dstr, '~');
  ASSERT_NEQ(res, NULL);
  ASSERT_EQ(dstr.str[dstr.len-1], '~');

  ASSERT_NEQm("modifying original doesn't change copy", strcmp(dest, dstr.str), 0);

  res = dynstr_clear(&dstr);
  ASSERT_NEQ(res, NULL);
  ASSERT_EQ(strlen(dstr.str), 0);

  ASSERT_EQm("copy is not cleared", strlen(dest), 35);
  /* printf("dstr static copy: %s\n", dest); */

  free(dest);
  res = dynstr_free_buffer(&dstr);
  ASSERT_NEQ(res, NULL);
  PASS();
}

SUITE(dynstr_basic_tests) {
  RUN_TEST(dynstr_init_test);
  RUN_TEST(dynstr_init_error_test);
  RUN_TEST(dynstr_free_buffer_test);
  RUN_TEST(dynstr_append_test);
  RUN_TEST(dynstr_clear_test);
  RUN_TEST(dynstr_clear_append_test);
  RUN_TEST(dynstr_copy_to_static_test);
}

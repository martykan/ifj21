#include "../../lib/greatest.h"
#include "../../src/codegen.c"
#include "../../src/scope.c"
#include "../../src/errors.c"
#include "../../src/expressions.c"
#include "../../src/parser.c"
#include "../../src/symtable.c"
#include "scanner_tests.h"

void expressions_init(void *arg) {
  (void)arg;
  scanner_init();
  parser_init_symtab();
}

void expressions_destroy(void *arg) {
  (void)arg;
  scanner_destroy();
}

TEST expressions_basic(void) {
  SET_INPUT("5+5");
  error_clear();
  token_t *tok = token_buff(TOKEN_NEW);
  printf("type: %d\n", tok->type);
  char type;
  ASSERT(expression_parse(&type));

  fclose(stdin);
  PASS();
}

TEST expressions_parentheses(void) {
  SET_INPUT("(69+420)*111");
  error_clear();
  token_buff(TOKEN_NEW);
  char type;
  ASSERT(expression_parse(&type));

  fclose(stdin);
  PASS();
}

TEST expressions_parentheses2(void) {
  SET_INPUT("((69+420)+22)*(111)");
  error_clear();
  token_buff(TOKEN_NEW);
  char type;
  ASSERT(expression_parse(&type));

  fclose(stdin);
  PASS();
}

TEST expressions_invalid1(void) {
  SET_INPUT("69+420(*5)");
  error_clear();
  token_buff(TOKEN_NEW);
  char type;
  ASSERT_EQ(false, expression_parse(&type));

  fclose(stdin);
  PASS();
}

SUITE(expressions_tests) {
  GREATEST_SET_SETUP_CB(expressions_init, NULL);
  GREATEST_SET_TEARDOWN_CB(expressions_destroy, NULL);
  RUN_TEST(expressions_basic);
  RUN_TEST(expressions_parentheses);
  RUN_TEST(expressions_parentheses2);
  RUN_TEST(expressions_invalid1);
}

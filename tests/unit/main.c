#include "../../lib/greatest.h"
#include "../../src/expressions.c"

TEST x_should_equal_1(void)
{
  int x = 1;

  ASSERT_EQ(1, x);

  ASSERT_EQm("X doesn't equal 1!", 1, x);

  PASS();
}

TEST expressions_basic(void)
{
  expression_symbol_t input[] = {SYM_I, SYM_PLUS, SYM_I, SYM_TIMES, SYM_I, SYM_S};
  ASSERT_EQ(expression_process(input), 1);
  PASS();
}

TEST expressions_parentheses(void)
{
  expression_symbol_t input[] = {SYM_LBRACKET, SYM_I, SYM_PLUS, SYM_I, SYM_RBRACKET, SYM_TIMES, SYM_I, SYM_S};
  ASSERT_EQ(expression_process(input), 1);
  PASS();
}

TEST expressions_parentheses2(void)
{
  expression_symbol_t input[] = {SYM_LBRACKET, SYM_LBRACKET, SYM_I, SYM_PLUS, SYM_I, SYM_RBRACKET, SYM_PLUS, SYM_I, SYM_RBRACKET, SYM_TIMES, SYM_LBRACKET, SYM_I, SYM_RBRACKET, SYM_S};
  ASSERT_EQ(expression_process(input), 1);
  PASS();
}

TEST expressions_invalid1(void)
{
  expression_symbol_t input[] = {SYM_I, SYM_PLUS, SYM_I, SYM_LBRACKET, SYM_TIMES, SYM_I, SYM_S};
  ASSERT_EQ(expression_process(input), 0);
  PASS();
}

SUITE(the_suite)
{
  RUN_TEST(x_should_equal_1);
  RUN_TEST(expressions_basic);
  RUN_TEST(expressions_parentheses);
  RUN_TEST(expressions_parentheses2);
  RUN_TEST(expressions_invalid1);
}

GREATEST_MAIN_DEFS();

int main(int argc, char **argv)
{
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(the_suite);
  GREATEST_MAIN_END();
}

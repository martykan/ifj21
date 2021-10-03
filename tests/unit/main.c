#include "../../lib/greatest.h"

TEST x_should_equal_1(void) {
  int x = 1;

  ASSERT_EQ(1, x);

  ASSERT_EQm("X doesn't equal 1!", 1, x);

  PASS();
}

SUITE(the_suite) { RUN_TEST(x_should_equal_1); }

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(the_suite);
  GREATEST_MAIN_END();
}

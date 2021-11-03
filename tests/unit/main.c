#include "../../lib/greatest.h"


SUITE_EXTERN(dynstr_basic_tests);
SUITE_EXTERN(scanner_basic_tests);
SUITE_EXTERN(scanner_input_file_tests);

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();

  RUN_SUITE(dynstr_basic_tests);
  RUN_SUITE(scanner_basic_tests);
  RUN_SUITE(scanner_input_file_tests);


  GREATEST_MAIN_END();
}

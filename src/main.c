#include <stdio.h>

#include "errors.h"
#include "parser.h"
#include "scanner.h"
#include "syntax.h"
#include "scope.h"

int main(int argc, char **argv) {
  scanner_init();
  parser_init_symtab();

  scope_init();

  printf(".IFJcode21\n");
  parser_start();
  printf("\n");

  scanner_destroy();

  int errcode = error_get();
  if (errcode > 0) {
    error_print_msg(NULL);
  }
  return errcode;
}

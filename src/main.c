#include <stdio.h>

#include "errors.h"
#include "parser.h"
#include "scanner.h"
#include "syntax.h"

int main(int argc, char **argv) {
  scanner_init();
  parser_init_symtab();

  printf(".IFJcode21\n");
  parser_start();
  printf("\n");

  scanner_destroy();

  return error_get();
}

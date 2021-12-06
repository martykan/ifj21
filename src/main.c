#include <stdio.h>

#include "codegen.h"
#include "errors.h"
#include "parser.h"
#include "scanner.h"
#include "scope.h"
#include "syntax.h"

int main(int argc, char **argv) {
  scanner_init();
  parser_init_symtab();
  codegen_init();
  scope_init();

  parser_start();

  scope_destroy();
  codegen_free();
  scanner_destroy();
  parser_destroy_symtab();
  token_buff(TOKEN_DELETE);

  int errcode = error_get();
  if (errcode > 0) {
    error_print_msg(NULL);
  }
  return errcode;
}

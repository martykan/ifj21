#include <stdio.h>
#include "errors.h"


err_t global_error = {EXITSTATUS_OK, true};



void error_set(exit_status_t status) {
  global_error.exit_status = status;
  global_error.handled = false;
}

void error_clear() {
  global_error.exit_status = EXITSTATUS_OK;
  global_error.handled = true;
}

exit_status_t error_get() {
  return global_error.exit_status;
}

void error_set_handled() {
  global_error.handled = true;
}


void error_print_msg(char *msg) {
  if (global_error.handled || global_error.exit_status == EXITSTATUS_OK) return;

  if (msg != NULL) {
    fprintf(stderr, "%s\n", msg);
    return;
  }


  fprintf(stderr, "ERR: ");
  switch (global_error.exit_status) {
  case EXITSTATUS_ERROR_LEXICAL:
    fprintf(stderr, "Wrong structure of lexeme.\n");
    break;
  case EXITSTATUS_ERROR_SYNTAX:
    fprintf(stderr, "Invalid syntax.\n");
    break;
  case EXITSTATUS_ERROR_SEMANTIC_IDENTIFIER:
    fprintf(stderr, "Undefined function or variable.\n");
    break;
  case EXITSTATUS_ERROR_SEMANTIC_ASSIGNMENT:
    fprintf(stderr, "Incompatable types in assignment.\n");
    break;
  case EXITSTATUS_ERROR_SEMANTIC_FUN_PARAMETERS:
    fprintf(stderr, "Incorrect count or type of function parameters.\n");
    break;
  case EXITSTATUS_ERROR_SEMANTIC_TYPE_EXPR:
    fprintf(stderr, "Incompatable types in expression.\n");
    break;
  case EXITSTATUS_ERROR_SEMANTIC_OTHER:
    fprintf(stderr, "Semantic error.\n");
    break;
  case EXITSTATUS_ERROR_UNEXPECTED_NIL:
    fprintf(stderr, "Unexpected nil value.\n");
    break;
  case EXITSTATUS_ERROR_DIVIDE_ZERO:
    fprintf(stderr, "Division by zero.\n");
    break;
  case EXITSTATUS_INTERNAL_ERROR:
    fprintf(stderr, "Internal error occurred.\n");
    break;
  default:
    fprintf(stderr, "Unhandled error code: %d.\n", global_error.exit_status);
  }
}


/**
 * @file
 * @brief Error codes definition
 * @author Patrik Korytar
 * @author Tomas Martykan
 * @author Filip Stolfa
 */

#ifndef __ERRORS_H
#define __ERRORS_H

#include <stdbool.h>
#include "scanner.h"

typedef enum {
  EXITSTATUS_OK = 0,   ///< Normal exit

  EXITSTATUS_ERROR_LEXICAL = 1,   ///< Wrong structure of lexeme

  EXITSTATUS_ERROR_SYNTAX = 2,   ///< Invalid syntax

  EXITSTATUS_ERROR_SEMANTIC_IDENTIFIER = 3,   ///< Semantic error - undefined function/variable, redefinition of variable

  EXITSTATUS_ERROR_SEMANTIC_ASSIGNMENT = 4,   ///< Semantic error - type incompatibility in assignment

  EXITSTATUS_ERROR_SEMANTIC_FUN_PARAMETERS = 5,   ///< Semantic error - incorrect count/type of parameters or return values of function call

  EXITSTATUS_ERROR_SEMANTIC_TYPE_EXPR = 6,   ///< Semantic error - type incompatibility in arithmetic, string or relation expressions

  EXITSTATUS_ERROR_SEMANTIC_OTHER = 7,   ///< Semantic error - other

  EXITSTATUS_ERROR_UNEXPECTED_NIL = 8,   ///< Runtime error - working with unexpected nil value

  EXITSTATUS_ERROR_DIVIDE_ZERO = 9,   ///< Runtime error - dividing by zero constant

  EXITSTATUS_INTERNAL_ERROR = 99,   ///< Internal compiler error (ex. memory allocation error)
} exit_status_t;

typedef struct {
  exit_status_t exit_status;
  bool handled;
} err_t;


void error_set(exit_status_t status);

void error_clear();

/** Get the current error value.
 * Used to check if an error is set. If there is no error, the value is 0.
 * Otherwise it is a non-zero value. Can be used in an if statement to check for errors,
 * or in a return to exit the program with the proper value.
 */
exit_status_t error_get();

void error_set_handled();

void error_print_msg(char *msg);

/* TODO(filip): construct specific err msg from token info. */
void error_construct_msg_scanner(char *buff, token_t tok, int col, int row);


#endif

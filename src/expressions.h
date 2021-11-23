/**
 * @file
 * @brief Expression parser API
 * @author Tomas Martykan
 * @author Patrik Korytar
 * @author Filip Stolfa
 */

#ifndef __EXPRESSIONS_H__
#define __EXPRESSIONS_H__

#include <stdbool.h>

#include "codegen.h"
#include "parser.h"
#include "scanner.h"

typedef enum {
  PREC_UNDEF,
  PREC_EQ,
  PREC_GT,
  PREC_GTE,
  PREC_LT,
  PREC_LTE,
} expression_precedence_t;

typedef enum {
  SYM_PLUS,
  SYM_MINUS,
  SYM_TIMES,
  SYM_DIVIDE,
  SYM_DIVIDE2,
  SYM_DOTDOT,
  SYM_GT,
  SYM_GTE,
  SYM_EQ,
  SYM_NEQ,
  SYM_LTE,
  SYM_LT,
  SYM_LBRACKET,
  SYM_RBRACKET,
  SYM_I,
  SYM_S,
  SYM_E,
  SYM_PREC_GT,
  SYM_PREC_LT,
} expression_symbol_t;

typedef struct symbol_stack_t symbol_stack_t;
struct symbol_stack_t {
  expression_symbol_t symbol;
  token_t *token;
  symbol_stack_t *next;
};

bool expression_process(symbol_stack_t *stack);
bool expression_parse();
expression_symbol_t expression_get_input();
void expression_next_input();

#endif
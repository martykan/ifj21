/**
 * @file
 * @brief Expression parser implementation
 * @author Tomas Martykan
 * @author Patrik Korytar
 * @author Filip Stolfa
 */
#include "expressions.h"

#include <stdio.h>
#include <stdlib.h>

#include "errors.h"

extern symtab_t *symtab;

#define TYPE_STRING 's'
#define TYPE_INTEGER 'i'
#define TYPE_NUMBER 'n'
#define TYPE_BOOL 'b'
#define TYPE_NIL 'x'
#define TYPE_NONE '-'

char *expression_symbol_t_names[] = {
    "SYM_PLUS",   "SYM_MINUS",  "SYM_TIMES", "SYM_DIVIDE",   "SYM_DIVIDE2",
    "SYM_DOTDOT", "SYM_STRLEN", "SYM_GT",    "SYM_GTE",      "SYM_EQ",
    "SYM_NEQ",    "SYM_LTE",    "SYM_LT",    "SYM_LBRACKET", "SYM_RBRACKET",
    "SYM_I",      "SYM_S",      "SYM_E",     "SYM_PREC_GT",  "SYM_PREC_LT",
};

const expression_precedence_t precedence_table[9][9] = {
    {
        PREC_GT,
        PREC_GT,
        PREC_GT,
        PREC_GT,
        PREC_GT,
        PREC_LT,
        PREC_GT,
        PREC_LT,
        PREC_GT,
    },
    {
        PREC_LT,
        PREC_GT,
        PREC_GT,
        PREC_GT,
        PREC_GT,
        PREC_LT,
        PREC_GT,
        PREC_LT,
        PREC_GT,
    },
    {
        PREC_LT,
        PREC_LT,
        PREC_GT,
        PREC_GT,
        PREC_GT,
        PREC_LT,
        PREC_GT,
        PREC_LT,
        PREC_GT,
    },
    {
        PREC_LT,
        PREC_LT,
        PREC_LT,
        PREC_GT,
        PREC_GT,
        PREC_LT,
        PREC_GT,
        PREC_LT,
        PREC_GT,
    },
    {
        PREC_LT,
        PREC_LT,
        PREC_LT,
        PREC_LT,
        PREC_UNDEF,
        PREC_LT,
        PREC_GT,
        PREC_LT,
        PREC_GT,
    },
    {
        PREC_LT,
        PREC_LT,
        PREC_LT,
        PREC_LT,
        PREC_LT,
        PREC_LT,
        PREC_EQ,
        PREC_LT,
        PREC_UNDEF,
    },
    {
        PREC_GT,
        PREC_GT,
        PREC_GT,
        PREC_GT,
        PREC_GT,
        PREC_UNDEF,
        PREC_GT,
        PREC_UNDEF,
        PREC_GT,
    },
    {
        PREC_GT,
        PREC_GT,
        PREC_GT,
        PREC_GT,
        PREC_GT,
        PREC_UNDEF,
        PREC_GT,
        PREC_UNDEF,
        PREC_GT,
    },
    {
        PREC_LT,
        PREC_LT,
        PREC_LT,
        PREC_LT,
        PREC_LT,
        PREC_LT,
        PREC_UNDEF,
        PREC_LT,
        PREC_UNDEF,
    },
};

/**
 * Get the index in the precedence table
 * @param symbol Symbol to check
 * @return Index of the symbol in the precedence table
 */
int precedence_table_index(expression_symbol_t symbol) {
  switch (symbol) {
    case SYM_STRLEN:
      return 0;
    case SYM_TIMES:
    case SYM_DIVIDE:
    case SYM_DIVIDE2:
      return 1;
    case SYM_PLUS:
    case SYM_MINUS:
      return 2;
    case SYM_DOTDOT:
      return 3;
    case SYM_GT:
    case SYM_GTE:
    case SYM_EQ:
    case SYM_NEQ:
    case SYM_LTE:
    case SYM_LT:
      return 4;
    case SYM_LBRACKET:
      return 5;
    case SYM_RBRACKET:
      return 6;
    case SYM_I:
      return 7;
    case SYM_S:
      return 8;
    default:
      return -1;
  }
}

/**
 * Push operation on symbol stack
 */
void symbol_stack_push(symbol_stack_t **stack, expression_symbol_t sym,
                       char type) {
  symbol_stack_t *old_stack = *stack;
  symbol_stack_t *new_stack = (symbol_stack_t *)malloc(sizeof(symbol_stack_t));
  new_stack->symbol = sym;
  new_stack->type = type;
  new_stack->token = token_buff(TOKEN_THIS);
  new_stack->next = old_stack;
  *stack = new_stack;
}

/**
 * Pop operation on symbol stack
 */
void symbol_stack_pop(symbol_stack_t **stack) {
  symbol_stack_t *new_stack = (*stack)->next;
  free(*stack);
  *stack = new_stack;
}

/**
 * Pop operation on symbol stack, multiple times
 */
void symbol_stack_pops(symbol_stack_t **stack, int count) {
  for (int i = 0; i < count; i++) {
    symbol_stack_pop(stack);
  }
}

/**
 * Get top symbol from stack (excluding E)
 */
expression_symbol_t get_top_symbol(symbol_stack_t *stack) {
  expression_symbol_t a;
  symbol_stack_t *stack_curr = stack;
  do {
    a = stack_curr->symbol;
    stack_curr = stack_curr->next;
  } while (a == SYM_E && stack_curr != NULL);
  return a;
}

void expression_typecheck_set_error(char type1, char type2) {
  if (type1 == TYPE_NIL || type2 == TYPE_NIL) {
    error_set(EXITSTATUS_ERROR_UNEXPECTED_NIL);
  } else {
    error_set(EXITSTATUS_ERROR_SEMANTIC_TYPE_EXPR);
  }
}

// For plus, minus, multiply
bool expression_typecheck_basic_arithmetics(char *out, char type1, char type2) {
  if ((type1 != TYPE_NUMBER && type1 != TYPE_INTEGER) ||
      (type2 != TYPE_NUMBER && type2 != TYPE_INTEGER)) {
    expression_typecheck_set_error(type1, type2);
    return false;
  }
  if (type1 == TYPE_INTEGER && type2 == TYPE_INTEGER) {
    *out = TYPE_INTEGER;
  } else {
    *out = TYPE_NUMBER;
    if (type1 == TYPE_INTEGER && type2 == TYPE_NUMBER) {
      codegen_cast_int_to_float1();
    } else if (type1 == TYPE_NUMBER && type2 == TYPE_INTEGER) {
      codegen_cast_int_to_float2();
    }
  }
  return true;
}

bool expression_typecheck_basic_logic_nullable(char *out, char type1,
                                               char type2) {
  if ((type1 == TYPE_NUMBER || type1 == TYPE_INTEGER || type1 == TYPE_NIL) &&
      (type2 == TYPE_NUMBER || type2 == TYPE_INTEGER || type1 == TYPE_NIL)) {
    *out = TYPE_BOOL;
    if (type1 == TYPE_INTEGER && type2 == TYPE_NUMBER) {
      codegen_cast_int_to_float1();
    } else if (type1 == TYPE_NUMBER && type2 == TYPE_INTEGER) {
      codegen_cast_int_to_float2();
    }
    return true;
  } else if ((type1 == TYPE_STRING || type1 == TYPE_NIL) &&
             (type2 == TYPE_STRING || type2 == TYPE_NIL)) {
    *out = TYPE_BOOL;
    return true;
  } else {
    expression_typecheck_set_error(type1, type2);
    return false;
  }
}

bool expression_typecheck_basic_logic(char *out, char type1, char type2) {
  if ((type1 == TYPE_NUMBER || type1 == TYPE_INTEGER) &&
      (type2 == TYPE_NUMBER || type2 == TYPE_INTEGER)) {
    *out = TYPE_BOOL;
    if (type1 == TYPE_INTEGER && type2 == TYPE_NUMBER) {
      codegen_cast_int_to_float1();
    } else if (type1 == TYPE_NUMBER && type2 == TYPE_INTEGER) {
      codegen_cast_int_to_float2();
    }
    return true;
  } else {
    expression_typecheck_set_error(type1, type2);
    return false;
  }
}

bool expression_test_rules(symbol_stack_t **stack, symbol_stack_t *s2,
                           symbol_stack_t *s3, symbol_stack_t *s4) {
  symbol_stack_t *s1 = *stack;
  if (s1->symbol == SYM_I) {
    if (s2->symbol == SYM_PREC_LT) {
      // E -> i
      char type = s1->type;
      symbol_stack_pops(stack, 2);
      symbol_stack_push(stack, SYM_E, type);
      return true;
    }
  } else if (s1->symbol == SYM_RBRACKET && s2->symbol == SYM_E &&
             s3->symbol == SYM_LBRACKET && s4->symbol == SYM_PREC_LT) {
    // E -> (E)
    char type = s2->type;
    symbol_stack_pops(stack, 4);
    symbol_stack_push(stack, SYM_E, type);
    return true;
  } else if (s1->symbol == SYM_E && s2->symbol == SYM_STRLEN &&
             s3->symbol == SYM_PREC_LT) {
    // E -> #E
    if (s1->type != TYPE_STRING) {
      error_set(EXITSTATUS_ERROR_SEMANTIC_TYPE_EXPR);
      return false;
    }
    symbol_stack_pops(stack, 3);
    symbol_stack_push(stack, SYM_E, TYPE_INTEGER);
    codegen_expression_strlen();
    return true;
  } else if (s1->symbol == SYM_E && s3->symbol == SYM_E &&
             s4->symbol == SYM_PREC_LT) {
    if (s2->symbol == SYM_PLUS) {
      // E -> E+E
      char type;
      if (!expression_typecheck_basic_arithmetics(&type, s1->type, s3->type))
        return false;
      symbol_stack_pops(stack, 4);
      symbol_stack_push(stack, SYM_E, type);
      codegen_expression_plus();
      return true;
    } else if (s2->symbol == SYM_MINUS) {
      // E -> E-E
      char type;
      if (!expression_typecheck_basic_arithmetics(&type, s1->type, s3->type))
        return false;
      symbol_stack_pops(stack, 4);
      symbol_stack_push(stack, SYM_E, type);
      codegen_expression_minus();
      return true;
    } else if (s2->symbol == SYM_TIMES) {
      // E -> E*E
      char type;
      if (!expression_typecheck_basic_arithmetics(&type, s1->type, s3->type))
        return false;
      symbol_stack_pops(stack, 4);
      symbol_stack_push(stack, SYM_E, type);
      codegen_expression_mul();
      return true;
    } else if (s2->symbol == SYM_DIVIDE) {
      // E -> E/E
      if (s1->type == TYPE_INTEGER) {
        s1->type = TYPE_NUMBER;
        codegen_cast_int_to_float1();
      }
      if (s3->type == TYPE_INTEGER) {
        s3->type = TYPE_NUMBER;
        codegen_cast_int_to_float2();
      }
      if (s1->type != TYPE_NUMBER && s3->type != TYPE_NUMBER) {
        expression_typecheck_set_error(s1->type, s3->type);
        return false;
      }
      if (s3->token->attr.num_val == 0.0) {
        error_set(EXITSTATUS_ERROR_DIVIDE_ZERO);
        return false;
      }
      symbol_stack_pops(stack, 4);
      symbol_stack_push(stack, SYM_E, TYPE_NUMBER);
      codegen_expression_div();
      return true;
    } else if (s2->symbol == SYM_DIVIDE2) {
      // E -> E//E
      if (s1->type == TYPE_NUMBER) {
        s1->type = TYPE_INTEGER;
        codegen_cast_float_to_int1();
      }
      if (s3->type == TYPE_NUMBER) {
        s3->type = TYPE_INTEGER;
        codegen_cast_float_to_int2();
      }
      if (s1->type != TYPE_INTEGER && s3->type != TYPE_INTEGER) {
        expression_typecheck_set_error(s1->type, s3->type);
        return false;
      }
      if (s3->token->attr.int_val == 0) {
        error_set(EXITSTATUS_ERROR_DIVIDE_ZERO);
        return false;
      }
      symbol_stack_pops(stack, 4);
      symbol_stack_push(stack, SYM_E, TYPE_INTEGER);
      codegen_expression_divint();
      return true;
    } else if (s2->symbol == SYM_DOTDOT) {
      // E -> E..E
      if (s1->type != TYPE_STRING && s3->type != TYPE_STRING) {
        expression_typecheck_set_error(s1->type, s3->type);
        return false;
      }
      symbol_stack_pops(stack, 4);
      symbol_stack_push(stack, SYM_E, TYPE_STRING);
      codegen_expression_concat();
      return true;
    } else if (s2->symbol == SYM_EQ) {
      // E -> E==E
      char type;
      if (!expression_typecheck_basic_logic_nullable(&type, s1->type, s3->type))
        return false;
      symbol_stack_pops(stack, 4);
      symbol_stack_push(stack, SYM_E, type);
      codegen_expression_eq();
      return true;
    } else if (s2->symbol == SYM_NEQ) {
      // E -> E~=E
      char type;
      if (!expression_typecheck_basic_logic_nullable(&type, s1->type, s3->type))
        return false;
      symbol_stack_pops(stack, 4);
      symbol_stack_push(stack, SYM_E, type);
      codegen_expression_neq();
      return true;
    } else if (s2->symbol == SYM_LT) {
      // E -> E<E
      char type;
      if (!expression_typecheck_basic_logic(&type, s1->type, s3->type))
        return false;
      symbol_stack_pops(stack, 4);
      symbol_stack_push(stack, SYM_E, type);
      codegen_expression_lt();
      return true;
    } else if (s2->symbol == SYM_GT) {
      // E -> E>E
      char type;
      if (!expression_typecheck_basic_logic(&type, s1->type, s3->type))
        return false;
      symbol_stack_pops(stack, 4);
      symbol_stack_push(stack, SYM_E, type);
      codegen_expression_gt();
      return true;
    } else if (s2->symbol == SYM_LTE) {
      // E -> E<=E
      char type;
      if (!expression_typecheck_basic_logic(&type, s1->type, s3->type))
        return false;
      symbol_stack_pops(stack, 4);
      symbol_stack_push(stack, SYM_E, type);
      codegen_expression_lte();
      return true;
    } else if (s2->symbol == SYM_GTE) {
      // E -> E>=E
      char type;
      if (!expression_typecheck_basic_logic(&type, s1->type, s3->type))
        return false;
      symbol_stack_pops(stack, 4);
      symbol_stack_push(stack, SYM_E, type);
      codegen_expression_gte();
      return true;
    }
  }
  return false;
}

/**
 * Print stack - for debugging
 */
void print_stack(symbol_stack_t *stack) {
  expression_symbol_t a;
  symbol_stack_t *stack_curr = stack;
  printf("# [");
  while (stack_curr != NULL) {
    a = stack_curr->symbol;
    printf("%s ", expression_symbol_t_names[a]);
    stack_curr = stack_curr->next;
  }
  printf("]\n");
}

/**
 * Process expression
 */
bool expression_process(symbol_stack_t *stack, char *exp_type) {
  expression_symbol_t a, b;
  do {
    a = get_top_symbol(stack);
    expression_symbol_t new_b = expression_get_input();
    if (error_get()) {
      return false;
    }
    if (b == SYM_S || (b == SYM_I && new_b == SYM_I)) {
      b = SYM_S;
    } else {
      b = new_b;
    }
    // printf("# %s %s\n", expression_symbol_t_names[a],
    //       expression_symbol_t_names[b]);
    // print_stack(stack);
    expression_precedence_t precedence =
        precedence_table[precedence_table_index(a)][precedence_table_index(b)];
    switch (precedence) {
      case PREC_EQ:
        symbol_stack_push(&stack, b, expression_get_type());
        expression_next_input();
        if (error_get()) {
          return false;
        }
        break;
      case PREC_LT:
        if (b == SYM_I) {
          token_t *token = token_buff(TOKEN_THIS);
          codegen_expression_push_value(token);
        }
        if (stack->symbol == SYM_E) {
          char type = stack->type;
          symbol_stack_pop(&stack);
          symbol_stack_push(&stack, SYM_PREC_LT, TYPE_NONE);
          symbol_stack_push(&stack, SYM_E, type);
        } else {
          symbol_stack_push(&stack, SYM_PREC_LT, TYPE_NONE);
        }
        symbol_stack_push(&stack, b, expression_get_type());
        expression_next_input();
        if (error_get()) {
          return false;
        }
        break;
      case PREC_GT: {
        symbol_stack_t *s2 = NULL;
        symbol_stack_t *s3 = NULL;
        symbol_stack_t *s4 = NULL;
        if (stack != NULL) s2 = stack->next;
        if (s2 != NULL) s3 = s2->next;
        if (s3 != NULL) s4 = s3->next;

        if (!expression_test_rules(&stack, s2, s3, s4)) {
          return false;
        }
        break;
      }
      default:
        // Error
        return false;
    }
  } while (b != SYM_S ||
           !(stack->symbol == SYM_E && stack->next->symbol == SYM_S));
  *exp_type = stack->type;
  return true;
}

char expression_get_type() {
  token_t *token = token_buff(TOKEN_THIS);
  if (error_get() || token == NULL) {
    return TYPE_NONE;
  }

  switch (token->type) {
    case TT_INTEGER:
      return TYPE_INTEGER;
    case TT_NUMBER:
      return TYPE_NUMBER;
    case TT_STRING:
      return TYPE_STRING;
    case TT_K_NIL:
      return TYPE_NIL;
    case TT_ID: {
      symtab_var_data_t *record = symtab_find_var(symtab, token->attr.str);
      if (record == NULL) return TYPE_NONE;
      return record->data_type;
    }
    default:
      return TYPE_NONE;
  }
}

expression_symbol_t expression_get_input() {
  token_t *token = token_buff(TOKEN_THIS);
  if (error_get() || token == NULL) {
    return SYM_S;
  }

  switch (token->type) {
    case TT_INTEGER:
    case TT_NUMBER:
    case TT_STRING:
    case TT_K_NIL:
    case TT_ID:
      return SYM_I;
    case TT_LPAR:
      return SYM_LBRACKET;
    case TT_RPAR:
      return SYM_RBRACKET;
    case TT_MOP_PLUS:
      return SYM_PLUS;
    case TT_MOP_MINUS:
      return SYM_MINUS;
    case TT_MOP_MUL:
      return SYM_TIMES;
    case TT_MOP_DIV:
      return SYM_DIVIDE;
    case TT_MOP_INT_DIV:
      return SYM_DIVIDE2;
    case TT_SOP_LENGTH:
      return SYM_STRLEN;
    case TT_SOP_CONCAT:
      return SYM_DOTDOT;
    case TT_COP_EQ:
      return SYM_EQ;
    case TT_COP_NEQ:
      return SYM_NEQ;
    case TT_COP_GT:
      return SYM_GT;
    case TT_COP_GE:
      return SYM_GTE;
    case TT_COP_LT:
      return SYM_LT;
    case TT_COP_LE:
      return SYM_LTE;
    default:
      return SYM_S;
  }
}
void expression_next_input() { token_buff(TOKEN_NEW); }

bool expression_parse(char *exp_type) {
  token_t *token = token_buff(TOKEN_THIS);
  if (error_get() || token == NULL) {
    return false;
  }
  switch (token->type) {
    case TT_INTEGER:
    case TT_NUMBER:
    case TT_STRING:
    case TT_LPAR:
    case TT_K_NIL:
    case TT_SOP_LENGTH:
    case TT_ID: {
      symbol_stack_t *stack = NULL;
      symbol_stack_push(&stack, SYM_S, TYPE_NONE);
      return expression_process(stack, exp_type);
    }
    default:
      return false;
  }
}
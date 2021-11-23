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

// TODO: missing code generation
// TODO: missing type checking and conversion

char *expression_symbol_t_names[] = {
    "SYM_PLUS",   "SYM_MINUS", "SYM_TIMES",    "SYM_DIVIDE",   "SYM_DIVIDE2",
    "SYM_DOTDOT", "SYM_GT",    "SYM_GTE",      "SYM_EQ",       "SYM_NEQ",
    "SYM_LTE",    "SYM_LT",    "SYM_LBRACKET", "SYM_RBRACKET", "SYM_I",
    "SYM_S",      "SYM_E",     "SYM_PREC_GT",  "SYM_PREC_LT",
};

const expression_precedence_t precedence_table[8][8] = {
    {
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
        PREC_EQ,
        PREC_LT,
        PREC_UNDEF,
    },
    {
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
    case SYM_TIMES:
    case SYM_DIVIDE:
    case SYM_DIVIDE2:
      return 0;
    case SYM_PLUS:
    case SYM_MINUS:
      return 1;
    case SYM_DOTDOT:
      return 2;
    case SYM_GT:
    case SYM_GTE:
    case SYM_EQ:
    case SYM_NEQ:
    case SYM_LTE:
    case SYM_LT:
      return 3;
    case SYM_LBRACKET:
      return 4;
    case SYM_RBRACKET:
      return 5;
    case SYM_I:
      return 6;
    case SYM_S:
      return 7;
    default:
      return -1;
  }
}

/**
 * Push operation on symbol stack
 */
void symbol_stack_push(symbol_stack_t **stack, expression_symbol_t sym) {
  symbol_stack_t *old_stack = *stack;
  symbol_stack_t *new_stack = (symbol_stack_t *)malloc(sizeof(symbol_stack_t));
  new_stack->symbol = sym;
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

/**
 * Print stack - for debugging
 */
void print_stack(symbol_stack_t *stack) {
  expression_symbol_t a;
  symbol_stack_t *stack_curr = stack;
  printf("[");
  do {
    a = stack_curr->symbol;
    printf("%s ", expression_symbol_t_names[a]);
    stack_curr = stack_curr->next;
  } while (stack_curr != NULL);
  printf("]\n");
}

/**
 * Process expression
 */
bool expression_process(symbol_stack_t *stack) {
  expression_symbol_t a, b;
  do {
    a = get_top_symbol(stack);
    b = expression_get_input();
    // print_stack(stack);
    // printf("Terminal %s, input %s\n", expression_symbol_t_names[a],
    // expression_symbol_t_names[b]);
    expression_precedence_t precedence =
        precedence_table[precedence_table_index(a)][precedence_table_index(b)];
    switch (precedence) {
      case PREC_EQ:
        // printf("PREC_EQ\n");
        symbol_stack_push(&stack, b);
        expression_next_input();
        break;
      case PREC_LT:
        // printf("PREC_LT\n");
        if (b == SYM_I) {
          token_t *token = token_buff(TOKEN_THIS);
          codegen_expression_push_value(token);
        }
        if (stack->symbol == SYM_E) {
          symbol_stack_pop(&stack);
          symbol_stack_push(&stack, SYM_PREC_LT);
          symbol_stack_push(&stack, SYM_E);
        } else {
          symbol_stack_push(&stack, SYM_PREC_LT);
        }
        symbol_stack_push(&stack, b);
        expression_next_input();
        break;
      case PREC_GT:
        // printf("PREC_GT\n");
        // Test rules
        if (stack->symbol == SYM_I) {
          if (stack->next->symbol == SYM_PREC_LT) {
            // E -> i
            symbol_stack_pops(&stack, 2);
            symbol_stack_push(&stack, SYM_E);
            break;
          }
        } else if (stack->symbol == SYM_RBRACKET &&
                   stack->next->symbol == SYM_E &&
                   stack->next->next->symbol == SYM_LBRACKET &&
                   stack->next->next->next->symbol == SYM_PREC_LT) {
          // E -> (E)
          symbol_stack_pops(&stack, 4);
          symbol_stack_push(&stack, SYM_E);
          break;
        } else if (stack->symbol == SYM_E) {
          if (stack->next->symbol == SYM_PLUS &&
              stack->next->next->symbol == SYM_E &&
              stack->next->next->next->symbol == SYM_PREC_LT) {
            // E -> E+E
            codegen_expression_plus();
            symbol_stack_pops(&stack, 4);
            symbol_stack_push(&stack, SYM_E);
            break;
          } else if (stack->next->symbol == SYM_MINUS &&
                     stack->next->next->symbol == SYM_E &&
                     stack->next->next->next->symbol == SYM_PREC_LT) {
            // E -> E-E
            codegen_expression_minus();
            symbol_stack_pops(&stack, 4);
            symbol_stack_push(&stack, SYM_E);
            break;
          } else if (stack->next->symbol == SYM_TIMES &&
                     stack->next->next->symbol == SYM_E &&
                     stack->next->next->next->symbol == SYM_PREC_LT) {
            // E -> E*E
            codegen_expression_mul();
            symbol_stack_pops(&stack, 4);
            symbol_stack_push(&stack, SYM_E);
            break;
          } else if (stack->next->symbol == SYM_DIVIDE &&
                     stack->next->next->symbol == SYM_E &&
                     stack->next->next->next->symbol == SYM_PREC_LT) {
            // E -> E/E
            codegen_expression_div();
            symbol_stack_pops(&stack, 4);
            symbol_stack_push(&stack, SYM_E);
            break;
          } else if (stack->next->symbol == SYM_DIVIDE2 &&
                     stack->next->next->symbol == SYM_E &&
                     stack->next->next->next->symbol == SYM_PREC_LT) {
            // E -> E//E
            codegen_expression_divint();
            symbol_stack_pops(&stack, 4);
            symbol_stack_push(&stack, SYM_E);
            break;
          } else if (stack->next->symbol == SYM_DOTDOT &&
                     stack->next->next->symbol == SYM_E &&
                     stack->next->next->next->symbol == SYM_PREC_LT) {
            // E -> E..E
            codegen_expression_concat();
            symbol_stack_pops(&stack, 4);
            symbol_stack_push(&stack, SYM_E);
            break;
          } else if (stack->next->symbol == SYM_EQ &&
                     stack->next->next->symbol == SYM_E &&
                     stack->next->next->next->symbol == SYM_PREC_LT) {
            // E -> E==E
            codegen_expression_eq();
            symbol_stack_pops(&stack, 4);
            symbol_stack_push(&stack, SYM_E);
            break;
          } else if (stack->next->symbol == SYM_NEQ &&
                     stack->next->next->symbol == SYM_E &&
                     stack->next->next->next->symbol == SYM_PREC_LT) {
            // E -> E~=E
            codegen_expression_neq();
            symbol_stack_pops(&stack, 4);
            symbol_stack_push(&stack, SYM_E);
            break;
          } else if (stack->next->symbol == SYM_LT &&
                     stack->next->next->symbol == SYM_E &&
                     stack->next->next->next->symbol == SYM_PREC_LT) {
            // E -> E<E
            codegen_expression_lt();
            symbol_stack_pops(&stack, 4);
            symbol_stack_push(&stack, SYM_E);
            break;
          } else if (stack->next->symbol == SYM_GT &&
                     stack->next->next->symbol == SYM_E &&
                     stack->next->next->next->symbol == SYM_PREC_LT) {
            // E -> E>E
            codegen_expression_gt();
            symbol_stack_pops(&stack, 4);
            symbol_stack_push(&stack, SYM_E);
            break;
          } else if (stack->next->symbol == SYM_LTE &&
                     stack->next->next->symbol == SYM_E &&
                     stack->next->next->next->symbol == SYM_PREC_LT) {
            // E -> E<=E
            codegen_expression_lte();
            symbol_stack_pops(&stack, 4);
            symbol_stack_push(&stack, SYM_E);
            break;
          } else if (stack->next->symbol == SYM_GTE &&
                     stack->next->next->symbol == SYM_E &&
                     stack->next->next->next->symbol == SYM_PREC_LT) {
            // E -> E>=E
            codegen_expression_gte();
            symbol_stack_pops(&stack, 4);
            symbol_stack_push(&stack, SYM_E);
            break;
          }
        }
        // Error no rule
        return false;
      default:
        // Error
        return false;
    }
  } while (b != SYM_S ||
           !(stack->symbol == SYM_E && stack->next->symbol == SYM_S));
  return true;
}

expression_symbol_t expression_get_input() {
  token_t *token = token_buff(TOKEN_THIS);

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

bool expression_parse() {
  token_t *token = token_buff(TOKEN_THIS);
  switch (token->type) {
    case TT_INTEGER:
    case TT_NUMBER:
    case TT_STRING:
    case TT_LPAR:
    case TT_K_NIL:
    case TT_SOP_LENGTH:
    case TT_ID: {
      symbol_stack_t *stack = NULL;
      symbol_stack_push(&stack, SYM_S);
      return expression_process(stack);
    }
    default:
      return false;
  }
}
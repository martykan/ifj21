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

/**
 * Get the index in the precedence table 
 * @param symbol Symbol to check
 * @return Index of the symbol in the precedence table
 */
int precedence_table_index(expression_symbol_t symbol)
{
  switch (symbol)
  {
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
void symbol_stack_push(symbol_stack_t **stack, expression_symbol_t sym)
{
  symbol_stack_t *old_stack = *stack;
  symbol_stack_t *new_stack = (symbol_stack_t *)malloc(sizeof(symbol_stack_t));
  new_stack->symbol = sym;
  new_stack->next = old_stack;
  *stack = new_stack;
}

/**
 * Pop operation on symbol stack
 */
void symbol_stack_pop(symbol_stack_t **stack)
{
  symbol_stack_t *new_stack = (*stack)->next;
  free(*stack);
  *stack = new_stack;
}

/**
 * Pop operation on symbol stack, multiple times
 */
void symbol_stack_pops(symbol_stack_t **stack, int count)
{
  for (int i = 0; i < count; i++)
  {
    symbol_stack_pop(stack);
  }
}

/**
 * Get top symbol from stack (excluding E)
 */
expression_symbol_t get_top_symbol(symbol_stack_t *stack)
{
  expression_symbol_t a;
  symbol_stack_t *stack_curr = stack;
  do
  {
    a = stack_curr->symbol;
    stack_curr = stack_curr->next;
  } while (a == SYM_E && stack_curr != NULL);
  return a;
}

/**
 * Print stack - for debugging
 */
void print_stack(symbol_stack_t *stack)
{
  expression_symbol_t a;
  symbol_stack_t *stack_curr = stack;
  printf("[");
  do
  {
    a = stack_curr->symbol;
    printf("%s ", expression_symbol_t_names[a]);
    stack_curr = stack_curr->next;
  } while (stack_curr != NULL);
  printf("]\n");
}

/**
 * Process expression
 */
int expression_process(expression_symbol_t *input)
{
  symbol_stack_t *stack = NULL;
  symbol_stack_push(&stack, SYM_S);
  expression_symbol_t a, b;
  int i = 0;
  do
  {
    a = get_top_symbol(stack);
    b = input[i];
    print_stack(stack);
    printf("Terminal %s, input %s\n", expression_symbol_t_names[a], expression_symbol_t_names[b]);
    expression_precedence_t precedence = precedence_table[precedence_table_index(a)][precedence_table_index(b)];
    switch (precedence)
    {
    case PREC_EQ:
      printf("PREC_EQ\n");
      symbol_stack_push(&stack, b);
      i++;
      break;
    case PREC_LT:
      printf("PREC_LT\n");
      if (b == SYM_I)
      {
        printf("PUSH value to stack\n");
      }
      if (stack->symbol == SYM_E)
      {
        symbol_stack_pop(&stack);
        symbol_stack_push(&stack, SYM_PREC_LT);
        symbol_stack_push(&stack, SYM_E);
      }
      else
      {
        symbol_stack_push(&stack, SYM_PREC_LT);
      }
      symbol_stack_push(&stack, b);
      i++;
      break;
    case PREC_GT:
      printf("PREC_GT\n");
      // Test rules
      if (stack->symbol == SYM_I)
      {
        if (stack->next->symbol == SYM_PREC_LT)
        {
          // E -> i
          printf("E -> i\n");
          symbol_stack_pops(&stack, 2);
          symbol_stack_push(&stack, SYM_E);
          break;
        }
      }
      else if (stack->symbol == SYM_RBRACKET && stack->next->symbol == SYM_E && stack->next->next->symbol == SYM_LBRACKET && stack->next->next->next->symbol == SYM_PREC_LT)
      {
        // E -> (E)
        printf("E -> (E)\n");
        symbol_stack_pops(&stack, 4);
        symbol_stack_push(&stack, SYM_E);
        break;
      }
      else if (stack->symbol == SYM_E)
      {
        if (stack->next->symbol == SYM_PLUS && stack->next->next->symbol == SYM_E && stack->next->next->next->symbol == SYM_PREC_LT)
        {
          // E -> E+E
          printf("E -> E+E\n");
          symbol_stack_pops(&stack, 4);
          symbol_stack_push(&stack, SYM_E);
          break;
        }
        else if (stack->next->symbol == SYM_MINUS && stack->next->next->symbol == SYM_E && stack->next->next->next->symbol == SYM_PREC_LT)
        {
          // E -> E-E
          printf("E -> E-E\n");
          symbol_stack_pops(&stack, 4);
          symbol_stack_push(&stack, SYM_E);
          break;
        }
        else if (stack->next->symbol == SYM_TIMES && stack->next->next->symbol == SYM_E && stack->next->next->next->symbol == SYM_PREC_LT)
        {
          // E -> E*E
          printf("E -> E*E\n");
          symbol_stack_pops(&stack, 4);
          symbol_stack_push(&stack, SYM_E);
          break;
        }
        else if (stack->next->symbol == SYM_DIVIDE && stack->next->next->symbol == SYM_E && stack->next->next->next->symbol == SYM_PREC_LT)
        {
          // E -> E/E
          printf("E -> E/E\n");
          symbol_stack_pops(&stack, 4);
          symbol_stack_push(&stack, SYM_E);
          break;
        }
        else if (stack->next->symbol == SYM_DIVIDE2 && stack->next->next->symbol == SYM_E && stack->next->next->next->symbol == SYM_PREC_LT)
        {
          // E -> E//E
          printf("E -> E//E\n");
          symbol_stack_pops(&stack, 4);
          symbol_stack_push(&stack, SYM_E);
          break;
        }
        else if (stack->next->symbol == SYM_DOTDOT && stack->next->next->symbol == SYM_E && stack->next->next->next->symbol == SYM_PREC_LT)
        {
          // E -> E..E
          printf("E -> E..E\n");
          symbol_stack_pops(&stack, 4);
          symbol_stack_push(&stack, SYM_E);
          break;
        }
        else if (stack->next->symbol == SYM_EQ && stack->next->next->symbol == SYM_E && stack->next->next->next->symbol == SYM_PREC_LT)
        {
          // E -> E==E
          printf("E -> E==E\n");
          symbol_stack_pops(&stack, 4);
          symbol_stack_push(&stack, SYM_E);
          break;
        }
        else if (stack->next->symbol == SYM_NEQ && stack->next->next->symbol == SYM_E && stack->next->next->next->symbol == SYM_PREC_LT)
        {
          // E -> E~=E
          printf("E -> E~=E\n");
          symbol_stack_pops(&stack, 4);
          symbol_stack_push(&stack, SYM_E);
          break;
        }
        else if (stack->next->symbol == SYM_LT && stack->next->next->symbol == SYM_E && stack->next->next->next->symbol == SYM_PREC_LT)
        {
          // E -> E<E
          printf("E -> E<E\n");
          symbol_stack_pops(&stack, 4);
          symbol_stack_push(&stack, SYM_E);
          break;
        }
        else if (stack->next->symbol == SYM_GT && stack->next->next->symbol == SYM_E && stack->next->next->next->symbol == SYM_PREC_LT)
        {
          // E -> E>E
          printf("E -> E>E\n");
          symbol_stack_pops(&stack, 4);
          symbol_stack_push(&stack, SYM_E);
          break;
        }
        else if (stack->next->symbol == SYM_LTE && stack->next->next->symbol == SYM_E && stack->next->next->next->symbol == SYM_PREC_LT)
        {
          // E -> E<=E
          printf("E -> E<=E\n");
          symbol_stack_pops(&stack, 4);
          symbol_stack_push(&stack, SYM_E);
          break;
        }
        else if (stack->next->symbol == SYM_GTE && stack->next->next->symbol == SYM_E && stack->next->next->next->symbol == SYM_PREC_LT)
        {
          // E -> E>=E
          printf("E -> E>=E\n");
          symbol_stack_pops(&stack, 4);
          symbol_stack_push(&stack, SYM_E);
          break;
        }
      }
      // Error no rule
      return 0;
    default:
      // Error
      return 0;
    }

  } while (b != SYM_S || !(stack->symbol == SYM_E && stack->next->symbol == SYM_S));
  return 1;
}

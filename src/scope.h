/**
 * @file
 * @brief Scope API
 * @author Tomas Martykan (xmarty07)
 * @author Filip Stolfa (xstolf00)
 * @author Patrik Korytar (xkoryt04)
 *
 * FIT VUT IFJ Project:
 * Compiler of IFJ21 Language
 */

#ifndef __SCOPE_H
#define __SCOPE_H

#include <stdbool.h>

#define SCOPE_STACK_SIZE 100

typedef struct {
  char type; ///< Either 'f' - for if; or 'w' - for while
  unsigned int lvl; ///< Current level
} scope_item_t;


typedef struct {
  unsigned int if_cnt;
  unsigned int while_cnt;
  int top;
  scope_item_t stack[SCOPE_STACK_SIZE];
} scope_info_t;

extern scope_info_t *scope_info;


/** Intialize scope stack and counters.
 * Allocates memory for the sice of scope_info_t. On error sets the global error flag.
 * Counters are set to 0, top pointer to -1.
 */
void scope_init();

/** Free scope stack.
 */
void scope_destroy();

/** Push new item onto the scope stack.
 * @param type Type of scope for which to push item
 */
void scope_push_item(char type, unsigned int lvl);

/** Get item from top of the stack.
 * @return Scope item from the top of the stack
 */
scope_item_t scope_get_item();

/** Determine if scope stack is empty.
 * @return True if scope stack is empty, false otherwise.
 */
bool scope_empty();

/** Create a new if scope.
 * Increments if counter and pushes a new item onto the stack.
 */
void scope_new_if();

/** Create a new while scope.
 * Increments while counter and pushes a new item onto the stack.
 */
void scope_new_while();

/** Remove a scope item from the top of the stack.
 */
void scope_pop_item();

/** Append the appropriate scope suffix to id string.
 * @return Pointer to the new id with correct suffix.
 */
char* scope_get_correct_id(char *id, int lvl);

#endif

#ifndef __SCOPE_H
#define __SCOPE_H

#include <stdbool.h>

typedef struct {
  char type; ///< Either 'f' - for if; or 'w' - for while
  unsigned int lvl; ///< Current level
} scope_item_t;


typedef struct {
  unsigned int if_cnt;
  unsigned int while_cnt;
  int top;
  scope_item_t stack[100];
} scope_info_t;

extern scope_info_t *scope_info;


// scope_info_t* scope_init();
void scope_init();

void scope_destroy();

void scope_push_item(char type, unsigned int lvl);

scope_item_t scope_get_item();

bool scope_empty();

void scope_new_if();

void scope_new_while();

void scope_pop_item();

char* scope_get_correct_id(char *id, int lvl);

#endif
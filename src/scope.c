#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "scope.h"
#include "errors.h"

scope_info_t *scope_info;

// scope_info_t* scope_init() {
void scope_init() {
  scope_info = malloc(sizeof(scope_info_t));
  if (scope_info == NULL) {
    if (!error_get()) {
      error_set(EXITSTATUS_INTERNAL_ERROR);
    }
    return;
  }

  scope_info->if_cnt = 0;
  scope_info->while_cnt = 0;
  scope_info->top = -1;
}

void scope_destroy() {
  free(scope_info);
}

void scope_push_item(char type, unsigned int lvl) {
  scope_info->top++;
  scope_info->stack[scope_info->top].lvl = lvl;
  scope_info->stack[scope_info->top].type = type;
}

scope_item_t scope_get_item(int offset) {
  return scope_info->stack[scope_info->top - offset];
}

bool scope_empty() {
  return scope_info->top <= -1;
}

void scope_new_if() {
  scope_info->if_cnt++;
  scope_push_item('f', scope_info->if_cnt);
}

void scope_new_while() {
  scope_info->while_cnt++;
  scope_push_item('w', scope_info->while_cnt);
}

void scope_pop_item() {
  if (scope_info->top >= 0) {
    scope_info->top--;
  }
}

int scope_len() {
  return scope_info->top + 1;
}

char* scope_get_correct_id(char *id, int lvl) {
  static char new_id[100] = {'\0'};
  if (!scope_empty() && scope_len() - lvl > 0) {
    scope_item_t si = scope_get_item(lvl);
    sprintf(new_id, "%s$%c%d", id, si.type, si.lvl);
  }
  else {
    sprintf(new_id, "%s", id);
  }
  return new_id;
}

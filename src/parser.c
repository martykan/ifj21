/**
 * @file
 * @brief Parser implementation
 * @author Patrik Korytar
 * @author Tomas Martykan
 * @author Filip Stolfa
 *
 * TODO...
 */

#include "parser.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "dynstr.h"
#include "errors.h"
#include "scanner.h"
#include "symtable.h"
#include "scope.h"

// INIT

symtab_t* symtab;

bool parser_init_symtab() {
  symtab = symtab_create();

  return symtab;
}

// GET TOKEN

token_t* token_buff(int operation) {
  static token_t* token = NULL;

  if (operation == TOKEN_THIS) {
    return token;
  } else if (operation == TOKEN_NEW) {
    scanner_token_destroy(token);
    token = scanner_get_next_token();
    if (error_get()) {
      return NULL;
    }

    return token;
  } else if (operation == TOKEN_DELETE) {
    scanner_token_destroy(token);
  }

  return NULL;
}

// DEFINE AND DECLARE IDS

bool parser_declare_var(const char* id, char data_type) {
  symtab_var_data_t* var_data = symtab_insert_var(symtab, id);
  if (error_get()) {
    return false;
  }

  var_data->var_name = malloc(sizeof(char) * (strlen(id) + 1));
  if (!var_data->var_name) {
    error_set(EXITSTATUS_INTERNAL_ERROR);
    return false;
  }
  strncpy(var_data->var_name, id, strlen(id) + 1);

  var_data->data_type = data_type;
  var_data->is_init = false;
  var_data->is_used = false;

  return true;
}

bool parser_declare_func(const char* id, const dynstr_t* param_types,
                         const dynstr_t* return_types) {
  symtab_func_data_t* func_data = symtab_insert_func(symtab, id);
  if (error_get()) {
    return false;
  }

  func_data->was_defined = false;
  func_data->params = NULL;

  func_data->param_types = NULL;
  func_data->return_types = NULL;

  func_data->func_name = malloc(sizeof(char) * (strlen(id) + 1));
  if (!func_data->func_name) {
    error_set(EXITSTATUS_INTERNAL_ERROR);
    return false;
  }
  strncpy(func_data->func_name, id, strlen(id) + 1);

  func_data->param_types = dynstr_copy_to_static(param_types);
  if (error_get()) {
    return false;
  }
  func_data->return_types = dynstr_copy_to_static(return_types);
  if (error_get()) {
    return false;
  }

  return true;
}

symtab_vars_t* parser_get_params(int param_cnt) {
  symtab_vars_t* params = malloc(sizeof(symtab_vars_t));
  if (!params) {
    error_set(EXITSTATUS_INTERNAL_ERROR);
    return NULL;
  }
  params->cnt = param_cnt;
  params->vars = malloc(param_cnt * sizeof(symtab_var_data_t*));
  params->vars[0] = NULL;
  symtab_get_top_vars(symtab, params);

  return params;
}

bool parser_define_var(const char* id) {
  symtab_var_data_t* var_data = symtab_find_var(symtab, id, NULL);
  if (!var_data) {
    return false;
  }

  var_data->is_init = true;

  return true;
}

bool parser_define_func(const char* id, symtab_vars_t* params) {
  symtab_func_data_t* func_data = symtab_find_func(symtab, id);
  if (!func_data) {
    return false;
  }

  func_data->was_defined = true;
  func_data->params = params;

  return true;
}

bool parser_isdeclared_var(const char* id) {
  return symtab_find_var(symtab, id, NULL);
}

bool parser_isdeclared_func(const char* id) {
  return symtab_find_func(symtab, id);
}

bool parser_isdefined_var(const char* id) {
  symtab_var_data_t* var_data = symtab_find_var(symtab, id, NULL);
  if (var_data) {
    return var_data->is_init;
  }

  return false;
}

bool parser_isdefined_func(const char* id) {
  symtab_func_data_t* func_data = symtab_find_func(symtab, id);
  if (func_data) {
    return func_data->was_defined;
  }

  return false;
}

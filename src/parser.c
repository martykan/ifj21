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
#include "symtable.h"
#include "scanner.h"
#include "dynstr.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// INIT

symtab_t* symtab;

bool parser_init() {
  symtab = symtab_create();

  return symtab;
}

// GET TOKEN

token_t* token_buff(int operation) {
  static token_t* token = NULL;

  if(operation == TOKEN_THIS) {
    return token;
  } else if(operation == TOKEN_NEW) {
    free(token);
    token = scanner_get_next_token();
    return token;
  } else if(operation == TOKEN_DELETE) {
    free(token);
  }

  return NULL;
}

// DEFINE AND DECLARE IDS

bool parser_declare_var(const char* id, char data_type) {
  symtab_var_data_t* var_data = symtab_insert_var(symtab, id);
  if(!var_data) {
    return false;
  }

  var_data->data_type = data_type;
  var_data->is_init = false;
  var_data->is_used = false;

  return true;
}

bool parser_declare_func(const char* id, const dynstr_t* param_types, const dynstr_t* return_types) {
  symtab_func_data_t* func_data = symtab_insert_func(symtab, id);
  if(!func_data) {
    return false;
  }

  func_data->param_types = dynstr_copy_to_static(param_types);
  func_data->return_types = dynstr_copy_to_static(return_types);
  func_data->was_defined = false;

  return true;
}

bool parser_define_var(const char* id) {
  symtab_var_data_t* var_data = symtab_find_var(symtab, id);
  if(!var_data) {
    return false;
  }

  var_data->is_init = true;

  return true;
}

bool parser_define_func(const char* id) {
  symtab_func_data_t* func_data = symtab_find_func(symtab, id);
  if(!func_data) {
    return false;
  }

  func_data->was_defined = true;

  return true;
}

bool parser_isdeclared_var(const char* id) {
  return symtab_find_var(symtab, id);
}

bool parser_isdeclared_func(const char* id) {
  return symtab_find_func(symtab, id);
}

bool parser_isdefined_var(const char* id) {
  symtab_var_data_t* var_data = symtab_find_var(symtab, id);
  if(var_data) {
    return var_data->is_init;
  }

  return false;
}

bool parser_isdefined_func(const char* id) {
  symtab_func_data_t* func_data = symtab_find_func(symtab, id);
  if(func_data) {
    return func_data->was_defined;
  }

  return false;
}

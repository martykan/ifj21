/**
 * @file
 * @brief Parser implementation
 * @author Tomas Martykan (xmarty07)
 * @author Filip Stolfa (xstolf00)
 * @author Patrik Korytar (xkoryt04)
 *
 * FIT VUT IFJ Project:
 * Compiler of IFJ21 Language
 */

#include "parser.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "dynstr.h"
#include "errors.h"
#include "scanner.h"
#include "symtable.h"

// SYMTABLE USED BY PARSER

symtab_t* symtab;

// FUNCTION DEFINITIONS

// TOKEN

token_t* token_buff(int operation) {
  static token_t* token = NULL;

  if (operation == TOKEN_THIS) {
    return token;
  } else if (operation == TOKEN_NEW) {
    scanner_token_destroy(token);
    token = scanner_get_next_token();
    if (error_get() || token == NULL) {
      return NULL;
    }

    return token;
  } else if (operation == TOKEN_DELETE) {
    scanner_token_destroy(token);
    token = NULL;
  }

  return NULL;
}

// ALLOCATION AND DEALLOCATION

bool parser_init_symtab() {
  symtab = symtab_create();

  return symtab;
}

/**
 * Check whether function was defined.
 * If not, sets error.
 * Used as callback in foreach.
 * @param data Data of record.
 */
void is_defined(symtab_data_t* data) {
  if(!data->func_data.was_defined) {
    error_set(EXITSTATUS_ERROR_SEMANTIC_IDENTIFIER);
  }
}

void parser_destroy_symtab() {
  symtab_subtab_foreach(symtab->global_scope, is_defined);
  symtab_free(symtab);
  symtab = NULL;
}

// DECLARATIONS / DEFINITIONS OF IDENTIFIERS

bool parser_declare_var(const char* id, char data_type) {
  symtab_var_data_t* var_data = symtab_insert_var(symtab, id);
  if (error_get()) {
    return false;
  }

  var_data->var_name = malloc(strlen(id) + 1);
  if (!var_data->var_name) {
    error_set(EXITSTATUS_INTERNAL_ERROR);
    return false;
  }
  strncpy(var_data->var_name, id, strlen(id) + 1);

  var_data->data_type = data_type;
  var_data->is_init = false;

  return true;
}

bool parser_declare_func(const char* id, const dynstr_t* param_types,
                         const dynstr_t* return_types) {
  symtab_func_data_t* func_data = symtab_insert_func(symtab, id);
  if (error_get()) {
    return false;
  }

  func_data->func_name = malloc(strlen(id) + 1);
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

  func_data->was_defined = false;

  return true;
}

bool parser_define_var(const char* id) {
  symtab_var_data_t* var_data = symtab_find_var(symtab, id, NULL);
  if (!var_data) {
    return false;
  }

  var_data->is_init = true;

  return true;
}

bool parser_define_func(const char* id) {
  symtab_func_data_t* func_data = symtab_find_func(symtab, id);
  if (!func_data) {
    return false;
  }

  func_data->was_defined = true;

  return true;
}

// CHECK OF DECLARATION / DEFINITION

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

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

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// COMPILE-TIME CONSTANTS

#define KEYWORDS_COUNT 15

// INTERNAL DATA

/**
 * @brief All keywords of the language.
 */
static const char* keywords[KEYWORDS_COUNT] = {
    "do",      "else",    "end",    "function", "global",
    "if",      "integer", "local",  "nil",      "number",
    "require", "return",  "string", "then",     "while"};

// FUNCTION DEFINITIONS

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

bool is_identifier(const token_t* token) {
  if (token->type != TT_KEYWORD_ID) {
    return false;
  }

  for (int i = 0; i < KEYWORDS_COUNT; i++) {
    if (!strcmp(token->attribute, keywords[i])) {
      return false;
    }
  }

  return true;
}

bool is_keyword(const token_t* token, const char* keyword) {
  if (token->type == TT_KEYWORD_ID && !strcmp(token->attribute, keyword)) {
    return true;
  }

  return false;
}

bool symtab_var_declaration(symtab_t* symtab, const token_t* token, char data_type, bool is_init) {
  symtab_var_data_t* var_data = symtab_insert_var(symtab, token->attribute);
  if(!var_data) {
    return false;
  }

  var_data->data_type = data_type;
  var_data->is_init = is_init;
  var_data->is_used = false;

  return true;
}

bool symtab_func_declaration(symtab_t* symtab, const token_t* token, char* param_types, char* return_types,
      bool was_defined) {
  symtab_func_data_t* func_data = symtab_insert_func(symtab, token->attribute);
  if(!func_data) {
    return false;
  }

  func_data->param_types = param_types;
  func_data->return_types = return_types;
  func_data->was_defined = was_defined;

  return true;
}

bool symtab_var_isdeclared(const symtab_t* symtab, const token_t* token) {
  return symtab_find_var(symtab, token->attribute);
}

bool symtab_func_isdeclared(const symtab_t* symtab, const token_t* token) {
  return symtab_find_func(symtab, token->attribute);
}

bool symtab_var_isdefined(const symtab_t* symtab, const token_t* token) {
  return symtab_find_var(symtab, token->attribute)->is_init;
}

bool symtab_func_isdefined(const symtab_t* symtab, const token_t* token) {
  return symtab_find_func(symtab, token->attribute)->was_defined;
}

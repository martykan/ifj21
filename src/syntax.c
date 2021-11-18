/**
 * @file
 * @brief Recursive parser implementation
 * @author Patrik Korytar
 * @author Tomas Martykan
 * @author Filip Stolfa
 */

#include "syntax.h"

#include <stdbool.h>
#include <string.h>

#include "parser.h"
#include "scanner.h"
#include "symtable.h"
#include "dynstr.h"

bool parser_stlist_global();
bool parser_st_global();
bool parser_function_def();
bool parser_function_call();
bool parser_function_dec();
bool parser_param_list(dynstr_t* types_str);
bool parser_param_append(dynstr_t* types_str);
bool parser_param(dynstr_t* types_str);
bool parser_type_list(dynstr_t* types_str);
bool parser_type_append(dynstr_t* types_str);
bool parser_type(dynstr_t* types_str);
bool parser_arg_list();
bool parser_arg_append();
bool parser_arg();
bool parser_stlist_local();
bool parser_st_local();
bool parser_var_dec();
bool parser_if_st();
bool parser_while_st();
bool parser_init();
bool parser_init_after();
bool parser_id_after();
bool parser_id_append();
bool parser_assign();
bool parser_exp_list();
bool parser_exp_append();
bool parser_exp();

bool parser_start() {
  token_t* token = token_buff(TOKEN_NEW);
  if(!token) {
    return false;
  }

  return parser_stlist_global();
}

bool parser_stlist_global() {
  token_t* token = token_buff(TOKEN_THIS);

  switch(token->type) {
    case TT_K_REQUIRE:
    case TT_K_FUNCTION:
    case TT_K_GLOBAL:
    case TT_ID:
      return parse_st_globarl() && parser_stlist_global();
    case TT_EOF:
      return true;
    default:
      return false;
  }
}

bool parser_st_global() {
  token_t* token = token_buff(TOKEN_THIS);

  switch(token->type) {
    case TT_K_REQUIRE:
      token = token_buff(TOKEN_NEW);
      token_buff(TOKEN_NEW);

      return token->type == TT_STRING;
    case TT_K_FUNCTION:
      token = token_buff(TOKEN_NEW);

      return parser_function_def();
    case TT_K_GLOBAL:
      token = token_buff(TOKEN_NEW);

      return parser_function_dec();
    case TT_ID:
      token = token_buff(TOKEN_NEW);

      return parser_function_call();
    default:
      return false;
  }
}

char* str_create_copy(char* str) {
  char* copy = malloc(strlen(str) + 1);
  if (!copy) {
    return NULL;
  }
  strcpy(copy, str);

  return copy;
}

bool parser_function_def() {
  token_t* token = token_buff(TOKEN_THIS);

  if(token->type == TT_ID) {
    char* id = str_create_copy(token->attr.str);
    symtab_func_data_t* func_data = symtab_find_func(symtab, id);
    if(func_data && func_data->was_defined) {
      free(id);
      return false;
    }
    token = token_buff(TOKEN_NEW);

    if(token->type == TT_LPAR) {
      token = token_buff(TOKEN_NEW);
      dynstr_t param_types;
      dynstr_init(&param_types);

      if(parser_param_list(&param_types)) {
        token = token_buff(TOKEN_THIS);

        if(token->type == TT_RPAR) {
          token = token_buff(TOKEN_NEW);
          dynstr_t ret_types;
          dynstr_init(&ret_types);

          if(parser_type_list(&ret_types) && parser_stlist_local()) {
            token = token_buff(TOKEN_THIS);

            if(token->type == TT_K_END) {
              token_buff(TOKEN_NEW);

              bool is_correct = true;
              if(func_data) {
                if(strcmp(func_data->param_types, param_types.str) || strcmp(func_data->return_types, ret_types.str)) {
                  is_correct = false;
                }
                else {
                  parser_define_func(id);
                }
              }
              else {
                parser_declare_func(id, &param_types, &ret_types);
                parser_define_func(id);
              }

              free(id);
              dynstr_free_buffer(&param_types);
              dynstr_free_buffer(&ret_types);

              return is_correct;
            }

            return false;
          }
        }
      }
    }
  }

  return false;
}

bool parser_function_call() {
  token_t* token = token_buff(TOKEN_THIS);

  if(token->type == TT_LPAR) {
    token = token_buff(TOKEN_NEW);

    if(parser_arg_list()) {
      token = token_buff(TOKEN_THIS);
      token_buff(TOKEN_NEW);

      return token->type == TT_RPAR;
    }
  }

  return false;
}

bool parser_function_dec() {
  token_t* token = token_buff(TOKEN_THIS);

  if(token->type == TT_ID) {
    char* id = str_create_copy(token->attr.str);
    token = token_buff(TOKEN_NEW);

    if(token->type == TT_COLON) {
      token = token_buff(TOKEN_NEW);

      if(token->type == TT_K_FUNCTION) {
        token = token_buff(TOKEN_NEW);

        if(token->type == TT_LPAR) {
          token = token_buff(TOKEN_NEW);
          dynstr_t param_types;
          dynstr_init(&param_types);

          if(parser_param_list(&param_types)) {
            token = token_buff(TOKEN_THIS);

            if(token->type == TT_RPAR) {
              token_buff(TOKEN_NEW);
              dynstr_t ret_types;
              dynstr_init(&ret_types);

              if(parser_type_list(&ret_types)) {
                symtab_func_data_t* func_data = symtab_find_func(symtab, id);

                bool is_correct = true;
                if(func_data) {
                  if(strcmp(func_data->param_types, param_types.str) || strcmp(func_data->return_types, ret_types.str)) {
                    is_correct = false;
                  }
                  else {
                    parser_define_func(id);
                  }
                }
                else {
                  parser_declare_func(id, &param_types, &ret_types);
                }

                free(id);
                dynstr_free_buffer(&param_types);
                dynstr_free_buffer(&ret_types);

                return is_correct;
              }

              return false;
            }
          }
        }
      }
    }

    free(id);
  }

  return false;
}

bool parser_param_list(dynstr_t* types_str) {
  token_t* token = token_buff(TOKEN_THIS);

  switch(token->type) {
    case TT_ID:
      return parser_param(types_str) && parser_param_append(types_str);
    case TT_RPAR:
      return true;
    default:
      return false;
  }
}

bool parser_param_append(dynstr_t* types_str) {
  token_t* token = token_buff(TOKEN_THIS);

  switch(token->type) {
    case TT_COMMA:
      token_buff(TOKEN_NEW);

      return parser_param(types_str) && parser_param_append(types_str);
    case TT_RPAR:
      return true;
    default:
      return false;
  }
}

bool parser_param(dynstr_t* types_str) {
  token_t* token = token_buff(TOKEN_THIS);

  if(token->type == TT_ID) {
    token = token_buff(TOKEN_NEW);

    if(token->type == TT_COLON) {
      token = token_buff(TOKEN_NEW);

      return parser_type(types_str);
    }
  }

  return false;
}

bool parser_type_list(dynstr_t* types_str) {
  token_t* token = token_buff(TOKEN_THIS);

  switch (token->type) {
    case TT_K_REQUIRE:
    case TT_K_FUNCTION:
    case TT_K_GLOBAL:
    case TT_K_LOCAL:
    case TT_K_IF:
    case TT_K_WHILE:
    case TT_K_RETURN:
    case TT_K_END:
    case TT_ID:
    case TT_EOF:
      return true;
    case TT_COLON:
      token_buff(TOKEN_NEW);

      return parser_type(types_str) && parser_type_append(types_str);
    default:
      return false;
  }
}

bool parser_type_append(dynstr_t* types_str) {
  token_t* token = token_buff(TOKEN_THIS);

  switch(token->type) {
    case TT_K_REQUIRE:
    case TT_K_FUNCTION:
    case TT_K_GLOBAL:
    case TT_K_LOCAL:
    case TT_K_IF:
    case TT_K_WHILE:
    case TT_K_RETURN:
    case TT_K_END:
    case TT_ID:
    case TT_EOF:
      return true;
    case TT_COMMA:
      token_buff(TOKEN_NEW);

      return parser_type(types_str) && parser_type_append(types_str);

    default:
      return false;
  }
}

bool parser_type(dynstr_t* types_str) {
  token_t* token = token_buff(TOKEN_THIS);

  switch(token->type) {
    case TT_K_INTEGER:
      dynstr_append(types_str, 'i');
      token_buff(TOKEN_NEW);
      return true;
    case TT_K_NUMBER:
      dynstr_append(types_str, 'n');
      token_buff(TOKEN_NEW);
      return true;
    case TT_K_STRING:
      dynstr_append(types_str, 's');
      token_buff(TOKEN_NEW);
      return true;
    default:
      return false;
  }
}

bool parser_arg_list() {
  token_t* token = token_buff(TOKEN_THIS);

  switch(token->type) {
    case TT_ID:
    case TT_INTEGER:
    case TT_NUMBER:
    case TT_STRING:
      return parser_arg() && parser_arg_append();
    case TT_RPAR:
      return true;
    default:
      return false;
  }
}

bool parser_arg_append() {
  token_t* token = token_buff(TOKEN_THIS);

  switch(token->type) {
    case TT_COMMA:
      token_buff(TOKEN_NEW);
      return parser_arg() && parser_arg_append();
    case TT_RPAR:
      return true;
    default:
      return false;
  }
}

bool parser_arg() {
  token_t* token = token_buff(TOKEN_THIS);

  switch(token->type) {
    case TT_ID:
    case TT_INTEGER:
    case TT_NUMBER:
    case TT_STRING:
      token_buff(TOKEN_NEW);
      return true;
    default:
      return false;
  }
}

bool parser_stlist_local() {
  token_t* token = token_buff(TOKEN_THIS);

  switch(token->type) {
    case TT_K_LOCAL:
    case TT_K_IF:
    case TT_K_WHILE:
    case TT_K_RETURN:
    case TT_ID:
      return parse_st_local() && parser_stlist_local();
    case TT_K_ELSE:
    case TT_K_END:
      return true;
    default:
      return false;
  }
}

bool parser_st_local() {
  token_t* token = token_buff(TOKEN_THIS);

  switch(token->type) {
    case TT_K_LOCAL:
      token = token_buff(TOKEN_NEW);
      return parser_var_dec();
    case TT_K_IF:
      token = token_buff(TOKEN_NEW);
      return parser_if_st();
    case TT_K_WHILE:
      token = token_buff(TOKEN_NEW);
      return parser_while_st();
    case TT_K_RETURN:
      token = token_buff(TOKEN_NEW);
      return parser_exp_list();
    case TT_ID:
      token = token_buff(TOKEN_NEW);
      return parser_id_after();
    default:
      return false;
  }
}

bool parser_var_dec() {
  token_t* token = token_buff(TOKEN_THIS);

  if(token->type == TT_ID) {
    char* id = str_create_copy(token->attr.str);
    symtab_var_data_t* var_data = symtab_find_var(symtab, id);
    if(var_data) {
      free(id);
      return false;
    }
    token = token_buff(TOKEN_NEW);

    if(token->type == TT_COLON) {
      token = token_buff(TOKEN_NEW);
      dynstr_t var_type;
      dynstr_init(&var_type);

      if(parser_type(&var_type) && parser_init()) {
        parser_declare_var(id, var_type.str[0]);

        free(id);
        dynstr_free_buffer(&var_type);

        return true;
      }

      return false;
    }
  }

  return false;
}

bool parser_if_st() {
  token_t* token = token_buff(TOKEN_THIS);

  if(token->type == TT_LPAR) {
    token = token_buff(TOKEN_NEW);

    if(parser_exp()) {
      token = token_buff(TOKEN_THIS);

      if(token->type == TT_RPAR) {
        token = token_buff(TOKEN_NEW);

        if(token->type == TT_K_THEN) {
          token = token_buff(TOKEN_NEW);

          if(st_list_local()) {
            token = token_buff(TOKEN_THIS);

            if(token->type == TT_K_ELSE) {
              token = token_buff(TOKEN_NEW);

              if(st_list_local()) {
                token = token_buff(TOKEN_THIS);

                return token->type == TT_K_END;
              }
            }
          }
        }
      }
    }
  }

  return false;
}

bool parser_while_st() {
  token_t* token = token_buff(TOKEN_THIS);

  if(token->type == TT_LPAR) {
    token = token_buff(TOKEN_NEW);

    if(parser_exp()) {
      token = token_buff(TOKEN_THIS);

      if(token->type == TT_RPAR) {
        token = token_buff(TOKEN_NEW);

        if(token->type == TT_K_DO) {
          token = token_buff(TOKEN_NEW);

          if(st_list_local()) {
            token = token_buff(TOKEN_THIS);
            return token->type == TT_K_END;
          }
        }
      }
    }
  }

  return false;
}

bool parser_init() {
  token_t* token = token_buff(TOKEN_THIS);

  switch(token->type) {
    case TT_COP_EQ:
      token_buff(TOKEN_NEW);
      return parser_init_after();
    case TT_K_LOCAL:
    case TT_K_IF:
    case TT_K_ELSE:
    case TT_K_WHILE:
    case TT_K_RETURN:
    case TT_K_END:
    case TT_ID:
      return true;
    default:
      return false;
  }
}

bool parser_init_after() {
  token_t* token = token_buff(TOKEN_THIS);

  switch(token->type) {
    case TT_INTEGER:
    case TT_NUMBER:
    case TT_STRING:
    case TT_LPAR:
    case TT_K_NIL:
    case TT_SOP_LENGTH:
      return parser_exp();
    case TT_ID:
      if(symtab_var_isdeclared(symtab, token)) {
        return parser_exp();
      } else if (symtab_func_isdeclared(symtab, token)) {
        token_buff(TOKEN_NEW);
        return parser_function_call();
      }
    default:
      return false;
  }
}

bool parser_id_after() {
  token_t* token = token_buff(TOKEN_THIS);

  switch(token->type) {
    case TT_LPAR:
      token_buff(TOKEN_NEW);

      if(parser_arg_list()) {
        token = token_buff(TOKEN_THIS);
        token_buff(TOKEN_NEW);

        return token->type == TT_RPAR;
      }
    case TT_COMMA:
    case TT_COP_EQ:
      if(parser_id_append()) {
        token = token_buff(TOKEN_THIS);

        if(token->type == TT_COP_EQ) {
          token_buff(TOKEN_NEW);
          return parser_assign();
        }
      }
    default:
      return false;
  }
}

bool parser_id_append() {
  token_t* token = token_buff(TOKEN_THIS);

  switch(token->type) {
    case TT_COMMA:
      token = token_buff(TOKEN_NEW);

      if(token->type == TT_ID) {
        token_buff(TOKEN_NEW);
        return parser_id_append();
      }
    case TT_COP_EQ:
      return true;
    default:
      return false;
  }
}

bool parser_assign() {
  token_t* token = token_buff(TOKEN_THIS);

  switch(token->type) {
    case TT_INTEGER:
    case TT_NUMBER:
    case TT_STRING:
    case TT_LPAR:
    case TT_K_NIL:
    case TT_SOP_LENGTH:
      return parser_exp_list();
    case TT_ID:
      if(symtab_var_isdeclared(symtab, token)) {
        return parser_exp_list();
      } else if (symtab_func_isdeclared(symtab, token)) {
        token_buff(TOKEN_NEW);
        return parser_function_call();
      }
    default:
      return false;
  }
}

bool parser_exp_list() {
  token_t* token = token_buff(TOKEN_THIS);

  switch(token->type) {
    case TT_INTEGER:
    case TT_NUMBER:
    case TT_STRING:
    case TT_LPAR:
    case TT_K_NIL:
    case TT_SOP_LENGTH:
    case TT_ID:
      return parser_exp() && parser_exp_append();
    default:
      return false;
  }
}

bool parser_exp_append() {
  token_t* token = token_buff(TOKEN_THIS);

  switch(token->type) {
    case TT_K_LOCAL:
    case TT_K_IF:
    case TT_K_ELSE:
    case TT_K_WHILE:
    case TT_K_RETURN:
    case TT_K_END:
    case TT_ID:
      return true;
    case TT_COMMA:
      token_buff(TOKEN_NEW);
      return parser_exp() && parser_exp_append();
    default:
      return false;
  }
}

bool parser_exp() {
  token_t* token = token_buff(TOKEN_THIS);

  switch(token->type) {
    case TT_INTEGER:
    case TT_NUMBER:
    case TT_STRING:
    case TT_LPAR:
    case TT_K_NIL:
    case TT_SOP_LENGTH:
    case TT_ID:
      return call_bt_parser();
    default:
      return false;
  }
}

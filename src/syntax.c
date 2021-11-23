/**
 * @file
 * @brief Recursive parser implementation
 * @author Patrik Korytar
 * @author Tomas Martykan
 * @author Filip Stolfa
 */

#include "syntax.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "codegen.h"
#include "dynstr.h"
#include "errors.h"
#include "expressions.h"
#include "other.h"
#include "parser.h"
#include "scanner.h"
#include "symtable.h"

bool parser_stlist_global();
bool parser_st_global();
bool parser_require();
bool parser_function_def();
bool parser_function_call_by_id(char* id);
bool parser_function_call(symtab_func_data_t* func);
bool parser_function_dec();
bool parser_param_list(dynstr_t* param_types);
bool parser_param_append(dynstr_t* param_types);
bool parser_param(char* param_type);
bool parser_type_list(dynstr_t* types);
bool parser_type_append(dynstr_t* types);
bool parser_type(char* type);
bool parser_arg_list(dynstr_t* arg_types, int* arg_pos);
bool parser_arg_append(dynstr_t* arg_types, int* arg_pos);
bool parser_arg(char* arg_type);
bool parser_local_scope();
bool parser_stlist_local();
bool parser_st_local();
bool parser_returned();
bool parser_return_what();
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

bool parser_arglist_match(char* arg1, char* arg2);

bool parser_start() {
  token_t* token = token_buff(TOKEN_NEW);
  if (error_get()) {
    return false;
  }

  return parser_stlist_global();
}

bool parser_stlist_global() {
  token_t* token = token_buff(TOKEN_THIS);

  switch (token->type) {
    case TT_K_REQUIRE:
    case TT_K_FUNCTION:
    case TT_K_GLOBAL:
    case TT_ID:
      return parser_st_global() && parser_stlist_global();
    case TT_EOF:
      return true;
    default:
      error_set(EXITSTATUS_ERROR_SYNTAX);
      return false;
  }
}

bool parser_st_global() {
  token_t* token = token_buff(TOKEN_THIS);

  switch (token->type) {
    case TT_K_REQUIRE:
      token = token_buff(TOKEN_NEW);
      if (error_get()) {
        return false;
      }

      return parser_require();
    case TT_K_FUNCTION:
      token = token_buff(TOKEN_NEW);
      if (error_get()) {
        return false;
      }

      return parser_function_def();
    case TT_K_GLOBAL:
      token = token_buff(TOKEN_NEW);
      if (error_get()) {
        return false;
      }

      return parser_function_dec();
    case TT_ID:
      return parser_function_call_by_id(token->attr.str);
    default:
      error_set(EXITSTATUS_ERROR_SYNTAX);
      return false;
  }
}

bool parser_require() {
  token_t* token = token_buff(TOKEN_THIS);

  if (token->type == TT_STRING) {
    token_buff(TOKEN_NEW);
    if (error_get()) {
      return false;
    }

    return true;
  }

  error_set(EXITSTATUS_ERROR_SYNTAX);
  return false;
}

bool parser_function_def() {
  token_t* token = token_buff(TOKEN_THIS);

  // is syntax correct
  bool is_correct = false;

  char* id = str_create_copy(token->attr.str);
  if (error_get()) {
    goto EXIT;
  }
  dynstr_t param_types;
  dynstr_init(&param_types);
  if (error_get()) {
    goto FREE_ID;
  }
  dynstr_t ret_types;
  dynstr_init(&ret_types);
  if (error_get()) {
    goto FREE_PARAM_TYPES;
  }
  symtab_subtab_push(symtab);
  if (error_get()) {
    goto FREE_RET_TYPES;
  }

  if (token->type == TT_ID) {
    symtab_func_data_t* declared_func = symtab_find_func(symtab, id);
    if (declared_func && declared_func->was_defined) {
      error_set(EXITSTATUS_ERROR_SEMANTIC_IDENTIFIER);
      goto POP_SUBTAB;
    }

    token = token_buff(TOKEN_NEW);
    if (error_get()) {
      goto POP_SUBTAB;
    }

    if (token->type == TT_LPAR) {
      token = token_buff(TOKEN_NEW);
      if (error_get()) {
        goto POP_SUBTAB;
      }

      if (parser_param_list(&param_types)) {
        token = token_buff(TOKEN_THIS);

        if (token->type == TT_RPAR) {
          token = token_buff(TOKEN_NEW);
          if (error_get()) {
            goto POP_SUBTAB;
          }

          codegen_function_definition_begin(id);
          if (parser_type_list(&ret_types) && parser_local_scope()) {
            token = token_buff(TOKEN_THIS);

            if (token->type == TT_K_END) {
              token_buff(TOKEN_NEW);
              if (error_get()) {
                goto POP_SUBTAB;
              }

              if (declared_func) {
                if (strcmp(declared_func->param_types, param_types.str) ||
                    strcmp(declared_func->return_types, ret_types.str)) {
                  // function declaration and definition dont match
                  error_set(EXITSTATUS_ERROR_SEMANTIC_IDENTIFIER);
                  goto POP_SUBTAB;
                } else {
                  parser_define_func(id);
                }
              } else {
                parser_declare_func(id, &param_types, &ret_types);
                if (error_get()) {
                  goto POP_SUBTAB;
                }
                parser_define_func(id);
              }

              is_correct = true;
              codegen_function_definition_end(id);
              goto POP_SUBTAB;
            }
          }
        }
      }
    }
  }

  // if error occured in called function
  if (error_get()) {
    goto POP_SUBTAB;
  }

  error_set(EXITSTATUS_ERROR_SYNTAX);

POP_SUBTAB:
  symtab_subtab_pop(symtab);
FREE_RET_TYPES:
  dynstr_free_buffer(&ret_types);
FREE_PARAM_TYPES:
  dynstr_free_buffer(&param_types);
FREE_ID:
  free(id);
EXIT:
  return is_correct;
}

bool parser_function_dec() {
  token_t* token = token_buff(TOKEN_THIS);

  // is syntax correct
  bool is_correct = false;

  char* id = str_create_copy(token->attr.str);
  if (error_get()) {
    goto EXIT;
  }
  dynstr_t param_types;
  dynstr_init(&param_types);
  if (error_get()) {
    goto FREE_ID;
  }
  dynstr_t ret_types;
  dynstr_init(&ret_types);
  if (error_get()) {
    goto FREE_PARAM_TYPES;
  }

  if (token->type == TT_ID) {
    symtab_func_data_t* declared_func = symtab_find_func(symtab, id);
    if (declared_func) {
      error_set(EXITSTATUS_ERROR_SEMANTIC_IDENTIFIER);
      goto FREE_RET_TYPES;
    }

    token = token_buff(TOKEN_NEW);
    if (error_get()) {
      goto FREE_RET_TYPES;
    }

    if (token->type == TT_COLON) {
      token = token_buff(TOKEN_NEW);
      if (error_get()) {
        goto FREE_RET_TYPES;
      }

      if (token->type == TT_K_FUNCTION) {
        token = token_buff(TOKEN_NEW);
        if (error_get()) {
          goto FREE_RET_TYPES;
        }

        if (token->type == TT_LPAR) {
          token = token_buff(TOKEN_NEW);
          if (error_get()) {
            goto FREE_RET_TYPES;
          }

          if (parser_param_list(&param_types)) {
            token = token_buff(TOKEN_THIS);

            if (token->type == TT_RPAR) {
              token_buff(TOKEN_NEW);
              if (error_get()) {
                goto FREE_RET_TYPES;
              }

              if (parser_type_list(&ret_types)) {
                parser_declare_func(id, &param_types, &ret_types);
                if (error_get()) {
                  goto FREE_RET_TYPES;
                }

                is_correct = true;
                goto FREE_RET_TYPES;
              }
            }
          }
        }
      }
    }
  }

  // if error occured in called function
  if (error_get()) {
    goto FREE_RET_TYPES;
  }

  error_set(EXITSTATUS_ERROR_SYNTAX);

FREE_RET_TYPES:
  dynstr_free_buffer(&ret_types);
FREE_PARAM_TYPES:
  dynstr_free_buffer(&param_types);
FREE_ID:
  free(id);
EXIT:
  return is_correct;
}

// checks whether called functions args match
// declared parameters
bool parser_func_call_match(char* params, char* args) {
  while (*params != '\0') {
    // more params than args
    if (*args == '\0') {
      if (*(++params) == '+') return true;
      (--params);
      return false;
    }

    if (*params != *args) {
      if (*params != 'a' && (*params != 'n' || *args != 'i')) {
        return false;
      }
    }

    params++;
    args++;
    if (*params == '+') {
      params--;
    }
  }

  // more args than params
  if (*args != '\0') {
    return false;
  }

  return true;
}

bool parser_function_call_by_id(char* id) {
  symtab_func_data_t* declared_func = symtab_find_func(symtab, id);
  if (!declared_func) {
    error_set(EXITSTATUS_ERROR_SEMANTIC_IDENTIFIER);
    return false;
  }

  token_t* token = token_buff(TOKEN_NEW);
  if (error_get()) {
    return false;
  }

  return parser_function_call(declared_func);
}

bool parser_function_call(symtab_func_data_t* func) {
  token_t* token = token_buff(TOKEN_THIS);

  // is syntax correct
  bool is_correct = false;

  dynstr_t arg_types;
  dynstr_init(&arg_types);
  if (error_get()) {
    goto EXIT;
  }

  if (token->type == TT_LPAR) {
    token = token_buff(TOKEN_NEW);
    if (error_get()) {
      goto FREE_PARAM_TYPES;
    }

    int arg_count = 0;
    codegen_function_call_begin(func->func_name);
    if (parser_arg_list(&arg_types, &arg_count)) {
      codegen_function_call_argument_count(arg_count);
      token = token_buff(TOKEN_THIS);

      if (token->type == TT_RPAR) {
        token_buff(TOKEN_NEW);
        if (error_get()) {
          goto FREE_PARAM_TYPES;
        }

        if (!parser_func_call_match(func->param_types, arg_types.str)) {
          // function declaration and call dont match
          error_set(EXITSTATUS_ERROR_SEMANTIC_FUN_PARAMETERS);
          goto FREE_PARAM_TYPES;
        }

        is_correct = true;
        codegen_function_call_do(func->func_name, arg_count);
        goto FREE_PARAM_TYPES;
      }
    }
  }

  // if error occured in called function
  if (error_get()) {
    goto FREE_PARAM_TYPES;
  }

  error_set(EXITSTATUS_ERROR_SYNTAX);

FREE_PARAM_TYPES:
  dynstr_free_buffer(&arg_types);
EXIT:
  return is_correct;
}

bool parser_param_list(dynstr_t* param_types) {
  token_t* token = token_buff(TOKEN_THIS);

  switch (token->type) {
    case TT_ID: {
      char param_type;
      if (parser_param(&param_type)) {
        dynstr_append(param_types, param_type);
        if (error_get()) {
          return false;
        }

        return parser_param_append(param_types);
      }
    }
    case TT_RPAR:
      return true;
    default:
      error_set(EXITSTATUS_ERROR_SYNTAX);
      return false;
  }
}

bool parser_param_append(dynstr_t* param_types) {
  token_t* token = token_buff(TOKEN_THIS);

  switch (token->type) {
    case TT_COMMA:
      token_buff(TOKEN_NEW);
      if (error_get()) {
        return false;
      }

      char param_type;
      if (parser_param(&param_type)) {
        dynstr_append(param_types, param_type);
        if (error_get()) {
          return false;
        }

        return parser_param_append(param_types);
      }
    case TT_RPAR:
      return true;
    default:
      error_set(EXITSTATUS_ERROR_SYNTAX);
      return false;
  }
}

bool parser_param(char* param_type) {
  token_t* token = token_buff(TOKEN_THIS);

  // is syntax correct
  bool is_correct = false;

  char* id = str_create_copy(token->attr.str);
  if (error_get()) {
    goto EXIT;
  }

  if (token->type == TT_ID) {
    symtab_var_data_t* declared_var = symtab_find_var(symtab, id);
    if (!declared_var) {
      error_set(EXITSTATUS_ERROR_SEMANTIC_IDENTIFIER);
      goto FREE_ID;
    }

    token = token_buff(TOKEN_NEW);
    if (error_get()) {
      goto FREE_ID;
    }

    if (token->type == TT_COLON) {
      token = token_buff(TOKEN_NEW);
      if (error_get()) {
        goto FREE_ID;
      }

      if (parser_type(param_type)) {
        parser_declare_var(id, *param_type);
        if (error_get()) {
          goto FREE_ID;
        }

        is_correct = true;
        goto FREE_ID;
      }
    }
  }

  // if error occured in called function
  if (error_get()) {
    goto FREE_ID;
  }

  error_set(EXITSTATUS_ERROR_SYNTAX);

FREE_ID:
  free(id);
EXIT:
  return is_correct;
}

bool parser_type_list(dynstr_t* types) {
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
      if (error_get()) {
        return false;
      }

      char type;
      if (parser_type(&type)) {
        dynstr_append(types, type);
        if (error_get()) {
          return false;
        }

        return parser_type_append(types);
      }
    default:
      error_set(EXITSTATUS_ERROR_SYNTAX);
      return false;
  }
}

bool parser_type_append(dynstr_t* types) {
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
    case TT_COMMA:
      token_buff(TOKEN_NEW);
      if (error_get()) {
        return false;
      }

      char type;
      if (parser_type(&type)) {
        dynstr_append(types, type);
        if (error_get()) {
          return false;
        }

        return parser_type_append(types);
      }
    default:
      error_set(EXITSTATUS_ERROR_SYNTAX);
      return false;
  }
}

bool parser_type(char* type) {
  token_t* token = token_buff(TOKEN_THIS);

  switch (token->type) {
    case TT_K_INTEGER:
      token_buff(TOKEN_NEW);
      if (error_get()) {
        return false;
      }

      *type = 'i';
      return true;
    case TT_K_NUMBER:
      token_buff(TOKEN_NEW);
      if (error_get()) {
        return false;
      }

      *type = 'n';
      return true;
    case TT_K_STRING:
      token_buff(TOKEN_NEW);
      if (error_get()) {
        return false;
      }

      *type = 's';
      return true;
    default:
      error_set(EXITSTATUS_ERROR_SYNTAX);
      return false;
  }
}

bool parser_arg_list(dynstr_t* arg_types, int* arg_pos) {
  token_t* token = token_buff(TOKEN_THIS);

  switch (token->type) {
    case TT_ID:
    case TT_INTEGER:
    case TT_NUMBER:
    case TT_STRING: {
      char arg_type;
      if (parser_arg(&arg_type)) {
        dynstr_append(arg_types, arg_type);
        if (error_get()) {
          return false;
        }
        codegen_function_call_argument(token, *arg_pos);
        ++(*arg_pos);

        token_buff(TOKEN_NEW);
        if (error_get()) {
          return false;
        }

        return parser_arg_append(arg_types, arg_pos);
      }
    }
    case TT_RPAR:
      return true;
    default:
      error_set(EXITSTATUS_ERROR_SYNTAX);
      return false;
  }
}

bool parser_arg_append(dynstr_t* arg_types, int* arg_pos) {
  token_t* token = token_buff(TOKEN_THIS);

  switch (token->type) {
    case TT_COMMA:
      token_buff(TOKEN_NEW);
      if (error_get()) {
        return false;
      }

      char arg_type;
      if (parser_arg(&arg_type)) {
        dynstr_append(arg_types, arg_type);
        if (error_get()) {
          return false;
        }
        codegen_function_call_argument(token, *arg_pos);
        ++(*arg_pos);

        token_buff(TOKEN_NEW);
        if (error_get()) {
          return false;
        }

        return parser_arg_append(arg_types, arg_pos);
      }
    case TT_RPAR:
      return true;
    default:
      error_set(EXITSTATUS_ERROR_SYNTAX);
      return false;
  }
}

bool parser_arg(char* arg_type) {
  token_t* token = token_buff(TOKEN_THIS);

  switch (token->type) {
    case TT_ID: {
      symtab_var_data_t* declared_var =
          symtab_find_var(symtab, token->attr.str);
      if (!declared_var) {
        error_set(EXITSTATUS_ERROR_SEMANTIC_IDENTIFIER);
        return false;
      }

      *arg_type = declared_var->data_type;
      return true;
    }
    case TT_INTEGER:
      *arg_type = 'i';
      return true;
    case TT_NUMBER:
      *arg_type = 'n';
      return true;
    case TT_STRING:
      *arg_type = 's';
      return true;
    default:
      error_set(EXITSTATUS_ERROR_SYNTAX);
      return false;
  }
}

bool parser_local_scope() {
  token_t* token = token_buff(TOKEN_THIS);

  switch (token->type) {
    case TT_K_LOCAL:
    case TT_K_IF:
    case TT_K_ELSE:
    case TT_K_WHILE:
    case TT_K_RETURN:
    case TT_K_END:
    case TT_ID:
      return parser_stlist_local() && parser_returned();
    default:
      error_set(EXITSTATUS_ERROR_SYNTAX);
      return false;
  }
}

bool parser_stlist_local() {
  token_t* token = token_buff(TOKEN_THIS);

  switch (token->type) {
    case TT_K_LOCAL:
    case TT_K_IF:
    case TT_K_WHILE:
    case TT_ID:
      return parser_st_local() && parser_stlist_local();
    case TT_K_ELSE:
    case TT_K_RETURN:
    case TT_K_END:
      return true;
    default:
      error_set(EXITSTATUS_ERROR_SYNTAX);
      return false;
  }
}

bool parser_st_local() {
  token_t* token = token_buff(TOKEN_THIS);

  switch (token->type) {
    case TT_K_LOCAL:
      token = token_buff(TOKEN_NEW);
      if (error_get()) {
        return false;
      }

      return parser_var_dec();
    case TT_K_IF:
      token = token_buff(TOKEN_NEW);
      if (error_get()) {
        return false;
      }

      return parser_if_st();
    case TT_K_WHILE:
      token = token_buff(TOKEN_NEW);
      if (error_get()) {
        return false;
      }

      return parser_while_st();
    case TT_ID:
      // TODO for now ignoring other cases, since parser_id_after is unfinished
      /*token = token_buff(TOKEN_NEW);
      if (error_get()) {
        return false;
      }

      return parser_id_after();*/
      return parser_function_call_by_id(token->attr.str);
    default:
      error_set(EXITSTATUS_ERROR_SYNTAX);
      return false;
  }
}

bool parser_returned() {
  token_t* token = token_buff(TOKEN_THIS);

  switch (token->type) {
    case TT_K_RETURN:
      token_buff(TOKEN_NEW);
      if (error_get()) {
        return false;
      }

      return parser_return_what();
    case TT_K_ELSE:
    case TT_K_END:
      return true;
    default:
      error_set(EXITSTATUS_ERROR_SYNTAX);
      return false;
  }
}

bool parser_return_what() {
  token_t* token = token_buff(TOKEN_THIS);

  switch (token->type) {
    case TT_INTEGER:
    case TT_NUMBER:
    case TT_STRING:
    case TT_LPAR:
    case TT_K_NIL:
    case TT_SOP_LENGTH:
    case TT_ID:
      return parser_exp_list();
    case TT_K_ELSE:
    case TT_K_END:
      return true;
    default:
      error_set(EXITSTATUS_ERROR_SYNTAX);
      return false;
  }
}

bool parser_var_dec() {
  token_t* token = token_buff(TOKEN_THIS);

  // is syntax correct
  bool is_correct = false;

  char* id = str_create_copy(token->attr.str);
  if (error_get()) {
    goto EXIT;
  }

  if (token->type == TT_ID) {
    symtab_var_data_t* declared_var = symtab_find_var(symtab, token->attr.str);
    if (!declared_var) {
      error_set(EXITSTATUS_ERROR_SEMANTIC_IDENTIFIER);
      goto FREE_ID;
    }

    token = token_buff(TOKEN_NEW);
    if (error_get()) {
      goto FREE_ID;
    }

    if (token->type == TT_COLON) {
      token = token_buff(TOKEN_NEW);
      if (error_get()) {
        goto FREE_ID;
      }

      char var_type;
      if (parser_type(&var_type) && parser_init_symtab()) {
        parser_declare_var(id, var_type);
        if (error_get()) {
          goto FREE_ID;
        }

        is_correct = true;
        goto FREE_ID;
      }
    }
  }

  // if error occured in called function
  if (error_get()) {
    goto FREE_ID;
  }

  error_set(EXITSTATUS_ERROR_SYNTAX);

FREE_ID:
  free(id);
EXIT:
  return is_correct;
}

bool parser_if_st() {
  token_t* token = token_buff(TOKEN_THIS);

  if (token->type == TT_LPAR) {
    token = token_buff(TOKEN_NEW);
    if (error_get()) {
      return false;
    }

    if (parser_exp()) {
      token = token_buff(TOKEN_THIS);

      if (token->type == TT_RPAR) {
        token = token_buff(TOKEN_NEW);
        if (error_get()) {
          return false;
        }

        if (token->type == TT_K_THEN) {
          token = token_buff(TOKEN_NEW);
          if (error_get()) {
            return false;
          }

          if (parser_local_scope()) {
            token = token_buff(TOKEN_THIS);

            if (token->type == TT_K_ELSE) {
              token = token_buff(TOKEN_NEW);
              if (error_get()) {
                return false;
              }

              if (parser_local_scope()) {
                token = token_buff(TOKEN_THIS);

                if (token->type == TT_K_END) {
                  token_buff(TOKEN_NEW);
                  if (error_get()) {
                    return false;
                  }

                  return true;
                }
              }
            }
          }
        }
      }
    }
  }

  // if error occured in called function
  if (error_get()) {
    goto EXIT;
  }

  error_set(EXITSTATUS_ERROR_SYNTAX);

EXIT:
  return false;
}

bool parser_while_st() {
  token_t* token = token_buff(TOKEN_THIS);

  if (token->type == TT_LPAR) {
    token = token_buff(TOKEN_NEW);
    if (error_get()) {
      goto EXIT;
    }

    if (parser_exp()) {
      token = token_buff(TOKEN_THIS);

      if (token->type == TT_RPAR) {
        token = token_buff(TOKEN_NEW);
        if (error_get()) {
          goto EXIT;
        }

        if (token->type == TT_K_DO) {
          token = token_buff(TOKEN_NEW);
          if (error_get()) {
            goto EXIT;
          }

          if (parser_local_scope()) {
            token = token_buff(TOKEN_THIS);

            if (token->type == TT_K_END) {
              token_buff(TOKEN_NEW);
              if (error_get()) {
                return false;
              }

              return true;
            }
          }
        }
      }
    }
  }

  // if error occured in called function
  if (error_get()) {
    goto EXIT;
  }

  error_set(EXITSTATUS_ERROR_SYNTAX);

EXIT:
  return false;
}

bool parser_init() {
  token_t* token = token_buff(TOKEN_THIS);

  switch (token->type) {
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

  switch (token->type) {
    case TT_INTEGER:
    case TT_NUMBER:
    case TT_STRING:
    case TT_LPAR:
    case TT_K_NIL:
    case TT_SOP_LENGTH:
      return parser_exp();
    case TT_ID:
      if (parser_isdeclared_var(token->attr.str)) {
        return parser_exp();
      } else if (parser_isdeclared_func(token->attr.str)) {
        return parser_function_call_by_id(token->attr.str);
      }
    default:
      return false;
  }
}

bool parser_id_after() {
  token_t* token = token_buff(TOKEN_THIS);

  switch (token->type) {
    case TT_LPAR:
      token_buff(TOKEN_NEW);

      dynstr_t arg_types;
      dynstr_init(&arg_types);
      if (error_get()) {
        return false;
      }

      int arg_count = 0;
      if (parser_arg_list(&arg_types, &arg_count)) {
        token = token_buff(TOKEN_THIS);
        token_buff(TOKEN_NEW);

        return token->type == TT_RPAR;
      }
    case TT_COMMA:
    case TT_COP_EQ:
      if (parser_id_append()) {
        token = token_buff(TOKEN_THIS);

        if (token->type == TT_COP_EQ) {
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

  switch (token->type) {
    case TT_COMMA:
      token = token_buff(TOKEN_NEW);

      if (token->type == TT_ID) {
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

  switch (token->type) {
    case TT_INTEGER:
    case TT_NUMBER:
    case TT_STRING:
    case TT_LPAR:
    case TT_K_NIL:
    case TT_SOP_LENGTH:
      return parser_exp_list();
    case TT_ID:
      if (parser_isdeclared_var(token->attr.str)) {
        return parser_exp();
      } else if (parser_isdeclared_func(token->attr.str)) {
        return parser_function_call_by_id(token->attr.str);
      }
    default:
      return false;
  }
}

bool parser_exp_list() {
  token_t* token = token_buff(TOKEN_THIS);

  switch (token->type) {
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

  switch (token->type) {
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

  switch (token->type) {
    case TT_INTEGER:
    case TT_NUMBER:
    case TT_STRING:
    case TT_LPAR:
    case TT_K_NIL:
    case TT_SOP_LENGTH:
    case TT_ID:
      return expression_parse();
    default:
      return false;
  }
}

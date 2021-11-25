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
#include "scope.h"
#include "symtable.h"

bool parser_stlist_global();
bool parser_st_global();
bool parser_require();
bool parser_function_def();
bool parser_function_call_by_id(char* id);
bool parser_function_call(symtab_func_data_t* func);
bool parser_function_dec();
bool parser_param_list(dynstr_t* param_types);
bool parser_param_append(dynstr_t* param_types, int argpos);
bool parser_param(char* param_type);
bool parser_type_list_param(dynstr_t* types);
bool parser_type_list_return(dynstr_t* types);
bool parser_type_append(dynstr_t* types);
bool parser_type(char* type);
bool parser_arg_list(dynstr_t* arg_types, int* arg_pos);
bool parser_arg_append(dynstr_t* arg_types, int* arg_pos);
bool parser_arg(char* arg_type, int* lvl);
bool parser_local_scope(const char* func_name, const dynstr_t* ret_types,
                        bool create_scope);
bool parser_stlist_local(const char* func_name, const dynstr_t* ret_types);
bool parser_st_local(const char* func_name, const dynstr_t* ret_types);
bool parser_returned(char* ret_types, dynstr_t* exp_types);
bool parser_return_what(char* ret_types, dynstr_t* exp_types);
bool parser_var_dec(const char* func_name);
bool parser_if_st(const char* func_name, const dynstr_t* ret_types);
bool parser_while_st(const char* func_name, const dynstr_t* ret_types);
bool parser_init(char var_type, bool* did_init);
bool parser_init_after(char var_type);
bool parser_id_after();
bool parser_assign_st(char* id);
bool parser_id_append(dynstr_t* id_types);
bool parser_assign_what(const dynstr_t* id_types);
bool parser_assign_exp(const dynstr_t* id_types);
bool parser_exp_list(char* id_types, dynstr_t* exp_types);
bool parser_exp_append(char* id_types, int pos, dynstr_t* exp_types);
bool parser_exp(char* exp_type);

bool parser_func_call_match(char* params, char* args);
bool parser_func_ret_match(char* declared, char* returned);
bool parser_init_func_match(char var_type, char* ret);
bool parser_assign_func_match(char* ids, char* ret);
bool parser_assign_exp_match(char* ids, char* exps);

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
    token = token_buff(TOKEN_NEW);
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

  symtab_vars_t* params = NULL;

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

      codegen_function_definition_begin(id);
      if (parser_param_list(&param_types)) {
        token = token_buff(TOKEN_THIS);

        params = parser_get_params(strlen(param_types.str));
        if (error_get()) {
          goto POP_SUBTAB;
        }

        if (token->type == TT_RPAR) {
          token = token_buff(TOKEN_NEW);
          if (error_get()) {
            goto FREE_PARAMS;
          }

          if (!parser_type_list_return(&ret_types)) return false;
          if (declared_func) {
            if (strcmp(declared_func->param_types, param_types.str) ||
                strcmp(declared_func->return_types, ret_types.str)) {
              // function declaration and definition dont match
              error_set(EXITSTATUS_ERROR_SEMANTIC_IDENTIFIER);
              goto FREE_PARAMS;
            }
          } else {
            parser_declare_func(id, &param_types, &ret_types);
            if (error_get()) {
              goto FREE_PARAMS;
            }
          }

          parser_define_func(id, params);
          if (parser_local_scope(id, &ret_types, false)) {
            token = token_buff(TOKEN_THIS);

            if (token->type == TT_K_END) {
              token = token_buff(TOKEN_NEW);
              if (error_get()) {
                goto FREE_PARAMS;
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

  // if no other error
  if (!error_get()) {
    error_set(EXITSTATUS_ERROR_SYNTAX);
  }

FREE_PARAMS:
  for (int i = 0; i < params->cnt; i++) {
    free(params->vars[i]);
  }
  free(params->vars);
  free(params);
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

          if (parser_type_list_param(&param_types)) {
            token = token_buff(TOKEN_THIS);

            if (token->type == TT_RPAR) {
              token = token_buff(TOKEN_NEW);
              if (error_get()) {
                goto FREE_RET_TYPES;
              }

              if (parser_type_list_return(&ret_types)) {
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

  // if no other error
  if (!error_get()) {
    error_set(EXITSTATUS_ERROR_SYNTAX);
  }

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

  id = NULL;
  token_t* token = token_buff(TOKEN_NEW);
  if (error_get()) {
    return false;
  }

  return parser_function_call(declared_func);
}

// SCOPE
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
        token = token_buff(TOKEN_NEW);
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

  // if no other error
  if (!error_get()) {
    error_set(EXITSTATUS_ERROR_SYNTAX);
  }

FREE_PARAM_TYPES:
  dynstr_free_buffer(&arg_types);
EXIT:
  return is_correct;
}

// SCOPE
bool parser_param_list(dynstr_t* param_types) {
  token_t* token = token_buff(TOKEN_THIS);

  switch (token->type) {
    case TT_ID: {
      char* name = str_create_copy(token->attr.str);
      char param_type;
      if (parser_param(&param_type)) {
        dynstr_append(param_types, param_type);
        if (error_get()) {
          return false;
          free(name);
        }
        codegen_function_definition_param(name, 0);
        free(name);

        return parser_param_append(param_types, 1);
      }

      break;
    }
    case TT_RPAR:
      return true;
    default:
      break;
  }

  error_set(EXITSTATUS_ERROR_SYNTAX);
  return false;
}

// SCOPE
bool parser_param_append(dynstr_t* param_types, int argpos) {
  token_t* token = token_buff(TOKEN_THIS);

  switch (token->type) {
    case TT_COMMA:
      token = token_buff(TOKEN_NEW);
      if (error_get()) {
        return false;
      }
      if (token->type != TT_ID) return false;
      char* name = str_create_copy(token->attr.str);

      char param_type;
      if (parser_param(&param_type)) {
        dynstr_append(param_types, param_type);
        if (error_get()) {
          free(name);
          return false;
        }
        codegen_function_definition_param(name, argpos);
        free(name);

        return parser_param_append(param_types, argpos + 1);
      }

      break;
    case TT_RPAR:
      return true;
    default:
      break;
  }

  error_set(EXITSTATUS_ERROR_SYNTAX);
  return false;
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
    symtab_var_data_t* declared_var = symtab_find_var(symtab, id, NULL);
    if (declared_var) {
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

  // if no other error
  if (!error_get()) {
    error_set(EXITSTATUS_ERROR_SYNTAX);
  }

FREE_ID:
  free(id);
EXIT:
  return is_correct;
}

bool parser_type_list_param(dynstr_t* types) {
  token_t* token = token_buff(TOKEN_THIS);

  switch (token->type) {
    case TT_RPAR:
      return true;
    case TT_K_NUMBER:
    case TT_K_INTEGER:
    case TT_K_STRING: {
      char type;
      if (parser_type(&type)) {
        dynstr_append(types, type);
        if (error_get()) {
          return false;
        }

        return parser_type_append(types);
      }
    } break;
    default:
      break;
  }

  error_set(EXITSTATUS_ERROR_SYNTAX);
  return false;
}

bool parser_type_list_return(dynstr_t* types) {
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
      token = token_buff(TOKEN_NEW);
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

      break;
    default:
      break;
  }

  error_set(EXITSTATUS_ERROR_SYNTAX);
  return false;
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
    case TT_RPAR:
      return true;
    case TT_COMMA:
      token = token_buff(TOKEN_NEW);
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

      break;
    default:
      break;
  }

  error_set(EXITSTATUS_ERROR_SYNTAX);
  return false;
}

bool parser_type(char* type) {
  token_t* token = token_buff(TOKEN_THIS);

  switch (token->type) {
    case TT_K_INTEGER:
      token = token_buff(TOKEN_NEW);
      if (error_get()) {
        return false;
      }

      *type = 'i';
      return true;
    case TT_K_NUMBER:
      token = token_buff(TOKEN_NEW);
      if (error_get()) {
        return false;
      }

      *type = 'n';
      return true;
    case TT_K_STRING:
      token = token_buff(TOKEN_NEW);
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
      int lvl = 0;
      if (parser_arg(&arg_type, &lvl)) {
        dynstr_append(arg_types, arg_type);
        if (error_get()) {
          return false;
        }
        codegen_function_call_argument(token, *arg_pos, lvl);
        ++(*arg_pos);

        token = token_buff(TOKEN_NEW);
        if (error_get()) {
          return false;
        }

        return parser_arg_append(arg_types, arg_pos);
      }
    }

    break;
    case TT_RPAR:
      return true;
    default:
      break;
  }

  error_set(EXITSTATUS_ERROR_SYNTAX);
  return false;
}

bool parser_arg_append(dynstr_t* arg_types, int* arg_pos) {
  token_t* token = token_buff(TOKEN_THIS);

  switch (token->type) {
    case TT_COMMA:
      token = token_buff(TOKEN_NEW);
      if (error_get()) {
        return false;
      }

      int lvl = 0;
      char arg_type;
      if (parser_arg(&arg_type, &lvl)) {
        dynstr_append(arg_types, arg_type);
        if (error_get()) {
          return false;
        }
        codegen_function_call_argument(token, *arg_pos, lvl);
        ++(*arg_pos);

        token = token_buff(TOKEN_NEW);
        if (error_get()) {
          return false;
        }

        return parser_arg_append(arg_types, arg_pos);
      }

      break;
    case TT_RPAR:
      return true;
    default:
      break;
  }

  error_set(EXITSTATUS_ERROR_SYNTAX);
  return false;
}

bool parser_arg(char* arg_type, int* lvl) {
  token_t* token = token_buff(TOKEN_THIS);

  switch (token->type) {
    case TT_ID: {
      symtab_var_data_t* declared_var =
          symtab_find_var(symtab, token->attr.str, lvl);
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

// checks whether returned expressions match
// declared return types
bool parser_func_ret_match(char* declared, char* returned) {
  while (*declared != '\0') {
    // more declared return values than returned
    // NILs are returned
    if (*returned == '\0') {
      return true;
    }

    if (*declared != *returned) {
      if (*declared != 'n' || *returned != 'i') {
        return false;
      }
    }

    declared++;
    returned++;
  }

  // more returned values than declared
  if (*returned != '\0') {
    return false;
  }

  return true;
}

bool parser_local_scope(const char* func_name, const dynstr_t* ret_types,
                        bool create_scope) {
  token_t* token = token_buff(TOKEN_THIS);

  // is syntax correct
  bool is_correct = false;

  if (create_scope) {
    symtab_subtab_push(symtab);
    if (error_get()) {
      goto EXIT;
    }
  }

  dynstr_t exp_types;
  dynstr_init(&exp_types);
  if (error_get()) {
    goto POP_SUBTAB;
  }

  switch (token->type) {
    case TT_K_LOCAL:
    case TT_K_IF:
    case TT_K_ELSE:
    case TT_K_WHILE:
    case TT_K_RETURN:
    case TT_K_END:
    case TT_ID:
      if (parser_stlist_local(func_name, ret_types) &&
          parser_returned(ret_types->str, &exp_types)) {
        if (!parser_func_ret_match(ret_types->str, exp_types.str)) {
          error_set(EXITSTATUS_ERROR_SEMANTIC_FUN_PARAMETERS);
          goto FREE_EXP_TYPES;
        }

        is_correct = true;
        goto FREE_EXP_TYPES;
      }

      break;
    default:
      break;
  }

  // if no other error
  if (!error_get()) {
    error_set(EXITSTATUS_ERROR_SYNTAX);
  }

FREE_EXP_TYPES:
  dynstr_free_buffer(&exp_types);
POP_SUBTAB:
  if (create_scope) {
    symtab_subtab_pop(symtab);
  }
EXIT:
  return is_correct;
}

bool parser_stlist_local(const char* func_name, const dynstr_t* ret_types) {
  token_t* token = token_buff(TOKEN_THIS);

  switch (token->type) {
    case TT_K_LOCAL:
    case TT_K_IF:
    case TT_K_WHILE:
    case TT_ID:
      return parser_st_local(func_name, ret_types) &&
             parser_stlist_local(func_name, ret_types);
    case TT_K_ELSE:
    case TT_K_END:
    case TT_K_RETURN:
      return true;
    default:
      error_set(EXITSTATUS_ERROR_SYNTAX);
      return false;
  }
}

bool parser_st_local(const char* func_name, const dynstr_t* ret_types) {
  token_t* token = token_buff(TOKEN_THIS);

  switch (token->type) {
    case TT_K_LOCAL:
      token = token_buff(TOKEN_NEW);
      if (error_get()) {
        return false;
      }

      return parser_var_dec(func_name);
    case TT_K_IF:
      token = token_buff(TOKEN_NEW);
      if (error_get()) {
        return false;
      }

      return parser_if_st(func_name, ret_types);
    case TT_K_WHILE:
      token = token_buff(TOKEN_NEW);
      if (error_get()) {
        return false;
      }

      return parser_while_st(func_name, ret_types);
    case TT_ID:
      // does not consume token

      return parser_id_after();
    default:
      error_set(EXITSTATUS_ERROR_SYNTAX);
      return false;
  }
}

bool parser_returned(char* ret_types, dynstr_t* exp_types) {
  token_t* token = token_buff(TOKEN_THIS);

  switch (token->type) {
    case TT_K_RETURN:
      token = token_buff(TOKEN_NEW);
      if (error_get()) {
        return false;
      }

      if (parser_return_what(ret_types, exp_types)) {
        codegen_function_return();
        return true;
      }
      return false;
    case TT_K_ELSE:
    case TT_K_END:
      return true;
    default:
      error_set(EXITSTATUS_ERROR_SYNTAX);
      return false;
  }
}

bool parser_return_what(char* ret_types, dynstr_t* exp_types) {
  token_t* token = token_buff(TOKEN_THIS);

  switch (token->type) {
    case TT_INTEGER:
    case TT_NUMBER:
    case TT_STRING:
    case TT_LPAR:
    case TT_K_NIL:
    case TT_SOP_LENGTH:
    case TT_ID:
      return parser_exp_list(ret_types, exp_types);
    case TT_K_ELSE:
    case TT_K_END:
      return true;
    default:
      error_set(EXITSTATUS_ERROR_SYNTAX);
      return false;
  }
}

bool parser_var_dec(const char* func_name) {
  token_t* token = token_buff(TOKEN_THIS);

  // is syntax correct
  bool is_correct = false;

  char* id = str_create_copy(token->attr.str);
  if (error_get()) {
    goto EXIT;
  }

  if (token->type == TT_ID) {
    // search local scope for a variable of the same name
    symtab_var_data_t* declared_var =
        symtab_find_var_local(symtab, token->attr.str);
    if (declared_var) {
      error_set(EXITSTATUS_ERROR_SEMANTIC_IDENTIFIER);
      goto FREE_ID;
    }
    symtab_func_data_t* declared_func =
        symtab_find_func(symtab, token->attr.str);
    if (declared_func) {
      error_set(EXITSTATUS_ERROR_SEMANTIC_IDENTIFIER);
      goto FREE_ID;
    }
    // name of var same as now being defined func
    if (!strcmp(func_name, token->attr.str)) {
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

      int lvl = 0;
      char var_type = 0;
      if (parser_type(&var_type)) {
        codegen_define_var(id, 0);
        bool did_init = false;
        if (parser_init(var_type, &did_init)) {
          parser_declare_var(id, var_type);
          if (did_init) {
            codegen_assign_expression_add(id, lvl);
            codegen_assign_expression_finish();
          }
          if (error_get()) {
            goto FREE_ID;
          }

          is_correct = true;
          goto FREE_ID;
        }
      }
    }
  }

  // if no other error
  if (!error_get()) {
    error_set(EXITSTATUS_ERROR_SYNTAX);
  }

FREE_ID:
  free(id);
EXIT:
  return is_correct;
}

bool parser_if_st(const char* func_name, const dynstr_t* ret_types) {
  token_t* token = token_buff(TOKEN_THIS);

  char cond_type;
  if (parser_exp(&cond_type)) {
    if (cond_type != 'b') {
      codegen_not_nil();
    }
    token = token_buff(TOKEN_THIS);

    scope_new_if();

    if (token->type == TT_K_THEN) {
      token = token_buff(TOKEN_NEW);
      if (error_get()) {
        return false;
      }

      codegen_if_begin();

      if (parser_local_scope(func_name, ret_types, true)) {
        token = token_buff(TOKEN_THIS);

        if (token->type == TT_K_ELSE) {
          token = token_buff(TOKEN_NEW);
          if (error_get()) {
            return false;
          }

          codegen_if_else();

          scope_pop_item();  // else
          scope_new_if();

          if (parser_local_scope(func_name, ret_types, true)) {
            token = token_buff(TOKEN_THIS);

            if (token->type == TT_K_END) {
              token = token_buff(TOKEN_NEW);
              if (error_get()) {
                return false;
              }

              codegen_if_end();

              scope_pop_item();  // end if

              return true;
            }
          }
        }
      }
    }
  }

  // if no other error
  if (!error_get()) {
    error_set(EXITSTATUS_ERROR_SYNTAX);
  }

EXIT:
  return false;
}

bool parser_while_st(const char* func_name, const dynstr_t* ret_types) {
  token_t* token = token_buff(TOKEN_THIS);

  // scope_new_while();
  codegen_while_begin();

  char cond_type;
  if (parser_exp(&cond_type)) {
    if (cond_type != 'b') {
      codegen_not_nil();
    }
    token = token_buff(TOKEN_THIS);

    if (token->type == TT_K_DO) {
      token = token_buff(TOKEN_NEW);
      if (error_get()) {
        goto EXIT;
      }

      codegen_while_expr();

      if (parser_local_scope(func_name, ret_types, true)) {
        token = token_buff(TOKEN_THIS);

        if (token->type == TT_K_END) {
          token = token_buff(TOKEN_NEW);
          if (error_get()) {
            return false;
          }

          codegen_while_end();

          // scope_pop_item();

          return true;
        }
      }
    }
  }

  // if no other error
  if (!error_get()) {
    error_set(EXITSTATUS_ERROR_SYNTAX);
  }

EXIT:
  return false;
}

bool parser_init(char var_type, bool* did_init) {
  token_t* token = token_buff(TOKEN_THIS);

  switch (token->type) {
    case TT_ASSIGN:
      token = token_buff(TOKEN_NEW);
      if (error_get()) {
        return false;
      }
      *did_init = true;

      return parser_init_after(var_type);
    case TT_K_LOCAL:
    case TT_K_IF:
    case TT_K_ELSE:
    case TT_K_WHILE:
    case TT_K_RETURN:
    case TT_K_END:
    case TT_ID:
      return true;
    default:
      error_set(EXITSTATUS_ERROR_SYNTAX);
      return false;
  }
}

bool parser_init_func_match(char var_type, char* ret) {
  if (var_type != *ret) {
    if (var_type != 'n' || *ret != 'i') {
      error_set(EXITSTATUS_ERROR_SEMANTIC_ASSIGNMENT);
      return false;
    }
  }

  // func can return more vals than expected

  return true;
}

bool parser_init_after(char var_type) {
  token_t* token = token_buff(TOKEN_THIS);

  switch (token->type) {
    case TT_INTEGER:
    case TT_NUMBER:
    case TT_STRING:
    case TT_LPAR:
    case TT_K_NIL:
    case TT_SOP_LENGTH: {
      char exp_type;
      if (parser_exp(&exp_type)) {
        if (var_type != exp_type) {
          if (var_type == 'n' && exp_type == 'i') {
            codegen_cast_int_to_float1();
          } else {
            if (exp_type == 'x') {
              error_set(EXITSTATUS_ERROR_UNEXPECTED_NIL);
            } else {
              error_set(EXITSTATUS_ERROR_SEMANTIC_FUN_PARAMETERS);
            }
            return false;
          }
        }

        return true;
      }
      break;
    }
    case TT_ID:
      // can be function call or
      // expression starting with id

      if (parser_isdeclared_var(token->attr.str)) {
        char exp_type;
        if (parser_exp(&exp_type)) {
          if (var_type != exp_type) {
            if (var_type == 'n' && exp_type == 'i') {
              codegen_cast_int_to_float1();
            } else {
              if (exp_type == 'x') {
                error_set(EXITSTATUS_ERROR_UNEXPECTED_NIL);
              } else {
                error_set(EXITSTATUS_ERROR_SEMANTIC_FUN_PARAMETERS);
              }
              return false;
            }
          }

          return true;
        }
      } else if (parser_isdeclared_func(token->attr.str)) {
        symtab_func_data_t* declared =
            symtab_find_func(symtab, token->attr.str);
        if (parser_function_call_by_id(token->attr.str)) {
          if (!parser_init_func_match(var_type, declared->return_types)) {
            return false;
          }

          return true;
        }
      } else {
        error_set(EXITSTATUS_ERROR_SEMANTIC_IDENTIFIER);
        return false;
      }

      break;
    default:
      break;
  }

  if (!error_get()) {
    error_set(EXITSTATUS_ERROR_SYNTAX);
  }
  return false;
}

bool parser_id_after() {
  token_t* token = token_buff(TOKEN_THIS);
  // token is ID

  // is syntax correct
  bool is_correct = false;

  char* id = str_create_copy(token->attr.str);
  if (error_get()) {
    goto EXIT;
  }

  token = token_buff(TOKEN_NEW);
  if (error_get()) {
    goto FREE_ID;
  }

  switch (token->type) {
    case TT_LPAR: {
      symtab_func_data_t* declared_func = symtab_find_func(symtab, id);
      if (!declared_func) {
        error_set(EXITSTATUS_ERROR_SEMANTIC_IDENTIFIER);
        return false;
      }
      if (parser_function_call(declared_func)) {
        is_correct = true;
        goto FREE_ID;
      }

    } break;
    case TT_COMMA:
    case TT_ASSIGN:
      if (parser_assign_st(id)) {
        is_correct = true;
        goto FREE_ID;
      }

      break;
    default:
      break;
  }

  // if no other error
  if (!error_get()) {
    error_set(EXITSTATUS_ERROR_SYNTAX);
  }

FREE_ID:
  free(id);
EXIT:
  return is_correct;
}

bool parser_assign_st(char* id) {
  // is syntax correct
  bool is_correct = false;

  int lvl = 0;
  symtab_var_data_t* declared_var = symtab_find_var(symtab, id, &lvl);
  if (!declared_var) {
    error_set(EXITSTATUS_ERROR_SEMANTIC_IDENTIFIER);
    goto EXIT;
  }

  dynstr_t id_types;
  dynstr_init(&id_types);
  if (error_get()) {
    goto EXIT;
  }

  dynstr_append(&id_types, declared_var->data_type);
  if (error_get()) {
    goto FREE_ID_TYPES;
  }
  codegen_assign_expression_add(id, lvl);

  if (parser_id_append(&id_types)) {
    token_t* token = token_buff(TOKEN_THIS);

    if (token->type == TT_ASSIGN) {
      token = token_buff(TOKEN_NEW);
      if (error_get()) {
        goto FREE_ID_TYPES;
      }

      if (parser_assign_what(&id_types)) {
        codegen_assign_expression_finish();
        is_correct = true;
        goto FREE_ID_TYPES;
      }
    }
  }

  // if no other error
  if (!error_get()) {
    error_set(EXITSTATUS_ERROR_SYNTAX);
  }

FREE_ID_TYPES:
  dynstr_free_buffer(&id_types);
EXIT:
  return is_correct;
}

bool parser_id_append(dynstr_t* id_types) {
  token_t* token = token_buff(TOKEN_THIS);

  switch (token->type) {
    case TT_COMMA:
      token = token_buff(TOKEN_NEW);
      if (error_get()) {
        return false;
      }

      int lvl = 0;
      if (token->type == TT_ID) {
        symtab_var_data_t* declared_var =
            symtab_find_var(symtab, token->attr.str, &lvl);
        if (!declared_var) {
          error_set(EXITSTATUS_ERROR_SEMANTIC_IDENTIFIER);
          return false;
        }

        dynstr_append(id_types, declared_var->data_type);
        if (error_get()) {
          return false;
        }
        codegen_assign_expression_add(token->attr.str, lvl);

        token = token_buff(TOKEN_NEW);
        if (error_get()) {
          return false;
        }

        return parser_id_append(id_types);
      }

      break;
    case TT_ASSIGN:
      return true;
    default:
      break;
  }

  error_set(EXITSTATUS_ERROR_SYNTAX);
  return false;
}

bool parser_assign_what(const dynstr_t* id_types) {
  token_t* token = token_buff(TOKEN_THIS);

  switch (token->type) {
    case TT_INTEGER:
    case TT_NUMBER:
    case TT_STRING:
    case TT_LPAR:
    case TT_K_NIL:
    case TT_SOP_LENGTH:
      return parser_assign_exp(id_types);
    case TT_ID:
      if (parser_isdeclared_var(token->attr.str)) {
        return parser_assign_exp(id_types);
      } else if (parser_isdeclared_func(token->attr.str)) {
        symtab_func_data_t* declared =
            symtab_find_func(symtab, token->attr.str);
        if (parser_function_call_by_id(token->attr.str)) {
          if (!parser_assign_func_match(id_types->str,
                                        declared->return_types)) {
            return false;
          }

          return true;
        }
      } else {
        error_set(EXITSTATUS_ERROR_SEMANTIC_IDENTIFIER);
        return false;
      }

      break;
    default:
      break;
  }

  error_set(EXITSTATUS_ERROR_SYNTAX);
  return false;
}

// checks whether id list matches
// assigned expressions
bool parser_assign_exp_match(char* ids, char* exps) {
  while (*ids != '\0') {
    // more ids than exps
    if (*exps == '\0') {
      error_set(EXITSTATUS_ERROR_SEMANTIC_FUN_PARAMETERS);
      return false;
    }

    if (*ids != *exps) {
      if (*ids != 'n' || *exps != 'i') {
        if (*exps == 'x') {
          error_set(EXITSTATUS_ERROR_UNEXPECTED_NIL);
        } else {
          error_set(EXITSTATUS_ERROR_SEMANTIC_ASSIGNMENT);
        }
        return false;
      }
    }

    ids++;
    exps++;
  }

  // more exps than ids
  if (*exps != '\0') {
    error_set(EXITSTATUS_ERROR_SEMANTIC_FUN_PARAMETERS);
    return false;
  }

  return true;
}

// checks whether id list matches
// assigned return values from function
bool parser_assign_func_match(char* ids, char* ret) {
  while (*ids != '\0') {
    // function does not return enought vals
    if (*ret == '\0') {
      error_set(EXITSTATUS_ERROR_SEMANTIC_FUN_PARAMETERS);
      return false;
    }

    if (*ids != *ret) {
      if (*ids != 'n' || *ret != 'i') {
        error_set(EXITSTATUS_ERROR_SEMANTIC_ASSIGNMENT);
        return false;
      }
    }

    ids++;
    ret++;
  }

  // func can return more vals than expected

  return true;
}

bool parser_assign_exp(const dynstr_t* id_types) {
  // is syntax correct
  bool is_correct = false;

  dynstr_t exp_types;
  dynstr_init(&exp_types);
  if (error_get()) {
    goto EXIT;
  }

  if (parser_exp_list(id_types->str, &exp_types)) {
    if (!parser_assign_exp_match(id_types->str, exp_types.str)) {
      goto FREE_EXP_TYPES;
    }

    is_correct = true;
    goto FREE_EXP_TYPES;
  }

  // if no other error
  if (!error_get()) {
    error_set(EXITSTATUS_ERROR_SYNTAX);
  }

FREE_EXP_TYPES:
  dynstr_free_buffer(&exp_types);
EXIT:
  return is_correct;
}

bool parser_exp_list(char* id_types, dynstr_t* exp_types) {
  token_t* token = token_buff(TOKEN_THIS);

  switch (token->type) {
    case TT_INTEGER:
    case TT_NUMBER:
    case TT_STRING:
    case TT_LPAR:
    case TT_K_NIL:
    case TT_SOP_LENGTH:
    case TT_ID: {
      char exp_type;
      char var_type = id_types[0];
      if (parser_exp(&exp_type)) {
        if (var_type == 'n' && exp_type == 'i') {
          codegen_cast_int_to_float1();
        }

        dynstr_append(exp_types, exp_type);
        if (error_get()) {
          return false;
        }

        return parser_exp_append(id_types, 1, exp_types);
      }
    }

    break;
    default:
      break;
  }

  error_set(EXITSTATUS_ERROR_SYNTAX);
  return false;
}

bool parser_exp_append(char* id_types, int pos, dynstr_t* exp_types) {
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
    case TT_COMMA: {
      token = token_buff(TOKEN_NEW);
      if (error_get()) {
        return false;
      }

      char exp_type;
      char var_type = id_types[pos];
      if (parser_exp(&exp_type)) {
        if (var_type == 'n' && exp_type == 'i') {
          codegen_cast_int_to_float1();
        }

        dynstr_append(exp_types, exp_type);
        if (error_get()) {
          return false;
        }

        return parser_exp_append(id_types, pos + 1, exp_types);
      }
    } break;
    default:
      break;
  }

  error_set(EXITSTATUS_ERROR_SYNTAX);
  return false;
}

bool parser_exp(char* exp_type) {
  token_t* token = token_buff(TOKEN_THIS);

  switch (token->type) {
    case TT_INTEGER:
    case TT_NUMBER:
    case TT_STRING:
    case TT_LPAR:
    case TT_K_NIL:
    case TT_SOP_LENGTH:
    case TT_ID:
      return expression_parse(exp_type);
    default:
      error_set(EXITSTATUS_ERROR_SYNTAX);
      return false;
  }
}

/**
 * @file
 * @brief Recursive parser implementation
 * @author Tomas Martykan (xmarty07)
 * @author Filip Stolfa (xstolf00)
 * @author Patrik Korytar (xkoryt04)
 *
 * FIT VUT IFJ Project:
 * Compiler of IFJ21 Language
 */

#include "syntax.h"

#include <stdbool.h>

#include "codegen.h"
#include "dynstr.h"
#include "errors.h"
#include "expressions.h"
#include "other.h"
#include "parser.h"
#include "scanner.h"
#include "scope.h"
#include "symtable.h"

// PRIVATE FUNCTION FORWARD DECLARATIONS

// GRAMMAR RULES

/**
 * Parsing function for rules with
 * non-terminal 'stlist_global' on left side.
 * @return True if correct. False otherwise.
 */
bool parser_stlist_global();

/**
 * Parsing function for rules with
 * non-terminal 'st_global' on left side.
 * @return True if correct. False otherwise.
 */
bool parser_st_global();

/**
 * Parsing function for require statement.
 * Expects TT_STRING token.
 * @return True if correct. False otherwise.
 */
bool parser_require();

/**
 * Parsing function for function definition.
 * Expects TT_ID token.
 * @return True if correct. False otherwise.
 */
bool parser_function_def();

/**
 * Parsing function for function declaration.
 * Expects TT_ID token.
 * @return True if correct. False otherwise.
 */
bool parser_function_dec();

/**
 * Parsing function for function call.
 * Expects TT_ID token.
 * @param id Name of now being parsed function.
 * @return True if correct. False otherwise.
 */
bool parser_function_call_by_id(const char* id);

/**
 * Parsing function for function call.
 * Does not expect TT_ID token, but TT_LPAR token.
 * @param func Function data in symtable of now being parsed function.
 * @return True if correct. False otherwise.
 */
bool parser_function_call(symtab_func_data_t* func);

/**
 * Parsing function for rules with
 * non-terminal 'param_list' on left side.
 * @param param_types Dynamic string where to store parameter types.
 * @return True if correct. False otherwise.
 */
bool parser_param_list(dynstr_t* param_types);

/**
 * Parsing function for rules with
 * non-terminal 'param_append' on left side.
 * @param param_types Dynamic string where to store parameter types.
 * @param param_pos Number of parameter currently being parsed,
 *  counting from zero.
 * @return True if correct. False otherwise.
 */
bool parser_param_append(dynstr_t* param_types, int param_pos);

/**
 * Parsing function for rules with
 * non-terminal 'param' on left side.
 * @param param_type Character where to store parameter type.
 * @return True if correct. False otherwise.
 */
bool parser_param(char* param_type);

/**
 * Parsing function for rules with
 * non-terminal 'type_list_param' on left side.
 * @param types Dynamic string where to store types.
 * @return True if correct. False otherwise.
 */
bool parser_type_list_param(dynstr_t* types);

/**
 * Parsing function for rules with
 * non-terminal 'type_list_return' on left side.
 * @param types Dynamic string where to store types.
 * @return True if correct. False otherwise.
 */
bool parser_type_list_return(dynstr_t* types);

/**
 * Parsing function for rules with
 * non-terminal 'type_append' on left side.
 * @param types Dynamic string where to store types.
 * @return True if correct. False otherwise.
 */
bool parser_type_append(dynstr_t* types);

/**
 * Parsing function for rules with
 * non-terminal 'type' on left side.
 * @param type Character where to store type.
 * @return True if correct. False otherwise.
 */
bool parser_type(char* type);

/**
 * Parsing function for rules with
 * non-terminal 'arg_list' on left side.
 * @param arg_types Dynamic string where to store argument types.
 * @param arg_pos Integer where to store number of argument currently
 * being parsed, counting from zero.
 * @return True if correct. False otherwise.
 */
bool parser_arg_list(dynstr_t* arg_types, int* arg_pos);

/**
 * Parsing function for rules with
 * non-terminal 'arg_append' on left side.
 * @param arg_types Dynamic string where to store argument types.
 * @param arg_pos Integer where to store number of argument currently
 * being parsed, counting from zero.
 * @return True if correct. False otherwise.
 */
bool parser_arg_append(dynstr_t* arg_types, int* arg_pos);

/**
 * Parsing function for rules with
 * non-terminal 'arg' on left side.
 * @param arg_type Character where to store argument type.
 * @param lvl If not NULL, integer where to store level of scope
 *  of currently being parsed argument.
 * @return True if correct. False otherwise.
 */
bool parser_arg(char* arg_type, int* lvl);

/**
 * Parsing function for rules with
 * non-terminal 'local_scope' on left side.
 * Nobody will read this. If so, gde body?
 * @param func_name Name of now being parsed function.
 * @param ret_types Dynamic string of return types of current function.
 * @param create_scope To create or not to create local scope in symtable?
 * @return True if correct. False otherwise.
 */
bool parser_local_scope(const char* func_name, const dynstr_t* ret_types,
                        bool create_scope);

/**
 * Parsing function for rules with
 * non-terminal 'stlist_local' on left side.
 * @param func_name Name of now being parsed function.
 * @param ret_types Dynamic string of return types of current function.
 * @return True if correct. False otherwise.
 */
bool parser_stlist_local(const char* func_name, const dynstr_t* ret_types);

/**
 * Parsing function for rules with
 * non-terminal 'st_local' on left side.
 * @param func_name Name of now being parsed function.
 * @param ret_types Dynamic string of return types of current function.
 * @return True if correct. False otherwise.
 */
bool parser_st_local(const char* func_name, const dynstr_t* ret_types);

/**
 * Parsing function for rules with
 * non-terminal 'returned' on left side.
 * @param ret_types String of return types of current function.
 * @param exp_types Dynamic string where to store
 *  expression types returned from function.
 * @return True if correct. False otherwise.
 */
bool parser_returned(const char* ret_types, dynstr_t* exp_types);

/**
 * Parsing function for rules with
 * non-terminal 'return_what' on left side.
 * @param ret_types String of return types of current function.
 * @param exp_types Dynamic string where to store
 *  expression types returned from function.
 * @return True if correct. False otherwise.
 */
bool parser_return_what(const char* ret_types, dynstr_t* exp_types);

/**
 * Parsing function for variable declaration.
 * Expects TT_ID token.
 * @param func_name Name of now being parsed function.
 * @return True if correct. False otherwise.
 */
bool parser_var_dec(const char* func_name);

/**
 * Parsing function for if statement.
 * Expects expression.
 * @param func_name Name of now being parsed function.
 * @param ret_types Dynamic string of return types of current function.
 * @return True if correct. False otherwise.
 */
bool parser_if_st(const char* func_name, const dynstr_t* ret_types);

/**
 * Parsing function for while statement.
 * Expects expression.
 * @param func_name Name of now being parsed function.
 * @param ret_types Dynamic string of return types of current function.
 * @return True if correct. False otherwise.
 */
bool parser_while_st(const char* func_name, const dynstr_t* ret_types);

/**
 * Parsing function for rules with
 * non-terminal 'init' on left side.
 * @param var_type Type of now being parsed variable.
 * @param did_init Boolean where to store whether was variable initialized.
 * @return True if correct. False otherwise.
 */
bool parser_init(char var_type, bool* did_init);

/**
 * Parsing function for rules with
 * non-terminal 'init_after' on left side.
 * @param var_type Type of now being parsed variable.
 * @return True if correct. False otherwise.
 */
bool parser_init_after(char var_type);

/**
 * Parsing function for expression initialization.
 * Expects expression.
 * @param var_type Type of now being parsed variable.
 * @return True if correct. False otherwise.
 */
bool parser_init_exp(char var_type);

/**
 * Parsing function for function initialization.
 * Expects TT_ID token.
 * @param var_type Type of now being parsed variable.
 * @return True if correct. False otherwise.
 */
bool parser_init_func(char var_type);

/**
 * Parsing function for rules with
 * non-terminal 'id_after' on left side.
 * @return True if correct. False otherwise.
 */
bool parser_id_after();

/**
 * Parsing function for assignment statement.
 * Expects TT_COMMA or TT_ASSIGN token.
 * @param id Name of fist variable from identifier list.
 * @return True if correct. False otherwise.
 */
bool parser_assign_st(char* id);

/**
 * Parsing function for rules with
 * non-terminal 'id_append' on left side.
 * @param id_types Dynamic string where to store
 *  identifier types in identifier list.
 * @return True if correct. False otherwise.
 */
bool parser_id_append(dynstr_t* id_types);

/**
 * Parsing function for rules with
 * non-terminal 'assign_what' on left side.
 * @param id_types Dynamic string of identifier types in identifier list.
 * @param assign_length Length of values on the right side.
 * @return True if correct. False otherwise.
 */
bool parser_assign_what(const dynstr_t* id_types, int* assign_length);

/**
 * Parsing function for assignment of expressions.
 * Expects expression list.
 * @param id_types Dynamic string of identifier types in identifier list.
 * @param assign_length Length of values on the right side.
 * @return True if correct. False otherwise.
 */
bool parser_assign_exp(const dynstr_t* id_types, int* assign_length);

/**
 * Parsing function for assignment of function.
 * Expects TT_ID token.
 * @param id_types Dynamic string of identifier types in identifier list.
 * @param assign_length Length of values on the right side.
 * @return True if correct. False otherwise.
 */
bool parser_assign_func(const dynstr_t* id_types, int* assign_length);

/**
 * Parsing function for rules with
 * non-terminal 'exp_list' on left side.
 * @param left_types String of left values types.
 * @param exp_types Dynamic string where to store expression types.
 * @return True if correct. False otherwise.
 */
bool parser_exp_list(const char* left_types, dynstr_t* exp_types);

/**
 * Parsing function for rules with
 * non-terminal 'exp_append' on left side.
 * @param left_types String of left values types.
 * @param pos Number of expression, counting from zero.
 * @param exp_types Dynamic string where to store expression types.
 * @return True if correct. False otherwise.
 */
bool parser_exp_append(const char* left_types, int pos, dynstr_t* exp_types);

/**
 * Parsing function for rules with
 * non-terminal 'exp' on left side.
 * @param exp_type Character where to store expression type.
 * @return True if correct. False otherwise.
 */
bool parser_exp(char* exp_type);

// CHECK FOR TYPE COMPATIBILITY

/**
 * Checks whether called function args match
 * types of declared parameters.
 * @param params Declared parameter types.
 * @param args Passed argument types.
 * @return True if matches. False otherwise.
 */
bool parser_func_call_match(const char* params, const char* args);

/**
 * Checks whether returned expressions match
 * declared return types.
 * @param declared Declared return types.
 * @param returned Returned expression types.
 * @return True if matches. False otherwise.
 */
bool parser_func_ret_match(const char* declared, const char* returned);

/**
 * Checks whether returned expressions match
 * type of initialized variable.
 * @param var_type Initialized variable type.
 * @param returned Returned expression types.
 * @return True if matches. False otherwise.
 */
bool parser_init_func_match(const char var_type, const char* returned);

/**
 * Checks whether identifier list matches
 * types of return values from function.
 * @param ids Identifier list types.
 * @param returned Declared return types.
 * @return True if matches. False otherwise.
 */
bool parser_assign_func_match(const char* ids, const char* returned);

/**
 * Checks whether identifier list matches
 * types of assigned expressions.
 * @param ids Identifier list types.
 * @param exps Assigned expression types.
 * @return True if matches. False otherwise.
 */
bool parser_assign_exp_match(const char* ids, const char* exps);

// FUNCTION DEFINITIONS

// GRAMMAR RULES

bool parser_start() {
  token_buff(TOKEN_NEW);
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

  char* id = NULL;
  if (token->type == TT_ID) {
    id = str_create_copy(token->attr.str);
    if (error_get()) {
      goto EXIT;
    }
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

      codegen_function_definition_begin(id);
      if (parser_param_list(&param_types)) {
        token = token_buff(TOKEN_THIS);

        if (token->type == TT_RPAR) {
          token = token_buff(TOKEN_NEW);
          if (error_get()) {
            goto POP_SUBTAB;
          }

          if (!parser_type_list_return(&ret_types)) {
            goto POP_SUBTAB;
          }

          if (declared_func) {
            if (strcmp(declared_func->param_types, param_types.str) ||
                strcmp(declared_func->return_types, ret_types.str)) {
              // function declaration and definition dont match
              error_set(EXITSTATUS_ERROR_SEMANTIC_IDENTIFIER);
              goto POP_SUBTAB;
            }
          } else {
            parser_declare_func(id, &param_types, &ret_types);
            if (error_get()) {
              goto POP_SUBTAB;
            }
          }

          codegen_function_definition_body();
          parser_define_func(id);

          if (parser_local_scope(id, &ret_types, false)) {
            token = token_buff(TOKEN_THIS);

            if (token->type == TT_K_END) {
              token = token_buff(TOKEN_NEW);
              if (error_get()) {
                goto POP_SUBTAB;
              }

              is_correct = true;
              codegen_function_definition_end(id, ret_types.len);
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

  char* id = NULL;
  if (token->type == TT_ID) {
    id = str_create_copy(token->attr.str);
    if (error_get()) {
      goto EXIT;
    }
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

bool parser_function_call_by_id(const char* id) {
  symtab_func_data_t* declared_func = symtab_find_func(symtab, id);
  if (!declared_func) {
    error_set(EXITSTATUS_ERROR_SEMANTIC_IDENTIFIER);
    return false;
  }

  id = NULL;
  token_buff(TOKEN_NEW);
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
      goto FREE_ARG_TYPES;
    }

    int arg_count = 0;
    codegen_function_call_begin(func->func_name);
    if (parser_arg_list(&arg_types, &arg_count)) {
      token = token_buff(TOKEN_THIS);

      if (token->type == TT_RPAR) {
        token = token_buff(TOKEN_NEW);
        if (error_get()) {
          goto FREE_ARG_TYPES;
        }

        if (!parser_func_call_match(func->param_types, arg_types.str)) {
          // function declaration and call dont match
          goto FREE_ARG_TYPES;
        }

        is_correct = true;
        codegen_function_call_do(func->func_name);
        goto FREE_ARG_TYPES;
      }
    }
  }

  // if no other error
  if (!error_get()) {
    error_set(EXITSTATUS_ERROR_SYNTAX);
  }

FREE_ARG_TYPES:
  dynstr_free_buffer(&arg_types);
EXIT:
  return is_correct;
}

// SCOPE
bool parser_param_list(dynstr_t* param_types) {
  token_t* token = token_buff(TOKEN_THIS);

  if (token->type == TT_RPAR) {
    return true;
  }

  // is syntax correct
  bool is_correct = false;

  char* name = NULL;

  if (token->type == TT_ID) {
    name = str_create_copy(token->attr.str);
    if (error_get()) {
      goto EXIT;
    }

    char param_type;
    if (parser_param(&param_type)) {
      dynstr_append(param_types, param_type);
      if (error_get()) {
        goto FREE_NAME;
      }

      codegen_function_definition_param(name, 0);

      if (parser_param_append(param_types, 1)) {
        is_correct = true;
        goto FREE_NAME;
      }
    }
  }

  // if no other error
  if (!error_get()) {
    error_set(EXITSTATUS_ERROR_SYNTAX);
  }

FREE_NAME:
  free(name);
EXIT:
  return is_correct;
}

// SCOPE
bool parser_param_append(dynstr_t* param_types, int param_pos) {
  token_t* token = token_buff(TOKEN_THIS);

  if (token->type == TT_RPAR) {
    return true;
  }

  // is syntax correct
  bool is_correct = false;

  char* name = NULL;

  if (token->type == TT_COMMA) {
    token = token_buff(TOKEN_NEW);
    if (error_get()) {
      goto EXIT;
    }

    if (token->type == TT_ID) {
      name = str_create_copy(token->attr.str);
      if (error_get()) {
        goto EXIT;
      }
    }

    char param_type;
    if (parser_param(&param_type)) {
      dynstr_append(param_types, param_type);
      if (error_get()) {
        goto FREE_NAME;
      }

      codegen_function_definition_param(name, param_pos);

      if (parser_param_append(param_types, param_pos + 1)) {
        is_correct = true;
        goto FREE_NAME;
      }
    }
  }

  // if no other error
  if (!error_get()) {
    error_set(EXITSTATUS_ERROR_SYNTAX);
  }

FREE_NAME:
  free(name);
EXIT:
  return is_correct;
}

bool parser_param(char* param_type) {
  token_t* token = token_buff(TOKEN_THIS);

  // is syntax correct
  bool is_correct = false;

  char* id = NULL;
  if (token->type == TT_ID) {
    id = str_create_copy(token->attr.str);
    if (error_get()) {
      goto EXIT;
    }
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
    case TT_K_STRING:
    case TT_K_NIL: {
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

  // if no other error
  if (!error_get()) {
    error_set(EXITSTATUS_ERROR_SYNTAX);
  }

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
    case TT_COLON: {
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
    } break;
    default:
      break;
  }

  // if no other error
  if (!error_get()) {
    error_set(EXITSTATUS_ERROR_SYNTAX);
  }

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
    case TT_COMMA: {
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
    } break;
    default:
      break;
  }

  // if no other error
  if (!error_get()) {
    error_set(EXITSTATUS_ERROR_SYNTAX);
  }

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
    case TT_K_NIL:
      token = token_buff(TOKEN_NEW);
      if (error_get()) {
        return false;
      }

      *type = 'x';
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
    case TT_STRING:
    case TT_K_NIL: {
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
    } break;
    case TT_RPAR:
      return true;
    default:
      break;
  }

  // if no other error
  if (!error_get()) {
    error_set(EXITSTATUS_ERROR_SYNTAX);
  }

  return false;
}

bool parser_arg_append(dynstr_t* arg_types, int* arg_pos) {
  token_t* token = token_buff(TOKEN_THIS);

  switch (token->type) {
    case TT_COMMA: {
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
    } break;
    case TT_RPAR:
      return true;
    default:
      break;
  }

  // if no other error
  if (!error_get()) {
    error_set(EXITSTATUS_ERROR_SYNTAX);
  }

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
    case TT_K_NIL:
      *arg_type = 'x';
      return true;
    default:
      error_set(EXITSTATUS_ERROR_SYNTAX);
      return false;
  }
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
          // declared and returned dont match
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

bool parser_returned(const char* ret_types, dynstr_t* exp_types) {
  token_t* token = token_buff(TOKEN_THIS);

  switch (token->type) {
    case TT_K_RETURN:
      token = token_buff(TOKEN_NEW);
      if (error_get()) {
        return false;
      }

      if (parser_return_what(ret_types, exp_types)) {
        codegen_function_return(strlen(ret_types), exp_types->len);
        return true;
      }

      break;
    case TT_K_ELSE:
    case TT_K_END:
      return true;
    default:
      break;
  }

  // if no other error
  if (!error_get()) {
    error_set(EXITSTATUS_ERROR_SYNTAX);
  }

  return false;
}

bool parser_return_what(const char* ret_types, dynstr_t* exp_types) {
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

  char* id = NULL;
  if (token->type == TT_ID) {
    id = str_create_copy(token->attr.str);
    if (error_get()) {
      goto EXIT;
    }
  }

  if (token->type == TT_ID) {
    // search current local scope for a variable of the same name
    symtab_var_data_t* declared_var =
        symtab_find_var_local(symtab, token->attr.str);
    if (declared_var) {
      error_set(EXITSTATUS_ERROR_SEMANTIC_IDENTIFIER);
      goto FREE_ID;
    }

    // function of same name as variable
    symtab_func_data_t* declared_func =
        symtab_find_func(symtab, token->attr.str);
    if (declared_func) {
      error_set(EXITSTATUS_ERROR_SEMANTIC_IDENTIFIER);
      goto FREE_ID;
    }

    // current function of same name as variable
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

      char var_type = 0;
      if (parser_type(&var_type)) {
        codegen_define_var(id, 0);
        bool did_init = false;
        if (parser_init(var_type, &did_init)) {
          parser_declare_var(id, var_type);
          if (did_init) {
            // TODO check
            codegen_assign_expression_add(id, 0);
            codegen_assign_expression_finish(1);
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

  return false;
}

bool parser_while_st(const char* func_name, const dynstr_t* ret_types) {
  token_t* token = token_buff(TOKEN_THIS);

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
        return false;
      }

      codegen_while_expr();
      scope_new_while();

      if (parser_local_scope(func_name, ret_types, true)) {
        token = token_buff(TOKEN_THIS);

        if (token->type == TT_K_END) {
          token = token_buff(TOKEN_NEW);
          if (error_get()) {
            return false;
          }

          codegen_while_end();
          scope_pop_item();

          return true;
        }
      }
    }
  }

  // if no other error
  if (!error_get()) {
    error_set(EXITSTATUS_ERROR_SYNTAX);
  }

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

bool parser_init_after(char var_type) {
  token_t* token = token_buff(TOKEN_THIS);

  switch (token->type) {
    case TT_INTEGER:
    case TT_NUMBER:
    case TT_STRING:
    case TT_LPAR:
    case TT_K_NIL:
    case TT_SOP_LENGTH:
      return parser_init_exp(var_type);
    case TT_ID:
      // can be function call or
      // expression starting with id
      if (parser_isdeclared_var(token->attr.str)) {
        return parser_init_exp(var_type);
      } else if (parser_isdeclared_func(token->attr.str)) {
        return parser_init_func(var_type);
      } else {
        error_set(EXITSTATUS_ERROR_SEMANTIC_IDENTIFIER);
        return false;
      }
    default:
      error_set(EXITSTATUS_ERROR_SYNTAX);
      return false;
  }
}

bool parser_init_exp(char var_type) {
  char exp_type;
  if (parser_exp(&exp_type)) {
    if (var_type != exp_type) {
      if (var_type == 'n' && exp_type == 'i') {
        codegen_cast_int_to_float1();
      } else if (exp_type != 'x') {
        error_set(EXITSTATUS_ERROR_SEMANTIC_ASSIGNMENT);
        return false;
      }
    }

    return true;
  }

  // if no other error
  if (!error_get()) {
    error_set(EXITSTATUS_ERROR_SYNTAX);
  }

  return false;
}

bool parser_init_func(char var_type) {
  token_t* token = token_buff(TOKEN_THIS);

  symtab_func_data_t* declared = symtab_find_func(symtab, token->attr.str);

  if (parser_function_call_by_id(token->attr.str)) {
    if (!parser_init_func_match(var_type, declared->return_types)) {
      // variable and returned values dont match
      return false;
    }

    return true;
  }

  // if no other error
  if (!error_get()) {
    error_set(EXITSTATUS_ERROR_SYNTAX);
  }

  return false;
}

bool parser_id_after() {
  // token should be ID
  token_t* token = token_buff(TOKEN_THIS);

  // is syntax correct
  bool is_correct = false;

  char* id = NULL;
  if (token->type == TT_ID) {
    id = str_create_copy(token->attr.str);
    if (error_get()) {
      goto EXIT;
    }
  }

  token = token_buff(TOKEN_NEW);
  if (error_get()) {
    goto FREE_ID;
  }

  if (token->type == TT_LPAR) {
    symtab_func_data_t* declared_func = symtab_find_func(symtab, id);
    if (!declared_func) {
      error_set(EXITSTATUS_ERROR_SEMANTIC_IDENTIFIER);
      goto FREE_ID;
    }

    if (parser_function_call(declared_func)) {
      is_correct = true;
      goto FREE_ID;
    }
  }

  if (token->type == TT_COMMA || token->type == TT_ASSIGN) {
    if (parser_assign_st(id)) {
      is_correct = true;
      goto FREE_ID;
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

      int assign_length = 0;
      if (parser_assign_what(&id_types, &assign_length)) {
        codegen_assign_expression_finish(assign_length);

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

  if (token->type == TT_ASSIGN) {
    return true;
  }

  if (token->type == TT_COMMA) {
    token = token_buff(TOKEN_NEW);
    if (error_get()) {
      return false;
    }

    if (token->type == TT_ID) {
      int lvl = 0;
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
  }

  // if no other error
  if (!error_get()) {
    error_set(EXITSTATUS_ERROR_SYNTAX);
  }

  return false;
}

bool parser_assign_what(const dynstr_t* id_types, int* assign_length) {
  token_t* token = token_buff(TOKEN_THIS);

  switch (token->type) {
    case TT_INTEGER:
    case TT_NUMBER:
    case TT_STRING:
    case TT_LPAR:
    case TT_K_NIL:
    case TT_SOP_LENGTH:
      return parser_assign_exp(id_types, assign_length);
    case TT_ID:
      // can be function call or
      // expression starting with id
      if (parser_isdeclared_var(token->attr.str)) {
        return parser_assign_exp(id_types, assign_length);
      } else if (parser_isdeclared_func(token->attr.str)) {
        return parser_assign_func(id_types, assign_length);
      } else {
        error_set(EXITSTATUS_ERROR_SEMANTIC_IDENTIFIER);
        return false;
      }
    default:
      error_set(EXITSTATUS_ERROR_SYNTAX);
      return false;
  }
}

bool parser_assign_exp(const dynstr_t* id_types, int* assign_length) {
  // is syntax correct
  bool is_correct = false;

  dynstr_t exp_types;
  dynstr_init(&exp_types);
  if (error_get()) {
    goto EXIT;
  }

  if (parser_exp_list(id_types->str, &exp_types)) {
    if (!parser_assign_exp_match(id_types->str, exp_types.str)) {
      // identifiers and assigned expressions dont match
      goto FREE_EXP_TYPES;
    }

    is_correct = true;
    *assign_length = exp_types.len;
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

bool parser_assign_func(const dynstr_t* id_types, int* assign_length) {
  token_t* token = token_buff(TOKEN_THIS);

  symtab_func_data_t* declared = symtab_find_func(symtab, token->attr.str);

  if (parser_function_call_by_id(token->attr.str)) {
    if (!parser_assign_func_match(id_types->str, declared->return_types)) {
      // identifiers and returned values dont match
      return false;
    }
    *assign_length = strlen(declared->return_types);

    return true;
  }

  // if no other error
  if (!error_get()) {
    error_set(EXITSTATUS_ERROR_SYNTAX);
  }

  return false;
}

bool parser_exp_list(const char* left_types, dynstr_t* exp_types) {
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
      char var_type = left_types[0];
      if (parser_exp(&exp_type)) {
        if (var_type == 'n' && exp_type == 'i') {
          codegen_cast_int_to_float1();
        }

        dynstr_append(exp_types, exp_type);
        if (error_get()) {
          return false;
        }

        return parser_exp_append(left_types, 1, exp_types);
      }
    } break;
    default:
      break;
  }

  // if no other error
  if (!error_get()) {
    error_set(EXITSTATUS_ERROR_SYNTAX);
  }

  return false;
}

bool parser_exp_append(const char* left_types, int pos, dynstr_t* exp_types) {
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
      char var_type = left_types[pos];
      if (parser_exp(&exp_type)) {
        if (var_type == 'n' && exp_type == 'i') {
          codegen_cast_int_to_float1();
        }

        dynstr_append(exp_types, exp_type);
        if (error_get()) {
          return false;
        }

        return parser_exp_append(left_types, pos + 1, exp_types);
      }
    } break;
    default:
      break;
  }

  // if no other error
  if (!error_get()) {
    error_set(EXITSTATUS_ERROR_SYNTAX);
  }

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

// CHECK FOR TYPE COMPATIBILITY

bool parser_func_call_match(const char* params, const char* args) {
  while (*params != '\0') {
    // more params than args
    if (*args == '\0') {
      if (*(++params) == '+') {
        return true;
      }
      (--params);

      error_set(EXITSTATUS_ERROR_SEMANTIC_FUN_PARAMETERS);
      return false;
    }

    if (*params != *args) {
      if (*params != 'a' && *args != 'x' && (*params != 'n' || *args != 'i')) {
        error_set(EXITSTATUS_ERROR_SEMANTIC_FUN_PARAMETERS);
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
    error_set(EXITSTATUS_ERROR_SEMANTIC_FUN_PARAMETERS);
    return false;
  }

  return true;
}

bool parser_func_ret_match(const char* declared, const char* returned) {
  while (*declared != '\0') {
    // more declared return values than returned
    // NILs are returned
    if (*returned == '\0') {
      return true;
    }

    if (*declared != *returned) {
      if (*returned != 'x' && (*declared != 'n' || *returned != 'i')) {
        error_set(EXITSTATUS_ERROR_SEMANTIC_FUN_PARAMETERS);
        return false;
      }
    }

    declared++;
    returned++;
  }

  // more returned values than declared
  if (*returned != '\0') {
    error_set(EXITSTATUS_ERROR_SEMANTIC_FUN_PARAMETERS);
    return false;
  }

  return true;
}

bool parser_init_func_match(char var_type, const char* returned) {
  // function does not return at least one character
  if (*returned == '\0') {
    error_set(EXITSTATUS_ERROR_SEMANTIC_FUN_PARAMETERS);
    return false;
  }

  if (var_type != *returned) {
    if (*returned != 'x' && (var_type != 'n' || *returned != 'i')) {
      error_set(EXITSTATUS_ERROR_SEMANTIC_FUN_PARAMETERS);
      return false;
    }
  }

  // func can return more vals than expected

  return true;
}

bool parser_assign_func_match(const char* ids, const char* returned) {
  while (*ids != '\0') {
    // function does not return enought vals
    if (*returned == '\0') {
      error_set(EXITSTATUS_ERROR_SEMANTIC_FUN_PARAMETERS);
      return false;
    }

    if (*ids != *returned) {
      if (*returned != 'x' && (*ids != 'n' || *returned != 'i')) {
        error_set(EXITSTATUS_ERROR_SEMANTIC_FUN_PARAMETERS);
        return false;
      }
    }

    ids++;
    returned++;
  }

  // func can return more vals than expected

  return true;
}

bool parser_assign_exp_match(const char* ids, const char* exps) {
  while (*ids != '\0') {
    // more ids than exps
    if (*exps == '\0') {
      error_set(EXITSTATUS_ERROR_SEMANTIC_FUN_PARAMETERS);
      return false;
    }

    if (*ids != *exps) {
      if (*exps != 'x' && (*ids != 'n' || *exps != 'i')) {
        error_set(EXITSTATUS_ERROR_SEMANTIC_ASSIGNMENT);
        return false;
      }
    }

    ids++;
    exps++;
  }

  // more exps than ids
  /*if (*exps != '\0') {
    error_set(EXITSTATUS_ERROR_SEMANTIC_FUN_PARAMETERS);
    return false;
  }*/

  return true;
}

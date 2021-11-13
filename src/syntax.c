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

bool parser_stlist_global();
bool parser_st_global();
bool parser_function_def();
bool parser_function_call();
bool parser_function_dec();
bool parser_param_list();
bool parser_param_append();
bool parser_param();
bool parser_type_list();
bool parser_type_append();
bool parser_type();
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
  }

  return false;
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
  }

  return false;
}

bool parser_function_def() {
  token_t* token = token_buff(TOKEN_THIS);

  if(token->type == TT_ID) {
    token = token_buff(TOKEN_NEW);

    if(token->type == TT_LPAR) {
      token = token_buff(TOKEN_NEW);

      if(parser_param_list()) {
        token = token_buff(TOKEN_THIS);

        if(token->type == TT_RPAR) {
          token = token_buff(TOKEN_NEW);

          if(parser_type_list() && parser_stlist_local()) {
            token = token_buff(TOKEN_THIS);
            token_buff(TOKEN_NEW);

            return token->type == TT_K_END;
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
    token = token_buff(TOKEN_NEW);

    if(token->type == TT_COLON) {
      token = token_buff(TOKEN_NEW);

      if(token->type == TT_K_FUNCTION) {
        token = token_buff(TOKEN_NEW);

        if(token->type == TT_LPAR) {
          token = token_buff(TOKEN_NEW);

          if(parser_param_list()) {
            token = token_buff(TOKEN_THIS);

            if(token->type == TT_RPAR) {
              token_buff(TOKEN_NEW);

              return parser_type_list();
            }
          }
        }
      }
    }
  }

  return false;
}

bool parser_param_list() {
  token_t* token = token_buff(TOKEN_THIS);

  switch(token->type) {
    case TT_ID:
      return parser_param() && parser_param_append();
    case TT_RPAR:
      return true;
  }

  return false;
}

bool parser_param_append() {
  token_t* token = token_buff(TOKEN_THIS);

  switch(token->type) {
    case TT_COMMA:
      token_buff(TOKEN_NEW);

      return parser_param() && parser_param_append();
    case TT_RPAR:
      return true;
  }

  return false;
}

bool parser_param() {
  token_t* token = token_buff(TOKEN_THIS);

  if(token->type == TT_ID) {
    token = token_buff(TOKEN_NEW);

    if(token->type = TT_COLON) {
      token = token_buff(TOKEN_NEW);

      return parser_type();
    }
  }

  return false;
}

bool parser_type_list() {
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

      return parser_type() && parser_type_append();
  }

  return false;
}

bool parser_type_append() {
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

      return parser_type() && parser_type_append();
  }

  return false;
}

bool parser_type() {
  token_t* token = token_buff(TOKEN_THIS);

  switch(token->type) {
    case TT_K_INTEGER:
    case TT_K_NUMBER:
    case TT_K_STRING:
      token_buff(TOKEN_NEW);

      return true;
  }

  return false;
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
  }

  return false;
}

bool parser_arg_append() {
  token_t* token = token_buff(TOKEN_THIS);

  switch(token->type) {
    case TT_COMMA:
      token_buff(TOKEN_NEW);
      return parser_arg() && parser_arg_append();
    case TT_RPAR:
      return true;
  }

  return false;
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
  }

  return false;
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
  }

  return false;
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
  }

  return false;
}

bool parser_var_dec() {
  token_t* token = token_buff(TOKEN_THIS);

  if(token->type == TT_ID) {
    token = token_buff(TOKEN_NEW);

    if(token->type == TT_COLON) {
      token = token_buff(TOKEN_NEW);

      return parser_type() && parser_init();
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
  }

  return false;
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
  }

  return false;
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
  }

  return false;
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
  }

  return false;
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
  }

  return false;
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
  }

  return false;
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
  }

  return false;
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
  }

  return false;
}

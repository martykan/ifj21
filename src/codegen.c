/**
 * @file
 * @brief IFJCode21 ouput generation
 * @author Tomas Martykan
 * @author Patrik Korytar
 * @author Filip Stolfa
 */

#include "codegen.h"

#include <stdio.h>
#include <string.h>

#include "errors.h"
#include "scanner.h"
#include "scope.h"

// Variables to keep track of temp vars
int tmpmax = 0;

// Variables to generate unique IDs for labels, while supporting nesting
int idmax = -1;
int iddepth = -1;
int idstack[100];

char* last_function_name;

dynstr_t main_buffer;
dynstr_t function_buffer;
dynstr_t expression_assign_buffer;

dynstr_t* active_buffer;

void codegen_init() {
  dynstr_init(&main_buffer);
  dynstr_init(&function_buffer);
  dynstr_init(&expression_assign_buffer);

  active_buffer = &main_buffer;

  printf(".IFJcode21\n");
}

void codegen_free() {
  printf("%s\n", main_buffer.str);
  dynstr_free_buffer(&main_buffer);
  dynstr_free_buffer(&function_buffer);
  dynstr_free_buffer(&expression_assign_buffer);
}

void codegen_get_temp_vars(int count) {
  for (int i = tmpmax; i < count; i++) {
    dynstr_append_str(active_buffer, "DEFVAR LF@$tmp");
    dynstr_append_int(active_buffer, i + 1);
    dynstr_append_str(active_buffer, "\n");
    tmpmax++;
  }
}

void codegen_function_call_begin(char* name) {
  last_function_name = name;
  if (strcmp(last_function_name, "write") == 0) return;
  if (strcmp(last_function_name, "readi") == 0) return;
  if (strcmp(last_function_name, "readn") == 0) return;
  if (strcmp(last_function_name, "reads") == 0) return;
  if (strcmp(last_function_name, "tointeger") == 0) return;
  if (strcmp(last_function_name, "substr") == 0) return;
  if (strcmp(last_function_name, "ord") == 0) return;
  if (strcmp(last_function_name, "chr") == 0) return;

  dynstr_append_str(active_buffer, "CREATEFRAME\n");
}

void codegen_literal(token_t* token, int lvl) {
  switch (token->type) {
    case TT_INTEGER:
      dynstr_append_str(active_buffer, "int@");
      dynstr_append_int(active_buffer, token->attr.int_val);
      dynstr_append_str(active_buffer, "\n");
      break;
    case TT_NUMBER:
      dynstr_append_str(active_buffer, "float@");
      dynstr_append_double(active_buffer, token->attr.num_val);
      dynstr_append_str(active_buffer, "\n");
      break;
    case TT_STRING:
      dynstr_append_str(active_buffer, "string@");
      dynstr_append_str(active_buffer, token->attr.str);
      dynstr_append_str(active_buffer, "\n");
      break;
    case TT_K_NIL:
      dynstr_append_str(active_buffer, "nil@nil\n");
      break;
    case TT_ID:
      dynstr_append_str(active_buffer, "LF@");
      dynstr_append_str(active_buffer,
                        scope_get_correct_id(token->attr.str, lvl));
      dynstr_append_str(active_buffer, "\n");
      break;
    default:
      // Error
      fprintf(stderr, "token error %d\n", token->type);
      error_set(EXITSTATUS_ERROR_SEMANTIC_FUN_PARAMETERS);
      return;
  }
}

int writeskip = 0;

void codegen_function_call_argument(token_t* token, int argpos, int lvl) {
  if (last_function_name == NULL) {
    return;
  }
  if (strcmp(last_function_name, "write") == 0) {
    dynstr_append_str(active_buffer, "JUMPIFEQ $write_nil");
    dynstr_append_int(active_buffer, writeskip);
    dynstr_append_str(active_buffer, " nil@nil ");
    codegen_literal(token, lvl);

    dynstr_append_str(active_buffer, "WRITE ");
    codegen_literal(token, lvl);

    dynstr_append_str(active_buffer, "JUMP $write_end");
    dynstr_append_int(active_buffer, writeskip);
    dynstr_append_str(active_buffer, "\n");

    dynstr_append_str(active_buffer, "LABEL $write_nil");
    dynstr_append_int(active_buffer, writeskip);
    dynstr_append_str(active_buffer, "\n");
    dynstr_append_str(active_buffer, "WRITE string@nil\n");
    dynstr_append_str(active_buffer, "LABEL $write_end");
    dynstr_append_int(active_buffer, writeskip);
    dynstr_append_str(active_buffer, "\n");

    writeskip++;
    return;
  }
  if (strcmp(last_function_name, "readi") == 0) return;
  if (strcmp(last_function_name, "readn") == 0) return;
  if (strcmp(last_function_name, "reads") == 0) return;
  if (strcmp(last_function_name, "tointeger") == 0) {
    if (argpos == 0) {
      codegen_tointeger_define();

      dynstr_append_str(active_buffer, "CREATEFRAME\n");
      dynstr_append_str(active_buffer, "DEFVAR TF@n\n");
      dynstr_append_str(active_buffer, "MOVE TF@n ");
      codegen_literal(token, lvl);
    }
    return;
  }
  if (strcmp(last_function_name, "substr") == 0) {
    if (argpos == 0) {
      codegen_substr_define();

      dynstr_append_str(active_buffer, "CREATEFRAME\n");
      dynstr_append_str(active_buffer, "DEFVAR TF@str\n");
      dynstr_append_str(active_buffer, "MOVE TF@str ");
      codegen_literal(token, lvl);
    }
    if (argpos == 1) {
      dynstr_append_str(active_buffer, "DEFVAR TF@i\n");
      dynstr_append_str(active_buffer, "MOVE TF@i ");
      codegen_literal(token, lvl);
    }
    if (argpos == 2) {
      dynstr_append_str(active_buffer, "DEFVAR TF@j\n");
      dynstr_append_str(active_buffer, "MOVE TF@j ");
      codegen_literal(token, lvl);
    }
    return;
  }
  if (strcmp(last_function_name, "ord") == 0) {
    if (argpos == 0) {
      codegen_ord_define();

      dynstr_append_str(active_buffer, "CREATEFRAME\n");
      dynstr_append_str(active_buffer, "DEFVAR TF@str\n");
      dynstr_append_str(active_buffer, "MOVE TF@str ");
      codegen_literal(token, lvl);
    } else {
      dynstr_append_str(active_buffer, "DEFVAR TF@i\n");
      dynstr_append_str(active_buffer, "MOVE TF@i ");
      codegen_literal(token, lvl);
    }
    return;
  }
  if (strcmp(last_function_name, "chr") == 0) {
    if (argpos == 0) {
      codegen_chr_define();

      dynstr_append_str(active_buffer, "CREATEFRAME\n");
      dynstr_append_str(active_buffer, "DEFVAR TF@i\n");
      dynstr_append_str(active_buffer, "MOVE TF@i ");
      codegen_literal(token, lvl);
    }
    return;
  }

  dynstr_append_str(active_buffer, "DEFVAR TF@$arg");
  dynstr_append_int(active_buffer, argpos);
  dynstr_append_str(active_buffer, "\n");
  dynstr_append_str(active_buffer, "MOVE TF@$arg");
  dynstr_append_int(active_buffer, argpos);
  dynstr_append_str(active_buffer, " ");
  codegen_literal(token, lvl);
}

void codegen_function_call_do(char* name) {
  last_function_name = NULL;
  if (strcmp(name, "write") == 0) {
    return;
  }
  if (strcmp(name, "reads") == 0) {
    codegen_get_temp_vars(1);
    dynstr_append_str(active_buffer, "READ LF@$tmp1 string\n");
    dynstr_append_str(active_buffer, "PUSHS LF@$tmp1\n");
    return;
  }
  if (strcmp(name, "readn") == 0) {
    codegen_get_temp_vars(1);
    dynstr_append_str(active_buffer, "READ LF@$tmp1 float\n");
    dynstr_append_str(active_buffer, "PUSHS LF@$tmp1\n");
    return;
  }
  if (strcmp(name, "readi") == 0) {
    codegen_get_temp_vars(1);
    dynstr_append_str(active_buffer, "READ LF@$tmp1 int\n");
    dynstr_append_str(active_buffer, "PUSHS LF@$tmp1\n");
    return;
  }
  if (strcmp(name, "tointeger") == 0) {
    dynstr_append_str(active_buffer, "CALL $tointeger\n");
    return;
  }
  if (strcmp(name, "substr") == 0) {
    dynstr_append_str(active_buffer, "CALL $substr\n");
    return;
  }
  if (strcmp(name, "ord") == 0) {
    dynstr_append_str(active_buffer, "CALL $ord\n");
    return;
  };
  if (strcmp(name, "chr") == 0) {
    dynstr_append_str(active_buffer, "CALL $chr\n");
    return;
  }
  dynstr_append_str(active_buffer, "CALL $fn_");
  dynstr_append_str(active_buffer, name);
  dynstr_append_str(active_buffer, "\n");
}

void codegen_function_definition_begin(char* name) {
  dynstr_append_str(active_buffer, "JUMP $endfn_");
  dynstr_append_str(active_buffer, name);
  dynstr_append_str(active_buffer, "\n");
  dynstr_append_str(active_buffer, "LABEL $fn_");
  dynstr_append_str(active_buffer, name);
  dynstr_append_str(active_buffer, "\n");
  dynstr_append_str(active_buffer, "PUSHFRAME\n");
}

void codegen_function_definition_body() { active_buffer = &function_buffer; }

void codegen_function_definition_param(char* name, int argpos) {
  dynstr_append_str(active_buffer, "DEFVAR LF@");
  dynstr_append_str(active_buffer, name);
  dynstr_append_str(active_buffer, "\n");
  dynstr_append_str(active_buffer, "MOVE LF@");
  dynstr_append_str(active_buffer, name);
  dynstr_append_str(active_buffer, " LF@$arg");
  dynstr_append_int(active_buffer, argpos);
  dynstr_append_str(active_buffer, "\n");
}

void codegen_function_definition_end(char* name, int ret_count) {
  for (int i = 0; i < ret_count; i++) {
    dynstr_append_str(active_buffer, "PUSHS nil@nil\n");
  }
  dynstr_append_str(active_buffer, "POPFRAME\n");
  dynstr_append_str(active_buffer, "RETURN\n");
  dynstr_append_str(active_buffer, "LABEL $endfn_");
  dynstr_append_str(active_buffer, name);
  dynstr_append_str(active_buffer, "\n\n");

  // Switch buffers back
  dynstr_append_str(&main_buffer, function_buffer.str);
  dynstr_clear(&function_buffer);
  active_buffer = &main_buffer;

  tmpmax = 0;
}

void codegen_function_return(int ret_count, int exp_count) {
  for (int i = 0; i < ret_count - exp_count; i++) {
    dynstr_append_str(active_buffer, "PUSHS nil@nil\n");
  }
  dynstr_append_str(active_buffer, "POPFRAME\n");
  dynstr_append_str(active_buffer, "RETURN\n");
}

void codegen_expression_push_value(token_t* token, int lvl) {
  dynstr_append_str(active_buffer, "PUSHS ");
  codegen_literal(token, lvl);
}

void codegen_expression_plus() { dynstr_append_str(active_buffer, "ADDS\n"); }
void codegen_expression_minus() { dynstr_append_str(active_buffer, "SUBS\n"); }
void codegen_expression_mul() { dynstr_append_str(active_buffer, "MULS\n"); }
void codegen_expression_div() { dynstr_append_str(active_buffer, "DIVS\n"); }
void codegen_expression_divint() {
  dynstr_append_str(active_buffer, "IDIVS\n");
}
void codegen_expression_eq() { dynstr_append_str(active_buffer, "EQS\n"); }
void codegen_expression_neq() {
  dynstr_append_str(active_buffer, "EQS\nNOTS\n");
}
void codegen_expression_lt() { dynstr_append_str(active_buffer, "LTS\n"); }
void codegen_expression_gt() { dynstr_append_str(active_buffer, "GTS\n"); }

void codegen_expression_concat() {
  codegen_get_temp_vars(3);
  dynstr_append_str(active_buffer, "POPS LF@$tmp1\n");
  dynstr_append_str(active_buffer, "POPS LF@$tmp2\n");
  dynstr_append_str(active_buffer, "CONCAT LF@$tmp3 LF@$tmp2 LF@$tmp1\n");
  dynstr_append_str(active_buffer, "PUSHS LF@$tmp3\n");
}
void codegen_expression_strlen() {
  codegen_get_temp_vars(2);
  dynstr_append_str(active_buffer, "POPS LF@$tmp1\n");
  dynstr_append_str(active_buffer, "STRLEN LF@$tmp2 LF@$tmp1\n");
  dynstr_append_str(active_buffer, "PUSHS LF@$tmp2\n");
}
void codegen_expression_lte() {
  codegen_get_temp_vars(3);
  dynstr_append_str(active_buffer, "POPS LF@$tmp1\n");
  dynstr_append_str(active_buffer, "POPS LF@$tmp2\n");
  dynstr_append_str(active_buffer, "EQ LF@$tmp3 LF@$tmp2 LF@$tmp1\n");
  dynstr_append_str(active_buffer, "PUSHS LF@$tmp3\n");
  dynstr_append_str(active_buffer, "LT LF@$tmp3 LF@$tmp2 LF@$tmp1\n");
  dynstr_append_str(active_buffer, "PUSHS LF@$tmp3\n");
  dynstr_append_str(active_buffer, "ORS\n");
}
void codegen_expression_gte() {
  codegen_get_temp_vars(3);
  dynstr_append_str(active_buffer, "POPS LF@$tmp1\n");
  dynstr_append_str(active_buffer, "POPS LF@$tmp2\n");
  dynstr_append_str(active_buffer, "EQ LF@$tmp3 LF@$tmp2 LF@$tmp1\n");
  dynstr_append_str(active_buffer, "PUSHS LF@$tmp3\n");
  dynstr_append_str(active_buffer, "GT LF@$tmp3 LF@$tmp2 LF@$tmp1\n");
  dynstr_append_str(active_buffer, "PUSHS LF@$tmp3\n");
  dynstr_append_str(active_buffer, "ORS\n");
}

void codegen_cast_int_to_float1() {
  dynstr_append_str(active_buffer, "INT2FLOATS\n");
}

void codegen_cast_int_to_float2() {
  codegen_get_temp_vars(1);
  dynstr_append_str(active_buffer, "POPS LF@$tmp1\n");
  dynstr_append_str(active_buffer, "INT2FLOATS\n");
  dynstr_append_str(active_buffer, "PUSHS LF@$tmp1\n");
}

void codegen_cast_float_to_int1() {
  dynstr_append_str(active_buffer, "FLOAT2INTS\n");
}

void codegen_cast_float_to_int2() {
  codegen_get_temp_vars(1);
  dynstr_append_str(active_buffer, "POPS LF@$tmp1\n");
  dynstr_append_str(active_buffer, "FLOAT2INTS\n");
  dynstr_append_str(active_buffer, "PUSHS LF@$tmp1\n");
}

void codegen_not_nil() {
  dynstr_append_str(active_buffer, "PUSHS nil@nil\n");
  dynstr_append_str(active_buffer, "EQS\nNOTS\n");
}

void codegen_define_var(char* old_id, int lvl) {
  char* id = scope_get_correct_id(old_id, lvl);

  dynstr_append_str(&main_buffer, "DEFVAR LF@");
  dynstr_append_str(&main_buffer, id);
  dynstr_append_str(&main_buffer, "\n");
  dynstr_append_str(&main_buffer, "MOVE LF@");
  dynstr_append_str(&main_buffer, id);
  dynstr_append_str(&main_buffer, " nil@nil\n");
}

void codegen_assign_expression_add(char* old_id, int lvl) {
  char* id = scope_get_correct_id(old_id, lvl);
  dynstr_prepend_str(&expression_assign_buffer, "\n");
  dynstr_prepend_str(&expression_assign_buffer, id);
  dynstr_prepend_str(&expression_assign_buffer, "POPS LF@");
}
void codegen_assign_expression_finish() {
  dynstr_append_str(active_buffer, expression_assign_buffer.str);
  dynstr_clear(&expression_assign_buffer);
}

void codegen_if_begin() {
  iddepth++;
  idmax++;
  idstack[iddepth] = idmax;
  dynstr_append_str(active_buffer, "# if_");
  dynstr_append_int(active_buffer, idmax);
  dynstr_append_str(active_buffer, "\n");
  codegen_get_temp_vars(1);
  dynstr_append_str(active_buffer, "POPS LF@$tmp1\n");
  dynstr_append_str(active_buffer, "JUMPIFEQ $else_");
  dynstr_append_int(active_buffer, idmax);
  dynstr_append_str(active_buffer, " LF@$tmp1 bool@false\n");
}

void codegen_if_else() {
  int id = idstack[iddepth];
  dynstr_append_str(active_buffer, "JUMP $end_");
  dynstr_append_int(active_buffer, id);
  dynstr_append_str(active_buffer, "\n");
  dynstr_append_str(active_buffer, "LABEL $else_");
  dynstr_append_int(active_buffer, id);
  dynstr_append_str(active_buffer, "\n");
}

void codegen_if_end() {
  int id = idstack[iddepth];
  dynstr_append_str(active_buffer, "LABEL $end_");
  dynstr_append_int(active_buffer, id);
  dynstr_append_str(active_buffer, "\n");
  iddepth--;
}

void codegen_while_begin() {
  iddepth++;
  idmax++;
  idstack[iddepth] = idmax;
  codegen_get_temp_vars(4);
  dynstr_append_str(active_buffer, "LABEL $while_");
  dynstr_append_int(active_buffer, idmax);
  dynstr_append_str(active_buffer, "\n");
}

void codegen_while_expr() {
  int id = idstack[iddepth];
  dynstr_append_str(active_buffer, "POPS LF@$tmp1\n");
  dynstr_append_str(active_buffer, "JUMPIFEQ $while_end_");
  dynstr_append_int(active_buffer, id);
  dynstr_append_str(active_buffer, " LF@$tmp1 bool@false\n");
}

void codegen_while_end() {
  int id = idstack[iddepth];
  dynstr_append_str(active_buffer, "JUMP $while_");
  dynstr_append_int(active_buffer, id);
  dynstr_append_str(active_buffer, "\n");
  dynstr_append_str(active_buffer, "LABEL $while_end_");
  dynstr_append_int(active_buffer, id);
  dynstr_append_str(active_buffer, "\n");
  iddepth--;
}

bool substr_defined = false;

void codegen_substr_define() {
  if (substr_defined) return;
  substr_defined = true;

  dynstr_append_str(active_buffer, "JUMP $substr_end\n");
  dynstr_append_str(active_buffer, "LABEL $substr\n");
  dynstr_append_str(active_buffer, "PUSHFRAME\n");

  dynstr_append_str(active_buffer, "DEFVAR LF@out\n");
  dynstr_append_str(active_buffer, "MOVE LF@out string@\n");
  dynstr_append_str(active_buffer, "DEFVAR LF@newchar\n");

  dynstr_append_str(active_buffer, "DEFVAR LF@check\n");
  dynstr_append_str(active_buffer, "LT LF@check int@0 LF@i\n");
  dynstr_append_str(active_buffer,
                    "JUMPIFEQ $substr_ret LF@check bool@false\n");
  dynstr_append_str(active_buffer, "SUB LF@i LF@i int@1\n");
  dynstr_append_str(active_buffer, "LT LF@check LF@i LF@j\n");
  dynstr_append_str(active_buffer,
                    "JUMPIFEQ $substr_ret LF@check bool@false\n");
  dynstr_append_str(active_buffer, "DEFVAR LF@strlen\n");
  dynstr_append_str(active_buffer, "STRLEN LF@strlen LF@str\n");
  dynstr_append_str(active_buffer, "ADD LF@strlen LF@strlen int@1\n");
  dynstr_append_str(active_buffer, "LT LF@check LF@j LF@strlen\n");
  dynstr_append_str(active_buffer,
                    "JUMPIFEQ $substr_ret LF@check bool@false\n");

  dynstr_append_str(active_buffer, "LABEL $substr_loop\n");
  dynstr_append_str(active_buffer, "GETCHAR LF@newchar LF@str LF@i\n");
  dynstr_append_str(active_buffer, "CONCAT LF@out LF@out LF@newchar\n");
  dynstr_append_str(active_buffer, "ADD LF@i LF@i int@1\n");
  dynstr_append_str(active_buffer, "JUMPIFNEQ $substr_loop LF@i LF@j\n");

  dynstr_append_str(active_buffer, "LABEL $substr_ret\n");

  dynstr_append_str(active_buffer, "PUSHS LF@out\n");
  dynstr_append_str(active_buffer, "POPFRAME\n");
  dynstr_append_str(active_buffer, "RETURN\n");
  dynstr_append_str(active_buffer, "LABEL $substr_end\n");
}

bool ord_defined = false;

void codegen_ord_define() {
  if (ord_defined) return;
  ord_defined = true;

  dynstr_append_str(active_buffer, "JUMP $ord_end\n");
  dynstr_append_str(active_buffer, "LABEL $ord\n");
  dynstr_append_str(active_buffer, "PUSHFRAME\n");

  dynstr_append_str(active_buffer, "DEFVAR LF@out\n");
  dynstr_append_str(active_buffer, "MOVE LF@out nil@nil\n");

  dynstr_append_str(active_buffer, "DEFVAR LF@check\n");
  dynstr_append_str(active_buffer, "LT LF@check int@0 LF@i\n");
  dynstr_append_str(active_buffer, "JUMPIFEQ $ord_ret LF@check bool@false\n");
  dynstr_append_str(active_buffer, "DEFVAR LF@strlen\n");
  dynstr_append_str(active_buffer, "STRLEN LF@strlen LF@str\n");
  dynstr_append_str(active_buffer, "ADD LF@strlen LF@strlen int@1\n");
  dynstr_append_str(active_buffer, "LT LF@check LF@i LF@strlen\n");
  dynstr_append_str(active_buffer, "JUMPIFEQ $ord_ret LF@check bool@false\n");

  dynstr_append_str(active_buffer, "SUB LF@i LF@i int@1\n");
  dynstr_append_str(active_buffer, "STRI2INT LF@out LF@str LF@i\n");

  dynstr_append_str(active_buffer, "LABEL $ord_ret\n");
  dynstr_append_str(active_buffer, "PUSHS LF@out\n");

  dynstr_append_str(active_buffer, "POPFRAME\n");
  dynstr_append_str(active_buffer, "RETURN\n");
  dynstr_append_str(active_buffer, "LABEL $ord_end\n");
}

bool chr_defined = false;

void codegen_chr_define() {
  if (chr_defined) return;
  chr_defined = true;

  dynstr_append_str(active_buffer, "JUMP $chr_end\n");
  dynstr_append_str(active_buffer, "LABEL $chr\n");
  dynstr_append_str(active_buffer, "PUSHFRAME\n");

  dynstr_append_str(active_buffer, "DEFVAR LF@cond\n");
  dynstr_append_str(active_buffer, "DEFVAR LF@cond2\n");
  dynstr_append_str(active_buffer, "LT LF@cond LF@i int@0\n");
  dynstr_append_str(active_buffer, "GT LF@cond2 LF@i int@255\n");
  dynstr_append_str(active_buffer, "OR LF@cond LF@cond LF@cond2\n");

  dynstr_append_str(active_buffer, "JUMPIFEQ $chr_expr LF@cond bool@false\n");
  dynstr_append_str(active_buffer, "PUSHS nil@nil\n");
  dynstr_append_str(active_buffer, "JUMP $chr_ret\n");
  dynstr_append_str(active_buffer, "LABEL $chr_expr\n");
  dynstr_append_str(active_buffer, "PUSHS LF@i\n");
  dynstr_append_str(active_buffer, "INT2CHARS\n");
  dynstr_append_str(active_buffer, "LABEL $chr_ret\n");

  dynstr_append_str(active_buffer, "POPFRAME\n");
  dynstr_append_str(active_buffer, "RETURN\n");
  dynstr_append_str(active_buffer, "LABEL $chr_end\n");
}

bool tointeger_defined = false;

void codegen_tointeger_define() {
  if (tointeger_defined) return;
  tointeger_defined = true;

  dynstr_append_str(active_buffer, "JUMP $tointeger_end\n");
  dynstr_append_str(active_buffer, "LABEL $tointeger\n");
  dynstr_append_str(active_buffer, "PUSHFRAME\n");

  dynstr_append_str(active_buffer, "JUMPIFNEQ $tointeger_expr LF@n nil@nil\n");
  dynstr_append_str(active_buffer, "PUSHS nil@nil\n");
  dynstr_append_str(active_buffer, "JUMP $tointeger_ret\n");
  dynstr_append_str(active_buffer, "LABEL $tointeger_expr\n");
  dynstr_append_str(active_buffer, "PUSHS LF@n\n");
  dynstr_append_str(active_buffer, "FLOAT2INTS\n");
  dynstr_append_str(active_buffer, "LABEL $tointeger_ret\n");

  dynstr_append_str(active_buffer, "POPFRAME\n");
  dynstr_append_str(active_buffer, "RETURN\n");
  dynstr_append_str(active_buffer, "LABEL $tointeger_end\n");
}

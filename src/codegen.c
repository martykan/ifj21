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

// Variables to keep track of temp vars
int tmpmax = 0;
void codegen_get_temp_vars(int count) {
  for (int i = tmpmax; i < count; i++) {
    printf("DEFVAR LF@$tmp%d\n", i + 1);
    tmpmax++;
  }
}

// Variables to generate unique IDs for labels, while supporting nesting
int idmax = -1;
int iddepth = -1;
int idstack[100];

char* last_function_name;

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

  printf("CREATEFRAME\n");
}

void codegen_literal(token_t* token) {
  switch (token->type) {
    case TT_INTEGER:
      printf("int@%d\n", token->attr.int_val);
      break;
    case TT_NUMBER:
      printf("float@%a\n", token->attr.num_val);
      break;
    case TT_STRING:
      printf("string@%s\n", token->attr.str);
      break;
    case TT_ID:
      printf("LF@%s\n", token->attr.str);
      break;
    default:
      // Error
      printf("token error %d\n", token->type);
      error_set(EXITSTATUS_ERROR_SEMANTIC_FUN_PARAMETERS);
      return;
  }
}

int substr_i, substr_j;

void codegen_function_call_argument(token_t* token, int argpos,
                                    symtab_func_data_t* func) {
  if (last_function_name == NULL) {
    return;
  }
  if (strcmp(last_function_name, "write") == 0) {
    printf("WRITE ");
    codegen_literal(token);
    return;
  }
  if (strcmp(last_function_name, "readi") == 0) return;
  if (strcmp(last_function_name, "readn") == 0) return;
  if (strcmp(last_function_name, "reads") == 0) return;
  if (strcmp(last_function_name, "tointeger") == 0) {
    printf("PUSHS ");
    codegen_literal(token);
    printf("FLOAT2INTS\n");
    return;
  }
  if (strcmp(last_function_name, "substr") == 0) {
    if (argpos == 0) {
      codegen_get_temp_vars(3);
      printf("MOVE LF@$tmp1 ");
      codegen_literal(token);
    }
    if (argpos == 1) substr_i = token->attr.int_val;
    if (argpos == 2) substr_j = token->attr.int_val;
    return;
  }
  if (strcmp(last_function_name, "ord") == 0) {
    printf("PUSHS ");
    codegen_literal(token);
    return;
  }
  if (strcmp(last_function_name, "chr") == 0) {
    printf("PUSHS ");
    codegen_literal(token);
    printf("INT2CHARS\n");
    return;
  }

  char* argname = func->params->vars[argpos]->var_name;
  printf("DEFVAR TF@%s\n", argname);
  printf("MOVE TF@%s ", argname);
  codegen_literal(token);
}

void codegen_function_call_argument_count(int argcount) {
  // Currently not used anywhere
  /*if (strcmp(last_function_name, "write") == 0) {
    return;
  }
  printf("DEFVAR TF@%%argcount\n");
  printf("MOVE TF@%%argcount int@%d\n", argcount);*/
}

void codegen_function_call_do(char* name, int argcount) {
  last_function_name = NULL;
  if (strcmp(name, "write") == 0) {
    return;
  }
  if (strcmp(name, "reads") == 0) {
    codegen_get_temp_vars(1);
    printf("READ LF@$tmp1 string\n");
    printf("PUSHS LF@$tmp1\n");
    return;
  }
  if (strcmp(name, "readn") == 0) {
    codegen_get_temp_vars(1);
    printf("READ LF@$tmp1 float\n");
    printf("PUSHS LF@$tmp1\n");
    return;
  }
  if (strcmp(name, "readi") == 0) {
    codegen_get_temp_vars(1);
    printf("READ LF@$tmp1 int\n");
    printf("PUSHS LF@$tmp1\n");
    return;
  }
  if (strcmp(name, "tointeger") == 0) return;
  if (strcmp(name, "substr") == 0) {
    if (substr_i < 1 || substr_i > substr_j) {
      printf("PUSHS string@\n");
      return;
    }
    // TODO Dynamic check - j < strlen
    printf("MOVE LF@$tmp2 string@\n");
    for (int i = substr_i; i <= substr_j; i++) {
      printf("GETCHAR LF@$tmp3 LF@$tmp1 int@%d\n", i - 1);
      printf("CONCAT LF@$tmp2 LF@$tmp2 LF@$tmp3\n");
    }
    printf("PUSHS LF@$tmp2\n");
    return;
  }
  if (strcmp(name, "ord") == 0) {
    printf("STRI2INTS\n");
    return;
  };
  if (strcmp(name, "chr") == 0) return;
  printf("CALL $fn_%s\n", name);
}

void codegen_function_definition_begin(char* name) {
  printf("JUMP $endfn_%s\n", name);
  printf("LABEL $fn_%s\n", name);
  printf("PUSHFRAME\n");
}

void codegen_function_definition_end(char* name) {
  printf("POPFRAME\n");
  printf("RETURN\n");
  printf("LABEL $endfn_%s\n\n", name);
  tmpmax = 0;
}

void codegen_expression_push_value(token_t* token) {
  printf("PUSHS ");
  codegen_literal(token);
}

void codegen_expression_plus() { printf("ADDS\n"); }
void codegen_expression_minus() { printf("SUBS\n"); }
void codegen_expression_mul() { printf("MULS\n"); }
void codegen_expression_div() { printf("DIVS\n"); }
void codegen_expression_divint() { printf("IDIVS\n"); }
void codegen_expression_eq() { printf("EQS\n"); }
void codegen_expression_neq() { printf("EQS\nNOTS\n"); }
void codegen_expression_lt() { printf("LTS\n"); }
void codegen_expression_gt() { printf("GTS\n"); }

void codegen_expression_concat() {
  codegen_get_temp_vars(3);
  printf("POPS LF@$tmp1\n");
  printf("POPS LF@$tmp2\n");
  printf("CONCAT LF@$tmp3 LF@$tmp2 LF@$tmp1\n");
  printf("PUSHS LF@$tmp3\n");
}
void codegen_expression_strlen() {
  codegen_get_temp_vars(2);
  printf("POPS LF@$tmp1\n");
  printf("STRLEN LF@$tmp2 LF@$tmp1\n");
  printf("PUSHS LF@$tmp2\n");
}
void codegen_expression_lte() { printf("# E -> E<=E\n"); }
void codegen_expression_gte() { printf("# E -> E>=E\n"); }

void codegen_cast_int_to_float1() { printf("INT2FLOATS\n"); }

void codegen_cast_int_to_float2() {
  codegen_get_temp_vars(1);
  printf("POPS LF@$tmp1\n");
  printf("INT2FLOATS\n");
  printf("PUSHS LF@$tmp1\n");
}

void codegen_define_var(char* id) { printf("DEFVAR LF@%s\n", id); }

dynstr_t expression_assign_buffer;

void codegen_assign_expression_add(char* id) {
  if (expression_assign_buffer.str == NULL) {
    dynstr_init(&expression_assign_buffer);
  }
  dynstr_prepend_str(&expression_assign_buffer, "\n");
  dynstr_prepend_str(&expression_assign_buffer, id);
  dynstr_prepend_str(&expression_assign_buffer, "POPS LF@");
}
void codegen_assign_expression_finish() {
  printf("%s", expression_assign_buffer.str);
  dynstr_clear(&expression_assign_buffer);
}

void codegen_if_begin() {
  iddepth++;
  idmax++;
  idstack[iddepth] = idmax;
  printf("# if_%d\n", idmax);
  codegen_get_temp_vars(1);
  printf("POPS LF@$tmp1\n");
  printf("JUMPIFEQ $else_%d LF@$tmp1 bool@false\n", idmax);
}

void codegen_if_else() {
  int id = idstack[iddepth];
  printf("JUMP $end_%d\n", id);
  printf("LABEL $else_%d\n", id);
}

void codegen_if_end() {
  int id = idstack[iddepth];
  printf("LABEL $end_%d\n", id);
  iddepth--;
}

void codegen_while_begin() {
  iddepth++;
  idmax++;
  idstack[iddepth] = idmax;
  codegen_get_temp_vars(1);
  printf("LABEL $while_%d\n", idmax);
}

void codegen_while_expr() {
  int id = idstack[iddepth];
  printf("POPS LF@$tmp1\n");
  printf("JUMPIFEQ $while_end_%d LF@$tmp1 bool@false\n", id);
}

void codegen_while_end() {
  int id = idstack[iddepth];
  printf("JUMP $while_%d\n", id);
  printf("LABEL $while_end_%d\n", id);
  iddepth--;
}
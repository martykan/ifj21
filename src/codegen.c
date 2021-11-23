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

char* last_function_name;

void codegen_function_call_begin(char* name) {
  last_function_name = name;
  if (strcmp(last_function_name, "write") == 0) {
    return;
  }
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

void codegen_function_call_argument(token_t* token, int argpos) {
  if (last_function_name == NULL) {
    return;
  }
  if (strcmp(last_function_name, "write") == 0) {
    if (argpos > 0) {
      printf("WRITE string@\\032\n");
    }
    printf("WRITE ");
  } else {
    printf("DEFVAR TF@%%arg%d\n", argpos);
    printf("MOVE TF@%%arg%d ", argpos);
  }
  codegen_literal(token);
}

void codegen_function_call_argument_count(int argcount) {
  if (strcmp(last_function_name, "write") == 0) {
    return;
  }
  printf("DEFVAR TF@%%argcount\n");
  printf("MOVE TF@%%argcount int@%d\n", argcount);
}

void codegen_function_call_do(char* name, int argcount) {
  last_function_name = NULL;
  if (strcmp(name, "write") == 0) {
    printf("WRITE string@\\010\n");
    return;
  }
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
  printf("POPS LF@$tmp1\n");
  printf("POPS LF@$tmp2\n");
  printf("CONCAT LF@$tmp3 LF@$tmp1 LF@$tmp2\n");
  printf("PUSHS LF@$tmp3\n");
}
void codegen_expression_lte() { printf("# E -> E<=E\n"); }
void codegen_expression_gte() { printf("# E -> E>=E\n"); }

void codegen_define_var(char* id) { printf("DEFVAR LF@%s\n", id); }
void codegen_assign_expression(char* id) { printf("POPS LF@%s\n", id); }
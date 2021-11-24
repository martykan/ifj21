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

void codegen_function_call_argument(token_t* token, int argpos,
                                    symtab_func_data_t* func) {
  if (last_function_name == NULL) {
    return;
  }
  if (strcmp(last_function_name, "write") == 0) {
    if (argpos > 0) {
      printf("WRITE string@\\032\n");
    }
    printf("WRITE ");
  } else {
    char* argname = func->params->vars[argpos]->var_name;
    printf("DEFVAR TF@%s\n", argname);
    printf("MOVE TF@%s ", argname);
  }
  codegen_literal(token);
}

void codegen_function_call_argument_count(int argcount) {
  if (strcmp(last_function_name, "write") == 0) {
    return;
  }
  // printf("DEFVAR TF@%%argcount\n");
  // printf("MOVE TF@%%argcount int@%d\n", argcount);
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
  printf("DEFVAR LF@$tmp1\n");
  printf("DEFVAR LF@$tmp2\n");
  printf("DEFVAR LF@$tmp3\n");
  printf("POPS LF@$tmp1\n");
  printf("POPS LF@$tmp2\n");
  printf("CONCAT LF@$tmp3 LF@$tmp2 LF@$tmp1\n");
  printf("PUSHS LF@$tmp3\n");
}
void codegen_expression_lte() { printf("# E -> E<=E\n"); }
void codegen_expression_gte() { printf("# E -> E>=E\n"); }

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
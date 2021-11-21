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
  switch (token->type) {
    case TT_INTEGER:
      printf("int@%d\n", token->attr.int_val);
      break;
    case TT_NUMBER:
      printf("float@%a\n", token->attr.num_val);
      break;
    case TT_STRING:
      printf("string@");
      int pos = 0;
      char c = 1;
      while (c) {
        c = token->attr.str[pos++];
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
          printf("%c", c);
        else
          printf("\\%03d", c);
      }
      printf("\n");
      break;
    default:
      // Error
      printf("token error %d\n", token->type);
      error_set(EXITSTATUS_ERROR_SEMANTIC_FUN_PARAMETERS);
      return;
  }
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
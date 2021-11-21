/**
 * @file
 * @brief IFJCode21 ouput generation
 * @author Tomas Martykan
 * @author Patrik Korytar
 * @author Filip Stolfa
 */

#ifndef __CODEGEN_H
#define __CODEGEN_H

#include "parser.h"

/** Begin a function call procedure */
void codegen_function_call_begin(char* name);

/** Save function arguments to a variable on TF */
void codegen_function_call_argument(token_t* token, int argpos);

/** Save the count of function arguments to a variable on TF */
void codegen_function_call_argument_count(int argcount);

/** Execute the function call */
void codegen_function_call_do(char* name, int argcount);

/** Begin a function definition */
void codegen_function_definition_begin(char* name);

/** End a function definition */
void codegen_function_definition_end(char* name);

#endif
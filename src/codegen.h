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
void codegen_function_call_argument(token_t* token, int argpos, int lvl);

/** Save the count of function arguments to a variable on TF */
void codegen_function_call_argument_count(int argcount);

/** Execute the function call */
void codegen_function_call_do(char* name, int argcount);

/** Begin a function definition */
void codegen_function_definition_begin(char* name);

/** Parameter in a function definition */
void codegen_function_definition_param(char* name, int argpos);

/** End a function definition */
void codegen_function_definition_end(char* name);

/** Return from a function early */
void codegen_function_return();

void codegen_expression_push_value(token_t* token, int lvl);
void codegen_expression_plus();
void codegen_expression_minus();
void codegen_expression_mul();
void codegen_expression_div();
void codegen_expression_divint();
void codegen_expression_concat();
void codegen_expression_strlen();
void codegen_expression_eq();
void codegen_expression_neq();
void codegen_expression_lt();
void codegen_expression_lte();
void codegen_expression_gt();
void codegen_expression_gte();

void codegen_cast_int_to_float1();
void codegen_cast_int_to_float2();
void codegen_cast_float_to_int1();
void codegen_cast_float_to_int2();
void codegen_not_nil();

void codegen_define_var(char* old_id, int lvl);
void codegen_assign_expression_add(char* id, int lvl);
void codegen_assign_expression_finish();

void codegen_if_begin();
void codegen_if_else();
void codegen_if_end();

void codegen_while_begin();
void codegen_while_expr();
void codegen_while_end();

void codegen_ord_define();
void codegen_substr_define();

#endif
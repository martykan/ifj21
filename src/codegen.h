/**
 * @file
 * @brief IFJCode21 ouput generation API
 * @author Tomas Martykan (xmarty07)
 * @author Filip Stolfa (xstolf00)
 * @author Patrik Korytar (xkoryt04)
 *
 * FIT VUT IFJ Project:
 * Compiler of IFJ21 Language
 */

#ifndef __CODEGEN_H
#define __CODEGEN_H

#include "parser.h"

/** Init codegen */
void codegen_init();

/** Free codegen */
void codegen_free();

/** Begin a function call procedure */
void codegen_function_call_begin(char* name);

/** Save function arguments to a variable on TF */
void codegen_function_call_argument(token_t* token, int argpos, int lvl);

/** Execute the function call */
void codegen_function_call_do(char* name);

/** Begin a function definition */
void codegen_function_definition_begin(char* name);

/** Parameter in a function definition */
void codegen_function_definition_param(char* name, int argpos);

/** Body of a function definition */
void codegen_function_definition_body();

/** End a function definition */
void codegen_function_definition_end(char* name, int ret_count);

/** Return from a function */
void codegen_function_return(int ret_count, int exp_count);

void codegen_expression_push_value(token_t* token, int lvl);

/** Mathematical operations */
void codegen_expression_plus();
void codegen_expression_minus();
void codegen_expression_mul();
void codegen_expression_div();
void codegen_expression_divint();

/** String operations */
void codegen_expression_concat();
void codegen_expression_strlen();

/** Logical operations */
void codegen_expression_eq();
void codegen_expression_neq();
void codegen_expression_lt();
void codegen_expression_lte();
void codegen_expression_gt();
void codegen_expression_gte();

/** Type casts */
void codegen_cast_int_to_float1();
void codegen_cast_int_to_float2();
void codegen_cast_float_to_int1();
void codegen_cast_float_to_int2();
void codegen_not_nil();

/** Define a variable */
void codegen_define_var(char* old_id, int lvl);
/** Add a new variable that is being assigned to */
void codegen_assign_expression_add(char* id, int lvl);
/** Complete assignment */
void codegen_assign_expression_finish(int count);

/** If-then-else blocks */
void codegen_if_begin();
void codegen_if_else();
void codegen_if_end();

/** While */
void codegen_while_begin();
void codegen_while_expr();
void codegen_while_end();

/** Builtin functions */
void codegen_ord_define();
void codegen_chr_define();
void codegen_substr_define();
void codegen_tointeger_define();

#endif

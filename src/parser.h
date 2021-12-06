/**
 * @file
 * @brief Parser API
 * @author Patrik Korytar
 * @author Tomas Martykan
 * @author Filip Stolfa
 *
 * @section DESCRIPTION
 *  Parser basic functions interface. Contains functions for
 *  declaring / defining variables and functions.
 */

#ifndef __PARSER_H__
#define __PARSER_H__

#include <stdbool.h>

#include "dynstr.h"
#include "scanner.h"
#include "symtable.h"

// SYMTABLE USED BY PARSER

/// Global symtable used by parser.
extern symtab_t* symtab;

// COMPILE-TIME CONSTANTS

#define TOKEN_THIS 0 /**< Get current token from buffer. @hideinitializer */
#define TOKEN_NEW \
  1 /**< Delete old and get new token from buffer. @hideinitializer */
#define TOKEN_DELETE 2 /**< Delete token in buffer. @hideinitializer */

// PUBLIC FUNCTIONS FORWARD DECLARATIONS

// TOKEN

/**
 * Buffer for token. Based on operation, gets current/new
 * or deletes a token.
 * @param operation Operation to execute.
 *  TOKEN_THIS - get current token.
 *  TOKEN_NEW - delete old and get new token.
 *  TOKEN_DELETE - delete token.
 * @return Current token after operation.
 */
token_t* token_buff(int operation);

// ALLOCATION AND DEALLOCATION

/**
 * Initializes parser - creates symtable.
 * @return True if successful. False otherwise.
 */
bool parser_init_symtab();

/**
 * Deconstructs parser - destroys symtable.
 */
void parser_destroy_symtab();

// DECLARATIONS / DEFINITIONS

/**
 * Declares variable inside symtable of parser.
 * @param id Name of variable to declare.
 * @param data_type Data type of variable.
 * @return True if successful. False otherwise.
 */
bool parser_declare_var(const char* id, char data_type);

/**
 * Declares function inside symtable of parser.
 * @param id Name of function to declare.
 * @param param_types String of parameter data types.
 * @param return_types String of return data types.
 * @return True if successful. False otherwise.
 */
bool parser_declare_func(const char* id, const dynstr_t* param_types,
                         const dynstr_t* return_types);

/**
 * Defines variable inside symtable of parser.
 * Variable has to be already declared.
 * @param id Name of variable to Define.
 * @return True if successful. False otherwise.
 */
bool parser_define_var(const char* id);

/**
 * Defines function inside symtable of parser.
 * Functon has to be already declared.
 * @param id Name of function to Define.
 * @return True if successful. False otherwise.
 */
bool parser_define_func(const char* id);

// CHECK OF DECLARATION / DEFINITION

/**
 * Tries to find variable inside symtable of parser.
 * @param id Name of variable to look for.
 * @return True if declared. False otherwise.
 */
bool parser_isdeclared_var(const char* id);

/**
 * Tries to find function inside symtable of parser.
 * @param id Name of function to look for.
 * @return True if declared. False otherwise.
 */
bool parser_isdeclared_func(const char* id);

/**
 * Tries to find variable inside symtable of parser
 * and checks whether is defined.
 * @param id Name of variable to look for.
 * @return True if defined. False otherwise.
 */
bool parser_isdefined_var(const char* id);

/**
 * Tries to find function inside symtable of parser
 * and checks whether is defined.
 * @param id Name of function to look for.
 * @return True if defined. False otherwise.
 */
bool parser_isdefined_func(const char* id);

#endif  // __PARSER_H__

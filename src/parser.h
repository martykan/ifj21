/**
 * @file
 * @brief Parser API
 * @author Patrik Korytar
 * @author Tomas Martykan
 * @author Filip Stolfa
 *
 * @section DESCRIPTION
 *  TODO
 *
 * @section IMPLEMENTATION
 *  TODO
 */

#ifndef __PARSER_H__
#define __PARSER_H__

#include "scanner.h"
#include "dynstr.h"
#include "symtable.h"

#include <stdbool.h>

extern symtab_t* symtab;

// COMPILE-TIME CONSTANTS

#define TOKEN_THIS 0 /**< Get current token from buffer. @hideinitializer */
#define TOKEN_NEW 1 /**< Delete old and get new token from buffer. @hideinitializer */
#define TOKEN_DELETE 2 /**< Delete token in buffer. @hideinitializer */

// PUBLIC FUNCTION FORWARD DECLARATIONS

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

bool parser_init_symtab();

bool parser_declare_var(const char* id, char data_type);
bool parser_declare_func(const char* id, const dynstr_t* param_types, const dynstr_t* return_types);
bool parser_define_var(const char* id);
bool parser_define_func(const char* id);

bool parser_isdeclared_var(const char* id);
bool parser_isdeclared_func(const char* id);
bool parser_isdefined_var(const char* id);
bool parser_isdefined_func(const char* id);

#endif  // __PARSER_H__

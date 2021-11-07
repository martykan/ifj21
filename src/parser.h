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

#include <stdbool.h>

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

/**
 * Checks whether token is valid identifier.
 * @param token Token to check.
 * @return True if token is valid identifier. False otherwise.
 */
bool is_id(const token_t* token);

/**
 * Checks whether token is given keyword.
 * @param token Token to check.
 * @param token Keyword to compare with.
 * @return True if token is given keyword. False otherwise.
 */
bool is_keyword(const token_t* token, const char* keyword);

bool symtab_var_declaration(symtab_t* symtab, const token_t* token, char data_type, bool is_init);
bool symtab_func_declaration(symtab_t* symtab, const token_t* token, char* param_types, char* return_types,
      bool was_defined);
bool symtab_var_isdeclared(const symtab_t* symtab, const token_t* token);
bool symtab_func_isdeclared(const symtab_t* symtab, const token_t* token);
bool symtab_var_isdefined(const symtab_t* symtab, const token_t* token);
bool symtab_func_isdefined(const symtab_t* symtab, const token_t* token);

#endif  // __PARSER_H__

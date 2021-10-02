/**
 * @file
 * @brief Scanner API
 * @author Patrik Korytar
 * @author Tomas Martykan
 * @author Filip Stolfa
 *
 * ---> TODO -- NOT WORKING RIGHT NOW <----
 *
 * @section DESCRIPTION
 * Scanner reads input program and converts it to lexems using state machine.
 */

#include "symtable.h"

/**
 * @brief Scanner states.
 * States scanner state machine can be in.
 */
typedef enum {
  BEGIN_STATE,
  COMMENTED,
  // TODO
  ERROR_TOKEN,
  END_OF_TOKEN
} scanner_state_t;

/**
 * @brief Token types.
 * Possible types of read token.
 */
typedef enum {
  ID,
  KEYWORD,
  EQ,
  LEFT_PARANTHESIS
  //TODO
} token_type_t;

/**
 * @struct token_t
 * @brief Token containing lexeme.
 * @var token_t::type
 * Type of a read token (identifier, literal, etc.).
 * @var token_t::value
 * Text of identifier or literal (blank for operators).
 */
typedef struct
{
  token_type_t type;
  string_t* value;
} token_t;

// STATE FUNCTIONS

/**
 * Determines new state from current state and read character.
 * @param state State from to determine new one.
 * @param c Read character to use for state determination.
 * @return New state.
 */
scanner_state_t scanner_determine_state(scanner_state_t state, char c);

// TOKEN FUNCTIONS

/**
 * Gets token;
 * This function is called by synctatic analyzer;
 * @param symtab If not NULL and token is identifier, store token in symbol table;
 * @return Formed token if successful. NULL otherwise.
 */
token_t* token_get(symtab_t *symtab);

/**
 * Reads stdin and forms a token.
 * @param err Out parameter to pass errors.
 * @return Formed token if successful. NULL otherwise.
 */
token_t* token_read(int* err);

/**
 * Stores token in symbol table.
 * @param symtab Symbol table where to store.
 * @param token Token to store.
 * @return True if successful. False otherwise.
 */
bool token_store(symtab_t *symtab, token_t* token);

/**
 * Destroys token.
 * @param token Token to destroy.
 */
void token_destroy(token_t* token);

/**
 * Determines type of token from state.
 * @param state State to determine token from.
 * @return Type of token.
 */
token_type_t token_determine_type(scanner_state_t state);

/**
 * @file
 * @brief Scanner API
 * @author Tomas Martykan (xmarty07)
 * @author Filip Stolfa (xstolf00)
 * @author Patrik Korytar (xkoryt04)
 *
 * FIT VUT IFJ Project:
 * Compiler of IFJ21 Language
 */

#ifndef __SCANNER_H
#define __SCANNER_H

/// Offset from 0 of the first keyword token type
#define TOK_KEYWORD_OFFSET 3

/**
 * Enum of all possible scanner token types.
 * The starting index (defined in ::TOK_KEYWORD_OFFSET) and order of
 * the keyword token types has to match the order of the keyword strings
 * in ::keywords.
 */
typedef enum {
  TT_ERROR,     ///< A lexical error was found.
  TT_NO_TYPE,   ///< Used to initialize an empty token.
  TT_EOF, //2

  //Keyword token types, order needs to match keywords array in scanner.c
  TT_K_LOCAL,
  TT_K_INTEGER,
  TT_K_NUMBER,
  TT_K_IF,
  TT_K_THEN,
  TT_K_ELSE,
  TT_K_DO,
  TT_K_WHILE,
  TT_K_STRING,
  TT_K_END,
  TT_K_FUNCTION,
  TT_K_GLOBAL,
  TT_K_NIL,
  TT_K_RETURN,
  TT_K_REQUIRE,
  //End of keyword token types.

  TT_ID,       ///< Identifier token.
  TT_STRING,   ///< String literal token.
  TT_INTEGER,  ///< Integer literal token.
  TT_NUMBER,   ///< Number (double) literal token.

  TT_ASSIGN, //7

  // compare operators
  TT_COP_EQ, //8
  TT_COP_NEQ,
  TT_COP_GT,
  TT_COP_GE,
  TT_COP_LT,
  TT_COP_LE, //13

  // math operators
  TT_MOP_PLUS, //14
  TT_MOP_MINUS,
  TT_MOP_MUL,
  TT_MOP_DIV,
  TT_MOP_INT_DIV, //18

  // string operators
  TT_SOP_CONCAT, //19
  TT_SOP_LENGTH,

  // separators
  TT_COMMA, //21
  TT_LPAR,
  TT_RPAR,
  TT_COLON //24
} token_type_t;

/** Token attribute union type. */
typedef union {
  char *str;      ///< String value for #TT_STRING and #TT_ID.
  int int_val;    ///< Integer value for #TT_INTEGER.
  double num_val; ///< Number (double) value for #TT_NUMBER.
} attr_t;

/**
 * @struct token_t
 * @brief Scanner token type.
 * @var token_t::type
 * Type of the token.
 * @var token_t::attr
 * Union of a string, integer and double. Depending on the token type,
 * the corresponding value is stored in the attribute.
 * If a token doesn't require an attribute, it is set to a NULL char pointer.
 */
typedef struct {
  token_type_t type;
  attr_t attr;
} token_t;


/** Initializes scanner for use.
 * Before it can be used, scanner needs its dynstr global
 * variable set. That variable is used as a buffer for the
 * string that has been processed so far. To avoid allocating
 * and deallocating the buffer every time scanner is called,
 * it is stored as a global variable.
 */
void scanner_init();

/** Free dynstr buffer used by scanner.
 * Scanner uses a global variable to store the processed string.
 * This function has to be called before the end of the program,
 * or when scanner is no longer needed.
 */
void scanner_destroy();

/** Free scanner token.
 * Frees dynamically allocated token (and token attribute, if it is a string) retuned
 * by scanner.
 * @param tok Pointer to a token to destroy.
 */
void scanner_token_destroy(token_t *tok);


/** Get next scanner token.
 * Reads from stdin until it finds a new valid token or until it finds an error.
 * Sets the global error flag when it finds an invalid lexeme or when an internal error occurs.
 * @return Pointer to the new token, or NULL on error.
 */
token_t *scanner_get_next_token();



#endif

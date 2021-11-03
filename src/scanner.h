/**
 * @file
 * @brief Scanner API
 * @author Patrik Korytar
 * @author Tomas Martykan
 * @author Filip Stolfa
 */

#ifndef __SCANNER_H
#define __SCANNER_H

/**
 * Possible scanner token types.
 * Enum of all possible scanner token types.
 */
typedef enum {
  TT_ERROR,
  TT_NO_TYPE,
  TT_EOF, //2

  TT_KEYWORD_ID, //3
  TT_STRING,
  TT_INTEGER,
  TT_NUMBER,
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


/**
 * @struct token_t
 * @brief Scanner token type.
 * @var token_t::type
 * Type of the token.
 * @var token_t::attribute
 * Pointer to a dynamically allocated string
 * containing the parsed token string or NULL if
 * a token doesn't require an attribute.
 * TODO: make attribute an union and store parsed numbers in attribute?
 * TODO: error information
 */
typedef struct {
  token_type_t type;
  char *attribute;
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
 * Frees dynamically allocated token (and token attribute) retuned
 * by scanner.
 * @param tok Pointer to a token to destroy.
 */
void scanner_token_destroy(token_t *tok);


token_t *scanner_get_next_token();



#endif

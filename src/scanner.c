/**
 * @file
 * @brief Scanner implementation
 * @author Patrik Korytar
 * @author Tomas Martykan
 * @author Filip Stolfa
 */

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>
#include "scanner.h"
#include "dynstr.h"
#include "errors.h"

/// Number of keywords in keywords array
#define KEYWORDS_COUNT 15

/**
 * Macro for appending a character to a dynstr_t and checking
 * if the operation was successful. If it wasn't destroy token and
 * set error flag.
 */
#define APPEND_CHAR(CHAR, TOK) do{\
  if (dynstr_append(&str_buffer, (CHAR)) == NULL) {\
    scanner_token_destroy((TOK));\
    error_set(EXITSTATUS_INTERNAL_ERROR);\
    return NULL;\
  }\
}while(0)



/// Global dynamic string for storing incomplete tokens
dynstr_t str_buffer;


/**
 * @brief All keywords of the ifj21 language.
 * The order of the keyword strings has to match the order of the ::token_type_t
 * declared in scanner.h.
 */
static const char* keywords[KEYWORDS_COUNT] = {
    "local",    "integer", "number", "if",     "then",
    "else",     "do",      "while",  "string", "end",
    "function", "global",  "nil",    "return", "require"};

/// Scanner states.
typedef enum {
  STATE_START,
  STATE_KEYWORD_ID,

  STATE_INTEGER,
  STATE_NUMBER_DOT,
  STATE_NUMBER_EXP_START,
  STATE_NUMBER_EXP_SIGN,
  STATE_NUMBER_FINAL,
  STATE_NUMBER_FINAL_WITH_EXP,

  STATE_STRING_START,
  STATE_STRING_ESC,
  /* TODO(filip): should we check and process esc code here? and replace it in the string? */
  STATE_STRING_ESC_CODE_1,
  STATE_STRING_ESC_CODE_2,

  STATE_ASSIGN,
  STATE_NEQ_START,
  STATE_GT,
  STATE_LT,

  STATE_NUMBER_DIV,
  STATE_MINUS,
  STATE_CONCAT_START,

  STATE_COMMENT_START,
  STATE_COMMENT_LINE,
  STATE_COMMENT_BLOCK_1,
  STATE_COMMENT_BLOCK_2,
  STATE_COMMENT_BLOCK_END_1
} scanner_state_t;



void scanner_init() {
  dynstr_t *res = dynstr_init(&str_buffer);
  if (res == NULL) {
    error_set(EXITSTATUS_INTERNAL_ERROR);
    return;
  }
}

void scanner_destroy() {
  dynstr_t *res = dynstr_free_buffer(&str_buffer);
  if (res == NULL) {
    return;
  }
}

/** Get the correct keyword token type.
 * Checks if the passed dynstr_t contains a valid ifj21 keyword.
 * If it does, it calculates the corresponding ::token_type_t. Otherwise,
 * the tested string is an identifier.
 * @param dstr Pointer to a dynamic string containing the string we want to check.
 * @return Keyword ::token_type_t if dstr is a keyword, #TT_ID otherwise.
 */
token_type_t scanner_get_keyword_type(dynstr_t *dstr) {
  for (int i = 0; i < KEYWORDS_COUNT; i++) {
    if (dynstr_equals_static(dstr, keywords[i])) {
      return TOK_KEYWORD_OFFSET + i; // corresponds to the token_type_t enum member
    }
  }
  return TT_ID;
}

void scanner_token_destroy(token_t *tok) {
  if (tok == NULL) {
    return;
  }

  if (tok->type != TT_NUMBER && tok->type != TT_INTEGER) {
    free(tok->attr.str);
  }
  free(tok);
}

/** Allocate new token.
 * Allocates a token, sets string attribute to NULL and type to TT_NO_TYPE.
 * @return Pointer to the allocated token.
 */
token_t* scanner_create_empty_token() {
  token_t *new_token = malloc(sizeof(token_t));
  if (new_token == NULL) {
    error_set(EXITSTATUS_INTERNAL_ERROR);
    return NULL;
  }
  new_token->attr.str = NULL;
  new_token->type = TT_NO_TYPE;

  return new_token;
}

/** Make token into an eof token.
 * Changes the type of the token to TT_EOF.
 * @param tok Pointer to the token to change.
 * @return Pointer to the changed token.
 */
token_t *scanner_make_eof_token(token_t *tok) {
  tok->type = TT_EOF;
  return tok;
}

/** Make token into an id/keyword token.
 * Changes the type of the token to TT_KEYWORD_ID and copies
 * the parsed token string to attr.str. The attribute char pointer can be NULL,
 * if the copy failed - then the error flag is also set.
 * @param tok Pointer to the token to change.
 * @return Pointer to the changed token.
 */
token_t *scanner_make_id_kw_token(token_t *tok) {
  token_type_t tok_type;
  if ((tok_type = scanner_get_keyword_type(&str_buffer)) != TT_ID) {
    tok->type = tok_type;
  }
  else {
    tok->type = tok_type;
    tok->attr.str = dynstr_copy_to_static(&str_buffer);
    if (tok->attr.str == NULL) {
      scanner_token_destroy(tok);
      error_set(EXITSTATUS_INTERNAL_ERROR);
      return NULL;
    }
  }
  return tok;
}

/** Make token into a integer token.
 * Changes the type of the token to TT_INTEGER and converts the parsed lexeme
 * into an integer and stores it into attr.int_val.
 * @param tok Pointer to the token to change.
 * @return Pointer to the changed token.
 */
token_t *scanner_make_int_token(token_t *tok) {
  tok->type = TT_INTEGER;
  tok->attr.int_val = dynstr_to_int(&str_buffer);
  /* TODO(filip): check error from strtol here? */
  return tok;
}

/** Make token into a number token.
 * Changes the type of the token to TT_NUMBER and converts the parsed lexeme
 * into a double and stores it into attr.num_val.
 * @param tok Pointer to the token to change.
 * @return Pointer to the changed token.
 */
token_t *scanner_make_number_token(token_t *tok) {
  tok->type = TT_NUMBER;
  tok->attr.num_val = dynstr_to_double(&str_buffer);
  return tok;
}


/** Make token into an operator token, if the character is a valid operator.
 * Checks, if curr_char can only be one operator and there is no possible
 * other valid token it could become. If that is true, set the type of the token
 * pointed to by tok to the correct type. Otherwise an error token is returned.
 * @param tok Pointer to the token to change.
 * @param curr_char Character being processed.
 * @return Pointer to the changed token if successful, error token otherwise.
 */
token_t *scanner_make_one_state_op_sep(token_t *tok, int curr_char) {
  if (curr_char == ',') {
    tok->type = TT_COMMA;
  }
  else if (curr_char == ':') {
    tok->type = TT_COLON;
  }
  else if (curr_char == '(') {
    tok->type = TT_LPAR;
  }
  else if (curr_char == ')') {
    tok->type = TT_RPAR;
  }
  else if (curr_char == '*') {
    tok->type = TT_MOP_MUL;
  }
  // !minus has to be handled differently
  else if (curr_char == '+') {
    tok->type = TT_MOP_PLUS;
  }
  else if (curr_char == '#') {
    tok->type = TT_SOP_LENGTH;
  }
  else {
    scanner_token_destroy(tok);
    error_set(EXITSTATUS_ERROR_LEXICAL); // invalid character on input
    return NULL;
  }

  return tok;
}

/** Make token into an operator token.
 * Changes the type of the token to tok_type.
 * @param tok Pointer to the token to change.
 * @param tok_type Type to assign to the token.
 * @return Pointer to the changed token.
 */
token_t *scanner_make_op_token(token_t *tok, token_type_t tok_type) {
  tok->type = tok_type;
  return tok;
}

/** Make token into a string token.
 * Changes the type of the token to TT_STRING, copies
 * the parsed token string to attribute. attribute can be NULL,
 * if the copy failed.
 * @param tok Pointer to the token to change.
 * @return Pointer to the changed token.
 */
token_t *scanner_make_string_tok(token_t *tok) {
  tok->type = TT_STRING;
  tok->attr.str = dynstr_copy_to_static(&str_buffer);
  return tok;
}

/** Checks if character can be part of an escape sequence.
 * @param c Character to be checked if it is escapable.
 * @return true if c is escapable, false otherwise.
 */
bool is_escapable_char(int c) {
  if (c == 'n' || c == 't' || c == '\\') {
    return true;
  }
  return false;
}

char get_escaped_cahr(int c) {
  if (c == 'n') {
    return '\n';
  }
  else if (c == 't') {
    return '\t';
  }
  else if (c =='\\') {
    return '\\';
  }
  return 0;
}


token_t *scanner_get_next_token() {
  if (dynstr_clear(&str_buffer) == NULL) {
    error_set(EXITSTATUS_INTERNAL_ERROR);
    return NULL;
  }

  token_t *new_token = scanner_create_empty_token();
  if (new_token == NULL) {
    error_set(EXITSTATUS_INTERNAL_ERROR);
    return NULL;
  }

  scanner_state_t state = STATE_START;
  int curr_char; // int so we can check for EOF

  for (;;) {
    curr_char = getchar();

    switch (state) {
      case STATE_START:

        if (isspace(curr_char)) {
          continue;
        }
        /* TODO(filip): what about different locales? */
        else if (isalpha(curr_char) || curr_char == '_') {
          APPEND_CHAR(curr_char, new_token);
          state = STATE_KEYWORD_ID;
        }
        else if (isdigit(curr_char)) {
          APPEND_CHAR(curr_char, new_token);
          state = STATE_INTEGER;
        }
        else if (curr_char == '"') {
          state = STATE_STRING_START;
        }
        // handle multi-char operators and comments
        else if (curr_char == '=') {
          state = STATE_ASSIGN;
        }
        else if (curr_char == '~') {
          state = STATE_NEQ_START;
        }
        else if (curr_char == '>') {
          state = STATE_GT;
        }
        else if (curr_char == '<') {
          state = STATE_LT;
        }
        else if (curr_char == '/') {
          state = STATE_NUMBER_DIV;
        }
        else if (curr_char == '.') {
          state = STATE_CONCAT_START;
        }
        else if (curr_char == '-') {
          state = STATE_MINUS;
        }

        else if (curr_char == EOF) {
          return scanner_make_eof_token(new_token);
        }
        // handle one character operators and separators
        // if curr_char is not a correct op or sep, it is an error
        else {
          return scanner_make_one_state_op_sep(new_token, curr_char);
        }

        break;

      // ****** end of start <> start of integer
      case STATE_KEYWORD_ID:
        if (isalnum(curr_char) || curr_char == '_') {
          APPEND_CHAR(curr_char, new_token);
        }
        else {
          ungetc(curr_char, stdin);
          return scanner_make_id_kw_token(new_token);
        }
        break;

      // ****** end of keyword/id <> start of integer
      case STATE_INTEGER:
        if (isdigit(curr_char)) {
          APPEND_CHAR(curr_char, new_token);
        }
        else if (curr_char == '.') {
          APPEND_CHAR(curr_char, new_token);
          state = STATE_NUMBER_DOT;
        }
        else if (tolower(curr_char) == 'e') {
          APPEND_CHAR(curr_char, new_token);
          state = STATE_NUMBER_EXP_START;
        }
        else {
          ungetc(curr_char, stdin);
          return scanner_make_int_token(new_token);
        }
        break;

      // ****** end of integer <> start of number
      case STATE_NUMBER_DOT:
        if (isdigit(curr_char)) {
          APPEND_CHAR(curr_char, new_token);
          state = STATE_NUMBER_FINAL;
        }
        else {
          scanner_token_destroy(new_token);
          error_set(EXITSTATUS_ERROR_LEXICAL);
          return NULL;
        }
        break;

      case STATE_NUMBER_EXP_START:
        if (isdigit(curr_char)) {
          APPEND_CHAR(curr_char, new_token);
          state = STATE_NUMBER_FINAL_WITH_EXP;
        }
        else if (curr_char == '-' || curr_char == '+') {
          APPEND_CHAR(curr_char, new_token);
          state = STATE_NUMBER_EXP_SIGN;
        }
        else {
          scanner_token_destroy(new_token);
          error_set(EXITSTATUS_ERROR_LEXICAL);
          return NULL;
        }
        break;

      case STATE_NUMBER_EXP_SIGN:
        if (isdigit(curr_char)) {
          APPEND_CHAR(curr_char, new_token);
          state = STATE_NUMBER_FINAL_WITH_EXP;
        }
        else {
          scanner_token_destroy(new_token);
          error_set(EXITSTATUS_ERROR_LEXICAL);
          return NULL;
        }
        break;

      case STATE_NUMBER_FINAL:
        if (isdigit(curr_char)) {
          APPEND_CHAR(curr_char, new_token);
        }
        else if (tolower(curr_char) == 'e') {
          APPEND_CHAR(curr_char, new_token);
          state = STATE_NUMBER_EXP_START;
        }
        else {
          ungetc(curr_char, stdin);
          return scanner_make_number_token(new_token);
        }
        break;
      case STATE_NUMBER_FINAL_WITH_EXP:
        if (isdigit(curr_char)) {
          APPEND_CHAR(curr_char, new_token);
        }
        else {
          ungetc(curr_char, stdin);
          return scanner_make_number_token(new_token);
        }
        break;
      // ***** end of number <> start of operators

      case STATE_ASSIGN:
        if (curr_char == '=') {
          return scanner_make_op_token(new_token, TT_COP_EQ);
        }
        else {
          ungetc(curr_char, stdin);
          return scanner_make_op_token(new_token, TT_ASSIGN);
        }
        break;
      case STATE_GT:
        if (curr_char == '=') {
          return scanner_make_op_token(new_token, TT_COP_GE);
        }
        else {
          ungetc(curr_char, stdin);
          return scanner_make_op_token(new_token, TT_COP_GT);
        }
        break;
      case STATE_LT:
        if (curr_char == '=') {
          return scanner_make_op_token(new_token, TT_COP_LE);
        }
        else {
          ungetc(curr_char, stdin);
          return scanner_make_op_token(new_token, TT_COP_LT);
        }
        break;
      case STATE_NEQ_START:
        if (curr_char == '=') {
          return scanner_make_op_token(new_token, TT_COP_NEQ);
        }
        else {
          ungetc(curr_char, stdin);
          scanner_token_destroy(new_token);
          error_set(EXITSTATUS_ERROR_LEXICAL);
          return NULL;
        }
        break;
      case STATE_NUMBER_DIV:
        if (curr_char == '/') {
          return scanner_make_op_token(new_token, TT_MOP_INT_DIV);
        }
        else {
          ungetc(curr_char, stdin);
          return scanner_make_op_token(new_token, TT_MOP_DIV);
        }
        break;
      case STATE_MINUS:
        if (curr_char == '-') {
          state = STATE_COMMENT_START;
        }
        else {
          ungetc(curr_char, stdin);
          return scanner_make_op_token(new_token, TT_MOP_MINUS);
        }
        break;
      case STATE_CONCAT_START:
        if (curr_char == '.') {
          return scanner_make_op_token(new_token, TT_SOP_CONCAT);
        }
        else {
          ungetc(curr_char, stdin);
          scanner_token_destroy(new_token);
          error_set(EXITSTATUS_ERROR_LEXICAL);
          return NULL;
        }
        break;

      // ****** end of operators <> start of strings
      case STATE_STRING_START:
        if (curr_char == '"') {
          return scanner_make_string_tok(new_token);
        }
        else if (curr_char == '\\') {
          state = STATE_STRING_ESC;
        }
        else if (curr_char == '\n' || curr_char == EOF) {
          scanner_token_destroy(new_token);
          error_set(EXITSTATUS_ERROR_LEXICAL);
          return NULL;
        }
        else if (curr_char <= 32 || curr_char == '#') { // escape whitespace
          if (dynstr_append_esc(&str_buffer, curr_char) == NULL) {
            scanner_token_destroy(new_token);
            error_set(EXITSTATUS_INTERNAL_ERROR);
            return NULL;
          }
        }
        else if (curr_char > 31) {
          APPEND_CHAR(curr_char, new_token);
        }
        else {
          scanner_token_destroy(new_token);
          error_set(EXITSTATUS_ERROR_LEXICAL);
          return NULL;
        }
        break;

      case STATE_STRING_ESC:
        if (isdigit(curr_char)) {
          APPEND_CHAR('\\', new_token);
          APPEND_CHAR(curr_char, new_token);
          state = STATE_STRING_ESC_CODE_1;
        }
        else if (curr_char == '"') {
          APPEND_CHAR('"', new_token);
          state = STATE_STRING_START;
        }
        else if (is_escapable_char(curr_char)) {
          char c = get_escaped_cahr(curr_char);
          if (dynstr_append_esc(&str_buffer, c) == NULL) {
            scanner_token_destroy(new_token);
            error_set(EXITSTATUS_INTERNAL_ERROR);
            return NULL;
          }
          state = STATE_STRING_START;
        }
        else {
          scanner_token_destroy(new_token);
          error_set(EXITSTATUS_ERROR_LEXICAL);
          return NULL;
        }
        break;

      case STATE_STRING_ESC_CODE_1:
        if (isdigit(curr_char)) {
          APPEND_CHAR(curr_char, new_token);
          state = STATE_STRING_ESC_CODE_2;
        }
        else {
          scanner_token_destroy(new_token);
          error_set(EXITSTATUS_ERROR_LEXICAL);
          return NULL;
        }
        break;

      case STATE_STRING_ESC_CODE_2:
        if (isdigit(curr_char)) {
          APPEND_CHAR(curr_char, new_token);
          state = STATE_STRING_START;
        }
        else {
          scanner_token_destroy(new_token);
          error_set(EXITSTATUS_ERROR_LEXICAL);
          return NULL;
        }
        break;

      case STATE_COMMENT_START:
        if (curr_char == '[') {
          state = STATE_COMMENT_BLOCK_1;
        }
        else {
          state = STATE_COMMENT_LINE;
        }
        break;
      case STATE_COMMENT_BLOCK_1:
        if (curr_char == '[') {
          state = STATE_COMMENT_BLOCK_2;
        }
        else {
          state = STATE_COMMENT_LINE;
        }
        break;
      case STATE_COMMENT_BLOCK_2:
        if (curr_char == ']') {
          state = STATE_COMMENT_BLOCK_END_1;
        }
        else if (curr_char == EOF) {
          scanner_token_destroy(new_token);
          error_set(EXITSTATUS_ERROR_LEXICAL);
          return NULL;
        }
        break;
      case STATE_COMMENT_BLOCK_END_1:
        if (curr_char == ']') {
          state = STATE_START;
        }
        else {
          state = STATE_COMMENT_BLOCK_2;
        }
        break;
      case STATE_COMMENT_LINE:
        if (curr_char == '\n' || curr_char == EOF) {
          state = STATE_START;
        }
        break;


      default:
        /* TODO(filip): report unhandled state */
        scanner_token_destroy(new_token);
        error_set(EXITSTATUS_INTERNAL_ERROR);
        return NULL;
    }
  }
}

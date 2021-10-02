/**
 * @file
 * @brief Scanner implementation
 * @author Patrik Korytar
 * @author Tomas Martykan
 * @author Filip Stolfa
 *
 * ---> TODO -- NOT WORKING RIGHT NOW <----
 *
 */

#include "scanner.h"
#include "symtable.h"

#include <stdio.h>

/**
 * @brief Store current state of the scanner state machine.
 */
static scanner_state_t curr_state = BEGIN_STATE;

token_t* token_get(symtab_t *symtab)
{
  // APPROXIMATION OF ALGORITHM

  int err;
  token_t* token = token_read(&err);

  if(!symtab) {
    if(token->type == ID) {
      token_store(symtab, token);
    }
    else {
      token_destroy(token);
      return NULL;
    }
  }

  return token;
}

token_t* token_read(int* err)
{
  // APPROXIMATION OF ALGORITHM

  string_t* lexeme;
  string_init(&lexeme);

  while(curr_state != END_OF_TOKEN)
  {
    static char char_buff; /**< Stores character right after last read lexeme. */
    char_buff = getchar();

    curr_state = scanner_determine_state(curr_state, char_buff);
    if(curr_state == ERROR_TOKEN) {
      // erroneous token
      *err = -1;
      return NULL;
    }

    if(curr_state == COMMENTED)
      continue;

    string_appendchar(lexeme, char_buff);
  }

  token_t* token = malloc(sizeof(token_t));
  if(token == NULL)
  {
    string_free(lexeme);
    return NULL;
  }

  token->type = token_determine_type(curr_state);
  if(curr_state == ID) {
    token->value = lexeme;
  }
  else {
    token->value = NULL;
    string_free(lexeme);
  }

  curr_state = BEGIN_STATE; // return to begin state

  *err = 0;
  return token;
}

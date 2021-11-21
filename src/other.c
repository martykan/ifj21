/**
 * @file
 * @brief Generally useful functions
 * @author Patrik Korytar
 * @author Tomas Martykan
 * @author Filip Stolfa
 *
 * TODO...
 */

#include "other.h"

#include <stdlib.h>
#include <string.h>

#include "errors.h"

char* str_create_copy(char* str) {
  char* copy = malloc(strlen(str) + 1);
  if (!copy) {
    error_set(EXITSTATUS_INTERNAL_ERROR);
    return NULL;
  }

  strcpy(copy, str);

  return copy;
}

/**
 * @file
 * @brief Generally useful functions
 * @author Tomas Martykan (xmarty07)
 * @author Filip Stolfa (xstolf00)
 * @author Patrik Korytar (xkoryt04)
 *
 * FIT VUT IFJ Project:
 * Compiler of IFJ21 Language
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

  strncpy(copy, str, strlen(str) + 1);

  return copy;
}

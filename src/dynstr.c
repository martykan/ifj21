/**
 * @file
 * @brief Dynamic string implementation.
 * @author Patrik Korytar
 * @author Tomas Martykan
 * @author Filip Stolfa
 */

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "dynstr.h"

#define DEFAULT_LEN 32
#define REALLOC_FAC 2



dynstr_t* dynstr_init(dynstr_t *dynstr) {
  if (dynstr == NULL) {
    /* TODO(filip): Set a global error flag */
    return NULL;
  }
  dynstr->len = 0;
  dynstr->alloced_bytes = DEFAULT_LEN;

  dynstr->str = malloc(sizeof(char) * dynstr->alloced_bytes);
  if (dynstr->str == NULL) {
    /* TODO(filip): Set a global error flag */
    return NULL;
  }

  dynstr->str[0] = '\0';
  return dynstr;
}

dynstr_t* dynstr_free_buffer(dynstr_t *dynstr) {
  if (dynstr == NULL) {
    /* TODO(filip): Set a global error flag */
    return NULL;
  }

  dynstr->len = 0;
  free(dynstr->str);
  dynstr->str = NULL;
  dynstr->alloced_bytes = 0;

  return dynstr;
}

dynstr_t* dynstr_clear(dynstr_t *dynstr) {
  if (dynstr == NULL || dynstr->str == NULL) {
    /* TODO(filip): Set a global error flag */
    return NULL;
  }


  dynstr->len = 0;
  dynstr->str[0] = '\0';
  return dynstr;
}

dynstr_t* dynstr_append(dynstr_t *dynstr, char c) {
  if (dynstr == NULL || dynstr->str == NULL) {
    /* TODO(filip): Set a global error flag */
    return NULL;
  }

  size_t new_str_len = dynstr->len + 1; // add 1 char

  if (new_str_len >= dynstr->alloced_bytes) {
    size_t new_buf_size = dynstr->alloced_bytes * REALLOC_FAC;
    char *tmp = NULL;
    tmp = realloc(dynstr->str, sizeof(char) * new_buf_size);
    if (tmp == NULL) { // realloc failed
      return NULL;
    }
    dynstr->str = tmp;
    dynstr->alloced_bytes = new_buf_size;
  }

  dynstr->str[dynstr->len] = c;
  dynstr->len++;
  // add terminating null byte
  dynstr->str[dynstr->len] = '\0';

  return dynstr;
}

dynstr_t* dynstr_append_esc(dynstr_t *dynstr, char c) {
  char esc_buf[4]; // eg. 032, 092

  sprintf(esc_buf, "%03d", c);

  if (dynstr_append(dynstr, '\\') == NULL) {
    return NULL;
  }

  for (int i = 0; i < 3; i++) {
    if (dynstr_append(dynstr, esc_buf[i]) == NULL) {
      return NULL;
    }
  }

  return dynstr;
}

char* dynstr_copy_to_static(dynstr_t *dynstr) {
  if (dynstr == NULL || dynstr->str == NULL) {
    /* TODO(filip): Set a global error flag */
    return NULL;
  }

  size_t new_buff_len = dynstr->len + 1; // len+'\0'
  char *dest = malloc(sizeof(char) * new_buff_len);
  if (dest == NULL) {
    /* TODO(filip): Set a global error flag */
    return NULL;
  }

  strncpy(dest, dynstr->str, new_buff_len);
  return dest;
}

bool dynstr_equals_static(dynstr_t *dynstr, const char *str) {
  if (strcmp(dynstr->str, str) == 0) {
    return true;
  }
  return false;
}

int dynstr_to_int(dynstr_t *dynstr) {
  char *end_ptr = dynstr->str;
  long int res = strtol(dynstr->str, &end_ptr, 10);
  if (*end_ptr != '\0' || dynstr->len == 0) {
    /* TODO(filip): report error */
    return -1;
  }
  return (int) res;
}

double dynstr_to_double(dynstr_t *dynstr) {
  char *end_ptr = dynstr->str;
  double res = strtod(dynstr->str, &end_ptr);
  if (*end_ptr != '\0' || dynstr->len == 0) {
    /* TODO(filip): report error */
    return -1.0;
  }
  return res;
}

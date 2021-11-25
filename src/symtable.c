/**
 * @file
 * @brief Symbol table implementation
 * @author Patrik Korytar
 * @author Tomas Martykan
 * @author Filip Stolfa
 */

#include "symtable.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "errors.h"

// PRIVATE FUNCTION FORWARD DECLARATIONS

// ALLOCATION FUNCTIONS

/**
 * Creates subtable.
 * @param n Bucket count.
 * @return Created subtable. NULL if failed to create.
 */
symtab_subtab_t* symtab_subtab_create(size_t n);

/**
 * Creates record of identifier.
 * @param key Name of the identifier.
 * @return Created record. NULL if failed to create.
 */
symtab_record_t* symtab_record_create(symtab_key_t key);

// DEALLOCATION FUNCTIONS

/**
 * Destroys whole subtable.
 * @param subtab Subtable to destroy.
 */
void symtab_subtab_free(symtab_subtab_t* subtab);

/**
 * Destroys all contents of subtable, but not subtable.
 * @param subtab Subtable to clear.
 */
void symtab_subtab_clear(symtab_subtab_t* subtab);

/**
 * Destroys record of identifier.
 * @param rec Record to destroy.
 */
void symtab_record_free(symtab_record_t* rec);

// MANIPULATION WITH RECORDS FUNCTIONS

/**
 * Searches subtable for identifier.
 * @param subtab Subtable to search on.
 * @param key Key to search for.
 * @return Found record data. NULL otherwise.
 */
symtab_data_t* symtab_subtab_find(const symtab_subtab_t* subtab,
                                  symtab_key_t key);

/**
 * Creates and inserts new record in subtable.
 * @param subtab Subtable to instert on.
 * @param key Key of the new record.
 * @return Created record. NULL if failed to create.
 */
symtab_data_t* symtab_subtab_insert(symtab_subtab_t* subtab, symtab_key_t key, char type);

/**
 * Erases record with key from subtable.
 * @param subtab Subtable to erase record from.
 * @param key Key of record to erase.
 * @return True if found and deleted. False if not found.
 */
bool symtab_subtab_erase(symtab_subtab_t* subtab, symtab_key_t key);

// HASH FUNCTION

/**
 * Hash function.
 * @param data Key to hash.
 * @return Index created from key.
 */
uint32_t SuperFastHash(symtab_key_t data);
//--------------------------------------------------------------------------------------
// APPLIES TO SINGLE FUNCTION DECLARATION ABOVE
// TAKEN FROM: http://www.azillionmonkeys.com/qed/hash.html
// COPYRIGHT: © Copyright 2004-2008 by Paul Hsieh
// LICENSE: GNU Lesser General Public License v2.1
// LICENSE TEXT: https://www.gnu.org/licenses/old-licenses/lgpl-2.1.txt
// CHANGES MADE: 01.10.2021 - changed parameter type and count; added local len
// variable
//--------------------------------------------------------------------------------------

// END OF FORWARD DECLARATIONS

// COMPILE-TIME CONSTANTS

#define SYMTAB_BUCKET_COUNT 83

// FUNCTION DEFINITIONS
void symtab_init_builtin(symtab_t* symtab, char* name, char* param_types,
                         char* return_types);

// ALLOCATION FUNCTIONS

symtab_t* symtab_create() {
  symtab_t* symtab = malloc(sizeof(symtab_t));
  if (!symtab) {
    error_set(EXITSTATUS_INTERNAL_ERROR);
    return NULL;
  }

  symtab->global_scope = symtab_subtab_create(SYMTAB_BUCKET_COUNT);
  if (error_get()) {
    free(symtab);
    return NULL;
  }

  symtab_init_builtin(symtab, "write", "a+", "");
  symtab_init_builtin(symtab, "reads", "", "s");
  symtab_init_builtin(symtab, "readi", "", "i");
  symtab_init_builtin(symtab, "readn", "", "n");
  symtab_init_builtin(symtab, "tointeger", "n", "i");
  symtab_init_builtin(symtab, "substr", "snn", "s");
  symtab_init_builtin(symtab, "ord", "si", "i");
  symtab_init_builtin(symtab, "chr", "i", "s");

  symtab->local_scopes = NULL;

  return symtab;
}

void symtab_init_builtin(symtab_t* symtab, char* id, char* param_types,
                         char* return_types) {
  symtab_func_data_t* func_data = symtab_insert_func(symtab, id);
  if (error_get()) {
    return;
  }

  func_data->func_name = malloc(strlen(id) + 1);
  if (!func_data->func_name) {
    error_set(EXITSTATUS_INTERNAL_ERROR);
    return;
  }
  strncpy(func_data->func_name, id, strlen(id) + 1);

  func_data->param_types = malloc(strlen(param_types) + 1);
  if (!func_data->param_types) {
    error_set(EXITSTATUS_INTERNAL_ERROR);
    return;
  }
  strncpy(func_data->param_types, param_types, strlen(param_types) + 1);

  func_data->return_types = malloc(strlen(return_types) + 1);
  if (!func_data->return_types) {
    error_set(EXITSTATUS_INTERNAL_ERROR);
    return;
  }
  strncpy(func_data->return_types, return_types, strlen(return_types) + 1);

  func_data->was_defined = true;
  func_data->params = NULL;
}

symtab_subtab_t* symtab_subtab_create(size_t n) {
  symtab_subtab_t* subtab =
      malloc(sizeof(symtab_subtab_t) + n * sizeof(symtab_record_t*));
  if (!subtab) {
    error_set(EXITSTATUS_INTERNAL_ERROR);
    return NULL;
  }

  subtab->next = NULL;
  subtab->bucket_cnt = n;
  for (unsigned i = 0; i < n; i++) {
    subtab->list[i] = NULL;
  }

  return subtab;
}

symtab_record_t* symtab_record_create(symtab_key_t key) {
  symtab_record_t* rec = malloc(sizeof(symtab_record_t));
  if (!rec) {
    error_set(EXITSTATUS_INTERNAL_ERROR);
    return NULL;
  }

  char* new_key = malloc(strlen(key) + 1);
  if (!new_key) {
    free(rec);
    error_set(EXITSTATUS_INTERNAL_ERROR);
    return NULL;
  }
  strcpy(new_key, key);

  rec->next = NULL;
  rec->key = new_key;

  return rec;
}

// DEALLOCATION FUNCTIONS

void symtab_free(symtab_t* symtab) {
  symtab_clear(symtab);
  free(symtab);
}

void symtab_clear(symtab_t* symtab) {
  symtab_subtab_free(symtab->global_scope);

  // delete first subtab while any left
  while (symtab->local_scopes != NULL) {
    symtab_subtab_t* next = symtab->local_scopes->next;
    symtab_subtab_free(symtab->local_scopes);
    symtab->local_scopes = next;
  }
}

void symtab_subtab_free(symtab_subtab_t* subtab) {
  symtab_subtab_clear(subtab);
  free(subtab);
}

void symtab_subtab_clear(symtab_subtab_t* subtab) {
  for (unsigned i = 0; i < subtab->bucket_cnt; i++) {
    // delete first record while any left
    while (subtab->list[i] != NULL) {
      symtab_record_t* next = subtab->list[i]->next;
      symtab_record_free(subtab->list[i]);
      subtab->list[i] = next;
    }
  }

  return;
}

void symtab_record_free(symtab_record_t* rec) {
  free((char*)rec->key);

  if (rec->what_data == 'f') {
    free(rec->data.func_data.func_name);
    free(rec->data.func_data.param_types);
    free(rec->data.func_data.return_types);
    if(rec->data.func_data.params) {
      free(rec->data.func_data.params->vars);
    }
    free(rec->data.func_data.params);
  } else if (rec->what_data == 'v') {
    free(rec->data.var_data.var_name);
  }

  free(rec);
}

// MANIPULATION WITH LOCAL SUBTABLES FUNCTIONS

bool symtab_subtab_push(symtab_t* symtab) {
  symtab_subtab_t* subtab = symtab_subtab_create(SYMTAB_BUCKET_COUNT);
  if (error_get()) {
    return false;
  }

  subtab->next = symtab->local_scopes;
  symtab->local_scopes = subtab;

  return true;
}

void symtab_subtab_pop(symtab_t* symtab) {
  symtab_subtab_t* next = symtab->local_scopes->next;
  symtab_subtab_free(symtab->local_scopes);
  symtab->local_scopes = next;
}

// MANIPULATION WITH RECORDS FUNCTIONS

symtab_var_data_t* symtab_find_var(const symtab_t* symtab, symtab_key_t key, int *lvl) {
  for (symtab_subtab_t* subtab = symtab->local_scopes; subtab != NULL;
       subtab = subtab->next) {
    symtab_data_t* data = symtab_subtab_find(subtab, key);
    if (data) {
      return &data->var_data;
    }
    if (lvl != NULL) {
      (*lvl)++;
    }
  }

  return NULL;
}

symtab_var_data_t* symtab_find_var_local(const symtab_t* symtab, symtab_key_t key) {
  symtab_data_t* data = symtab_subtab_find(symtab->local_scopes, key);
  if (data) {
    return &data->var_data;
  }
  return NULL;
}

symtab_func_data_t* symtab_find_func(const symtab_t* symtab, symtab_key_t key) {
  symtab_data_t* data = symtab_subtab_find(symtab->global_scope, key);
  if (data) {
    return &data->func_data;
  }

  return NULL;
}

symtab_data_t* symtab_subtab_find(const symtab_subtab_t* subtab,
                                  symtab_key_t key) {
  size_t index = SuperFastHash(key) % subtab->bucket_cnt;

  for (symtab_record_t* rec = subtab->list[index]; rec != NULL;
       rec = rec->next) {
    if (!strcmp(rec->key, key)) {
      return &rec->data;
    }
  }

  return NULL;
}

symtab_var_data_t* symtab_insert_var(symtab_t* symtab, symtab_key_t key) {
  return &symtab_subtab_insert(symtab->local_scopes, key, 'v')->var_data;
}

symtab_func_data_t* symtab_insert_func(symtab_t* symtab, symtab_key_t key) {
  return &symtab_subtab_insert(symtab->global_scope, key, 'f')->func_data;
}

symtab_data_t* symtab_subtab_insert(symtab_subtab_t* subtab, symtab_key_t key, char type) {
  size_t index = SuperFastHash(key) % subtab->bucket_cnt;

  symtab_record_t* last = NULL;
  for (last = subtab->list[index]; last != NULL; last = last->next)
    ;

  symtab_record_t* new_rec = symtab_record_create(key);
  if (error_get()) {
    return NULL;
  }

  new_rec->what_data = type;

  if (!last)  // if no record on current bucket
    subtab->list[index] = new_rec;
  else  // otherwise append
    last->next = new_rec;

  return &new_rec->data;
}

void symtab_get_top_vars(const symtab_t* symtab, symtab_vars_t* vars) {
  symtab_subtab_t* subtab = symtab->local_scopes;
  int found = 0;

  for (unsigned i = 0; i < subtab->bucket_cnt; i++) {
    symtab_record_t* rec = subtab->list[i];
    while (rec != NULL) {
      vars->vars[found] = malloc(sizeof(rec->data.var_data));
      memcpy(vars->vars[found], &rec->data.var_data,
             sizeof(rec->data.var_data));
      found++;
      rec = rec->next;
    }
  }
  vars->cnt = found;
}

void symtab_subtab_foreach(const symtab_subtab_t* subtab,
                           void (*f)(symtab_data_t* data)) {
  for (unsigned i = 0; i < subtab->bucket_cnt; i++) {
    for (symtab_record_t* rec = subtab->list[i]; rec != NULL; rec = rec->next) {
      (*f)(&rec->data);
    }
  }
}

bool symtab_subtab_erase(symtab_subtab_t* subtab, symtab_key_t key) {
  size_t index = SuperFastHash(key) % subtab->bucket_cnt;

  symtab_record_t* last = NULL;
  for (symtab_record_t* rec = subtab->list[index]; rec != NULL;
       rec = rec->next) {
    if (!strcmp(rec->key, key)) {
      if (!last) {  // if no record on current bucket
        subtab->list[index] = rec->next;
      } else {
        last->next = rec->next;
      }

      symtab_record_free(rec);
      return true;
    }

    last = rec;
  }

  return false;
}

// HASH FUNCTION

//--------------------------------------------------------------------------------------
// TAKEN CODE
// TAKEN FROM: http://www.azillionmonkeys.com/qed/hash.html
// COPYRIGHT: © Copyright 2004-2008 by Paul Hsieh
// LICENSE: GNU Lesser General Public License v2.1
// LICENSE TEXT: https://www.gnu.org/licenses/old-licenses/lgpl-2.1.txt
// CHANGES MADE: 01.10.2021 - changed parameter type and count; added local len
//                            variable

#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) || \
    defined(_MSC_VER) || defined(__BORLANDC__) || defined(__TURBOC__)
#define get16bits(d) (*((const uint16_t*)(d)))
#endif

#if !defined(get16bits)
#define get16bits(d)                               \
  ((((uint32_t)(((const uint8_t*)(d))[1])) << 8) + \
   (uint32_t)(((const uint8_t*)(d))[0]))
#endif

uint32_t SuperFastHash(symtab_key_t data) {
  int len = strlen(data);
  uint32_t hash = len, tmp;
  int rem;

  if (len <= 0 || data == NULL) return 0;

  rem = len & 3;
  len >>= 2;

  /* Main loop */
  for (; len > 0; len--) {
    hash += get16bits(data);
    tmp = (get16bits(data + 2) << 11) ^ hash;
    hash = (hash << 16) ^ tmp;
    data += 2 * sizeof(uint16_t);
    hash += hash >> 11;
  }

  /* Handle end cases */
  switch (rem) {
    case 3:
      hash += get16bits(data);
      hash ^= hash << 16;
      hash ^= ((signed char)data[sizeof(uint16_t)]) << 18;
      hash += hash >> 11;
      break;
    case 2:
      hash += get16bits(data);
      hash ^= hash << 11;
      hash += hash >> 17;
      break;
    case 1:
      hash += (signed char)*data;
      hash ^= hash << 10;
      hash += hash >> 1;
  }

  /* Force "avalanching" of final 127 bits */
  hash ^= hash << 3;
  hash += hash >> 5;
  hash ^= hash << 4;
  hash += hash >> 17;
  hash ^= hash << 25;
  hash += hash >> 6;

  return hash;
}

// END OF TAKEN CODE
//--------------------------------------------------------------------------------------

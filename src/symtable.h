/**
 * @file
 * @brief Symbol table API
 * @author Patrik Korytar
 * @author Tomas Martykan
 * @author Filip Stolfa
 *
 * @section DESCRIPTION
 *  Symbol table interface for interaction. Used by other components
 *  of compiler for storing identifiers.
 *
 * @section IMPLEMENTATION
 *  Symbol table is composed of hash tables for global scope and local
 *  scopes. Local scope tables are inside a stack, the topmost of which
 *  represents most nested scope.
 */

#ifndef __SYMTAB_H__
#define __SYMTAB_H__

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// DATA STRUCTURES

/**
 * @brief Type of key used in hash tables.
 */
typedef const char* symtab_key_t;

// Data type is represented by single character.
// i - integer
// n - number
// s - string

/**
 * @struct symtab_var_data_t
 * @brief Data of the variable identifier.
 * @var symtab_var_data_t::data_type
 *  Data type of the variable.
 * @var symtab_var_data_t::is_init
 *  Was the variable initialized?
 */
typedef struct {
  char* var_name;
  char data_type;
  bool is_init;
} symtab_var_data_t;

typedef struct {
  int cnt;
  symtab_var_data_t** vars;
} symtab_vars_t;

/**
 * @struct symtab_func_data_t
 * @brief Data of the function identifier.
 * @var symtab_func_data_t::param_types
 *  Data types of the parameters.
 *  Each represented as single character in string.
 * @var symtab_func_data_t::return_types
 *  Data types of the return values.
 *  Each represented as single character in string.
 * @var symtab_func_data_t::was_defined
 *  Was the function body already defined?
 * @var symtab_func_data_t::params
 *  Pointers to symtable where arguments are stored.
 */
typedef struct {
  char* func_name;
  char* param_types;
  char* return_types;
  bool was_defined;
} symtab_func_data_t;

/**
 * @union symtab_data_t
 * @brief Data of either variable or function identifier.
 * @var symtab_data_t::var_data
 *  Variable identifier data.
 * @var symtab_data_t::func_data
 *  Function identifier data.
 */
typedef union {
  symtab_var_data_t var_data;
  symtab_func_data_t func_data;
} symtab_data_t;

/**
 * @struct symtab_record_t
 * @brief Record representing identifier.
 * @var symtab_record_t::next
 *  Next record on the same bucket (if collision occured).
 * @var symtab_record_t::key
 *  Key of the hash function.
 * @var symtab_record_t::what_data
 *  If 'v' -> variable record.
 *  If 'f' -> function record.
 * @var symtab_record_t::data
 *  Union containing data of either variable or function identifier.
 */
typedef struct symtab_record {
  struct symtab_record* next;
  symtab_key_t key;
  char what_data;
  symtab_data_t data;
} symtab_record_t;

/**
 * @struct symtab_subtab_t
 * @brief Subtable of identifiers of one scope.
 * @var symtab_subtab_t::next
 *  Next subtable on the stack.
 * @var symtab_subtab_t::bucket_cnt
 *  Bucket count of the subtable.
 * @var symtab_subtab_t::list
 *  Array of buckets, each pointing to record.
 */
typedef struct symtab_subtab {
  struct symtab_subtab* next;
  size_t bucket_cnt;
  symtab_record_t* list[];
} symtab_subtab_t;

/**
 * @struct symtab_t
 * @brief Hierarchical symbol table.
 *  Includes tables for both global and local scopes.
 *  Most nested local scope table is on top of stack.
 * @var symtab_t::global_scope
 *  Table containing global identifiers, ie. functions.
 * @var symtab_t::local_scopes
 *  Top of stack of tables containing local identifiers, ie. variables.
 */
typedef struct {
  symtab_subtab_t* global_scope;
  symtab_subtab_t* local_scopes;
} symtab_t;

// PUBLIC FUNCTIONS FORWARD DECLARATIONS

// ALLOCATION FUNCTIONS

/**
 * Creates symbol table.
 * Creates empty global scope table.
 * Does not create any local scope table.
 * @return Created symbol table. NULL if failed to create.
 */
symtab_t* symtab_create();

// DEALLOCATION FUNCTIONS

/**
 * Destroys whole symbol table
 * @param symtab Symbol table to destroy.
 */
void symtab_free(symtab_t* symtab);

/**
 * Destroys contents of symbol table, but not symbol table.
 * @param symtab Symbol table to clear.
 */
void symtab_clear(symtab_t* symtab);

// MANIPULATION WITH LOCAL SUBTABLES FUNCTIONS

/**
 * Creates and pushes local table on the top of the stack of symbol table.
 * @param symtab Symbol table to push on.
 * @return True if successful. False otherwise.
 */
bool symtab_subtab_push(symtab_t* symtab);

/**
 * Pops and destroys local table on the top of the stack of symbol table.
 * @param symtab Symbol table to pop from.
 */
void symtab_subtab_pop(symtab_t* symtab);

// MANIPULATION WITH RECORDS FUNCTIONS

/**
 * Searches whole stack of local tables for variable identifier.
 * @param symtab Symbol table to search on.
 * @param key Key to search for.
 * @return Found record data. NULL otherwise.
 */
symtab_var_data_t* symtab_find_var(const symtab_t* symtab, symtab_key_t key,
                                   int* lvl);

symtab_var_data_t* symtab_find_var_local(const symtab_t* symtab,
                                         symtab_key_t key);

/**
 * Searches global table for function identifier.
 * @param symtab Symbol table to search on.
 * @param key Key to search for.
 * @return Found record data. NULL otherwise.
 */
symtab_func_data_t* symtab_find_func(const symtab_t* symtab, symtab_key_t key);

/**
 * Creates and inserts new record in topmost local table (most nested scope).
 * @param symtab Symbol table to instert in.
 * @param key Key of the new record.
 * @return Created record. NULL if failed to create.
 */
symtab_var_data_t* symtab_insert_var(symtab_t* symtab, symtab_key_t key);

/**
 * Creates and inserts new record in global local table.
 * @param symtab Symbol table to instert in.
 * @param key Key of the new record.
 * @return Created record. NULL if failed to create.
 */
symtab_func_data_t* symtab_insert_func(symtab_t* symtab, symtab_key_t key);

/**
 * Executes function for each record in subtable.
 * @param subtab Subtable to operate on.
 * @param f Function to execute.
 * @param write_to Where to save data.
 */
void symtab_subtab_foreach(const symtab_subtab_t* subtab,
                           void (*f)(symtab_data_t* data));

void symtab_get_top_vars(const symtab_t* symtab, symtab_vars_t* vars);

#endif  // __SYMTAB_H__

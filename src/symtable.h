/**
 * @file
 * @brief Symbol table API
 * @author Patrik Korytar
 * @author Tomas Martykan
 * @author Filip Stolfa
 *
 * @section DESCRIPTION
 * Symbol table interface for interaction. Used by other components
 * of compiler for stroring identifiers.
 *
 * @section IMPLEMENTATION
 * Symbol table is composed of keywords hash table and identifiers hash
 * tables. Keywords table contains all keywords of language. Identifiers tables
 * are a stack of subtables where each represents a single scope. The most nested
 * scope is associated with topmost table on stack.
 */

#ifndef __SYMTAB_H__
#define __SYMTAB_H__

#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

// COMPILE-TIME CONSTANTS

#define SYMTAB_BUCKET_COUNT 83
#define KEYWORDS_COUNT 15

// TYPEDEFS

typedef struct symtab symtab_t;
typedef struct symtab_subtab symtab_subtab_t;
typedef struct symtab_record symtab_record_t;
typedef struct symtab_pair symtab_pair_t;
typedef struct symtab_value symtab_value_t;
/**
 * @brief Type of key used in hash tables.
 */
typedef const char* symtab_key_t;

/**
 * @struct symtab_t
 * @brief Symbol table consisting of identifiers subtables and keywords table.
 * @var symtab_t::keywords
 * Points to keywords table.
 * @var symtab_t::subtabs
 * Points to the head of the stack of identifiers subtables.
 */
struct symtab
{
  symtab_subtab_t*  keywords;
  symtab_subtab_t*  subtabs;
};

/**
 * @struct symtab_subtab_t
 * @brief Single identifiers subtable associated with one scope.
 * @var symtab_subtab_t::next
 * Next subtable on the stack (under this one).
 * @var symtab_subtab_t::bucket_cnt
 * Bucket count of the subtable.
 * @var symtab_subtab_t::records_cnt
 * Current count of the stored records.
 * @var symtab_subtab_t::list
 * Array of hash table buckets.
 */
struct symtab_subtab {
  symtab_subtab_t*  next;
  size_t            bucket_cnt;
  size_t            records_cnt;
  symtab_record_t*  list[];
};

/**
 * @struct symtab_record_t
 * @brief One record representing identifier.
 * @var symtab_record_t::next
 * Next record on the same bucket (if collision occured).
 * @var symtab_record_t::pair
 * Key-value pair of the record.
 */
struct symtab_record {
  symtab_record_t*  next;
  symtab_pair_t     pair;
};

/**
 * @struct symtab_pair_t
 * @brief Key-value pair of the record.
 * @var symtab_pair_t::key
 * Key for the hash function.
 * @var symtab_pair_t::value
 * Data of the record.
 */
struct symtab_pair {
  symtab_key_t    key;
  symtab_value_t  value;
};

/**
 * @struct symtab_value_t
 * @brief Contains data of the identifier record. TODO
 * @var symtab_value_t::x
 * TODO
 */
struct symtab_value {
  int x;
};

// INITIALIZATION FUNCTIONS

/**
 * Creates symbol table.
 * Creates keywords table and no identifiers subtables.
 * @return Created symbol table. NULL if failed to create.
 */
symtab_t* symtab_create();

/**
 * Creates keywords table.
 * @return Created keywords table. NULL if failed to create.
 */
symtab_subtab_t* symtab_keywords_create();

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
 * Destroys whole symbol table
 * @param symtab Symbol table to destroy.
 */
void symtab_free(symtab_t* symtab);

/**
 * Destroys all subtables and keywords table of symbol table.
 * @param symtab Symbol table to clear of subtables.
 */
void symtab_clear(symtab_t* symtab);

/**
 * Destroys whole subtable.
 * @param subtab Subtable to destroy.
 */
void symtab_subtab_free(symtab_subtab_t* subtab);

/**
 * Destroys all records of the subtable.
 * @param subtab Subtable to clear of records.
 */
void symtab_subtab_clear(symtab_subtab_t* subtab);

/**
 * Destroys identifier record.
 * @param rec Record to destroy.
 */
void symtab_record_free(symtab_record_t* rec);

// MANIPULATION WITH SUBSYMTABLES FUNCTIONS

/**
 * Creates and pushes subtable on the top of the stack of symbol table.
 * @param symtab Symbol table to push on.
 * @return True if successful. False otherwise.
 */
bool symtab_subtab_push(symtab_t* symtab);

/**
 * Pops and destroys subtable on the top of the stack of symbol table.
 * @param symtab Symbol table to pop from.
 */
void symtab_subtab_pop(symtab_t* symtab);

// MANIPULATION WITH RECORDS FUNCTIONS

/**
 * Searches whole stack of subtables for identifier.
 * @param symtab Symbol table to search on.
 * @param key Key to search for.
 * @return Found record. NULL otherwise.
 */
symtab_pair_t* symtab_find(const symtab_t* symtab, symtab_key_t key);

/**
 * Searches topmost subtable on the stack for identifier.
 * @param symtab Symbol table to search on.
 * @param key Key to search for.
 * @return Found record. NULL otherwise.
 */
symtab_pair_t* symtab_top_find(const symtab_t* symtab, symtab_key_t key);

/**
 * Searches keywords table for keyword.
 * @param symtab Symbol table to search on.
 * @param key Key to search for.
 * @return Found record. NULL otherwise.
 */
symtab_pair_t* symtab_keyword_find(const symtab_t* symtab, symtab_key_t key);

/**
 * Creates and inserts new record in topmost subtable (most nested scope).
 * @param symtab Symbol table to instert on.
 * @param key Key of the new record.
 * @return Created record. NULL if failed to create.
 */
symtab_pair_t* symtab_insert(symtab_t* symtab, symtab_key_t key);

/**
 * Searches subtable for identifier.
 * @param subtab Subtable to search on.
 * @param key Key to search for.
 * @return Found record. NULL otherwise.
 */
symtab_pair_t* symtab_subtab_find(const symtab_subtab_t* subtab, symtab_key_t key);

/**
 * Creates and inserts new record in subtable.
 * @param subtab Subtable to instert on.
 * @param key Key of the new record.
 * @return Created record. NULL if failed to create.
 */
symtab_pair_t* symtab_subtab_insert(symtab_subtab_t* subtab, symtab_key_t key);

/**
 * Executes function for each record in subtable.
 * @param subtab Subtable to operate on.
 * @param f Function to execute.
 */
void symtab_subtab_foreach(const symtab_subtab_t *subtab, void (*f)(symtab_pair_t *data));

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
// APPLIES TO SINGLE LINE ABOVE
// TAKEN FROM: http://www.azillionmonkeys.com/qed/hash.html
// COPYRIGHT: © Copyright 2004-2008 by Paul Hsieh
// LICENSE: GNU Lesser General Public License v2.1
// LICENSE TEXT: https://www.gnu.org/licenses/old-licenses/lgpl-2.1.txt
// CHANGES MADE: 01.10.2021 - changed parameter type and count; added local len variable
//--------------------------------------------------------------------------------------

#endif // __SYMTAB_H__

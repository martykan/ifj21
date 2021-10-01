/*
*IFJ project
*file: symtable.h
*date: 28.09.21
*author: Patrik Korytar, xkoryt04
*school: FIT VUT
*file description: symmbol table API
*/

#ifndef __SYMTAB_H__
#define __SYMTAB_H__

#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

// COMPILE TIME CONSTANTS for sizes of subtables
#define SYMTAB_BUCKET_COUNT 83
#define KEYWORDS_COUNT 15

typedef const char* symtab_key_t;

typedef struct {
  // TODO
  int x;
} symtab_value_t;

typedef struct {
  symtab_key_t    key;
  symtab_value_t  value;
} symtab_pair_t;

typedef struct symtab_record symtab_record_t;
struct symtab_record {
  symtab_record_t*  next; // next record
  symtab_pair_t     pair; // data of record
};

typedef struct subsymtab subsymtab_t;
struct subsymtab {
  subsymtab_t*      next;         // next subtab on stack
  size_t            bucket_cnt;   // bucket count
  size_t            records_cnt;  // number of records
  symtab_record_t*  list[];       // records
};

typedef struct
{
  subsymtab_t*  keywords;   // table of keywords
  subsymtab_t*  subtabs;    // tables of symbols
} symtab_t;

// INITIALIZATION
symtab_t* symtab_init();
subsymtab_t* symtab_keywords_init();
subsymtab_t* subsymtab_init(size_t n);
symtab_record_t* symtab_record_init(symtab_key_t key);


// DEALLOCATION
void symtab_free(symtab_t* symtab);
void symtab_clear(symtab_t* symtab);
void subsymtab_free(subsymtab_t* subtab);
void subsymtab_clear(subsymtab_t* subtab);
void symtab_record_free(symtab_record_t* rec);

// MANIPULATION WITH SUBSYMTABLES
bool push_subsymtab(symtab_t* symtab);
void pop_subsymtab(symtab_t* symtab);

// MANIPULATION WITH RECORDS
symtab_pair_t* symtab_find(const symtab_t* symtab, symtab_key_t key);
symtab_pair_t* symtab_top_find(const symtab_t* symtab, symtab_key_t key);
symtab_pair_t* symtab_keyword_find(const symtab_t* symtab, symtab_key_t key);
symtab_pair_t* symtab_insert(symtab_t* symtab, symtab_key_t key);
symtab_pair_t* subsymtab_find(const subsymtab_t* subtab, symtab_key_t key);
symtab_pair_t* subsymtab_insert(subsymtab_t* subtab, symtab_key_t key);
void subsymtab_for_each(const subsymtab_t *subtab, void (*f)(symtab_pair_t *data));
bool subsystam_erase(subsymtab_t* subtab, symtab_key_t key);

// HASH FUNCTION
uint32_t SuperFastHash(symtab_key_t data);

#endif // __SYMTAB_H__

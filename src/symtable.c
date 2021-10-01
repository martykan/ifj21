/*
*IFJ project
*file: symtable.c
*date: 28.09.21
*author: Patrik Korytar, xkoryt04
*school: FIT VUT
*file description: symmbol table implemented using hash table
*/

#include "symtable.h"

#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

// KEYWORDS ARRAY

static const char* keyword_ids[KEYWORDS_COUNT] = {
  "do",
  "else",
  "end",
  "function",
  "global",
  "if",
  "integer",
  "local",
  "nil",
  "number",
  "require",
  "return",
  "string",
  "then",
  "while"
};

// INITIALIZATION

symtab_t* symtab_init()
{
  symtab_t* symtab = malloc(sizeof(symtab_t));
  if(symtab == NULL)
    return NULL;

  symtab->keywords = symtab_keywords_init();
  if(!symtab->keywords) {
    free(symtab);
    return NULL;
  }

  symtab->subtabs = NULL;

  return symtab;
}

subsymtab_t* symtab_keywords_init()
{
  subsymtab_t* keywords = subsymtab_init(KEYWORDS_COUNT);
  if(!keywords)
    return NULL;

  for(int i = 0; i < KEYWORDS_COUNT; i++)
    subsymtab_insert(keywords, keyword_ids[i]);

  return keywords;
}

subsymtab_t* subsymtab_init(size_t n)
{
  subsymtab_t* subtab = malloc(sizeof(subsymtab_t) + n*sizeof(symtab_record_t *));
  if(subtab == NULL)
    return NULL;

  subtab->next = NULL;
  subtab->bucket_cnt = n;
  subtab->records_cnt = 0;
  for(unsigned i = 0; i < n; i++)
    subtab->list[i] = NULL;

  return subtab;
}

symtab_record_t* symtab_record_init(symtab_key_t key)
{
  symtab_record_t* rec = malloc(sizeof(symtab_record_t));
  if(rec == NULL)
    return NULL;

  char* new_key = malloc(strlen(key) + 1);
  if(new_key == NULL)
  {
    free(rec);
    return NULL;
  }
  strcpy(new_key, key);

  rec->next = NULL;
  rec->pair.key = new_key;
  rec->pair.value.x = 0;

  return rec;
}

// DEALLOCATION

void symtab_free(symtab_t* symtab)
{
  symtab_clear(symtab);
  free(symtab);
}

void symtab_clear(symtab_t* symtab)
{
  subsymtab_free(symtab->keywords);

  // delete first subtab while any left
  while(symtab->subtabs != NULL)
  {
      // next subtab after subtab now being deleted
      subsymtab_t* next = symtab->subtabs->next;
      subsymtab_free(symtab->subtabs);
      symtab->subtabs = next;
  }
}

void subsymtab_free(subsymtab_t* subtab)
{
  subsymtab_clear(subtab);
  free(subtab);
}

void subsymtab_clear(subsymtab_t* subtab)
{
  for(unsigned i = 0; i < subtab->bucket_cnt; i++)
  {
    // delete first record while any left
    while(subtab->list[i] != NULL)
    {
      // next record after record now being deleted
      symtab_record_t* next = subtab->list[i]->next;
      symtab_record_free(subtab->list[i]);
      subtab->list[i] = next;
    }
  }

  subtab->records_cnt = 0;

  return;
}

void symtab_record_free(symtab_record_t* rec)
{
  free((char *)rec->pair.key);
  free(rec);
}

// MANIPULATION WITH SUBSYMTABLES

bool push_subsymtab(symtab_t* symtab)
{
  subsymtab_t* subtab = subsymtab_init(SYMTAB_BUCKET_COUNT);
  if(subtab == NULL)
    return false;

  subtab->next = symtab->subtabs;
  symtab->subtabs = subtab;

  return true;
}

void pop_subsymtab(symtab_t* symtab)
{
  subsymtab_t* next = symtab->subtabs->next;
  subsymtab_free(symtab->subtabs);
  symtab->subtabs = next;
}

// MANIPULATION WITH RECORDS

symtab_pair_t* symtab_find(const symtab_t* symtab, symtab_key_t key)
{
  for(subsymtab_t* subtab = symtab->subtabs; subtab != NULL; subtab = subtab->next)
  {
    symtab_pair_t* pair = subsymtab_find(subtab, key);
    if(pair)
      return pair;
  }

  return NULL;
}

symtab_pair_t* symtab_top_find(const symtab_t* symtab, symtab_key_t key)
{
  return subsymtab_find(symtab->subtabs, key);
}

symtab_pair_t* symtab_keyword_find(const symtab_t* symtab, symtab_key_t key)
{
  return subsymtab_find(symtab->keywords, key);
}

symtab_pair_t* symtab_insert(symtab_t* symtab, symtab_key_t key)
{
  return subsymtab_insert(symtab->subtabs, key);
}

symtab_pair_t* subsymtab_find(const subsymtab_t* subtab, symtab_key_t key)
{
  size_t index = SuperFastHash(key) % subtab->bucket_cnt;

  for(symtab_record_t* rec = subtab->list[index]; rec != NULL; rec = rec->next)
  {
    if(!strcmp(rec->pair.key, key))
      return &rec->pair;
  }

  return NULL;
}

symtab_pair_t* subsymtab_insert(subsymtab_t* subtab, symtab_key_t key)
{
  size_t index = SuperFastHash(key) % subtab->bucket_cnt;

  symtab_record_t* last = NULL;
  for(symtab_record_t* rec = subtab->list[index]; rec != NULL; rec = rec->next)
  {
    if(!strcmp(rec->pair.key, key))
      return NULL;

    last = rec;
  }

  // key not found, create new record
  symtab_record_t* new_item = symtab_record_init(key);

  subtab->records_cnt++;
  if(!last) // if no record on current bucket
    subtab->list[index] = new_item;
  else // otherwise append
    last->next = new_item;

  return &new_item->pair;
}

void subsymtab_for_each(const subsymtab_t *subtab, void (*f)(symtab_pair_t *data))
{
  // cycle through all records of t structure and apply function f
  for(unsigned i = 0; i < subtab->bucket_cnt; i++)
    for(symtab_record_t* rec = subtab->list[i]; rec != NULL; rec = rec->next)
      (*f)(&rec->pair);

  return;
}

bool subsystam_erase(subsymtab_t* subtab, symtab_key_t key)
{
  size_t index = SuperFastHash(key) % subtab->bucket_cnt;

  symtab_record_t* last = NULL;
  for(symtab_record_t* rec = subtab->list[index]; rec != NULL; rec = rec->next)
  {
    if(!strcmp(rec->pair.key, key))
    {
      if(!last) // if no record on current bucket
        subtab->list[index] = rec->next;
      else
        last->next = rec->next;

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
// CHANGES MADE: 01.10.2021 - changed parameter type and count; added local len variable

#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
  || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif

#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
                       +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif

uint32_t SuperFastHash(symtab_key_t data)
{
  int len = strlen(data);
  uint32_t hash = len, tmp;
  int rem;

  if (len <= 0 || data == NULL) return 0;

  rem = len & 3;
  len >>= 2;

  /* Main loop */
  for (;len > 0; len--) {
    hash  += get16bits (data);
    tmp    = (get16bits (data+2) << 11) ^ hash;
    hash   = (hash << 16) ^ tmp;
    data  += 2*sizeof (uint16_t);
    hash  += hash >> 11;
  }

  /* Handle end cases */
  switch (rem) {
    case 3: hash += get16bits (data);
      hash ^= hash << 16;
      hash ^= ((signed char)data[sizeof (uint16_t)]) << 18;
      hash += hash >> 11;
      break;
    case 2: hash += get16bits (data);
      hash ^= hash << 11;
      hash += hash >> 17;
      break;
    case 1: hash += (signed char)*data;
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

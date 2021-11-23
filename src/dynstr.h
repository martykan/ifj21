/**
 * @file
 * @brief Dynamic string API
 * @author Patrik Korytar
 * @author Tomas Martykan
 * @author Filip Stolfa
 *
 * @section DESCRIPTION
 * Dynamic string interface. Used by other components
 * of compiler for easy string appending.
 *
 * @section IMPLEMENTATION
 * Dynamic string has a pointer to a character buffer, and it has
 * information about the current length of the string stored in the buffer,
 * as well as information about the current number of bytes allocated
 * for the buffer. It provides operations for appending unlimited number of
 * characters to the string, clearing the character buffer and copying the string
 * to another character buffer. Before use it has to be initialized and the character
 * buffer has to be freed before loosing access to the dynamic string.
 */

#ifndef __DYNSTR_H
#define __DYNSTR_H

#include <string.h>
#include <stdbool.h>

/**
 * @struct dynstr_t
 * @brief Dynamic string type.
 * @var dynstr_t::len
 * Current lenght of the dynamic string.
 * @var dynstr_t::alloced_bytes
 * Number of bytes allocated for the string buffer.
 * @var dynstr_t::str
 * String buffer.
 */
typedef struct {
  size_t len;
  size_t alloced_bytes;
  char *str;
} dynstr_t;


/** Initializes a dynamic string.
 *  Sets the info integers to their starter values, allocates the string buffer.
 *  Doesn't allocate the dynstr struct.
 *  @param dynstr Pointer to an existing dynstr struct to be initialized.
 *  @return The original dynstr pointer. NULL if failed to initialize.
 */
dynstr_t* dynstr_init(dynstr_t *dynstr);


/** Free the buffer of a dynamic string.
 *  Frees the allocated string buffer. Sets info ints to 0.
 *  Doesn't free the dynstr struct - needs to be handled by the caller, if necessary.
 *  @param dynstr Pointer to an existing dynstr struct.
 *  @return The original dynstr pointer. NULL if failed.
 */
dynstr_t* dynstr_free_buffer(dynstr_t *dynstr);


/** Clears the dynamic string.
 *  Sets the string buffer to an empty string. Sets len to 0.
 *  Doesn't change the size of the allocated string buffer.
 *  @param dynstr Pointer to an existing dynstr struct to be cleared.
 *  @return The original dynstr pointer. NULL if failed.
 */
dynstr_t* dynstr_clear(dynstr_t *dynstr);


/** Appends a character to a dynamic string.
 *  Adds a character at the end of the string buffer. Increments len.
 *  Automatically appends a terminating null char after the new character.
 *  If necessary, reallocates the string buffer to fit the new character.
 *  @param dynstr Pointer to an existing dynstr struct.
 *  @param c Character to append.
 *  @return The original dynstr pointer. NULL if failed.
 */
dynstr_t* dynstr_append(dynstr_t *dynstr, char c);


// TODO(filip): comment
dynstr_t* dynstr_append_esc(dynstr_t *dynstr, char c);


/** Makes a static copy of the string.
 *  Copies the characters from the dynstr string buffer into
 *  a new character buffer.
 *  The new buffer is allocated with the exact size needed.
 *  @param dynstr Pointer to an existing dynstr struct.
 *  @return Pointer to the new character buffer. NULL if failed.
 */
char* dynstr_copy_to_static(dynstr_t *dynstr);


/** Compares a dynamic and classic string.
 *  Only checks if the strings are equal. Doesn't check
 *  which non-matching character has a lower value.
 *  @param dynstr Pointer to an existing dynstr struct.
 *  @param str Pointer to a valid c string. Can't be NULL.
 *  @return True if strings are equal, false otherwise.
 */
bool dynstr_equals_static(dynstr_t *dynstr, const char *str);


/** Converts a dynstr_t string value to an integer value.
 *  Uses strtol to convert the string stored in the passed dynstr_t
 *  into an integer value. If the string doesn't represent a valid integer,
 *  sets the global error flag.
 *  @return Integer value converted from passed string, or -1 if no conversion was possible.
 */
int dynstr_to_int(dynstr_t *dynstr);

/** Converts a dynstr_t string value to a floating point value.
 *  Uses strtod to convert the string stored in the passed dynstr_t
 *  into a double value. If the string doesn't represent a valid double,
 *  sets the global error flag.
 *  @return Double value converted from passed string, or -1.0 if no conversion was possible.
 */
double dynstr_to_double(dynstr_t *dynstr);


#endif

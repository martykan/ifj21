/**
 * @file
 * @brief Recursive parser API
 * @author Patrik Korytar
 * @author Tomas Martykan
 * @author Filip Stolfa
 *
 * @section DESCRIPTION
 *  Top-down parser implemented using recursive technique.
 *  Analyzes whole stream of tokens, except expressions,
 *  and their order.
 */

#ifndef __REC_PARSER_H__
#define __REC_PARSER_H__

#include <stdbool.h>

/**
 * Starts recursive parsing process.
 * Recursive parser automatically launches
 * expression parser when needed.
 * @return True if program is syntactically and semantically
 *  correct. False otherwise.
 */
bool parser_start();

#endif  // __REC_PARSER_H__

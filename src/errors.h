/**
 * @file
 * @brief Error codes definition
 * @author Patrik Korytar
 * @author Tomas Martykan
 * @author Filip Stolfa
 */

/** Normal exit */
#define EXITSTATUS_OK 0
/** Wrong structure of lexeme */
#define EXITSTATUS_ERROR_LEXICAL 1
/** Invalid syntax */
#define EXITSTATUS_ERROR_SYNTAX 2
/** Semantic error - undefined function/variable, redefinition of variable */
#define EXITSTATUS_ERROR_SEMANTIC_IDENTIFIER 3
/** Semantic error - type incompatibility in assignment */
#define EXITSTATUS_ERROR_SEMANTIC_ASSIGNMENT 4
/** Semantic error - incorrect count/type of parameters or return values of function call */
#define EXITSTATUS_ERROR_SEMANTIC_FUN_PARAMETERS 5
/** Semantic error - type incompatibility in arithmetic, string or relation expressions */
#define EXITSTATUS_ERROR_SEMANTIC_TYPE_EXPR 6
/** Semantic error - other */
#define EXITSTATUS_ERROR_SEMANTIC_OTHER 7
/** Runtime error - working with unexpected nil value */
#define EXITSTATUS_ERROR_UNEXPECTED_NIL 8
/** Runtime error - dividing by zero constant */
#define EXITSTATUS_ERROR_DIVIDE_ZERO 9
/** Internal compiler error (ex. memory allocation error) */
#define EXITSTATUS_INTERNAL_ERROR 99

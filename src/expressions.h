/**
 * @file
 * @brief EPREC_UNDEF, pression parser API
 * @author Tomas Martykan
 * @author Patrik Korytar
 * @author Filip Stolfa
 */

typedef enum
{
    PREC_UNDEF,
    PREC_EQ,
    PREC_GT,
    PREC_GTE,
    PREC_LT,
    PREC_LTE,
} expression_precedence_t;

typedef enum
{
    SYM_PLUS,
    SYM_MINUS,
    SYM_TIMES,
    SYM_DIVIDE,
    SYM_DIVIDE2,
    SYM_DOTDOT,
    SYM_GT,
    SYM_GTE,
    SYM_EQ,
    SYM_NEQ,
    SYM_LTE,
    SYM_LT,
    SYM_LBRACKET,
    SYM_RBRACKET,
    SYM_I,
    SYM_S,
    SYM_E,
    SYM_PREC_GT,
    SYM_PREC_LT,
} expression_symbol_t;

char *expression_symbol_t_names[] = {
    "SYM_PLUS",
    "SYM_MINUS",
    "SYM_TIMES",
    "SYM_DIVIDE",
    "SYM_DIVIDE2",
    "SYM_DOTDOT",
    "SYM_GT",
    "SYM_GTE",
    "SYM_EQ",
    "SYM_NEQ",
    "SYM_LTE",
    "SYM_LT",
    "SYM_LBRACKET",
    "SYM_RBRACKET",
    "SYM_I",
    "SYM_S",
    "SYM_E",
    "SYM_PREC_GT",
    "SYM_PREC_LT",
};

const expression_precedence_t precedence_table[8][8] = {
    {
        PREC_GT,
        PREC_GT,
        PREC_GT,
        PREC_GT,
        PREC_LT,
        PREC_GT,
        PREC_LT,
        PREC_GT,
    },
    {
        PREC_LT,
        PREC_GT,
        PREC_GT,
        PREC_GT,
        PREC_LT,
        PREC_GT,
        PREC_LT,
        PREC_GT,
    },
    {
        PREC_LT,
        PREC_LT,
        PREC_GT,
        PREC_GT,
        PREC_LT,
        PREC_GT,
        PREC_LT,
        PREC_GT,
    },
    {
        PREC_LT,
        PREC_LT,
        PREC_LT,
        PREC_GT,
        PREC_LT,
        PREC_GT,
        PREC_LT,
        PREC_GT,
    },
    {
        PREC_LT,
        PREC_LT,
        PREC_LT,
        PREC_LT,
        PREC_LT,
        PREC_EQ,
        PREC_LT,
        PREC_UNDEF,
    },
    {
        PREC_GT,
        PREC_GT,
        PREC_GT,
        PREC_GT,
        PREC_UNDEF,
        PREC_GT,
        PREC_UNDEF,
        PREC_GT,
    },
    {
        PREC_GT,
        PREC_GT,
        PREC_GT,
        PREC_GT,
        PREC_UNDEF,
        PREC_GT,
        PREC_UNDEF,
        PREC_GT,
    },
    {
        PREC_LT,
        PREC_LT,
        PREC_LT,
        PREC_LT,
        PREC_LT,
        PREC_UNDEF,
        PREC_LT,
        PREC_UNDEF,
    },
};

typedef struct symbol_stack_t symbol_stack_t;
struct symbol_stack_t
{
    expression_symbol_t symbol;
    symbol_stack_t *next;
};

int expression_process(expression_symbol_t *input);

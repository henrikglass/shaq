#ifndef PARSER_H
#define PARSER_H

/*--- Include files ---------------------------------------------------------------------*/

#include "lexer.h"

/*--- Public macros ---------------------------------------------------------------------*/

/*--- Public type definitions -----------------------------------------------------------*/

typedef enum
{
    ADD,
    SUB,
    MUL,
    DIV,
    REM,
    NEG,
    PAREN,
    FUNC,
    ARGLIST,
    LIT,
    CONST,
} ExprKind;

/**
 * An expression may be either binary, and have two children `lhs` and `rhs`, unary, 
 * and have a single child `child`, or atomic, and have no children. 
 *
 * Whether an expression is unary, binary, or atomic is determined by its `kind`. E.g. 
 * `ADD`, `MUL`, and `REM` are binary operations, and thus binary expressions; `FUNC` 
 * and `PAREN` are unary expressions; `LIT` is atomic.
 */
typedef struct Expr
{
    ExprKind kind;
    Token token;
    union {
        struct Expr *child;
        struct Expr *lhs;
    };
    struct Expr *rhs;
} Expr;

/*--- Public variables ------------------------------------------------------------------*/

/*--- Public function prototypes --------------------------------------------------------*/

Expr *parse_expr(Lexer *l);
void print_expr(Expr *e);
void print_expr_tree(Expr *e);

#endif /* PARSER_H */


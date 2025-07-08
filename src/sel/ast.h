
#ifndef AST_H
#define AST_H

/*--- Include files ---------------------------------------------------------------------*/

#include "sel/lexer.h"

/*--- Public macros ---------------------------------------------------------------------*/

/*--- Public type definitions -----------------------------------------------------------*/

typedef enum
{
    EXPR_ADD,
    EXPR_SUB,
    EXPR_MUL,
    EXPR_DIV,
    EXPR_REM,
    EXPR_NEG,
    EXPR_PAREN,
    EXPR_FUNC,
    EXPR_ARGLIST,
    EXPR_LIT,
    EXPR_CONST,
} ExprKind;

typedef enum
{
    TYPE_NIL = 0,
    TYPE_BOOL,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_BVEC2,
    TYPE_BVEC3,
    TYPE_BVEC4,
    TYPE_VEC2,
    TYPE_VEC3,
    TYPE_VEC4,
    TYPE_IVEC2,
    TYPE_IVEC3,
    TYPE_IVEC4,
    TYPE_MAT2,
    TYPE_MAT3,
    TYPE_MAT4,
    TYPE_IMAGE,
    TYPECHECKER_ERROR,
    NAMECHECKER_ERROR,
} Type;

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
    Type type;
    Token token;
    union {
        struct Expr *child;
        struct Expr *lhs;
    };
    struct Expr *rhs;
} Expr;

/*--- Public variables ------------------------------------------------------------------*/

/*--- Public function prototypes --------------------------------------------------------*/

#endif /* AST_H */


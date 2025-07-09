#ifndef CODEGEN_H
#define CODEGEN_H

/*--- Include files ---------------------------------------------------------------------*/

#include "ast.h"

#include "hgl_int.h"
#include "hgl_float.h"
#include "hgl_da.h"

/*--- Public macros ---------------------------------------------------------------------*/

/*--- Public type definitions -----------------------------------------------------------*/

typedef enum
{
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_REM,
    OP_NEG,
    OP_FUNC,
} OpKind;

typedef struct
{
    OpKind kind;
    Type type;
} Op;

typedef struct
{
    Op *arr;
    u32 count;
    u32 capacity;
} Ops;

typedef struct
{
    u8 *arr;
    u32 count;
    u32 capacity;
} Values;

/* "executable" expression */
typedef struct
{
    Ops ops;
    Values vals;
} EExpr;

/*--- Public variables ------------------------------------------------------------------*/

/*--- Public function prototypes --------------------------------------------------------*/

EExpr compile(const Expr *e);

#endif /* CODEGEN_H */


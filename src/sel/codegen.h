#ifndef CODEGEN_H
#define CODEGEN_H

/*--- Include files ---------------------------------------------------------------------*/

#include "sel/ast.h"

#include "hgl_int.h"
#include "hgl_float.h"
#include "hgl_da.h"

/*--- Public macros ---------------------------------------------------------------------*/

/*--- Public type definitions -----------------------------------------------------------*/

typedef enum
{
    OP_PUSH,
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
    u8 kind;
    u8 type;
    u8 argsize;
    u8 pad[1];
} Op;
static_assert(sizeof(Op) == 4, "");

/* "executable" expression */
typedef struct
{
    u8 *code;
    u32 size;
    u32 capacity;
    Type type;
} EExpr;

/*--- Public variables ------------------------------------------------------------------*/

/*--- Public function prototypes --------------------------------------------------------*/

EExpr codegen(const Expr *e);

#endif /* CODEGEN_H */


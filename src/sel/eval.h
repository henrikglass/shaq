#ifndef EVAL_H
#define EVAL_H

/*--- Include files ---------------------------------------------------------------------*/

#include "sel/codegen.h"

/*--- Public macros ---------------------------------------------------------------------*/

/*--- Public type definitions -----------------------------------------------------------*/

typedef struct
{
    const EExpr *exe;
    u8 *stack;
    u32 pc;
    u32 sp;
} ExeCtx;

/*--- Public variables ------------------------------------------------------------------*/

/*--- Public function prototypes --------------------------------------------------------*/

ExeCtx make_execution_context(const EExpr *exe);
void reset_execution_context(ExeCtx *ctx);
void *eval(ExeCtx *ctx);

#endif /* EVAL_H */


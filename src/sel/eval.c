/*--- Include files ---------------------------------------------------------------------*/

#include "eval.h"

#include "alloc.h"

/*--- Private macros --------------------------------------------------------------------*/

#define STACK_SIZE 4096

/*--- Private type definitions ----------------------------------------------------------*/

/*--- Private function prototypes -------------------------------------------------------*/

static inline void *next_bytes(ExeCtx *ctx, u32 size);
static inline Op *next_op(ExeCtx *ctx);
static inline void stack_push(ExeCtx *ctx, void *data, u32 size);
static inline void *stack_pop(ExeCtx *ctx, u32 size);

static inline i32 addi(i32 *lhs, i32 *rhs);
static inline f32 addf(f32 *lhs, f32 *rhs);
static inline i32 subi(i32 *lhs, i32 *rhs);
static inline f32 subf(f32 *lhs, f32 *rhs);
static inline i32 muli(i32 *lhs, i32 *rhs);
static inline f32 mulf(f32 *lhs, f32 *rhs);
static inline i32 divi(i32 *lhs, i32 *rhs);
static inline f32 divf(f32 *lhs, f32 *rhs);
static inline i32 remi(i32 *lhs, i32 *rhs);
static inline i32 negi(i32 *val);
static inline f32 negf(f32 *val);

/*--- Public variables ------------------------------------------------------------------*/

/*--- Private variables -----------------------------------------------------------------*/

// TODO use same allocator as codegen
static HglArena bajs_allocator = HGL_ARENA_INITIALIZER(1024*1024);

/*--- Public functions ------------------------------------------------------------------*/

ExeCtx make_execution_context(const EExpr *exe)
{
    return (ExeCtx) {
        .exe = exe,
        .stack = arena_alloc(&bajs_allocator, STACK_SIZE),
        .pc = 0,
        .sp = 0,
    };
}

void reset_execution_context(ExeCtx *ctx)
{
    ctx->pc = 0;
    ctx->sp = 0;
}

void *eval(ExeCtx *ctx)
{
    while (true) {

        /* end-of-program */
        if (ctx->pc >= ctx->exe->size) {
            break;
        }

        Op *op = next_op(ctx);
        u32 tsize = TYPE_TO_SIZE[op->type];
        
        switch (op->kind) {
            case OP_PUSH: {
                stack_push(ctx, next_bytes(ctx, op->argsize), op->argsize);
            } break;

            case OP_ADD: {
                void *rhs = stack_pop(ctx, tsize);
                void *lhs = stack_pop(ctx, tsize);
                switch (op->type) {
                    case TYPE_INT: {i32 tmp = addi(lhs, rhs); stack_push(ctx, &tmp, sizeof(tmp));} break;
                    case TYPE_FLOAT: {f32 tmp = addf(lhs, rhs); stack_push(ctx, &tmp, sizeof(tmp));} break;
                    default: assert(false);
                }
            } break;

            case OP_SUB: {
                void *rhs = stack_pop(ctx, tsize);
                void *lhs = stack_pop(ctx, tsize);
                switch (op->type) {
                    case TYPE_INT: {i32 tmp = subi(lhs, rhs); stack_push(ctx, &tmp, sizeof(tmp));} break;
                    case TYPE_FLOAT: {f32 tmp = subf(lhs, rhs); stack_push(ctx, &tmp, sizeof(tmp));} break;
                    default: assert(false);
                }
            } break;

            case OP_MUL: {
                void *rhs = stack_pop(ctx, tsize);
                void *lhs = stack_pop(ctx, tsize);
                switch (op->type) {
                    case TYPE_INT: {i32 tmp = muli(lhs, rhs); stack_push(ctx, &tmp, sizeof(tmp));} break;
                    case TYPE_FLOAT: {f32 tmp = mulf(lhs, rhs); stack_push(ctx, &tmp, sizeof(tmp));} break;
                    default: assert(false);
                }
            } break;

            case OP_DIV: {
                void *rhs = stack_pop(ctx, tsize);
                void *lhs = stack_pop(ctx, tsize);
                switch (op->type) {
                    case TYPE_INT: {i32 tmp = divi(lhs, rhs); stack_push(ctx, &tmp, sizeof(tmp));} break;
                    case TYPE_FLOAT: {f32 tmp = divf(lhs, rhs); stack_push(ctx, &tmp, sizeof(tmp));} break;
                    default: assert(false);
                }
            } break;

            case OP_REM: {
                void *rhs = stack_pop(ctx, tsize);
                void *lhs = stack_pop(ctx, tsize);
                switch (op->type) {
                    case TYPE_INT: {i32 tmp = remi(lhs, rhs); stack_push(ctx, &tmp, sizeof(tmp));} break;
                    default: assert(false);
                }
            } break;

            case OP_NEG: {
                void *val = stack_pop(ctx, tsize);
                switch (op->type) {
                    case TYPE_INT: {i32 tmp = negi(val); stack_push(ctx, &tmp, sizeof(tmp));} break;
                    case TYPE_FLOAT: {i32 tmp = negf(val); stack_push(ctx, &tmp, sizeof(tmp));} break;
                    default: assert(false);
                }
            } break;

            case OP_FUNC: {
                assert(false);
            } break;
        }
    }

    assert(ctx->sp - TYPE_TO_SIZE[ctx->exe->type] == 0);
    assert(ctx->pc == ctx->exe->size);
    void *res = &ctx->stack[0];
    reset_execution_context(ctx); 
    return res;
}

/*--- Private functions -----------------------------------------------------------------*/

static inline void *next_bytes(ExeCtx *ctx, u32 size)
{
    void *data = (void *)&ctx->exe->code[ctx->pc]; 
    ctx->pc += size;
    return data;
}

static inline Op *next_op(ExeCtx *ctx)
{
    Op *op = (Op *)&ctx->exe->code[ctx->pc];
    ctx->pc += sizeof(Op);
    return op;
}

static inline void stack_push(ExeCtx *ctx, void *data, u32 size)
{
    if (ctx->sp + size > STACK_SIZE) {
        assert(false && "out of stack space");
    }
    memcpy(&ctx->stack[ctx->sp], data, size);
    ctx->sp += size;
}

static inline void *stack_pop(ExeCtx *ctx, u32 size)
{
    if (ctx->sp < size) {
        assert(false && "popping below stack boundary");
    }
    ctx->sp -= size;
    return &ctx->stack[ctx->sp];
}

static inline i32 addi(i32 *lhs, i32 *rhs) { return (*lhs) + (*rhs); }
static inline f32 addf(f32 *lhs, f32 *rhs) { return (*lhs) + (*rhs); }
static inline i32 subi(i32 *lhs, i32 *rhs) { return (*lhs) - (*rhs); }
static inline f32 subf(f32 *lhs, f32 *rhs) { return (*lhs) - (*rhs); }
static inline i32 muli(i32 *lhs, i32 *rhs) { return (*lhs) * (*rhs); }
static inline f32 mulf(f32 *lhs, f32 *rhs) { return (*lhs) * (*rhs); }
static inline i32 divi(i32 *lhs, i32 *rhs) { return (*lhs) / (*rhs); }
static inline f32 divf(f32 *lhs, f32 *rhs) { return (*lhs) / (*rhs); }
static inline i32 remi(i32 *lhs, i32 *rhs) { return (*lhs) % (*rhs); }
static inline i32 negi(i32 *val) { return -(*val); }
static inline f32 negf(f32 *val) { return -(*val); }



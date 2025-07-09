/*--- Include files ---------------------------------------------------------------------*/

#include "codegen.h"

#include "sel/builtins.h"
#include "alloc.h"

#include <assert.h>

/*--- Private macros --------------------------------------------------------------------*/

/*--- Private type definitions ----------------------------------------------------------*/

/*--- Private function prototypes -------------------------------------------------------*/

void compile_expr(EExpr *exe, const Expr *e);

void push_op(Ops *ops, Op op);
void push_val(Values *vals, const void *val, u32 size);

/*--- Public variables ------------------------------------------------------------------*/

/*--- Private variables -----------------------------------------------------------------*/

static HglArena ops_allocator  = HGL_ARENA_INITIALIZER(1024*1024);
static HglArena vals_allocator = HGL_ARENA_INITIALIZER(1024*1024);

/*--- Public functions ------------------------------------------------------------------*/

EExpr compile(const Expr *e)
{
    EExpr exe = {
        .ops = {0},
        .vals = {0},
    };

    compile_expr(&exe, e);

    return exe;
}

/*--- Private functions -----------------------------------------------------------------*/

void compile_expr(EExpr *exe, const Expr *e)
{
    static OpKind expr_to_op[] = {
        [EXPR_ADD]  = OP_ADD,
        [EXPR_SUB]  = OP_SUB,
        [EXPR_MUL]  = OP_MUL,
        [EXPR_DIV]  = OP_DIV,
        [EXPR_REM]  = OP_REM,
        [EXPR_NEG]  = OP_NEG,
        [EXPR_FUNC] = OP_FUNC,
    };


    switch (e->kind) {
        case EXPR_ADD:
        case EXPR_SUB:
        case EXPR_MUL:
        case EXPR_DIV:
        case EXPR_REM: {
            compile_expr(exe, e->lhs);
            compile_expr(exe, e->rhs);
            printf("push op: %d\n", expr_to_op[e->kind]);
            push_op(&exe->ops, (Op){expr_to_op[e->kind], e->type});
        } break;

        case EXPR_NEG: {
            compile_expr(exe, e->child);
            printf("push op: %d\n", expr_to_op[e->kind]);
            push_op(&exe->ops, (Op){OP_NEG, e->type});
        } break;

        case EXPR_PAREN: {
            compile_expr(exe, e->child);
        } break;

        case EXPR_FUNC: {
            assert(false && "wallahi");
            compile_expr(exe, e->child);
        } break;

        case EXPR_ARGLIST: {
            assert(false && "wallaha");
        } break;

        case EXPR_LIT: {
            if (e->type == TYPE_INT) {
                i32 val = (i32) hgl_sv_to_i64(e->token.text);
                printf("push int: %d\n", val);
                push_val(&exe->vals, &val, sizeof(i32));
            } else if (e->type == TYPE_FLOAT) {
                f32 val = (f32) hgl_sv_to_f64(e->token.text);
                printf("push float: %f\n", (f64)val);
                push_val(&exe->vals, &val, sizeof(f32));
            } else {
                assert(false && "wallaho");
            }
        } break;

        case EXPR_CONST: {
            for (size_t i = 0; i < N_BUILTIN_CONSTANTS; i++) {
                if (hgl_sv_equals(e->token.text, BUILTIN_CONSTANTS[i].id)) {
                    f32 val = BUILTIN_CONSTANTS[i].value;
                    printf("push float: %f\n", (f64)val);
                    push_val(&exe->vals, &val, sizeof(f32));
                    break; 
                } 
            }
        } break;

    } 

    return; 
}

void push_op(Ops *ops, Op op)
{
    if (ops->arr == NULL) {
        ops->count = 0;
        ops->capacity = 16;
        ops->arr = hgl_stack_alloc(&ops_allocator, ops->capacity * sizeof(*ops->arr));
    } 
    if (ops->capacity < ops->count + 1) {
        ops->capacity *= 2;
        ops->arr = hgl_stack_realloc(&ops_allocator, ops->arr, ops->capacity * sizeof(*ops->arr));
    }
    assert(ops->arr != NULL && "ops_allocator alloc failed");
    ops->arr[ops->count++] = op;
}

void push_val(Values *vals, const void *val, u32 size)
{
    if (vals->arr == NULL) {
        vals->count = 0;
        vals->capacity = 64;
        vals->arr = hgl_stack_alloc(&vals_allocator, vals->capacity * sizeof(*vals->arr));
    } 
    if (vals->capacity < vals->count + size) {
        while (vals->capacity < vals->count + size) {
            vals->capacity *= 2;
        }
        vals->arr = hgl_stack_realloc(&vals_allocator, vals->arr, vals->capacity * sizeof(*vals->arr));
    }
    assert(vals->arr != NULL && "vals_allocator alloc failed");
    memcpy(&vals->arr[vals->count], val, size);
    vals->count += size;
}



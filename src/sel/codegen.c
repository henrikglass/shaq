/*--- Include files ---------------------------------------------------------------------*/

#include "codegen.h"

#include "sel/builtins.h"
#include "alloc.h"

#include <assert.h>

/*--- Private macros --------------------------------------------------------------------*/

/*--- Private type definitions ----------------------------------------------------------*/

/*--- Private function prototypes -------------------------------------------------------*/

void codegen_expr(EExpr *exe, const Expr *e);

void push_op(EExpr *exe, Op op);
void push_i32(EExpr *exe, i32 v);
void push_f32(EExpr *exe, f32 v);
void push(EExpr *exe, const void *val, u32 size);

/*--- Public variables ------------------------------------------------------------------*/

/*--- Private variables -----------------------------------------------------------------*/

static HglArena eexpr_allocator = HGL_ARENA_INITIALIZER(1024*1024);

/*--- Public functions ------------------------------------------------------------------*/

EExpr codegen(const Expr *e)
{
    EExpr exe = {0};

    codegen_expr(&exe, e);

    return exe;
}

/*--- Private functions -----------------------------------------------------------------*/

void codegen_expr(EExpr *exe, const Expr *e)
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
            codegen_expr(exe, e->lhs);
            codegen_expr(exe, e->rhs);
            push_op(exe, (Op){
                .kind = expr_to_op[e->kind], 
                .type = e->type,
            });
            printf("ARITH: %d\n", expr_to_op[e->kind]);
        } break;

        case EXPR_NEG: {
            codegen_expr(exe, e->child);
            push_op(exe, (Op){
                .kind = OP_NEG, 
                .type = e->type,
            });
            printf("NEG: %d\n", expr_to_op[e->kind]);
        } break;

        case EXPR_PAREN: {
            codegen_expr(exe, e->child);
        } break;

        case EXPR_FUNC: {
            assert(false && "wallahi");
            codegen_expr(exe, e->child);
        } break;

        case EXPR_ARGLIST: {
            assert(false && "wallaha");
        } break;

        case EXPR_LIT: {
            if (e->type == TYPE_INT) {
                i32 val = (i32) hgl_sv_to_i64(e->token.text);
                push_op(exe, (Op){
                    .kind    = OP_PUSH,
                    .type    = e->type,
                    .argsize = sizeof(i32),
                });
                printf("PUSH: %d\n", val);
                push_i32(exe, val);
            } else if (e->type == TYPE_FLOAT) {
                f32 val = (f32) hgl_sv_to_f64(e->token.text);
                push_op(exe, (Op){
                    .kind    = OP_PUSH,
                    .type    = e->type,
                    .argsize = sizeof(f32),
                });
                printf("PUSH: %f\n", (f64)val);
                push_f32(exe, val);
            } else {
                assert(false && "wallaho");
            }
        } break;

        case EXPR_CONST: {
            for (size_t i = 0; i < N_BUILTIN_CONSTANTS; i++) {
                if (hgl_sv_equals(e->token.text, BUILTIN_CONSTANTS[i].id)) {
                    f32 val = BUILTIN_CONSTANTS[i].value;
                    push_op(exe, (Op){
                        .kind    = OP_PUSH,
                        .type    = e->type,
                        .argsize = sizeof(f32),
                    });
                    printf("PUSH: %f\n", (f64)val);
                    push_f32(exe, val);
                    break; 
                } 
            }
        } break;

        case N_EXPR_KINDS: {
            assert(false && "-");
        } break;

    } 

    return; 
}

void push_op(EExpr *exe, Op op)
{
    push(exe, &op, sizeof(op));
}

void push_i32(EExpr *exe, i32 v)
{
    push(exe, &v, sizeof(v));
}

void push_f32(EExpr *exe, f32 v)
{
    push(exe, &v, sizeof(v));
}

void push(EExpr *exe, const void *val, u32 size)
{
    if (exe->code == NULL) {
        exe->count = 0;
        exe->capacity = 64;
        exe->code = hgl_stack_alloc(&eexpr_allocator, exe->capacity * sizeof(*exe->code));
    } 
    if (exe->capacity < exe->count + size) {
        while (exe->capacity < exe->count + size) {
            exe->capacity *= 2;
        }
        exe->code = hgl_stack_realloc(&eexpr_allocator, exe->code, exe->capacity * sizeof(*exe->code));
    }
    assert(exe->code != NULL && "eexe_allocator alloc failed");
    memcpy(&exe->code[exe->count], val, size);
    exe->count += size;
}



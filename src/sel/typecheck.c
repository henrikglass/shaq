/*--- Include files ---------------------------------------------------------------------*/

#include "typecheck.h"
#include "builtins.h"

#include <stdio.h>

/*--- Private macros --------------------------------------------------------------------*/

#define TYPECHECK_ASSERT(cond, ...)                       \
    if (!(cond)) {                                        \
        fprintf(stderr, "Typecheck error. " __VA_ARGS__); \
        fprintf(stderr, "\n");                            \
        return TYPECHECKER_ERROR;                         \
    }

/*--- Private type definitions ----------------------------------------------------------*/

/*--- Private function prototypes -------------------------------------------------------*/

/*--- Public variables ------------------------------------------------------------------*/

/*--- Private variables -----------------------------------------------------------------*/

/*--- Public functions ------------------------------------------------------------------*/

Type typecheck(Expr *e)
{
    if (e == NULL) {
        return TYPE_NIL;
    }

    switch (e->kind) {
        
        case EXPR_ADD:
        case EXPR_SUB:
        case EXPR_MUL:
        case EXPR_DIV: {
            Type t0 = typecheck(e->lhs); 
            Type t1 = typecheck(e->rhs); 
            TYPECHECK_ASSERT(t0 == t1, "Operands to arithmetic operation are of different types.");
            return t0;
        } break;
        
        case EXPR_REM: {
            Type t0 = typecheck(e->lhs); 
            Type t1 = typecheck(e->rhs); 
            TYPECHECK_ASSERT(t0 == TYPE_INT, "Left-hand-side operand to remainder operation is not an INT.");
            TYPECHECK_ASSERT(t1 == TYPE_INT, "Right-hand-side operand to remainder operation is not an INT.");
            return TYPE_INT;
        } break;
        
        case EXPR_NEG: {
            // TODO better rule
            Type t0 = typecheck(e->child);
            TYPECHECK_ASSERT(t0 == TYPE_INT || t0 == TYPE_FLOAT , "Operand to unary minus operator must be of type INT or FLOAT");
        } break;
        
        case EXPR_PAREN: {
            return typecheck(e->child);
        } break;
        
        case EXPR_FUNC: {
            for (size_t i = 0; i < N_BUILTIN_FUNCTIONS; i++) {
                if (hgl_sv_equals(e->token.text, BUILTIN_FUNCTIONS[i].id)) {
                    // TODO typecheck argslist
                    return BUILTIN_FUNCTIONS[i].type;
                } 
            }
            return NAMECHECKER_ERROR;
        } break;
        
        case EXPR_ARGLIST: {
            TYPECHECK_ASSERT(false, "You should not see this.");
        } break;
        
        case EXPR_LIT: {
            if (e->token.kind == TOK_FLOAT_LITERAL) {
                return TYPE_FLOAT;
            } else if (e->token.kind == TOK_INT_LITERAL) {
                return TYPE_INT;
            } else {
                return TYPECHECKER_ERROR;
            }
        } break;
        
        case EXPR_CONST: {
            for (size_t i = 0; i < N_BUILTIN_CONSTANTS; i++) {
                if (hgl_sv_equals(e->token.text, BUILTIN_CONSTANTS[i].id)) {
                    return TYPE_FLOAT;
                } 
            }
            return NAMECHECKER_ERROR;
        } break;
    }

    return TYPECHECKER_ERROR;
}

/*--- Private functions -----------------------------------------------------------------*/


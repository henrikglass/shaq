/*--- Include files ---------------------------------------------------------------------*/

#include "typecheck.h"
#include "builtins.h"

#include <stdio.h>

/*--- Private macros --------------------------------------------------------------------*/

#define NAMECHECK_ERROR(...)                              \
    do {                                                  \
        fprintf(stderr, "Namecheck error: " __VA_ARGS__); \
        fprintf(stderr, "\n");                            \
        return NAME_ERROR_;                               \
    } while (0)

#define TYPECHECK_ERROR(...)                              \
    do {                                                  \
        fprintf(stderr, "Typecheck error: " __VA_ARGS__); \
        fprintf(stderr, "\n");                            \
        return TYPE_ERROR_;                               \
    } while (0)

#define TYPECHECK_ASSERT(cond, ...)                       \
    if (!(cond)) {                                        \
        fprintf(stderr, "Typecheck error: " __VA_ARGS__); \
        fprintf(stderr, "\n");                            \
        return TYPE_ERROR_;                               \
    }

/*--- Private type definitions ----------------------------------------------------------*/

/*--- Private function prototypes -------------------------------------------------------*/

static Type typecheck_argslist(Expr *e, const HglStringView *func_id, const Type *argtypes);

/*--- Public variables ------------------------------------------------------------------*/

/*--- Private variables -----------------------------------------------------------------*/

/*--- Public functions ------------------------------------------------------------------*/

Type typecheck(Expr *e)
{
    Type t0, t1;

    if (e == NULL) {
        return TYPE_NIL;
    }

    switch (e->kind) {
        
        case EXPR_ADD:
        case EXPR_SUB:
        case EXPR_MUL:
        case EXPR_DIV: {
            t0 = typecheck(e->lhs); 
            t1 = typecheck(e->rhs); 
            TYPECHECK_ASSERT(t0 == t1, "Operands to arithmetic operation are of different types: "
                             "Got `%s` and `%s`.", TYPE_TO_STR[t0], TYPE_TO_STR[t1]);
            return t0;
        } break;
        
        case EXPR_REM: {
            t0 = typecheck(e->lhs); 
            t1 = typecheck(e->rhs); 
            TYPECHECK_ASSERT(t0 == TYPE_INT, "Left-hand-side operand to remainder operation is not an INT.");
            TYPECHECK_ASSERT(t1 == TYPE_INT, "Right-hand-side operand to remainder operation is not an INT.");
            return t0;
        } break;
        
        case EXPR_NEG: {
            // TODO better rule
            t0 = typecheck(e->child);
            TYPECHECK_ASSERT(t0 == TYPE_INT || t0 == TYPE_FLOAT , "Operand to unary minus operator must be of type INT or FLOAT");
            return t0;
        } break;
        
        case EXPR_PAREN: {
            return typecheck(e->child);
        } break;
        
        case EXPR_FUNC: {
            for (size_t i = 0; i < N_BUILTIN_FUNCTIONS; i++) {
                if (hgl_sv_equals(e->token.text, BUILTIN_FUNCTIONS[i].id)) {
                    t0 = typecheck_argslist(e->child, &e->token.text, BUILTIN_FUNCTIONS[i].argtypes);
                    TYPECHECK_ASSERT(t0 == TYPE_NIL, 
                                     "Arguments to built-in function `" HGL_SV_FMT "(..)` "
                                     "does not match its signature.", HGL_SV_ARG(e->token.text));
                    return BUILTIN_FUNCTIONS[i].type;
                } 
            }
            NAMECHECK_ERROR("No such function: `" HGL_SV_FMT "(..)`", HGL_SV_ARG(e->token.text));
        } break;
        
        case EXPR_ARGLIST: {
            TYPECHECK_ASSERT(false, "You should not see this #1");
        } break;
        
        case EXPR_LIT: {
            if (e->token.kind == TOK_FLOAT_LITERAL) {
                return TYPE_FLOAT;
            } else if (e->token.kind == TOK_INT_LITERAL) {
                return TYPE_INT;
            } else {
                return TYPE_ERROR_;
            }
        } break;
        
        case EXPR_CONST: {
            for (size_t i = 0; i < N_BUILTIN_CONSTANTS; i++) {
                if (hgl_sv_equals(e->token.text, BUILTIN_CONSTANTS[i].id)) {
                    return TYPE_FLOAT;
                } 
            }
            NAMECHECK_ERROR("No such constant: `" HGL_SV_FMT "`", HGL_SV_ARG(e->token.text));
        } break;
    }

    return TYPE_ERROR_;
}

/*--- Private functions -----------------------------------------------------------------*/

static Type typecheck_argslist(Expr *e, const HglStringView *func_id, const Type *argtypes)
{
    /* no more actual arguments? */
    if (e == NULL) {
        TYPECHECK_ASSERT(*argtypes == TYPE_NIL, 
                         "Too few arguments to built-in function: `"
                          HGL_SV_FMT "(..)`", HGL_SV_ARG(*func_id));
        return TYPE_NIL;
    }

    /* no more arguments expected? */
    if (*argtypes == TYPE_NIL) {
        TYPECHECK_ERROR("Too many arguments to built-in function: `"
                         HGL_SV_FMT "(..)`", HGL_SV_ARG(*func_id));
        return TYPE_NIL;
    }

    /* Typecheck left-hand-side expression (head of argslist)*/
    TYPECHECK_ASSERT(e->kind == EXPR_ARGLIST, "You should not see this #2");
    Type t = typecheck(e->lhs);
    TYPECHECK_ASSERT(t == *argtypes, 
                     "Type mismatch in arguments to built-in function: `"
                      HGL_SV_FMT "(..)`. Expected `%s`, got `%s`", 
                      HGL_SV_ARG(*func_id), TYPE_TO_STR[*argtypes],
                      TYPE_TO_STR[t]);

    /* Typecheck right-hand-side expression (tail of argslist)*/
    return typecheck_argslist(e->rhs, func_id, ++argtypes);

}




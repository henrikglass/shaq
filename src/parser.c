/*--- Include files ---------------------------------------------------------------------*/

#include "parser.h"

#include "alloc.h"

#include <stdio.h> // DEBUG
#include <assert.h> // DEBUG

/*--- Private macros --------------------------------------------------------------------*/

/*--- Private type definitions ----------------------------------------------------------*/

/*--- Private function prototypes -------------------------------------------------------*/

static Expr *alloc_binary_expr(ExprKind kind, Token token, Expr *lhs, Expr *rhs);
static Expr *alloc_unary_expr(ExprKind kind, Token token, Expr *child);
static Expr *alloc_atom_expr(ExprKind kind, Token token);
static Expr *parse_add_expr(Lexer *l);
static Expr *parse_mul_expr(Lexer *l);
static Expr *parse_unary_or_atom_expr(Lexer *l);
static Expr *parse_arglist_expr(Lexer *l);
static void print_expr_tree_helper(Expr *e, int indent);

/*--- Public variables ------------------------------------------------------------------*/

/*--- Private variables -----------------------------------------------------------------*/

/*--- Public functions ------------------------------------------------------------------*/

Expr *parse_expr(Lexer *l)
{
    return parse_add_expr(l);
}

void print_expr(Expr *e)
{
    fflush(stdout);

    if (e == NULL) {
        return;
    }

    if (e->kind == LIT || e->kind == CONST) {
        token_print(&e->token);
        return;
    }

    if (e->kind == PAREN) {
        print_expr(e->child);
        return;
    }

    if (e->kind == FUNC) {
        token_print(&e->token);
        printf("<");
        print_expr(e->child);
        printf(">");
        return;
    }

    if (e->kind == NEG) {
        printf("-(");
        print_expr(e->child);
        printf(")");
        return;
    }

    if (e->kind == ARGLIST) {
        print_expr(e->lhs);
        printf(",");
        print_expr(e->rhs);
        return;
    }

    printf("(");
    print_expr(e->lhs);
    switch (e->kind) {
        case ADD: printf("+"); break;
        case SUB: printf("-"); break;
        case MUL: printf("*"); break;
        case DIV: printf("/"); break;
        case REM: printf("%%"); break;
        case NEG:
        case PAREN:
        case FUNC:
        case ARGLIST:
        case LIT:
        case CONST:
        default: assert(false); 
    }
    print_expr(e->rhs);
    printf(")");
}

void print_expr_tree(Expr *e)
{
    print_expr_tree_helper(e, 0); 
}

/*--- Private functions -----------------------------------------------------------------*/

static Expr *alloc_binary_expr(ExprKind kind, Token token, Expr *lhs, Expr *rhs)
{
    Expr *e = alloc(temp_allocator, sizeof(Expr));
    e->kind = kind;
    e->token = token;
    e->lhs = lhs;
    e->rhs = rhs;
    return e;
}

static Expr *alloc_unary_expr(ExprKind kind, Token token, Expr *child)
{
    Expr *e = alloc(temp_allocator, sizeof(Expr));
    e->kind = kind;
    e->token = token;
    e->child = child;
    return e;
}

static Expr *alloc_atom_expr(ExprKind kind, Token token)
{
    Expr *e = alloc(temp_allocator, sizeof(Expr));
    e->kind = kind;
    e->token = token;
    return e;
}

static Expr *parse_add_expr(Lexer *l)
{
    Expr *a, *b;

    a = parse_mul_expr(l);
    while (true) {
        Token t = lexer_peek(l);
        if (t.kind != PLUS && t.kind != MINUS) {
            break;
        }
        lexer_eat(l);
        b = parse_mul_expr(l);
        a = (t.kind == PLUS) ? alloc_binary_expr(ADD, t, a, b) : 
                               alloc_binary_expr(SUB, t, a, b);
    }

    return a;
}

static Expr *parse_mul_expr(Lexer *l)
{
    Expr *a, *b;

    a = parse_unary_or_atom_expr(l);
    while (true) {
        Token t = lexer_peek(l);
        if (t.kind != STAR && t.kind != FSLASH && t.kind != PERCENT) {
            break;
        }
        lexer_eat(l);
        b = parse_unary_or_atom_expr(l);
        a = (t.kind == STAR)   ? alloc_binary_expr(MUL, t, a, b) : 
            (t.kind == FSLASH) ? alloc_binary_expr(DIV, t, a, b) : 
                                 alloc_binary_expr(REM, t, a, b);
    }

    return a;
}

static Expr *parse_unary_or_atom_expr(Lexer *l)
{
    Expr *e;
    Token t;

    if (lexer_peek(l).kind == RPAREN) {
        return NULL;
    }

    t = lexer_next(l);
    switch (t.kind) {
        case LPAREN: {
            e = alloc_unary_expr(PAREN, t, parse_add_expr(l));
            lexer_expect(l, RPAREN);
        } break;

        case MINUS: {
            e = alloc_unary_expr(NEG, t, parse_unary_or_atom_expr(l));
        } break;

        case IDENTIFIER: {
            if (lexer_peek(l).kind == LPAREN) {
                lexer_eat(l);
                e = alloc_unary_expr(FUNC, t, parse_arglist_expr(l));
                lexer_expect(l, RPAREN);
            } else {
                e = alloc_atom_expr(CONST, t);            
            }
        } break;

        case INT_LITERAL: 
        case FLOAT_LITERAL: {
            e = alloc_atom_expr(LIT, t);            
        } break;

        case RPAREN:
        case PLUS:
        case STAR:
        case FSLASH:
        case PERCENT:
        case COMMA:
        case EOF_TOKEN_:
        case LEXER_ERROR_:
        case NO_TOKEN_:
        case N_TOKENS_:
        default: 
            assert(false);
    }

    return e;
}

static Expr *parse_arglist_expr(Lexer *l)
{
    Expr *a, *b;

    a = parse_add_expr(l);
    while (true) {
        Token t = lexer_peek(l);
        if (t.kind != COMMA) {
            break;
        }
        lexer_eat(l);
        b = parse_add_expr(l);
        a = alloc_binary_expr(ARGLIST, t, a, b);
    }

    return a;
}

static void print_expr_tree_helper(Expr *e, int indent)
{
    int pad = -2*indent;
    if (e == NULL) return;
    switch (e->kind) {
        case ADD:     printf("%*s+\n", pad, "");  print_expr_tree_helper(e->lhs, indent + 1); print_expr_tree_helper(e->rhs, indent + 1); break;
        case SUB:     printf("%*s-\n", pad, "");  print_expr_tree_helper(e->lhs, indent + 1); print_expr_tree_helper(e->rhs, indent + 1); break;
        case MUL:     printf("%*s*\n", pad, "");  print_expr_tree_helper(e->lhs, indent + 1); print_expr_tree_helper(e->rhs, indent + 1); break;
        case DIV:     printf("%*s/\n", pad, "");  print_expr_tree_helper(e->lhs, indent + 1); print_expr_tree_helper(e->rhs, indent + 1); break;
        case REM:     printf("%*s%%\n", pad, ""); print_expr_tree_helper(e->lhs, indent + 1); print_expr_tree_helper(e->rhs, indent + 1); break;
        case NEG:     printf("%*sN\n", pad, "");  print_expr_tree_helper(e->child, indent + 1);break;
        case PAREN:   
                      printf("%*s(\n", pad, ""); 
                      print_expr_tree_helper(e->child, indent + 1); 
                      printf("%*s)\n", pad, ""); 
                      break;
        case FUNC:    printf("%*s" HGL_SV_FMT "\n", pad, "", HGL_SV_ARG(e->token.text)); 
                      printf("%*s(\n", pad, ""); 
                      print_expr_tree_helper(e->child, indent + 1); 
                      printf("%*s)\n", pad, ""); 
                      break;
        case ARGLIST: print_expr_tree_helper(e->lhs, indent); 
                      printf("%*s,\n", pad, ""); 
                      print_expr_tree_helper(e->rhs, indent); 
                      break;
        case CONST:
        case LIT:     printf("%*s", pad, ""); 
                      token_print(&e->token); 
                      printf("\n"); 
                      break;
    }
}


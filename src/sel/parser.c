/*--- Include files ---------------------------------------------------------------------*/

#include "sel/parser.h"

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

static HglArena temp_allocator = HGL_ARENA_INITIALIZER(1024*1024);

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

    if (e->kind == EXPR_LIT || e->kind == EXPR_CONST) {
        token_print(&e->token);
        return;
    }

    if (e->kind == EXPR_PAREN) {
        print_expr(e->child);
        return;
    }

    if (e->kind == EXPR_FUNC) {
        token_print(&e->token);
        printf("<");
        print_expr(e->child);
        printf(">");
        return;
    }

    if (e->kind == EXPR_NEG) {
        printf("-(");
        print_expr(e->child);
        printf(")");
        return;
    }

    if (e->kind == EXPR_ARGLIST) {
        print_expr(e->lhs);
        printf(",");
        print_expr(e->rhs);
        return;
    }

    printf("(");
    print_expr(e->lhs);
    switch (e->kind) {
        case EXPR_ADD: printf("+"); break;
        case EXPR_SUB: printf("-"); break;
        case EXPR_MUL: printf("*"); break;
        case EXPR_DIV: printf("/"); break;
        case EXPR_REM: printf("%%"); break;
        case EXPR_NEG:
        case EXPR_PAREN:
        case EXPR_FUNC:
        case EXPR_ARGLIST:
        case EXPR_LIT:
        case EXPR_CONST:
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
    Expr *e = arena_alloc(&temp_allocator, sizeof(Expr));
    e->kind = kind;
    e->token = token;
    e->lhs = lhs;
    e->rhs = rhs;
    return e;
}

static Expr *alloc_unary_expr(ExprKind kind, Token token, Expr *child)
{
    Expr *e = arena_alloc(&temp_allocator, sizeof(Expr));
    e->kind = kind;
    e->token = token;
    e->child = child;
    return e;
}

static Expr *alloc_atom_expr(ExprKind kind, Token token)
{
    Expr *e = arena_alloc(&temp_allocator, sizeof(Expr));
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
        if (t.kind != TOK_PLUS && t.kind != TOK_MINUS) {
            break;
        }
        lexer_eat(l);
        b = parse_mul_expr(l);
        a = (t.kind == TOK_PLUS) ? alloc_binary_expr(EXPR_ADD, t, a, b) : 
                                   alloc_binary_expr(EXPR_SUB, t, a, b);
    }

    return a;
}

static Expr *parse_mul_expr(Lexer *l)
{
    Expr *a, *b;

    a = parse_unary_or_atom_expr(l);
    while (true) {
        Token t = lexer_peek(l);
        if (t.kind != TOK_STAR && t.kind != TOK_FSLASH && t.kind != TOK_PERCENT) {
            break;
        }
        lexer_eat(l);
        b = parse_unary_or_atom_expr(l);
        a = (t.kind == TOK_STAR)   ? alloc_binary_expr(EXPR_MUL, t, a, b) : 
            (t.kind == TOK_FSLASH) ? alloc_binary_expr(EXPR_DIV, t, a, b) : 
                                     alloc_binary_expr(EXPR_REM, t, a, b);
    }

    return a;
}

static Expr *parse_unary_or_atom_expr(Lexer *l)
{
    Expr *e;
    Token t;

    if (lexer_peek(l).kind == TOK_RPAREN) {
        return NULL;
    }

    t = lexer_next(l);
    switch (t.kind) {
        case TOK_LPAREN: {
            // TODO PARSER_ASSERT(lexer_peek(l).kind != RPAREN);
            e = alloc_unary_expr(EXPR_PAREN, t, parse_add_expr(l));
            lexer_expect(l, TOK_RPAREN);
        } break;

        case TOK_MINUS: {
            e = alloc_unary_expr(EXPR_NEG, t, parse_unary_or_atom_expr(l));
        } break;

        case TOK_IDENTIFIER: {
            if (lexer_peek(l).kind == TOK_LPAREN) {
                lexer_eat(l);
                e = alloc_unary_expr(EXPR_FUNC, t, parse_arglist_expr(l));
                lexer_expect(l, TOK_RPAREN);
            } else {
                e = alloc_atom_expr(EXPR_CONST, t);            
            }
        } break;

        case TOK_INT_LITERAL: 
        case TOK_FLOAT_LITERAL: {
            e = alloc_atom_expr(EXPR_LIT, t);            
        } break;

        case TOK_RPAREN:
        case TOK_PLUS:
        case TOK_STAR:
        case TOK_FSLASH:
        case TOK_PERCENT:
        case TOK_COMMA:
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

    // TODO parse into array?


    //Expr *a, *b;

    //a = parse_add_expr(l);
    //while (true) {
    //    Token t = lexer_peek(l);
    //    if (t.kind != TOK_COMMA) {
    //        break;
    //    }
    //    lexer_eat(l);
    //    b = parse_add_expr(l);
    //    a = alloc_binary_expr(EXPR_ARGLIST, t, a, b);
    //}
    //Expr *a, *b;

    //a = parse_add_expr(l);
    //while (true) {
    //    Token t = lexer_peek(l);
    //    if (t.kind != TOK_COMMA) {
    //        a = alloc_binary_expr(EXPR_ARGLIST, t, a, NULL);
    //        break;
    //    }
    //    lexer_eat(l);
    //    b = parse_add_expr(l);
    //    a = alloc_binary_expr(EXPR_ARGLIST, t, a, b);
    //}

    Expr *a;

    Token t = lexer_peek(l);
    if (t.kind == TOK_RPAREN) {
        return NULL;
    }
    a = alloc_binary_expr(EXPR_ARGLIST, t, parse_add_expr(l), NULL);
    while (true) {
        t = lexer_peek(l);
        if (t.kind != TOK_COMMA) {
            break;
        }
        lexer_eat(l);
        a->rhs = parse_arglist_expr(l);
    }

    return a;
}

static void print_expr_tree_helper(Expr *e, int indent)
{
    int pad = -2*indent;
    if (e == NULL) return;
    switch (e->kind) {
        case EXPR_ADD:     printf("%*s+\n", pad, "");  print_expr_tree_helper(e->lhs, indent + 1); print_expr_tree_helper(e->rhs, indent + 1); break;
        case EXPR_SUB:     printf("%*s-\n", pad, "");  print_expr_tree_helper(e->lhs, indent + 1); print_expr_tree_helper(e->rhs, indent + 1); break;
        case EXPR_MUL:     printf("%*s*\n", pad, "");  print_expr_tree_helper(e->lhs, indent + 1); print_expr_tree_helper(e->rhs, indent + 1); break;
        case EXPR_DIV:     printf("%*s/\n", pad, "");  print_expr_tree_helper(e->lhs, indent + 1); print_expr_tree_helper(e->rhs, indent + 1); break;
        case EXPR_REM:     printf("%*s%%\n", pad, ""); print_expr_tree_helper(e->lhs, indent + 1); print_expr_tree_helper(e->rhs, indent + 1); break;
        case EXPR_NEG:     printf("%*sN\n", pad, "");  print_expr_tree_helper(e->child, indent + 1);break;
        case EXPR_PAREN:   
                           printf("%*s(\n", pad, ""); 
                           print_expr_tree_helper(e->child, indent + 1); 
                           printf("%*s)\n", pad, ""); 
                           break;
        case EXPR_FUNC:    printf("%*s" HGL_SV_FMT "\n", pad, "", HGL_SV_ARG(e->token.text)); 
                           printf("%*s(\n", pad, ""); 
                           print_expr_tree_helper(e->child, indent + 1); 
                           printf("%*s)\n", pad, ""); 
                           break;
        case EXPR_ARGLIST: print_expr_tree_helper(e->lhs, indent); 
                           printf("%*s,\n", pad, ""); 
                           print_expr_tree_helper(e->rhs, indent); 
                           break;
        case EXPR_CONST:
        case EXPR_LIT:     printf("%*s", pad, ""); 
                           token_print(&e->token); 
                           printf("\n"); 
                           break;
    }
}


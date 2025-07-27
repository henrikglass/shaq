/*--- Include files ---------------------------------------------------------------------*/

#include "sel.h"
#include "alloc.h"
#include "glad/glad.h"
#include "log.h"

#include <assert.h>
#include <stdio.h>

/*--- Private macros --------------------------------------------------------------------*/

#define TRY(expr_)                                         \
    do {                                                   \
        i32 err_ = (i32) (expr_);                          \
        if (err_ != 0) {                                   \
            return err_;                                   \
        }                                                  \
    } while (0)

#define LEXER_ASSERT(cond_, ...)                           \
    if (!(cond_)) {                                        \
        log_error("Lexer error: " __VA_ARGS__);            \
        return (Token) {.kind = LEXER_ERROR_};             \
    }                                                      \

#define PARSER_ERROR(...)                                  \
    do {                                                   \
        log_error("Parser error: " __VA_ARGS__);           \
        return -1;                                         \
    } while (0)                                            \

#define PARSER_ASSERT(cond_, ...)                          \
    if (!(cond_)) {                                        \
        log_error("Parser error: " __VA_ARGS__);           \
        return -1;                                         \
    }                                                      \

#define TYPE_AND_NAMECHECK_ERROR(...)                      \
    do {                                                   \
        log_error("Type-/namecheck error: " __VA_ARGS__);  \
        return (TypeAndQualifier) {                        \
            TYPE_AND_NAMECHECKER_ERROR_,                   \
            QUALIFIER_NONE                                 \
        };                                                 \
    } while (0)

#define TYPE_AND_NAMECHECK_ASSERT(cond_, ...)              \
    if (!(cond_)) {                                        \
        log_error("Type-/namecheck error: " __VA_ARGS__);  \
        return (TypeAndQualifier) {                        \
            TYPE_AND_NAMECHECKER_ERROR_,                   \
            QUALIFIER_NONE                                 \
        };                                                 \
    }


/*--- Private type definitions ----------------------------------------------------------*/

typedef enum
{
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_PLUS,
    TOK_MINUS,
    TOK_STAR,
    TOK_FSLASH,
    TOK_PERCENT,
    TOK_COMMA,
    TOK_BOOL_LITERAL,
    TOK_INT_LITERAL,
    TOK_UINT_LITERAL,
    TOK_FLOAT_LITERAL,
    TOK_STR_LITERAL,
    TOK_IDENTIFIER,
    EOF_TOKEN_,
    LEXER_ERROR_,
    N_TOKENS_,
} TokenKind;

typedef struct
{
    TokenKind kind;
    StringView text; 
    u32 length;
} Token;

typedef struct
{
    StringView buf; 
} Lexer;

typedef enum
{
    EXPR_ADD,
    EXPR_SUB,
    EXPR_MUL,
    EXPR_DIV,
    EXPR_REM,
    EXPR_NEG,
    EXPR_PAREN,
    EXPR_FUNC,
    EXPR_ARGLIST,
    EXPR_LIT,
    EXPR_CONST,
    N_EXPR_KINDS,
} ExprKind;

static_assert(N_EXPR_KINDS <= 256, "");

typedef struct
{
    StringView id;
    Type type;
    SelValue value;
} Const;

typedef struct
{
    Type type;
    TypeQualifier qualifier;
} TypeAndQualifier;

/**
 * An expression may be either binary, and have two children `lhs` and `rhs`, unary, 
 * and have a single child `child`, or atomic, and have no children. 
 *
 * Whether an expression is unary, binary, or atomic is determined by its `kind`. E.g. 
 * `ADD`, `MUL`, and `REM` are binary operations, and thus binary expressions; `FUNC` 
 * and `PAREN` are unary expressions; `LIT` is atomic.
 */
typedef struct ExprTree
{
    ExprKind kind;
    Token token;
    Type type;
    TypeQualifier qualifier;
    union {
        struct ExprTree *child;
        struct ExprTree *lhs;
    };
    struct ExprTree *rhs;
} ExprTree;


/*--- Private function prototypes -------------------------------------------------------*/

/* lexer */
static Lexer lexer_begin(const char *str);
static Token lexer_next(Lexer *l);
static void lexer_eat(Lexer *l);
static Token lexer_peek(Lexer *l);
static ExprTree *new_binary_expr(ExprKind kind, Token token, ExprTree *lhs, ExprTree *rhs);
static ExprTree *new_unary_expr(ExprKind kind, Token token, ExprTree *child);
static ExprTree *new_atom_expr(ExprKind kind, Token token);

/* parser */
static ExprTree *parse_expr(const char *str);
static i32 parse_add_expr(ExprTree **e, Lexer *l);
static i32 parse_mul_expr(ExprTree **e, Lexer *l);
static i32 parse_unary_or_atom_expr(ExprTree **e, Lexer *l);
static i32 parse_arglist_expr(ExprTree **e, Lexer *l);

/* Type-/namechecker */
static TypeAndQualifier type_and_namecheck(ExprTree *e);
static TypeAndQualifier type_and_namecheck_function(ExprTree *e, const Func *f, const Type *argtypes, b8 const_args);

/* codegen */
static ExeExpr *codegen(const ExprTree *e);
static void codegen_expr(ExeExpr *exe, const ExprTree *e);
static void exe_append_op(ExeExpr *exe, Op op);
static void exe_append_i32(ExeExpr *exe, i32 v);
static void exe_append_u32(ExeExpr *exe, u32 v);
static void exe_append_f32(ExeExpr *exe, f32 v);
static void exe_append(ExeExpr *exe, const void *val, u32 size);

/* misc. debug */
static void token_print(Token *t);
static b8 is_identifier_char(i32 c);
static b8 is_decimal_char(i32 c);
static b8 is_hexadecimal_char(i32 c);
static inline void print_expr(ExprTree *e);
static inline void print_expr_tree(ExprTree *e);
static inline void print_expr_tree_helper(ExprTree *e, i32 indent);

/*--- Public variables ------------------------------------------------------------------*/

/*--- Private variables -----------------------------------------------------------------*/

static const char *const TOKEN_TO_STR[] =
{
    [TOK_LPAREN]        = "(",
    [TOK_RPAREN]        = ")",
    [TOK_PLUS]          = "+",
    [TOK_MINUS]         = "-",
    [TOK_STAR]          = "*",
    [TOK_FSLASH]        = "/",
    [TOK_PERCENT]       = "%",
    [TOK_COMMA]         = ",",
    [TOK_BOOL_LITERAL]  = "<bool-literal>",
    [TOK_INT_LITERAL]   = "<int-literal>",
    [TOK_FLOAT_LITERAL] = "<float-literal>",
    [TOK_IDENTIFIER]    = "<identifier>",
    [EOF_TOKEN_]        = "<EOF>",
    [LEXER_ERROR_]      = "<LEXER-ERROR>",
};

const Const BUILTIN_CONSTANTS[] = 
{
    {.id = HGL_SV_LIT("PI"),                 .type = TYPE_FLOAT, .value.val_f32 =     3.1415926535},
    {.id = HGL_SV_LIT("TAU"),                .type = TYPE_FLOAT, .value.val_f32 = 2.0*3.1415926535},
    {.id = HGL_SV_LIT("PHI"),                .type = TYPE_FLOAT, .value.val_f32 =     1.6180339887},
    {.id = HGL_SV_LIT("e"),                  .type = TYPE_FLOAT, .value.val_f32 =     2.7182818284},
    {.id = HGL_SV_LIT("GL_NEAREST"),         .type = TYPE_UINT,  .value.val_f32 = GL_NEAREST },
    {.id = HGL_SV_LIT("GL_LINEAR"),          .type = TYPE_UINT,  .value.val_f32 = GL_LINEAR },
    {.id = HGL_SV_LIT("GL_REPEAT"),          .type = TYPE_UINT,  .value.val_f32 = GL_REPEAT },
    {.id = HGL_SV_LIT("GL_MIRRORED_REPEAT"), .type = TYPE_UINT,  .value.val_f32 = GL_MIRRORED_REPEAT },
    {.id = HGL_SV_LIT("GL_CLAMP_TO_EDGE"),   .type = TYPE_UINT,  .value.val_f32 = GL_CLAMP_TO_EDGE },
    {.id = HGL_SV_LIT("GL_CLAMP_TO_BORDER"), .type = TYPE_UINT,  .value.val_f32 = GL_CLAMP_TO_BORDER },
};
static const size_t N_BUILTIN_CONSTANTS = sizeof(BUILTIN_CONSTANTS) / sizeof(BUILTIN_CONSTANTS[0]);

/*--- Public functions ------------------------------------------------------------------*/

ExeExpr *sel_compile(const char *src)
{
    ExeExpr *exe = NULL;

    /* lexer + parser step */
    ExprTree *e = parse_expr(src);
    if (e == NULL) {
        goto out;
    }

    /* type + namecheck step */
    TypeAndQualifier t = type_and_namecheck(e);
    assert(t.type != TYPE_NIL); // should not be possible
    if (t.type == TYPE_AND_NAMECHECKER_ERROR_) {
        goto out;
    }

    /* codegen step. *Should* never fail if the previous steps succeed */
    exe = codegen(e);

    /* Attach source code reference */
    exe->source_code = src;

out:
    return exe;
}

void sel_list_builtins(void) {
    printf("Constants:\n");
    for (u32 i = 0; i < N_BUILTIN_CONSTANTS; i++) {
        printf("  %-80.*s TYPE: %s\n", 
               HGL_SV_ARG(BUILTIN_CONSTANTS[i].id), 
               TYPE_TO_STR[BUILTIN_CONSTANTS[i].type]);
    }

    printf("Functions:\n");
    for (u32 i = 0; i < N_BUILTIN_FUNCTIONS; i++) {
        printf("  %-80s %s\n", 
               BUILTIN_FUNCTIONS[i].synopsis,
               (BUILTIN_FUNCTIONS[i].desc != NULL) ? BUILTIN_FUNCTIONS[i].desc : "-");
    }
}

void sel_print_value(Type t, SelValue v)
{
    switch (t) {
        case TYPE_BOOL:    printf("%s\n", v.val_bool ? "true" : "false"); break;
        case TYPE_INT:     printf("%d\n", v.val_i32); break;
        case TYPE_UINT:    printf("%uu\n", v.val_u32); break;
        case TYPE_FLOAT:   printf("%f\n", (double)v.val_f32); break;
        case TYPE_VEC2:    vec2_print(v.val_vec2); break;
        case TYPE_VEC3:    vec3_print(v.val_vec3); break;
        case TYPE_VEC4:    vec4_print(v.val_vec4); break;
        case TYPE_IVEC2:   ivec2_print(v.val_ivec2); break;
        case TYPE_IVEC3:   ivec3_print(v.val_ivec3); break;
        case TYPE_IVEC4:   ivec4_print(v.val_ivec4); break;
        case TYPE_MAT2:    mat2_print(v.val_mat2); break;
        case TYPE_MAT3:    mat3_print(v.val_mat3); break;
        case TYPE_MAT4:    mat4_print(v.val_mat4); break;
        case TYPE_STR:     printf("\""HGL_SV_FMT"\"" "\n", HGL_SV_ARG(v.val_str)); break;
        case TYPE_TEXTURE: {
            if (v.val_tex.error) printf("ERROR\n"); 
            else if (v.val_tex.kind == SHADER_INDEX) printf("render texture: %u\n", v.val_tex.render_texture_index); 
            else if (v.val_tex.kind == LOADED_TEXTURE_INDEX) printf("loaded texture: %u\n", v.val_tex.loaded_texture_index);
        } break;
        case TYPE_NIL:     printf("<NIL>\n"); break;
        case TYPE_AND_NAMECHECKER_ERROR_:
        case N_TYPES:
            printf("-\n");
    }
}

/*--- LEXER -----------------------------------------------------------------------------*/

static Lexer lexer_begin(const char *str)
{
    Lexer l = {0};
    l.buf = sv_from_cstr(str);
    return l;
}

static Token lexer_next(Lexer *l)
{
    Token t = lexer_peek(l);
    sv_lchop(&l->buf, t.length);
    return t;
}

static void lexer_eat(Lexer *l)
{
    (void) lexer_next(l);
}

static Token lexer_peek(Lexer *l)
{
    // TODO anneal this brittle crap

    l->buf = sv_ltrim(l->buf);

    if (l->buf.length == 0) {
        return (Token) {.kind = EOF_TOKEN_};
    }

    char c = l->buf.start[0];
    switch (c) {
        case '\0': return (Token) {.kind = EOF_TOKEN_}; break;
        case '\n': return (Token) {.kind = EOF_TOKEN_}; break;
        case '(':  return (Token) {.kind = TOK_LPAREN,  .text = sv_substr(l->buf, 0, 1), .length = 1}; break;
        case ')':  return (Token) {.kind = TOK_RPAREN,  .text = sv_substr(l->buf, 0, 1), .length = 1}; break;
        case '+':  return (Token) {.kind = TOK_PLUS,    .text = sv_substr(l->buf, 0, 1), .length = 1}; break;
        case '-':  return (Token) {.kind = TOK_MINUS,   .text = sv_substr(l->buf, 0, 1), .length = 1}; break;
        case '*':  return (Token) {.kind = TOK_STAR,    .text = sv_substr(l->buf, 0, 1), .length = 1}; break;
        case '/':  return (Token) {.kind = TOK_FSLASH,  .text = sv_substr(l->buf, 0, 1), .length = 1}; break;
        case '%':  return (Token) {.kind = TOK_PERCENT, .text = sv_substr(l->buf, 0, 1), .length = 1}; break;
        case ',':  return (Token) {.kind = TOK_COMMA,   .text = sv_substr(l->buf, 0, 1), .length = 1}; break;

        case '"': {
            size_t i = 1;
            for (; i < l->buf.length; i++) {
                c = l->buf.start[i];
                if (c == '"') {
                    i++; 
                    return (Token) {
                        .kind = TOK_STR_LITERAL, 
                        .text = sv_substr(l->buf, 1, i - 2), // discard opening and closing quotation marks
                        .length = i,
                    };
                }
            }
            return (Token) {.kind = LEXER_ERROR_, .text = l->buf, .length = l->buf.length};
        } break;

        case 'a' ... 'z': case 'A' ... 'Z': case '_': {
            size_t i = 1;
            for (; i < l->buf.length; i++) {
                c = l->buf.start[i];
                if (!is_identifier_char(c)) break;
            }

            StringView s = sv_substr(l->buf, 0, i);

            /* boolean literal? */
            if (sv_equals(s, HGL_SV_LIT("true")) ||
                sv_equals(s, HGL_SV_LIT("false"))) {
                return (Token) {
                    .kind = TOK_BOOL_LITERAL, 
                    .text = s,
                    .length = (u32) s.length,
                };
            }

            return (Token) {
                .kind = TOK_IDENTIFIER, 
                .text = s,
                .length = (u32) s.length,
            };
        } break;

        case '0' ... '9': {
            size_t i = 1;

            /* hexadecimal int literal? */
            if (c == '0' && l->buf.start[i] == 'x') {
                i++;
                for (; i < l->buf.length; i++) {
                    c = l->buf.start[i];
                    if (!is_hexadecimal_char(c)) break;
                }
            } else {
                for (; i < l->buf.length; i++) {
                    c = l->buf.start[i];
                    if (!is_decimal_char(c)) break;
                }
            }

            /* decimal or octal int literal */
            if (c != '.') {
                // Not the job of the lexer to check this honestly
                if (i != l->buf.length) {
                    LEXER_ASSERT(!is_identifier_char(c) || c == 'u', "hmm");
                }

                b8 is_unsigned = false;
                if (c == 'u') {
                    is_unsigned = true;
                    i++;
                }
                return (Token) {
                    .kind = is_unsigned ? TOK_UINT_LITERAL : TOK_INT_LITERAL,
                    .text = sv_substr(l->buf, 0, i),
                    .length = i,
                };
            }

            /* float literal */
            i++;
            for (; i < l->buf.length; i++) {
                c = l->buf.start[i];
                if (!is_decimal_char(c)) break;
            }

            // Not the job of the lexer to check this honestly
            if (i != l->buf.length) {
                LEXER_ASSERT(!is_identifier_char(c));
            }
            return (Token) {
                .kind = TOK_FLOAT_LITERAL,
                .text = sv_substr(l->buf, 0, i),
                .length = i,
            };

        } break;
    }

    //LEXER_ASSERT(!is_identifier_char(c));
    return (Token) {.kind = LEXER_ERROR_, .text = l->buf, .length = l->buf.length};
}


/*--- PARSER ----------------------------------------------------------------------------*/

static ExprTree *parse_expr(const char *str)
{
    i32 err;
    Lexer l = lexer_begin(str);
    ExprTree *e = NULL;
    err = parse_add_expr(&e, &l);
    if (err != 0) {
        return NULL;
    }
    if (lexer_peek(&l).kind != EOF_TOKEN_) {
        log_error("Parser error: Trailing characters at end of complete expression: `" 
                  HGL_SV_FMT "`", HGL_SV_ARG(lexer_peek(&l).text));
        //fprintf(stderr, "[SEL] Parser error: Trailing characters at end of complete "
        //        "expression: `" HGL_SV_FMT "`\n", HGL_SV_ARG(lexer_peek(&l).text));
        return NULL;
    }

    return e;
}

static i32 parse_add_expr(ExprTree **e, Lexer *l)
{
    ExprTree *tmp;
    Token t;

    TRY(parse_mul_expr(e, l));
    while (true) {
        t = lexer_peek(l);
        if (t.kind != TOK_PLUS && t.kind != TOK_MINUS) {
            break;
        }
        lexer_eat(l);
        TRY(parse_mul_expr(&tmp, l));
        *e = (t.kind == TOK_PLUS) ? new_binary_expr(EXPR_ADD, t, *e, tmp) : 
                                    new_binary_expr(EXPR_SUB, t, *e, tmp);
    }

    return 0;
}

static i32 parse_mul_expr(ExprTree **e, Lexer *l)
{
    ExprTree *tmp;

    TRY(parse_unary_or_atom_expr(e, l));
    while (true) {
        Token t = lexer_peek(l);
        if (t.kind != TOK_STAR && t.kind != TOK_FSLASH && t.kind != TOK_PERCENT) {
            break;
        }
        lexer_eat(l);
        TRY(parse_unary_or_atom_expr(&tmp, l));
        *e = (t.kind == TOK_STAR)   ? new_binary_expr(EXPR_MUL, t, *e, tmp) : 
             (t.kind == TOK_FSLASH) ? new_binary_expr(EXPR_DIV, t, *e, tmp) : 
                                      new_binary_expr(EXPR_REM, t, *e, tmp);
    }

    return 0;
}

static i32 parse_unary_or_atom_expr(ExprTree **e, Lexer *l)
{
    ExprTree *tmp;
    Token t;

    //if (lexer_peek(l).kind == TOK_RPAREN) {
    //    *e = NULL;
    //    return -1;
    //}

    t = lexer_next(l);
    switch (t.kind) {
        case TOK_LPAREN: {
            PARSER_ASSERT(lexer_peek(l).kind != TOK_RPAREN, "Expected expression after opening '('.");
            TRY(parse_add_expr(&tmp, l));
            *e = new_unary_expr(EXPR_PAREN, t, tmp);
            PARSER_ASSERT(lexer_next(l).kind == TOK_RPAREN, "Expected closing ')'.");
        } break;

        case TOK_MINUS: {
            TRY(parse_unary_or_atom_expr(&tmp, l));
            *e = new_unary_expr(EXPR_NEG, t, tmp);
        } break;

        case TOK_IDENTIFIER: {
            if (lexer_peek(l).kind == TOK_LPAREN) {
                lexer_eat(l);
                TRY(parse_arglist_expr(&tmp, l));
                *e = new_unary_expr(EXPR_FUNC, t, tmp);
                PARSER_ASSERT(lexer_next(l).kind == TOK_RPAREN, "Expected closing ')'.");
            } else {
                *e = new_atom_expr(EXPR_CONST, t);            
            }
        } break;

        case TOK_BOOL_LITERAL: 
        case TOK_INT_LITERAL: 
        case TOK_UINT_LITERAL: 
        case TOK_FLOAT_LITERAL: 
        case TOK_STR_LITERAL: {
            *e = new_atom_expr(EXPR_LIT, t);            
        } break;

        case TOK_RPAREN:
        case TOK_PLUS:
        case TOK_STAR:
        case TOK_FSLASH:
        case TOK_PERCENT:
        case TOK_COMMA: {
            PARSER_ERROR("Unexpected token: `%s`.", TOKEN_TO_STR[t.kind]);
        } break;

        case LEXER_ERROR_: {
            PARSER_ERROR("Failed to tokenize: `" HGL_SV_FMT "`.", HGL_SV_ARG(t.text));
        } break;

        case EOF_TOKEN_: {
            PARSER_ERROR("Reached the end of the token stream with an incomplete parse tree.");
        } break;

        case N_TOKENS_:
        default: 
            PARSER_ERROR("Something extremely silly is going on.");
            return -1;
    }

    return 0;
}

static i32 parse_arglist_expr(ExprTree **e, Lexer *l)
{
    ExprTree *tmp;

    Token t = lexer_peek(l);
    if (t.kind == TOK_RPAREN) {
        *e = NULL;
        return 0;
    }
    TRY(parse_add_expr(&tmp, l));
    *e = new_binary_expr(EXPR_ARGLIST, t, tmp, NULL);
    while (true) {
        t = lexer_peek(l);
        if (t.kind != TOK_COMMA) {
            break;
        }
        lexer_eat(l);
        TRY(parse_arglist_expr(&tmp, l));
        (*e)->rhs = tmp;
    }

    return 0;
}

static ExprTree *new_binary_expr(ExprKind kind, Token token, ExprTree *lhs, ExprTree *rhs)
{
    ExprTree *e = arena_alloc(g_frame_arena, sizeof(ExprTree));
    e->kind = kind;
    e->token = token;
    e->lhs = lhs;
    e->rhs = rhs;
    return e;
}

static ExprTree *new_unary_expr(ExprKind kind, Token token, ExprTree *child)
{
    ExprTree *e = arena_alloc(g_frame_arena, sizeof(ExprTree));
    e->kind = kind;
    e->token = token;
    e->child = child;
    return e;
}

static ExprTree *new_atom_expr(ExprKind kind, Token token)
{
    ExprTree *e = arena_alloc(g_frame_arena, sizeof(ExprTree));
    e->kind = kind;
    e->token = token;
    return e;
}


/*--- TYPE-/NAMECHECKER -----------------------------------------------------------------*/

static TypeAndQualifier type_and_namecheck(ExprTree *e)
{
    TypeAndQualifier t0 = {TYPE_NIL, QUALIFIER_NONE};
    TypeAndQualifier t1;

    if (e == NULL) {
        return (TypeAndQualifier) {TYPE_NIL, QUALIFIER_NONE};
    }

    switch (e->kind) {
        
        case EXPR_ADD:
        case EXPR_SUB: {
            t0 = type_and_namecheck(e->lhs); 
            t1 = type_and_namecheck(e->rhs); 
            TYPE_AND_NAMECHECK_ASSERT(t0.type == t1.type, "Operands to arithmetic operation are of different types: "
                                      "Got `%s` and `%s`.", TYPE_TO_STR[t0.type], TYPE_TO_STR[t1.type]);
            // TODO support implicit type conversions Add lhs + rhs types to Op?
            TYPE_AND_NAMECHECK_ASSERT(t0.type != TYPE_BOOL, "No arithmetic on bools is allowed (yet).");
            TYPE_AND_NAMECHECK_ASSERT(t0.type != TYPE_STR, "Arithmetic is not allowed on strings.");
            TYPE_AND_NAMECHECK_ASSERT(t0.type != TYPE_TEXTURE, "Arithmetic is not allowed on textures.");
            TYPE_AND_NAMECHECK_ASSERT(t0.type != TYPE_MAT2 && t0.type != TYPE_MAT3 && t0.type != TYPE_MAT4, "Matricies may not (yet) be directly added. Use built-in functions instead.");
            TYPE_AND_NAMECHECK_ASSERT(t0.type != TYPE_IVEC2 && t0.type != TYPE_IVEC2 && t0.type != TYPE_IVEC2, "Integer vectors may not (yet) be directly added. Use built-in functions instead.");
            if ((t0.qualifier == QUALIFIER_CONST) && 
                (t1.qualifier == QUALIFIER_CONST)) {
                t0.qualifier = QUALIFIER_CONST;
            } else {
                t0.qualifier = QUALIFIER_NONE;
            }
        } break;

        case EXPR_MUL:
        case EXPR_DIV: {
            t0 = type_and_namecheck(e->lhs); 
            t1 = type_and_namecheck(e->rhs); 
            TYPE_AND_NAMECHECK_ASSERT(t0.type == t1.type, "Operands to arithmetic operation are of different types: "
                                      "Got `%s` and `%s`.", TYPE_TO_STR[t0.type], TYPE_TO_STR[t1.type]);
            TYPE_AND_NAMECHECK_ASSERT(t0.type != TYPE_BOOL, "No arithmetic on bools is allowed (yet).");
            TYPE_AND_NAMECHECK_ASSERT(t0.type != TYPE_STR, "Arithmetic is not allowed on strings.");
            TYPE_AND_NAMECHECK_ASSERT(t0.type != TYPE_TEXTURE, "Arithmetic is not allowed on textures.");
            TYPE_AND_NAMECHECK_ASSERT(t0.type != TYPE_MAT2 && t0.type != TYPE_MAT3 && t0.type != TYPE_MAT4, "Matricies may not (yet) be directly multiplied. Use built-in functions instead.");
            TYPE_AND_NAMECHECK_ASSERT(t0.type != TYPE_IVEC2 && t0.type != TYPE_IVEC2 && t0.type != TYPE_IVEC2, "Integer vectors may not (yet) be directly multiplied. Use built-in functions instead.");
            if ((t0.qualifier == QUALIFIER_CONST) && 
                (t1.qualifier == QUALIFIER_CONST)) {
                t0.qualifier = QUALIFIER_CONST;
            } else {
                t0.qualifier = QUALIFIER_NONE;
            }
        } break;
        
        case EXPR_REM: {
            t0 = type_and_namecheck(e->lhs); 
            t1 = type_and_namecheck(e->rhs); 
            TYPE_AND_NAMECHECK_ASSERT(t0.type == t1.type, "Operands to arithmetic operation are of different types: "
                                      "Got `%s` and `%s`.", TYPE_TO_STR[t0.type], TYPE_TO_STR[t1.type]);
            TYPE_AND_NAMECHECK_ASSERT(t0.type == TYPE_INT || t0.type == TYPE_UINT, "Operands to `%%` operator must be of type INT or UINT.");
            if ((t0.qualifier == QUALIFIER_CONST) && 
                (t1.qualifier == QUALIFIER_CONST)) {
                t0.qualifier = QUALIFIER_CONST;
            } else {
                t0.qualifier = QUALIFIER_NONE;
            }
        } break;
        
        case EXPR_NEG: {
            // TODO better rule / support more types
            t0 = type_and_namecheck(e->child);
            TYPE_AND_NAMECHECK_ASSERT(t0.type == TYPE_INT || t0.type == TYPE_FLOAT , 
                                      "Operand to unary `-` operator must be of type INT or FLOAT.");
        } break;
        
        case EXPR_PAREN: {
            t0 = type_and_namecheck(e->child);
        } break;
        
        case EXPR_FUNC: {
            for (size_t i = 0; i < N_BUILTIN_FUNCTIONS; i++) {
                if (sv_equals(e->token.text, BUILTIN_FUNCTIONS[i].id)) {
                    t0 = type_and_namecheck_function(e->child, &BUILTIN_FUNCTIONS[i], BUILTIN_FUNCTIONS[i].argtypes, true);
                    goto out;
                } 
            }
            TYPE_AND_NAMECHECK_ERROR("No such function: `" HGL_SV_FMT "(..)`.", HGL_SV_ARG(e->token.text));
        } break;
        
        case EXPR_ARGLIST: {
            TYPE_AND_NAMECHECK_ASSERT(false, "You should not see this #1"); // TODO
        } break;
        
        case EXPR_LIT: {
            if (e->token.kind == TOK_BOOL_LITERAL) {
                t0.type = TYPE_BOOL;
            } else if (e->token.kind == TOK_INT_LITERAL) {
                t0.type = TYPE_INT;
            } else if (e->token.kind == TOK_UINT_LITERAL) {
                t0.type = TYPE_UINT;
            } else if (e->token.kind == TOK_FLOAT_LITERAL) {
                t0.type = TYPE_FLOAT;
            } else if (e->token.kind == TOK_STR_LITERAL) {
                t0.type = TYPE_STR;
            } else {
                TYPE_AND_NAMECHECK_ASSERT(false, "You should not see this #2"); // TODO
            }
            t0.qualifier = QUALIFIER_CONST;
        } break;
        
        case EXPR_CONST: {
            for (size_t i = 0; i < N_BUILTIN_CONSTANTS; i++) {
                if (sv_equals(e->token.text, BUILTIN_CONSTANTS[i].id)) {
                    t0 = (TypeAndQualifier) {BUILTIN_CONSTANTS[i].type, QUALIFIER_CONST};
                    goto out;
                } 
            }
            TYPE_AND_NAMECHECK_ERROR("No such constant: `" HGL_SV_FMT "`.", HGL_SV_ARG(e->token.text));
        } break;

        case N_EXPR_KINDS: {
            TYPE_AND_NAMECHECK_ASSERT(false, "You should not see this #3"); // TODO
        } break;
    }

out:
    e->type = t0.type;
    e->qualifier = t0.qualifier;
    return t0;
}

static TypeAndQualifier type_and_namecheck_function(ExprTree *e, const Func *f, const Type *argtypes, b8 const_args)
{
    /* no more actual arguments? */
    if (e == NULL) {
        TYPE_AND_NAMECHECK_ASSERT(*argtypes == TYPE_NIL, "Too few arguments to built-in function: `%s`.", f->synopsis);
        return (TypeAndQualifier){
            .type = f->type, 
            .qualifier = ((f->qualifier == QUALIFIER_PURE) && const_args) ? QUALIFIER_CONST : QUALIFIER_NONE,
        };
    }

    /* no more arguments expected? */
    if (*argtypes == TYPE_NIL) {
        TYPE_AND_NAMECHECK_ERROR("Too many arguments to built-in function: `%s`.", f->synopsis);
        return (TypeAndQualifier){
            .type = f->type, 
            .qualifier = ((f->qualifier == QUALIFIER_PURE) && const_args) ? QUALIFIER_CONST : QUALIFIER_NONE,
        };
    }

    /* Typecheck left-hand-side expression (head of argslist)*/
    TYPE_AND_NAMECHECK_ASSERT(e->kind == EXPR_ARGLIST, "You should not see this #4");
    TypeAndQualifier t = type_and_namecheck(e->lhs);
    if (t.type == TYPE_AND_NAMECHECKER_ERROR_) {
        return t;
    }
    TYPE_AND_NAMECHECK_ASSERT(t.type == *argtypes, "Type mismatch in arguments to built-in function: "
                              "`%s`. Expected `%s` - Got `%s`.", f->synopsis, TYPE_TO_STR[*argtypes], 
                              TYPE_TO_STR[t.type]);

    /* Typecheck right-hand-side expression (tail of argslist)*/
    return type_and_namecheck_function(e->rhs, f, ++argtypes, const_args && (t.qualifier == QUALIFIER_CONST));

}


/*--- CODEGEN ---------------------------------------------------------------------------*/

static ExeExpr *codegen(const ExprTree *e)
{
    ExeExpr *exe = arena_alloc(g_longterm_arena, sizeof(ExeExpr));
    memset(exe, 0, sizeof(ExeExpr));
    exe->type = e->type;
    exe->qualifier = e->qualifier;
    exe->has_been_computed_once = false;
    codegen_expr(exe, e);
    return exe;
}

static void codegen_expr(ExeExpr *exe, const ExprTree *e)
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

    if (e == NULL) {
        return;
    }

    switch (e->kind) {
        case EXPR_ADD:
        case EXPR_SUB:
        case EXPR_MUL:
        case EXPR_DIV:
        case EXPR_REM: {
            codegen_expr(exe, e->lhs);
            codegen_expr(exe, e->rhs);
            exe_append_op(exe, (Op){
                .kind = expr_to_op[e->kind], 
                .type = e->type,
            });
            //printf("ARITH: %d\n", expr_to_op[e->kind]);
        } break;

        case EXPR_NEG: {
            codegen_expr(exe, e->child);
            exe_append_op(exe, (Op){
                .kind = OP_NEG, 
                .type = e->type,
            });
            //printf("NEG: %d\n", expr_to_op[e->kind]);
        } break;

        case EXPR_PAREN: {
            codegen_expr(exe, e->child);
        } break;

        case EXPR_FUNC: {
            codegen_expr(exe, e->child);
            u32 i = 0;
            for (i = 0; i < (u32)N_BUILTIN_FUNCTIONS; i++) {
                if (sv_equals(e->token.text, BUILTIN_FUNCTIONS[i].id)) {
                    break;
                }
            }
            exe_append_op(exe, (Op){
                .kind = expr_to_op[e->kind], 
                .type = e->type,
                .argsize = sizeof(u32),
            });
            exe_append_u32(exe, i);
            //printf("FUNC: " HGL_SV_FMT "\n", HGL_SV_ARG(e->token.text));
        } break;

        case EXPR_ARGLIST: {
            codegen_expr(exe, e->lhs);
            codegen_expr(exe, e->rhs);
        } break;

        case EXPR_LIT: {
            exe_append_op(exe, (Op){
                .kind    = OP_PUSH,
                .type    = e->type,
                .argsize = TYPE_TO_SIZE[e->type],
            });
            if (e->type == TYPE_BOOL) {
                i32 val = sv_equals(e->token.text, HGL_SV_LIT("true")) ? 1 : 0;
                exe_append_i32(exe, val);
            } else if (e->type == TYPE_INT) {
                i32 val = (i32) sv_to_i64(e->token.text);
                exe_append_i32(exe, val);
            } else if (e->type == TYPE_UINT) {
                u32 val = (u32) sv_to_u64(e->token.text);
                exe_append_u32(exe, val);
            } else if (e->type == TYPE_FLOAT) {
                f32 val = (f32) sv_to_f64(e->token.text);
                exe_append_f32(exe, val);
            } else if (e->type == TYPE_STR) {
                exe_append(exe, &e->token.text, TYPE_TO_SIZE[e->type]);
            } else {
                assert(false && "Logic error in previous compiler steps... #1");
            }
        } break;

        case EXPR_CONST: {
            for (size_t i = 0; i < N_BUILTIN_CONSTANTS; i++) {
                if (sv_equals(e->token.text, BUILTIN_CONSTANTS[i].id)) {
                    exe_append_op(exe, (Op){
                        .kind    = OP_PUSH,
                        .type    = e->type,
                        .argsize = TYPE_TO_SIZE[BUILTIN_CONSTANTS[i].type],
                    });
                    exe_append(exe, &BUILTIN_CONSTANTS[i].value, 
                               TYPE_TO_SIZE[BUILTIN_CONSTANTS[i].type]);
                    break; 
                } 
            }
        } break;

        case N_EXPR_KINDS: {
            assert(false && "Logic error in previous compiler steps... #2");
        } break;

    } 

    return; 
}

static void exe_append_op(ExeExpr *exe, Op op)
{
    exe_append(exe, &op, sizeof(op));
}

static void exe_append_i32(ExeExpr *exe, i32 v)
{
    exe_append(exe, &v, sizeof(v));
}

static void exe_append_u32(ExeExpr *exe, u32 v)
{
    exe_append(exe, &v, sizeof(v));
}

static void exe_append_f32(ExeExpr *exe, f32 v)
{
    exe_append(exe, &v, sizeof(v));
}

static void exe_append(ExeExpr *exe, const void *val, u32 size)
{
    if (exe->code == NULL) {
        exe->size = 0;
        exe->capacity = 64;
        exe->code = arena_alloc(g_longterm_arena, exe->capacity * sizeof(*exe->code));
    } 
    if (exe->capacity < exe->size + size) {
        while (exe->capacity < exe->size + size) {
            exe->capacity *= 2;
        }
        exe->code = arena_realloc(g_longterm_arena, exe->code, exe->capacity * sizeof(*exe->code));
    }
    assert(exe->code != NULL && "eexe_allocator alloc failed");
    memcpy(&exe->code[exe->size], val, size);
    exe->size += size;
}

/*--- Misc. -----------------------------------------------------------------------------*/

static void token_print(Token *token)
{
    printf(HGL_SV_FMT, HGL_SV_ARG(token->text));
}

static b8 is_identifier_char(i32 c)
{
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') ||
           (c == '_');
}

static b8 is_decimal_char(i32 c)
{
    return (c >= '0' && c <= '9');
}

static b8 is_hexadecimal_char(i32 c)
{
    return (c >= '0' && c <= '9') ||
           (c >= 'a' && c <= 'f') ||
           (c >= 'A' && c <= 'F');
}

static void print_expr(ExprTree *e)
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
        case N_EXPR_KINDS:
        default: assert(false); 
    }
    print_expr(e->rhs);
    printf(")");
}

static void print_expr_tree(ExprTree *e)
{
    print_expr_tree_helper(e, 0); 
}

static void print_expr_tree_helper(ExprTree *e, i32 indent)
{
    i32 pad = -2*indent;
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
        case N_EXPR_KINDS: assert(false);
    }
}


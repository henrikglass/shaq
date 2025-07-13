/*--- Include files ---------------------------------------------------------------------*/

#include "sel.h"

#define HGL_STRING_IMPLEMENTATION
#include "hgl_string.h"
#include "alloc.h"
#include "builtins.h"

#include <assert.h>

/*--- Private macros --------------------------------------------------------------------*/

#define TRY(expr_)                                                    \
    do {                                                              \
        int err_ = (int) (expr_);                                     \
        if (err_ != 0) {                                              \
            return err_;                                              \
        }                                                             \
    } while (0)

#define LEXER_ASSERT(cond_, ...)                                      \
    if (!(cond_)) {                                                   \
        fprintf(stderr, "[SEL] Lexer error: " __VA_ARGS__);           \
        fprintf(stderr, "\n");                                        \
        return (Token) {.kind = LEXER_ERROR_};                        \
    }                                                                 \

#define PARSER_ERROR(...)                                             \
    do {                                                              \
        fprintf(stderr, "[SEL] Parser error: " __VA_ARGS__);          \
        fprintf(stderr, "\n");                                        \
        return -1;                                                    \
    } while (0)                                                       \

#define PARSER_ASSERT(cond_, ...)                                     \
    if (!(cond_)) {                                                   \
        fprintf(stderr, "[SEL] Parser error: " __VA_ARGS__);          \
        fprintf(stderr, "\n");                                        \
        return -1;                                                    \
    }                                                                 \

#define TYPE_AND_NAMECHECK_ERROR(...)                                 \
    do {                                                              \
        fprintf(stderr, "[SEL] Type-/namecheck error: " __VA_ARGS__); \
        fprintf(stderr, "\n");                                        \
        return TYPE_AND_NAMECHECKER_ERROR_;                           \
    } while (0)

#define TYPE_AND_NAMECHECK_ASSERT(cond_, ...)                         \
    if (!(cond_)) {                                                   \
        fprintf(stderr, "[SEL] Type-/namecheck error: " __VA_ARGS__); \
        fprintf(stderr, "\n");                                        \
        return TYPE_AND_NAMECHECKER_ERROR_;                           \
    }

#define SVM_STACK_SIZE (16*1024)

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
    TOK_FLOAT_LITERAL,
    TOK_IDENTIFIER,
    EOF_TOKEN_,
    LEXER_ERROR_,
    N_TOKENS_,
} TokenKind;

typedef struct
{
    TokenKind kind;
    HglStringView text; 
} Token;

typedef struct
{
    HglStringView buf; 
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
    union {
        struct ExprTree *child;
        struct ExprTree *lhs;
    };
    struct ExprTree *rhs;
} ExprTree;

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
    // Todo LHS/RHS types? Union?
    u8 argsize;
    u8 pad[1];
} Op;
static_assert(sizeof(Op) == 4, "");

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
static int parse_add_expr(ExprTree **e, Lexer *l);
static int parse_mul_expr(ExprTree **e, Lexer *l);
static int parse_unary_or_atom_expr(ExprTree **e, Lexer *l);
static int parse_arglist_expr(ExprTree **e, Lexer *l);

/* Type-/namechecker */
static Type type_and_namecheck(ExprTree *e);
static Type type_and_namecheck_function(ExprTree *e, const Function *f, const Type *argtypes);

/* codegen */
static ExeExpr *codegen(const ExprTree *e);
static void codegen_expr(ExeExpr *exe, const ExprTree *e);
static void exe_append_op(ExeExpr *exe, Op op);
static void exe_append_bool(ExeExpr *exe, bool v);
static void exe_append_i32(ExeExpr *exe, i32 v);
static void exe_append_i32(ExeExpr *exe, i32 v);
static void exe_append_f32(ExeExpr *exe, f32 v);
static void exe_append(ExeExpr *exe, const void *val, u32 size);

/* virtual machine/interpreter */
static void svm_run(void);
static void svm_reset(void);
static inline void *svm_next_bytes(u32 size);
static inline Op *svm_next_op(void);
static inline void svm_stack_push(void *data, u32 size);
static inline void svm_stack_push_selvalue(SelValue v, Type t);
static inline void *svm_stack_pop(u32 size);
static inline i32 addi(i32 *lhs, i32 *rhs);
static inline f32 addf(f32 *lhs, f32 *rhs);
static inline Vec2 addv2(Vec2 *lhs, Vec2 *rhs);
static inline Vec3 addv3(Vec3 *lhs, Vec3 *rhs);
static inline Vec4 addv4(Vec4 *lhs, Vec4 *rhs);
static inline i32 subi(i32 *lhs, i32 *rhs);
static inline f32 subf(f32 *lhs, f32 *rhs);
static inline Vec2 subv2(Vec2 *lhs, Vec2 *rhs);
static inline Vec3 subv3(Vec3 *lhs, Vec3 *rhs);
static inline Vec4 subv4(Vec4 *lhs, Vec4 *rhs);
static inline i32 muli(i32 *lhs, i32 *rhs);
static inline f32 mulf(f32 *lhs, f32 *rhs);
static inline Vec2 mulv2(Vec2 *lhs, Vec2 *rhs);
static inline Vec3 mulv3(Vec3 *lhs, Vec3 *rhs);
static inline Vec4 mulv4(Vec4 *lhs, Vec4 *rhs);
static inline i32 divi(i32 *lhs, i32 *rhs);
static inline f32 divf(f32 *lhs, f32 *rhs);
static inline Vec2 divv2(Vec2 *lhs, Vec2 *rhs);
static inline Vec3 divv3(Vec3 *lhs, Vec3 *rhs);
static inline Vec4 divv4(Vec4 *lhs, Vec4 *rhs);
static inline i32 remi(i32 *lhs, i32 *rhs);
static inline i32 negi(i32 *val);
static inline f32 negf(f32 *val);

/* misc. debug */
static void token_print(Token *t);
static bool is_identifier_char(int c);
static bool is_decimal_char(int c);
static bool is_hexadecimal_char(int c);
static inline void print_expr(ExprTree *e);
static inline void print_expr_tree(ExprTree *e);
static inline void print_expr_tree_helper(ExprTree *e, int indent);


/*--- Public variables ------------------------------------------------------------------*/

/*--- Private variables -----------------------------------------------------------------*/

static const char *const TYPE_TO_STR[] =
{
    [TYPE_NIL]    = "nil",
    [TYPE_BOOL]   = "bool",
    [TYPE_INT]    = "int",
    [TYPE_FLOAT]  = "float",
    //[TYPE_BVEC2]  = "bvec2",
    //[TYPE_BVEC3]  = "bvec3",
    //[TYPE_BVEC4]  = "bvec4",
    [TYPE_VEC2]   = "vec2",
    [TYPE_VEC3]   = "vec3",
    [TYPE_VEC4]   = "vec4",
    [TYPE_IVEC2]  = "ivec2",
    [TYPE_IVEC3]  = "ivec3",
    [TYPE_IVEC4]  = "ivec4",
    [TYPE_MAT2]   = "mat2",
    [TYPE_MAT3]   = "mat3",
    [TYPE_MAT4]   = "mat4",
    //[TYPE_IMAGE]  = "image",
    [TYPE_AND_NAMECHECKER_ERROR_] = "type-/namechecker error",
};

static const u32 TYPE_TO_SIZE[] = 
{
    [TYPE_NIL]    = 0,
    [TYPE_BOOL]   = sizeof(bool),
    [TYPE_INT]    = sizeof(i32),
    [TYPE_FLOAT]  = sizeof(f32),
    //[TYPE_BVEC2]  = 2,
    //[TYPE_BVEC3]  = 3,
    //[TYPE_BVEC4]  = 4,
    [TYPE_VEC2]   = 8,
    [TYPE_VEC3]   = 12,
    [TYPE_VEC4]   = 16,
    [TYPE_IVEC2]  = 8,
    [TYPE_IVEC3]  = 12,
    [TYPE_IVEC4]  = 16,
    [TYPE_MAT2]   = 16,
    [TYPE_MAT3]   = 36,
    [TYPE_MAT4]   = 64,
};

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

/* Simple Expression Language Virtual Machine */
static struct SVM {
    const ExeExpr *exe;
    u8 stack[SVM_STACK_SIZE];
    u32 pc;
    u32 sp;
} svm = {0};

static HglArena temp_allocator = HGL_ARENA_INITIALIZER(1024*1024);
static HglArena eexpr_allocator = HGL_ARENA_INITIALIZER(1024*1024);

/*--- Public functions ------------------------------------------------------------------*/

ExeExpr *sel_compile(const char *src)
{
    /* lexer + parser step */
    ExprTree *e = parse_expr(src);
    if (e == NULL) {
        return NULL;
    }

    /* type + namecheck step */
    Type type = type_and_namecheck(e);
    assert(type != TYPE_NIL); // should not be possible
    if (type == TYPE_AND_NAMECHECKER_ERROR_) {
        return NULL;
    }

    /* codegen step. *Should* never fail if the previous steps succeed */
    ExeExpr *exe = codegen(e);

    arena_free_all(&temp_allocator);
    return exe;
}

SelValue sel_run(ExeExpr *exe)
{
    /* Reset SVM & load program */
    svm_reset();
    svm.exe = exe;

    /* Execute in interpreter */
    svm_run(); 

    /* Assert machine state is as expected */
    u32 tsize = TYPE_TO_SIZE[svm.exe->type];
    assert(svm.sp - tsize == 0);
    assert(svm.pc == svm.exe->size);

    /* retreive, pack, and return the result result */
    void *raw_result = svm_stack_pop(tsize);
    SelValue result = {0};
    memcpy(&result, raw_result, tsize); // Okay? Otherwise switch on exe->type

    return result;
}

/*--- LEXER -----------------------------------------------------------------------------*/

static Lexer lexer_begin(const char *str)
{
    Lexer l = {0};
    l.buf = hgl_sv_from_cstr(str);
    return l;
}

static Token lexer_next(Lexer *l)
{
    Token t = lexer_peek(l);
    hgl_sv_lchop(&l->buf, t.text.length);
    return t;
}

static void lexer_eat(Lexer *l)
{
    (void) lexer_next(l);
}

static Token lexer_peek(Lexer *l)
{
    // TODO anneal this brittle crap

    l->buf = hgl_sv_ltrim(l->buf);

    if (l->buf.length == 0) {
        return (Token) {.kind = EOF_TOKEN_};
    }

    char c = l->buf.start[0];
    switch (c) {
        case '\0': return (Token) {.kind = EOF_TOKEN_}; break;
        case '\n': return (Token) {.kind = EOF_TOKEN_}; break;
        case '(':  return (Token) {.kind = TOK_LPAREN,  .text = hgl_sv_substr(l->buf, 0, 1)}; break;
        case ')':  return (Token) {.kind = TOK_RPAREN,  .text = hgl_sv_substr(l->buf, 0, 1)}; break;
        case '+':  return (Token) {.kind = TOK_PLUS,    .text = hgl_sv_substr(l->buf, 0, 1)}; break;
        case '-':  return (Token) {.kind = TOK_MINUS,   .text = hgl_sv_substr(l->buf, 0, 1)}; break;
        case '*':  return (Token) {.kind = TOK_STAR,    .text = hgl_sv_substr(l->buf, 0, 1)}; break;
        case '/':  return (Token) {.kind = TOK_FSLASH,  .text = hgl_sv_substr(l->buf, 0, 1)}; break;
        case '%':  return (Token) {.kind = TOK_PERCENT, .text = hgl_sv_substr(l->buf, 0, 1)}; break;
        case ',':  return (Token) {.kind = TOK_COMMA,   .text = hgl_sv_substr(l->buf, 0, 1)}; break;

        case 'a' ... 'z': case 'A' ... 'Z': case '_': {
            size_t i = 1;
            for (; i < l->buf.length; i++) {
                c = l->buf.start[i];
                if (!is_identifier_char(c)) break;
            }

            HglStringView s = hgl_sv_substr(l->buf, 0, i);

            /* boolean literal? */
            if (hgl_sv_equals(s, HGL_SV_LIT("true")) ||
                hgl_sv_equals(s, HGL_SV_LIT("false"))) {
                return (Token) {
                    .kind = TOK_BOOL_LITERAL, 
                    .text = s,
                };
            }

            return (Token) {
                .kind = TOK_IDENTIFIER, 
                .text = s,
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
                    LEXER_ASSERT(!is_identifier_char(c));
                }
                return (Token) {
                    .kind = TOK_INT_LITERAL,
                    .text = hgl_sv_substr(l->buf, 0, i),
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
                .text = hgl_sv_substr(l->buf, 0, i),
            };

        } break;
    }

    //LEXER_ASSERT(!is_identifier_char(c));
    return (Token) {.kind = LEXER_ERROR_, .text = l->buf};
}


/*--- PARSER ----------------------------------------------------------------------------*/

static ExprTree *parse_expr(const char *str)
{
    Lexer l = lexer_begin(str);
    ExprTree *e = NULL;
    int err = parse_add_expr(&e, &l);
    if (err != 0) {
        return NULL;
    }
    return e;
}

static int parse_add_expr(ExprTree **e, Lexer *l)
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

static int parse_mul_expr(ExprTree **e, Lexer *l)
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

static int parse_unary_or_atom_expr(ExprTree **e, Lexer *l)
{
    ExprTree *tmp;
    Token t;

    // TODO fix this behavior
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
        case TOK_FLOAT_LITERAL: {
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

static int parse_arglist_expr(ExprTree **e, Lexer *l)
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
    ExprTree *e = arena_alloc(&temp_allocator, sizeof(ExprTree));
    e->kind = kind;
    e->token = token;
    e->lhs = lhs;
    e->rhs = rhs;
    return e;
}

static ExprTree *new_unary_expr(ExprKind kind, Token token, ExprTree *child)
{
    ExprTree *e = arena_alloc(&temp_allocator, sizeof(ExprTree));
    e->kind = kind;
    e->token = token;
    e->child = child;
    return e;
}

static ExprTree *new_atom_expr(ExprKind kind, Token token)
{
    ExprTree *e = arena_alloc(&temp_allocator, sizeof(ExprTree));
    e->kind = kind;
    e->token = token;
    return e;
}


/*--- TYPE-/NAMECHECKER -----------------------------------------------------------------*/

static Type type_and_namecheck(ExprTree *e)
{
    Type t0, t1;

    if (e == NULL) {
        return TYPE_NIL;
    }

    switch (e->kind) {
        
        case EXPR_ADD:
        case EXPR_SUB: {
            t0 = type_and_namecheck(e->lhs); 
            t1 = type_and_namecheck(e->rhs); 
            TYPE_AND_NAMECHECK_ASSERT(t0 == t1, "Operands to arithmetic operation are of different types: "
                                      "Got `%s` and `%s`.", TYPE_TO_STR[t0], TYPE_TO_STR[t1]);
            // TODO support implicit type conversions Add lhs + rhs types to Op?
            TYPE_AND_NAMECHECK_ASSERT(t0 != TYPE_BOOL, "No arithmetic on bools is allowed (yet).");
            TYPE_AND_NAMECHECK_ASSERT(t0 != TYPE_MAT2 && t0 != TYPE_MAT3 && t0 != TYPE_MAT4, "Matricies may not (yet) be directly added. Use built-in functions instead.");
            TYPE_AND_NAMECHECK_ASSERT(t0 != TYPE_IVEC2 && t0 != TYPE_IVEC2 && t0 != TYPE_IVEC2, "Integer vectors may not (yet) be directly added. Use built-in functions instead.");
        } break;

        case EXPR_MUL:
        case EXPR_DIV: {
            t0 = type_and_namecheck(e->lhs); 
            t1 = type_and_namecheck(e->rhs); 
            TYPE_AND_NAMECHECK_ASSERT(t0 == t1, "Operands to arithmetic operation are of different types: "
                                      "Got `%s` and `%s`.", TYPE_TO_STR[t0], TYPE_TO_STR[t1]);
            TYPE_AND_NAMECHECK_ASSERT(t0 != TYPE_BOOL, "No arithmetic on bools is allowed (yet).");
            TYPE_AND_NAMECHECK_ASSERT(t0 != TYPE_MAT2 && t0 != TYPE_MAT3 && t0 != TYPE_MAT4, "Matricies may not (yet) be directly multiplied. Use built-in functions instead.");
            TYPE_AND_NAMECHECK_ASSERT(t0 != TYPE_IVEC2 && t0 != TYPE_IVEC2 && t0 != TYPE_IVEC2, "Integer vectors may not (yet) be directly multiplied. Use built-in functions instead.");
        } break;
        
        case EXPR_REM: {
            t0 = type_and_namecheck(e->lhs); 
            t1 = type_and_namecheck(e->rhs); 
            TYPE_AND_NAMECHECK_ASSERT(t0 == TYPE_INT, "Left-hand-side operand to remainder operation is not an INT.");
            TYPE_AND_NAMECHECK_ASSERT(t1 == TYPE_INT, "Right-hand-side operand to remainder operation is not an INT.");
        } break;
        
        case EXPR_NEG: {
            // TODO better rule
            t0 = type_and_namecheck(e->child);
            TYPE_AND_NAMECHECK_ASSERT(t0 == TYPE_INT || t0 == TYPE_FLOAT , 
                                      "Operand to unary minus operator must be of type INT or FLOAT.");
        } break;
        
        case EXPR_PAREN: {
            t0 = type_and_namecheck(e->child);
        } break;
        
        case EXPR_FUNC: {
            for (size_t i = 0; i < N_BUILTIN_FUNCTIONS; i++) {
                if (hgl_sv_equals(e->token.text, BUILTIN_FUNCTIONS[i].id)) {
                    t0 = type_and_namecheck_function(e->child, &BUILTIN_FUNCTIONS[i], BUILTIN_FUNCTIONS[i].argtypes);
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
                t0 = TYPE_BOOL;
            } else if (e->token.kind == TOK_INT_LITERAL) {
                t0 = TYPE_INT;
            } else if (e->token.kind == TOK_FLOAT_LITERAL) {
                t0 = TYPE_FLOAT;
            } else {
                TYPE_AND_NAMECHECK_ASSERT(false, "You should not see this #2"); // TODO
            }
        } break;
        
        case EXPR_CONST: {
            for (size_t i = 0; i < N_BUILTIN_CONSTANTS; i++) {
                if (hgl_sv_equals(e->token.text, BUILTIN_CONSTANTS[i].id)) {
                    t0 = TYPE_FLOAT;
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
    e->type = t0;
    return e->type;
}

static Type type_and_namecheck_function(ExprTree *e, const Function *f, const Type *argtypes)
{
    /* no more actual arguments? */
    if (e == NULL) {
        TYPE_AND_NAMECHECK_ASSERT(*argtypes == TYPE_NIL, "Too few arguments to built-in function: `%s`.", f->synopsis);
        return f->type;
    }

    /* no more arguments expected? */
    if (*argtypes == TYPE_NIL) {
        TYPE_AND_NAMECHECK_ERROR("Too many arguments to built-in function: `%s`.", f->synopsis);
        return f->type;
    }

    /* Typecheck left-hand-side expression (head of argslist)*/
    TYPE_AND_NAMECHECK_ASSERT(e->kind == EXPR_ARGLIST, "You should not see this #4");
    Type t = type_and_namecheck(e->lhs);
    if (t == TYPE_AND_NAMECHECKER_ERROR_) {
        return t;
    }
    TYPE_AND_NAMECHECK_ASSERT(t == *argtypes, "Type mismatch in arguments to built-in function: `%s`. Expected `%s` - Got `%s`.",
                              f->synopsis, TYPE_TO_STR[*argtypes], TYPE_TO_STR[t]);

    /* Typecheck right-hand-side expression (tail of argslist)*/
    return type_and_namecheck_function(e->rhs, f, ++argtypes);

}


/*--- CODEGEN ---------------------------------------------------------------------------*/

static ExeExpr *codegen(const ExprTree *e)
{
    ExeExpr *exe = arena_alloc(&eexpr_allocator, sizeof(ExeExpr));
    memset(exe, 0, sizeof(ExeExpr));
    exe->type = e->type;
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
            i32 i = 0;
            for (i = 0; i < (i32)N_BUILTIN_FUNCTIONS; i++) {
                if (hgl_sv_equals(e->token.text, BUILTIN_FUNCTIONS[i].id)) {
                    break;
                }
            }
            exe_append_op(exe, (Op){
                .kind = expr_to_op[e->kind], 
                .type = e->type,
                .argsize = sizeof(i32),
            });
            exe_append_i32(exe, i);
            //printf("FUNC: " HGL_SV_FMT "\n", HGL_SV_ARG(e->token.text));
        } break;

        case EXPR_ARGLIST: {
            codegen_expr(exe, e->lhs);
            codegen_expr(exe, e->rhs);
        } break;

        case EXPR_LIT: {
            if (e->type == TYPE_BOOL) {
                bool val = hgl_sv_equals(e->token.text, HGL_SV_LIT("true"));
                exe_append_op(exe, (Op){
                    .kind    = OP_PUSH,
                    .type    = e->type,
                    .argsize = sizeof(bool),
                });
                exe_append_bool(exe, val);
            } else if (e->type == TYPE_INT) {
                i32 val = (i32) hgl_sv_to_i64(e->token.text);
                exe_append_op(exe, (Op){
                    .kind    = OP_PUSH,
                    .type    = e->type,
                    .argsize = sizeof(i32),
                });
                //printf("PUSH: %d\n", val);
                exe_append_i32(exe, val);
            } else if (e->type == TYPE_FLOAT) {
                f32 val = (f32) hgl_sv_to_f64(e->token.text);
                exe_append_op(exe, (Op){
                    .kind    = OP_PUSH,
                    .type    = e->type,
                    .argsize = sizeof(f32),
                });
                //printf("PUSH: %f\n", (f64)val);
                exe_append_f32(exe, val);
            } else {
                assert(false && "Logic error in previous compiler steps... #1");
            }
        } break;

        case EXPR_CONST: {
            for (size_t i = 0; i < N_BUILTIN_CONSTANTS; i++) {
                if (hgl_sv_equals(e->token.text, BUILTIN_CONSTANTS[i].id)) {
                    f32 val = BUILTIN_CONSTANTS[i].value;
                    exe_append_op(exe, (Op){
                        .kind    = OP_PUSH,
                        .type    = e->type,
                        .argsize = sizeof(f32),
                    });
                    //printf("PUSH: %f\n", (f64)val);
                    exe_append_f32(exe, val);
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

static void exe_append_bool(ExeExpr *exe, bool v)
{
    exe_append(exe, &v, sizeof(v));
}

static void exe_append_i32(ExeExpr *exe, i32 v)
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
        exe->code = arena_alloc(&eexpr_allocator, exe->capacity * sizeof(*exe->code));
    } 
    if (exe->capacity < exe->size + size) {
        while (exe->capacity < exe->size + size) {
            exe->capacity *= 2;
        }
        exe->code = arena_realloc(&eexpr_allocator, exe->code, exe->capacity * sizeof(*exe->code));
    }
    assert(exe->code != NULL && "eexe_allocator alloc failed");
    memcpy(&exe->code[exe->size], val, size);
    exe->size += size;
}


/*--- VIRTUAL MACHINE/INTERPRETER -------------------------------------------------------*/

static void svm_run()
{
    while (true) {

        /* end-of-program */
        if (svm.pc >= svm.exe->size) {
            break;
        }

        Op *op = svm_next_op();
        u32 tsize = TYPE_TO_SIZE[op->type];
        
        switch (op->kind) {
            case OP_PUSH: {
                svm_stack_push(svm_next_bytes(op->argsize), op->argsize);
            } break;

            case OP_ADD: {
                void *rhs = svm_stack_pop(tsize);
                void *lhs = svm_stack_pop(tsize);
                switch (op->type) {
                    case TYPE_INT: {i32 tmp = addi(lhs, rhs); svm_stack_push(&tmp, sizeof(tmp));} break;
                    case TYPE_FLOAT: {f32 tmp = addf(lhs, rhs); svm_stack_push(&tmp, sizeof(tmp));} break;
                    case TYPE_VEC2:  {Vec2 tmp = addv2(lhs, rhs); svm_stack_push(&tmp, sizeof(tmp));} break;
                    case TYPE_VEC3:  {Vec3 tmp = addv3(lhs, rhs); svm_stack_push(&tmp, sizeof(tmp));} break;
                    case TYPE_VEC4:  {Vec4 tmp = addv4(lhs, rhs); svm_stack_push(&tmp, sizeof(tmp));} break;
                    default: assert(false);
                }
            } break;

            case OP_SUB: {
                void *rhs = svm_stack_pop(tsize);
                void *lhs = svm_stack_pop(tsize);
                switch (op->type) {
                    case TYPE_INT: {i32 tmp = subi(lhs, rhs); svm_stack_push(&tmp, sizeof(tmp));} break;
                    case TYPE_FLOAT: {f32 tmp = subf(lhs, rhs); svm_stack_push(&tmp, sizeof(tmp));} break;
                    case TYPE_VEC2:  {Vec2 tmp = subv2(lhs, rhs); svm_stack_push(&tmp, sizeof(tmp));} break;
                    case TYPE_VEC3:  {Vec3 tmp = subv3(lhs, rhs); svm_stack_push(&tmp, sizeof(tmp));} break;
                    case TYPE_VEC4:  {Vec4 tmp = subv4(lhs, rhs); svm_stack_push(&tmp, sizeof(tmp));} break;
                    default: assert(false);
                }
            } break;

            case OP_MUL: {
                void *rhs = svm_stack_pop(tsize);
                void *lhs = svm_stack_pop(tsize);
                switch (op->type) {
                    case TYPE_INT: {i32 tmp = muli(lhs, rhs); svm_stack_push(&tmp, sizeof(tmp));} break;
                    case TYPE_FLOAT: {f32 tmp = mulf(lhs, rhs); svm_stack_push(&tmp, sizeof(tmp));} break;
                    case TYPE_VEC2:  {Vec2 tmp = mulv2(lhs, rhs); svm_stack_push(&tmp, sizeof(tmp));} break;
                    case TYPE_VEC3:  {Vec3 tmp = mulv3(lhs, rhs); svm_stack_push(&tmp, sizeof(tmp));} break;
                    case TYPE_VEC4:  {Vec4 tmp = mulv4(lhs, rhs); svm_stack_push(&tmp, sizeof(tmp));} break;
                    default: assert(false);
                }
            } break;

            case OP_DIV: {
                void *rhs = svm_stack_pop(tsize);
                void *lhs = svm_stack_pop(tsize);
                switch (op->type) {
                    case TYPE_INT: {i32 tmp = divi(lhs, rhs); svm_stack_push(&tmp, sizeof(tmp));} break;
                    case TYPE_FLOAT: {f32 tmp = divf(lhs, rhs); svm_stack_push(&tmp, sizeof(tmp));} break;
                    case TYPE_VEC2:  {Vec2 tmp = divv2(lhs, rhs); svm_stack_push(&tmp, sizeof(tmp));} break;
                    case TYPE_VEC3:  {Vec3 tmp = divv3(lhs, rhs); svm_stack_push(&tmp, sizeof(tmp));} break;
                    case TYPE_VEC4:  {Vec4 tmp = divv4(lhs, rhs); svm_stack_push(&tmp, sizeof(tmp));} break;
                    default: assert(false);
                }
            } break;

            case OP_REM: {
                void *rhs = svm_stack_pop(tsize);
                void *lhs = svm_stack_pop(tsize);
                switch (op->type) {
                    case TYPE_INT: {i32 tmp = remi(lhs, rhs); svm_stack_push(&tmp, sizeof(tmp));} break;
                    default: assert(false);
                }
            } break;

            case OP_NEG: {
                void *val = svm_stack_pop(tsize);
                switch (op->type) {
                    case TYPE_INT: {i32 tmp = negi(val); svm_stack_push(&tmp, sizeof(tmp));} break;
                    case TYPE_FLOAT: {f32 tmp = negf(val); svm_stack_push(&tmp, sizeof(tmp));} break;
                    default: assert(false);
                }
            } break;

            case OP_FUNC: {
                i32 func_id = *(i32*)svm_next_bytes(sizeof(i32));
                const Function *func = &BUILTIN_FUNCTIONS[func_id];
                for (i32 i = 0; i < BUILTIN_MAX_N_ARGS; i++) {
                    if (func->argtypes[i] == TYPE_NIL) {
                        break;
                    }
                    svm_stack_pop(TYPE_TO_SIZE[func->argtypes[i]]);
                }
                SelValue res = (func->impl)(&svm.stack[svm.sp]);
                svm_stack_push_selvalue(res, func->type);
            } break;
        }
    }
}

static void svm_reset(void)
{
    svm.exe = NULL;
    svm.pc  = 0;
    svm.sp  = 0;
}

static inline void *svm_next_bytes(u32 size)
{
    void *data = (void *)&svm.exe->code[svm.pc]; 
    svm.pc += size;
    return data;
}

static inline Op *svm_next_op()
{
    Op *op = (Op *)&svm.exe->code[svm.pc];
    svm.pc += sizeof(Op);
    return op;
}

static inline void svm_stack_push(void *data, u32 size)
{
    if (svm.sp + size > SVM_STACK_SIZE) {
        assert(false && "out of stack space");
    }
    memcpy(&svm.stack[svm.sp], data, size);
    svm.sp += size;
}

static inline void svm_stack_push_selvalue(SelValue v, Type t)
{
    memcpy(&svm.stack[svm.sp], &v, TYPE_TO_SIZE[t]);
    svm.sp += TYPE_TO_SIZE[t];
}

static inline void *svm_stack_pop(u32 size)
{
    if (svm.sp < size) {
        assert(false && "popping below stack boundary");
    }
    svm.sp -= size;
    return &svm.stack[svm.sp];
}

static inline i32 addi(i32 *lhs, i32 *rhs) { return (*lhs) + (*rhs); }
static inline f32 addf(f32 *lhs, f32 *rhs) { return (*lhs) + (*rhs); }
static inline Vec2 addv2(Vec2 *lhs, Vec2 *rhs) {return vec2_add(*lhs, *rhs);}
static inline Vec3 addv3(Vec3 *lhs, Vec3 *rhs) {return vec3_add(*lhs, *rhs);}
static inline Vec4 addv4(Vec4 *lhs, Vec4 *rhs) {return vec4_add(*lhs, *rhs);}

static inline i32 subi(i32 *lhs, i32 *rhs) { return (*lhs) - (*rhs); }
static inline f32 subf(f32 *lhs, f32 *rhs) { return (*lhs) - (*rhs); }
static inline Vec2 subv2(Vec2 *lhs, Vec2 *rhs) {return vec2_sub(*lhs, *rhs);}
static inline Vec3 subv3(Vec3 *lhs, Vec3 *rhs) {return vec3_sub(*lhs, *rhs);}
static inline Vec4 subv4(Vec4 *lhs, Vec4 *rhs) {return vec4_sub(*lhs, *rhs);}

static inline i32 muli(i32 *lhs, i32 *rhs) { return (*lhs) * (*rhs); }
static inline f32 mulf(f32 *lhs, f32 *rhs) { return (*lhs) * (*rhs); }
static inline Vec2 mulv2(Vec2 *lhs, Vec2 *rhs) {return vec2_hadamard(*lhs, *rhs);}
static inline Vec3 mulv3(Vec3 *lhs, Vec3 *rhs) {return vec3_hadamard(*lhs, *rhs);}
static inline Vec4 mulv4(Vec4 *lhs, Vec4 *rhs) {return vec4_hadamard(*lhs, *rhs);}

static inline i32 divi(i32 *lhs, i32 *rhs) { return (*lhs) / (*rhs); }
static inline f32 divf(f32 *lhs, f32 *rhs) { return (*lhs) / (*rhs); }
static inline Vec2 divv2(Vec2 *lhs, Vec2 *rhs) {return vec2_hadamard(*lhs, vec2_recip(*rhs));}
static inline Vec3 divv3(Vec3 *lhs, Vec3 *rhs) {return vec3_hadamard(*lhs, vec3_recip(*rhs));}
static inline Vec4 divv4(Vec4 *lhs, Vec4 *rhs) {return vec4_hadamard(*lhs, vec4_recip(*rhs));}

static inline i32 remi(i32 *lhs, i32 *rhs) { return (*lhs) % (*rhs); }
static inline i32 negi(i32 *val) { return -(*val); }
static inline f32 negf(f32 *val) { return -(*val); }

/*--- Misc. -----------------------------------------------------------------------------*/

static void token_print(Token *token)
{
    printf(HGL_SV_FMT, HGL_SV_ARG(token->text));
}

static bool is_identifier_char(int c)
{
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') ||
           (c == '_');
}

static bool is_decimal_char(int c)
{
    return (c >= '0' && c <= '9') ||
           (c == '_');
}

static bool is_hexadecimal_char(int c)
{
    return (c >= '0' && c <= '9') ||
           (c >= 'a' && c <= 'f') ||
           (c >= 'A' && c <= 'F') ||
           (c == '_');
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

static void print_expr_tree_helper(ExprTree *e, int indent)
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
        case N_EXPR_KINDS: assert(false);
    }
}


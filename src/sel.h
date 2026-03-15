#ifndef SEL_H
#define SEL_H

/*--- Include files ---------------------------------------------------------------------*/

#include "hgl_int.h"
#include "hgl_float.h"

#include "str.h"
#include "vecmath.h"

/*--- Public macros ---------------------------------------------------------------------*/

#define SEL_FUNC_MAX_N_ARGS 8
#define SEL_EMPTY_SVM_CONTEXT (SVMContext){.shader = NULL}

/*--- Public type definitions -----------------------------------------------------------*/

typedef enum
{
    TYPE_NIL = 0,
    TYPE_BOOL,
    TYPE_INT,
    TYPE_UINT,
    TYPE_FLOAT,
    TYPE_VEC2,
    TYPE_VEC3,
    TYPE_VEC4,
    TYPE_IVEC2,
    TYPE_IVEC3,
    TYPE_IVEC4,
    //TYPE_UVEC2, // TODO
    //TYPE_UVEC3,
    //TYPE_UVEC4,
    TYPE_MAT2,
    TYPE_MAT3,
    TYPE_MAT4,
    TYPE_STR,
    TYPE_TEXTURE,
    TYPE_OR_NAME_ERR_,
    N_TYPES,
} Type;
static_assert(N_TYPES <= 256, "");

typedef enum
{
    QUALIFIER_NONE  =  0,
    QUALIFIER_CONST = (1 << 0), // for constant expression
    QUALIFIER_PURE  = (1 << 1), // for pure functions
    QUALIFIER_ERR_  = (1 << 2),
} TypeQualifier;

typedef enum
{
    SHADER_CURRENT_RENDER_TEXTURE,
    SHADER_LAST_RENDER_TEXTURE,
    LOADED_TEXTURE,
} TextureKind;

typedef struct
{
    u32 error; // TODO move
    TextureKind kind;
    i32 id;
    i32 filter; // Both min & mag filters
    i32 wrap;   // Both S & T directions
} TextureDescriptor;

typedef union
{
    i32 val_bool; 
    i32 val_i32; 
    u32 val_u32; 
    f32 val_f32; 
    Vec2 val_vec2;
    Vec3 val_vec3;
    Vec4 val_vec4;
    IVec2 val_ivec2;
    IVec3 val_ivec3;
    IVec4 val_ivec4;
    Mat2 val_mat2;
    Mat3 val_mat3;
    Mat4 val_mat4;
    StringView val_str;
    TextureDescriptor val_tex;
} SelValue;

typedef struct
{
    StringView id;
    Type type;
    SelValue value;
} Const;

typedef struct
{
    StringView id;
    TypeQualifier qualifier;
    Type type;
    SelValue (*impl)(void *args);
    Type argtypes[SEL_FUNC_MAX_N_ARGS];
    const char *synopsis;
    const char *desc;
} Func;

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
    OP_SWIZZLE,
    OP_TYPECONV,
} OpKind;

typedef struct
{
    u8 kind;
    u8 res_type;
    union {
        struct {
            u8 lhs_type;
            u8 rhs_type;
        };
        struct {
            u8 from_type;
            u8 pad_;
        };
        u8 argsize;
    };
} Op;
static_assert(sizeof(Op) == 4, "");

/* "executable" expression */
typedef struct
{
    u8 *code;
    u32 size;
    u32 capacity;
    Type type;
    TypeQualifier qualifier;
    SelValue cached_computed_value;
    b8 has_been_computed_once;
    const char *source_code;
    struct ExprTree *tree; // TODO remove
} ExeExpr;

/* 
 * Describes the context in which an executable expression
 * is evaluated.
 */
typedef struct
{
    struct Shader *shader;
} SVMContext;

/*--- Public variables ------------------------------------------------------------------*/

extern const Const BUILTIN_CONSTANTS[];
extern const u32 N_BUILTIN_CONSTANTS;
extern const Func BUILTIN_FUNCTIONS[];
extern const u32 N_BUILTIN_FUNCTIONS;

static const char *const TYPE_TO_STR[] =
{
    [TYPE_NIL]        = "nil",
    [TYPE_BOOL]       = "bool",
    [TYPE_INT]        = "int",
    [TYPE_UINT]       = "uint",
    [TYPE_FLOAT]      = "float",
    [TYPE_VEC2]       = "vec2",
    [TYPE_VEC3]       = "vec3",
    [TYPE_VEC4]       = "vec4",
    [TYPE_IVEC2]      = "ivec2",
    [TYPE_IVEC3]      = "ivec3",
    [TYPE_IVEC4]      = "ivec4",
    [TYPE_MAT2]       = "mat2",
    [TYPE_MAT3]       = "mat3",
    [TYPE_MAT4]       = "mat4",
    [TYPE_STR]        = "str",
    [TYPE_TEXTURE]    = "sampler2D",
    [TYPE_OR_NAME_ERR_] = "type-/namechecker error",
};

static const u32 TYPE_TO_SIZE[] = 
{
    [TYPE_NIL]       = 0,
    [TYPE_BOOL]      = sizeof(i32),
    [TYPE_INT]       = sizeof(i32),
    [TYPE_UINT]      = sizeof(u32),
    [TYPE_FLOAT]     = sizeof(f32),
    [TYPE_VEC2]      = 8,
    [TYPE_VEC3]      = 12,
    [TYPE_VEC4]      = 16,
    [TYPE_IVEC2]     = 8,
    [TYPE_IVEC3]     = 12,
    [TYPE_IVEC4]     = 16,
    [TYPE_MAT2]      = 16,
    [TYPE_MAT3]      = 36,
    [TYPE_MAT4]      = 64,
    [TYPE_STR]       = sizeof(StringView),
    [TYPE_TEXTURE]   = sizeof(TextureDescriptor),
    [TYPE_OR_NAME_ERR_] = 0,
};
static_assert(sizeof(TYPE_TO_SIZE)/sizeof(TYPE_TO_SIZE[0]) == N_TYPES);

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
    TOK_DOT,
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
    EXPR_SWIZZLE,
    EXPR_PAREN,
    EXPR_FUNC,
    EXPR_ARGLIST,
    EXPR_LIT,
    EXPR_ID,
    N_EXPR_KINDS,
} ExprKind;

static_assert(N_EXPR_KINDS <= 256, "");

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
    Type convert_to; // If this value is != TYPE_NIL, The result of the computation
                     // of this expression tree must be converted from type `type`
                     // to type `convert_to`
    u32 func_id;     // If this tree node is a function call then this value will be
                     // assigned the ID of the built-in function in the type- and
                     // namechecker pass. If no matching built-in function is found
                     // the type- and namechecker pass will not succeed, and the root
                     // of the tree will have sentinel type TYPE_OR_NAME_ERR_.
    //b8 asymmetric;   // true iff all of the following are true:
    //                 //     - this tree node is a binary expression 
    //                 //     - lhs and rhs are of different types
    //                 //     - neither lhs or rhs must be implicitly type converted
    //                 //
    //                 // This is used to mark binary operations such as matrix-vector-, 
    //                 // matrix-scalar-, and vector-scalar multiplications. Conversely,
    //                 // regular bool-float-, float-int, and uint-int, multiplications
    //                 // are not marked as asymmetric, meaning one of the operands will
    //                 // be implicitly type converted into the type of the other operand,
    //                 // according to the type promotion rules, before being operated upon.
} ExprTree;


/*--- Public function prototypes --------------------------------------------------------*/

ExeExpr *sel_compile(const char *src, Allocator *alloc); // selc.c
void sel_list_builtins(void); // selc.c
void sel_print_value(Type t, SelValue v); // selc.c
SelValue sel_eval(ExeExpr *exe, SVMContext ctx, b8 force_recompute); // selvm.c

#endif /* SEL_H */


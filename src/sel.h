#ifndef SEL_H
#define SEL_H

/*--- Include files ---------------------------------------------------------------------*/

#include "hgl_int.h"
#include "hgl_float.h"

#include "str.h"
#include "vecmath.h"

/*--- Public macros ---------------------------------------------------------------------*/

#define SEL_FUNC_MAX_N_ARGS 8

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
    TYPE_AND_NAMECHECKER_ERROR_,
    N_TYPES,
} Type;
static_assert(N_TYPES <= 256, "");

typedef enum
{
    QUALIFIER_NONE  =  0,
    QUALIFIER_CONST = (1 << 0), // for constant expression
    QUALIFIER_PURE  = (1 << 1), // for pure functions
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
    u32 id;
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
} ExeExpr;

/*--- Public variables ------------------------------------------------------------------*/

extern const Func BUILTIN_FUNCTIONS[];
extern const size_t N_BUILTIN_FUNCTIONS;

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
    [TYPE_TEXTURE]    = "texture",
    [TYPE_AND_NAMECHECKER_ERROR_] = "type-/namechecker error",
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
    [TYPE_AND_NAMECHECKER_ERROR_] = 0,
};
static_assert(sizeof(TYPE_TO_SIZE)/sizeof(TYPE_TO_SIZE[0]) == N_TYPES);


/*--- Public function prototypes --------------------------------------------------------*/

ExeExpr *sel_compile(const char *src); // selc.c
void sel_list_builtins(void); // selc.c
void sel_print_value(Type t, SelValue v); // selc.c

SelValue sel_eval(ExeExpr *exe, b8 force_recompute); // selvm.c

#endif /* SEL_H */


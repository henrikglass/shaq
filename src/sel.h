#ifndef SEL_H
#define SEL_H

/*--- Include files ---------------------------------------------------------------------*/

#include "hgl_int.h"
#include "hgl_float.h"
#include "hgl_string.h"
#include "hglm.h"
#include "hglm_aliases.h"

/*--- Public macros ---------------------------------------------------------------------*/

#define SEL_FUNC_MAX_N_ARGS 8

/*--- Public type definitions -----------------------------------------------------------*/

typedef enum
{
    TYPE_NIL = 0,
    TYPE_BOOL,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_VEC2,
    TYPE_VEC3,
    TYPE_VEC4,
    TYPE_IVEC2,
    TYPE_IVEC3,
    TYPE_IVEC4,
    TYPE_MAT2,
    TYPE_MAT3,
    TYPE_MAT4,
    TYPE_STR,
    TYPE_TEXTURE,
    TYPE_AND_NAMECHECKER_ERROR_,
    N_TYPES,
} Type;
static_assert(N_TYPES <= 256, "");

typedef union
{
    bool val_bool; 
    i32 val_i32; 
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
    HglStringView val_str;
    i32 val_tex;
} SelValue;

typedef struct
{
    HglStringView id;
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
} ExeExpr;

/*--- Public variables ------------------------------------------------------------------*/

extern const Func BUILTIN_FUNCTIONS[];
extern const size_t N_BUILTIN_FUNCTIONS;

/*--- Public function prototypes --------------------------------------------------------*/

ExeExpr *sel_compile(const char *src); // selc.c
void sel_list_builtins(void); // selc.c
void sel_print_value(Type t, SelValue v); // selc.c

SelValue sel_run(ExeExpr *exe); // selvm.c

#endif /* SEL_H */


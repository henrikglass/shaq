#ifndef SEL_H
#define SEL_H

/*--- Include files ---------------------------------------------------------------------*/

#include "hgl_int.h"
#include "hgl_float.h"

/*--- Public macros ---------------------------------------------------------------------*/

/*--- Public type definitions -----------------------------------------------------------*/

typedef enum
{
    TYPE_NIL = 0,
    TYPE_BOOL,
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_BVEC2,
    TYPE_BVEC3,
    TYPE_BVEC4,
    TYPE_VEC2,
    TYPE_VEC3,
    TYPE_VEC4,
    TYPE_IVEC2,
    TYPE_IVEC3,
    TYPE_IVEC4,
    TYPE_MAT2,
    TYPE_MAT3,
    TYPE_MAT4,
    TYPE_IMAGE,
    TYPE_ERROR_,
    NAME_ERROR_,
    N_TYPES,
} Type;

static_assert(N_TYPES <= 256, "");
typedef union
{
    bool val_bool; 
    i32 val_i32; 
    f32 val_f32; 
    /* todo ... */
} SelValue;

/* "executable" expression */
typedef struct
{
    u8 *code;
    u32 size;
    u32 capacity;
    Type type;
} ExeExpr;

/*--- Public variables ------------------------------------------------------------------*/

/*--- Public function prototypes --------------------------------------------------------*/

ExeExpr *sel_compile(const char *src);
SelValue sel_run(ExeExpr *exe);

#endif /* SEL_H */


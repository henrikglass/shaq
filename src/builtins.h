#ifndef BUILTINS_H
#define BUILTINS_H

#include "sel.h"

#include <math.h>

#define BUILTIN_MAX_N_ARGS 16

typedef struct
{
    HglStringView id;
    //Type type;
    float value;
} Constant;

typedef struct
{
    HglStringView id;
    Type type;
    SelValue (*impl)(void *args);
    Type argtypes[BUILTIN_MAX_N_ARGS];
} Function;

static inline SelValue builtin_time_(void *args)
{
    (void) args;
    return (SelValue) {.val_f32 = 13.37f}; 
}

static inline SelValue builtin_sin_(void *args)
{
    f32 a0 = *(f32*)args;
    return (SelValue) {.val_f32 = sinf(a0)}; 
}

static inline SelValue builtin_int_(void *args)
{
    f32 a0 = *(f32*)args;
    return (SelValue) {.val_i32 = (i32)a0}; 
}

static inline SelValue builtin_float_(void *args)
{
    i32 a0 = *(i32*)args;
    return (SelValue) {.val_f32 = (f32)a0}; 
}

static const Constant BUILTIN_CONSTANTS[] = 
{
    {.id = HGL_SV_LIT("PI"),  .value =   3.1415926535},
    {.id = HGL_SV_LIT("TAU"), .value = 2*3.1415926535},
    {.id = HGL_SV_LIT("PHI"), .value =   1.6180339887},
    {.id = HGL_SV_LIT("e"),   .value =   2.7182818284},
};
static const size_t N_BUILTIN_CONSTANTS = sizeof(BUILTIN_CONSTANTS) / sizeof(BUILTIN_CONSTANTS[0]);

static const Function BUILTIN_FUNCTIONS[] = 
{
    {.id = HGL_SV_LIT("time"),  .type = TYPE_FLOAT, .impl = builtin_time_,  .argtypes = {TYPE_NIL}},
    {.id = HGL_SV_LIT("sin"),   .type = TYPE_FLOAT, .impl = builtin_sin_,   .argtypes = {TYPE_FLOAT, TYPE_NIL}},
    {.id = HGL_SV_LIT("int"),   .type = TYPE_INT,   .impl = builtin_int_,   .argtypes = {TYPE_FLOAT, TYPE_NIL}},
    {.id = HGL_SV_LIT("float"), .type = TYPE_FLOAT, .impl = builtin_float_, .argtypes = {TYPE_INT, TYPE_NIL}},
};
static const size_t N_BUILTIN_FUNCTIONS = sizeof(BUILTIN_FUNCTIONS) / sizeof(BUILTIN_FUNCTIONS[0]);

#endif /* BUILTINS_H */


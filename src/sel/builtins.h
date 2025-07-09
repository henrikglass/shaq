#ifndef BUILTINS_H
#define BUILTINS_H

/*--- Include files ---------------------------------------------------------------------*/

#include "sel/ast.h"

/*--- Public macros ---------------------------------------------------------------------*/

/*--- Public type definitions -----------------------------------------------------------*/

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
    Type argtypes[17];
} Function;

/*--- Public variables ------------------------------------------------------------------*/

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
    {.id = HGL_SV_LIT("time"),  .type = TYPE_FLOAT, .argtypes = {TYPE_NIL}},
    {.id = HGL_SV_LIT("sin"),   .type = TYPE_FLOAT, .argtypes = {TYPE_FLOAT, TYPE_NIL}},
    {.id = HGL_SV_LIT("int"),   .type = TYPE_INT,   .argtypes = {TYPE_FLOAT, TYPE_NIL}},
    {.id = HGL_SV_LIT("float"), .type = TYPE_FLOAT, .argtypes = {TYPE_INT, TYPE_NIL}},
};
static const size_t N_BUILTIN_FUNCTIONS = sizeof(BUILTIN_FUNCTIONS) / sizeof(BUILTIN_FUNCTIONS[0]);

/*--- Public function prototypes --------------------------------------------------------*/

#endif /* BUILTINS_H */


#ifndef BUILTINS_H
#define BUILTINS_H

/*--- Include files ---------------------------------------------------------------------*/

#include "sel.h"

#include "hglm.h"

#include <stdlib.h>
#include <time.h>

/*--- Public macros ---------------------------------------------------------------------*/

#define BUILTIN_MAX_N_ARGS 16


/*--- Public type definitions -----------------------------------------------------------*/

typedef struct
{
    HglStringView id;
    //Type type;
    f32 value;
} SelConstant;

typedef struct
{
    HglStringView id;
    Type type;
    SelValue (*impl)(void *args);
    Type argtypes[BUILTIN_MAX_N_ARGS];
    const char *synopsis;
    const char *desc;
} SelFunction;

/*--- Built-in function implementations -------------------------------------------------*/

static inline SelValue builtin_int_(void *args)
{
    f32 a0 = *(f32*)args;
    return (SelValue) {.val_i32 = (i32)a0}; 
}

static inline SelValue builtin_mini_(void *args)
{
    i32 *args_i32 = (i32 *) args;
    return (SelValue) {.val_i32 = (args_i32[0] < args_i32[1]) ? args_i32[0] : args_i32[1]}; 
}

static inline SelValue builtin_maxi_(void *args)
{
    i32 *args_i32 = (i32 *) args;
    return (SelValue) {.val_i32 = (args_i32[0] > args_i32[1]) ? args_i32[0] : args_i32[1]}; 
}

static inline SelValue builtin_randi_(void *args)
{
    static bool first_call = true;
    if (first_call) {
        srand(time(NULL)); // NOTE: duplicated in builtin_rand_. TODO add this to shaq_core.c
        first_call = false;
    }

    i32 *args_i32 = (i32 *) args;
    i32 min = args_i32[0];
    i32 max = args_i32[1];
    return (SelValue) {.val_i32 = rand() % (max + 1 - min) + min}; 
}

static inline SelValue builtin_iota_(void *args)
{
    (void) args;
    static i32 iota = 0;
    return (SelValue) {.val_i32 = iota++}; 
}



static inline SelValue builtin_float_(void *args)
{
    i32 a0 = *(i32*)args;
    return (SelValue) {.val_f32 = (f32)a0}; 
}

static inline SelValue builtin_time_(void *args)
{
    (void) args;
    return (SelValue) {.val_f32 = 13.37f};  // TODO shaq_core.c
}

static inline SelValue builtin_deltatime_(void *args)
{
    (void) args;
    return (SelValue) {.val_f32 = 69.420};  // TODO shaq_core.c
}

static inline SelValue builtin_rand_(void *args)
{
    static bool first_call = true;
    if (first_call) {
        srand(time(NULL)); // NOTE: duplicated in builtin_rand_. TODO add this to shaq_core.c
        first_call = false;
    }

    f32 *args_f32 = (f32 *) args;
    f32 min = args_f32[0];
    f32 max = args_f32[1];
    f32 range = max - min;
    return (SelValue) {.val_f32 = ((f32)rand()/(f32)RAND_MAX)*range + min}; 
}

static inline SelValue builtin_sqrt_(void *args)
{
    f32 a0 = *(f32*)args;
    return (SelValue) {.val_f32 = sqrtf(a0)}; 
}

static inline SelValue builtin_pow_(void *args)
{
    f32 *args_f32 = (f32 *) args;
    return (SelValue) {.val_f32 = powf(args_f32[0], args_f32[1])}; 
}

static inline SelValue builtin_exp_(void *args)
{
    f32 a0 = *(f32*)args;
    return (SelValue) {.val_f32 = expf(a0)}; 
}

static inline SelValue builtin_log_(void *args)
{
    f32 a0 = *(f32*)args;
    return (SelValue) {.val_f32 = logf(a0)}; 
}

static inline SelValue builtin_exp2_(void *args)
{
    f32 a0 = *(f32*)args;
    return (SelValue) {.val_f32 = exp2f(a0)}; 
}

static inline SelValue builtin_log2_(void *args)
{
    f32 a0 = *(f32*)args;
    return (SelValue) {.val_f32 = log2f(a0)}; 
}

static inline SelValue builtin_sin_(void *args)
{
    f32 a0 = *(f32*)args;
    return (SelValue) {.val_f32 = sinf(a0)}; 
}

static inline SelValue builtin_cos_(void *args)
{
    f32 a0 = *(f32*)args;
    return (SelValue) {.val_f32 = cosf(a0)}; 
}

static inline SelValue builtin_tan_(void *args)
{
    f32 a0 = *(f32*)args;
    return (SelValue) {.val_f32 = tanf(a0)}; 
}

static inline SelValue builtin_asin_(void *args)
{
    f32 a0 = *(f32*)args;
    return (SelValue) {.val_f32 = asinf(a0)}; 
}

static inline SelValue builtin_acos_(void *args)
{
    f32 a0 = *(f32*)args;
    return (SelValue) {.val_f32 = acosf(a0)}; 
}

static inline SelValue builtin_atan_(void *args)
{
    f32 a0 = *(f32*)args;
    return (SelValue) {.val_f32 = atanf(a0)}; 
}

static inline SelValue builtin_atan2_(void *args)
{
    f32 *args_f32 = (f32 *) args;
    return (SelValue) {.val_f32 = atan2f(args_f32[0], args_f32[1])}; 
}

static inline SelValue builtin_round_(void *args)
{
    f32 a0 = *(f32*)args;
    return (SelValue) {.val_f32 = roundf(a0)}; 
}

static inline SelValue builtin_floor_(void *args)
{
    f32 a0 = *(f32*)args;
    return (SelValue) {.val_f32 = floorf(a0)}; 
}

static inline SelValue builtin_ceil_(void *args)
{
    f32 a0 = *(f32*)args;
    return (SelValue) {.val_f32 = ceilf(a0)}; 
}

static inline SelValue builtin_fract_(void *args)
{
    f32 a0 = *(f32*)args;
    return (SelValue) {.val_f32 = a0 - floorf(a0)}; 
}

static inline SelValue builtin_min_(void *args)
{
    f32 *args_f32 = (f32 *) args;
    return (SelValue) {.val_f32 = (args_f32[0] < args_f32[1]) ? args_f32[0] : args_f32[1]}; 
}

static inline SelValue builtin_max_(void *args)
{
    f32 *args_f32 = (f32 *) args;
    return (SelValue) {.val_f32 = (args_f32[0] > args_f32[1]) ? args_f32[0] : args_f32[1]}; 
}

static inline SelValue builtin_clamp_(void *args)
{
    f32 *args_f32 = (f32 *) args;
    return (SelValue) {.val_f32 = hglm_clamp(args_f32[0], args_f32[1], args_f32[2])}; 
}

static inline SelValue builtin_lerp_(void *args)
{
    f32 *args_f32 = (f32 *) args;
    return (SelValue) {.val_f32 = hglm_lerp(args_f32[0], args_f32[1], args_f32[2])}; 
}

static inline SelValue builtin_ilerp_(void *args)
{
    f32 *args_f32 = (f32 *) args;
    return (SelValue) {.val_f32 = hglm_ilerp(args_f32[0], args_f32[1], args_f32[2])}; 
}

static inline SelValue builtin_remap_(void *args)
{
    f32 *args_f32 = (f32 *) args;
    return (SelValue) {.val_f32 = hglm_remap(args_f32[0], args_f32[1], args_f32[2], args_f32[3], args_f32[4])}; 
}

static inline SelValue builtin_lerpsmooth_(void *args)
{
    f32 *args_f32 = (f32 *) args;
    return (SelValue) {.val_f32 = hglm_lerpsmooth(args_f32[0], args_f32[1], args_f32[2], args_f32[3])}; 
}

static inline SelValue builtin_smoothstep_(void *args)
{
    f32 a0 = *(f32*)args;
    return (SelValue) {.val_f32 = hglm_smoothstep(a0)}; 
}

static inline SelValue builtin_radians_(void *args)
{
    f32 a0 = *(f32*)args;
    return (SelValue) {.val_f32 = (f32)((2.0*HGLM_PI/360.0)) * a0}; 
}


static inline SelValue builtin_vec2_(void *args)
{
    f32 *args_f32 = (f32 *) args;
    return (SelValue) {.val_vec2 = hglm_vec2_make(args_f32[0], args_f32[1])};
}

static inline SelValue builtin_vec3_(void *args)
{
    f32 *args_f32 = (f32 *) args;
    return (SelValue) {.val_vec3 = hglm_vec3_make(args_f32[0], args_f32[1], args_f32[2])};
}

static inline SelValue builtin_vec4_(void *args)
{
    f32 *args_f32 = (f32 *) args;
    return (SelValue) {.val_vec4 = hglm_vec4_make(args_f32[0], args_f32[1], args_f32[2], args_f32[3])};
}

static inline SelValue builtin_ivec2_(void *args)
{
    i32 *args_i32 = (i32 *) args;
    return (SelValue) {.val_ivec2 = hglm_ivec2_make(args_i32[0], args_i32[1])};
}

static inline SelValue builtin_ivec3_(void *args)
{
    i32 *args_i32 = (i32 *) args;
    return (SelValue) {.val_ivec3 = hglm_ivec3_make(args_i32[0], args_i32[1], args_i32[2])};
}

static inline SelValue builtin_ivec4_(void *args)
{
    i32 *args_i32 = (i32 *) args;
    return (SelValue) {.val_ivec4 = hglm_ivec4_make(args_i32[0], args_i32[1], args_i32[2], args_i32[3])};
}

static inline SelValue builtin_mat2_(void *args)
{
    Vec2 *args_v2 = (Vec2 *) args;
    return (SelValue) {.val_mat2 = hglm_mat2_make(args_v2[0], args_v2[1])};
}

static inline SelValue builtin_mat3_(void *args)
{
    Vec3 *args_v3 = (Vec3 *) args;
    return (SelValue) {.val_mat3 = hglm_mat3_make(args_v3[0], args_v3[1], args_v3[2])};
}

static inline SelValue builtin_mat4_(void *args)
{
    Vec4 *args_v4 = (Vec4 *) args;
    return (SelValue) {.val_mat4 = hglm_mat4_make(args_v4[0], args_v4[1], args_v4[2], args_v4[3])};
}

static inline SelValue builtin_mat2_id_(void *args)
{
    (void) args;
    return (SelValue) {.val_mat2 = HGLM_MAT2_IDENTITY};
}

static inline SelValue builtin_mat3_id_(void *args)
{
    (void) args;
    return (SelValue) {.val_mat3 = HGLM_MAT3_IDENTITY};
}

static inline SelValue builtin_mat4_id_(void *args)
{
    (void) args;
    return (SelValue) {.val_mat4 = HGLM_MAT4_IDENTITY};
}

/*--- Built-in tables -------------------------------------------------------------------*/

static const SelConstant BUILTIN_CONSTANTS[] = 
{
    {.id = HGL_SV_LIT("PI"),  .value =   3.1415926535},
    {.id = HGL_SV_LIT("TAU"), .value = 2*3.1415926535},
    {.id = HGL_SV_LIT("PHI"), .value =   1.6180339887},
    {.id = HGL_SV_LIT("e"),   .value =   2.7182818284},
};
static const size_t N_BUILTIN_CONSTANTS = sizeof(BUILTIN_CONSTANTS) / sizeof(BUILTIN_CONSTANTS[0]);

static const SelFunction BUILTIN_FUNCTIONS[] = 
{
    { .id = HGL_SV_LIT("int"),        .type = TYPE_INT,   .impl = builtin_int_,        .argtypes = {TYPE_FLOAT, TYPE_NIL},                                                 .synopsis = "int int(float x)", .desc = "Typecast float to int.", },
    { .id = HGL_SV_LIT("mini"),       .type = TYPE_INT,   .impl = builtin_mini_,       .argtypes = {TYPE_INT, TYPE_INT, TYPE_NIL},                                         .synopsis = "int mini(int a, int b)", .desc = "Returns the minimum of `a` and `b`.", },
    { .id = HGL_SV_LIT("maxi"),       .type = TYPE_INT,   .impl = builtin_maxi_,       .argtypes = {TYPE_INT, TYPE_INT, TYPE_NIL},                                         .synopsis = "int maxi(int a, int b)", .desc = "Returns the maximum of `a` and `b`.", },
    { .id = HGL_SV_LIT("randi"),      .type = TYPE_INT,   .impl = builtin_randi_,      .argtypes = {TYPE_INT, TYPE_INT, TYPE_NIL},                                         .synopsis = "int randi(int min, int max)", .desc = "Returns a random number in [`min`, `max`].", },
    { .id = HGL_SV_LIT("iota"),       .type = TYPE_INT,   .impl = builtin_iota_,       .argtypes = {TYPE_NIL},                                                             .synopsis = "int iota()", .desc = "Returns the number of times it's been called. See the `iota` in golang.", },

    { .id = HGL_SV_LIT("float"),      .type = TYPE_FLOAT, .impl = builtin_float_,      .argtypes = {TYPE_INT, TYPE_NIL},                                                   .synopsis = "float float(int x)", .desc = "Typecast int to float.", },
    { .id = HGL_SV_LIT("time"),       .type = TYPE_FLOAT, .impl = builtin_time_,       .argtypes = {TYPE_NIL},                                                             .synopsis = "float time()", .desc = "Returns the program runtime in seconds.", },
    { .id = HGL_SV_LIT("deltatime"),  .type = TYPE_FLOAT, .impl = builtin_deltatime_,  .argtypes = {TYPE_NIL},                                                             .synopsis = "float deltatime()", .desc = "Returns the frame delta time in seconds.", },
    { .id = HGL_SV_LIT("rand"),       .type = TYPE_FLOAT, .impl = builtin_rand_,       .argtypes = {TYPE_FLOAT, TYPE_FLOAT, TYPE_NIL},                                     .synopsis = "float rand(float min, float max)", .desc = "Returns a random number in [`min`, `max`].", },
    { .id = HGL_SV_LIT("sqrt"),       .type = TYPE_FLOAT, .impl = builtin_sqrt_,       .argtypes = {TYPE_FLOAT, TYPE_NIL},                                                 .synopsis = "float sqrt(float x)", .desc = NULL, },
    { .id = HGL_SV_LIT("pow"),        .type = TYPE_FLOAT, .impl = builtin_pow_,        .argtypes = {TYPE_FLOAT, TYPE_FLOAT, TYPE_NIL},                                     .synopsis = "float pow(float x, float y)", .desc = NULL, },
    { .id = HGL_SV_LIT("exp"),        .type = TYPE_FLOAT, .impl = builtin_exp_,        .argtypes = {TYPE_FLOAT, TYPE_NIL},                                                 .synopsis = "float exp(float x)", .desc = NULL, },
    { .id = HGL_SV_LIT("log"),        .type = TYPE_FLOAT, .impl = builtin_log_,        .argtypes = {TYPE_FLOAT, TYPE_NIL},                                                 .synopsis = "float log(float x)", .desc = NULL, },
    { .id = HGL_SV_LIT("exp2"),       .type = TYPE_FLOAT, .impl = builtin_exp2_,       .argtypes = {TYPE_FLOAT, TYPE_NIL},                                                 .synopsis = "float exp2(float x)", .desc = NULL, },
    { .id = HGL_SV_LIT("log2"),       .type = TYPE_FLOAT, .impl = builtin_log2_,       .argtypes = {TYPE_FLOAT, TYPE_NIL},                                                 .synopsis = "float log2(float x)", .desc = NULL, },
    { .id = HGL_SV_LIT("sin"),        .type = TYPE_FLOAT, .impl = builtin_sin_,        .argtypes = {TYPE_FLOAT, TYPE_NIL},                                                 .synopsis = "float sin(float x)", .desc = NULL, },
    { .id = HGL_SV_LIT("cos"),        .type = TYPE_FLOAT, .impl = builtin_cos_,        .argtypes = {TYPE_FLOAT, TYPE_NIL},                                                 .synopsis = "float cos(float x)", .desc = NULL, },
    { .id = HGL_SV_LIT("tan"),        .type = TYPE_FLOAT, .impl = builtin_tan_,        .argtypes = {TYPE_FLOAT, TYPE_NIL},                                                 .synopsis = "float tan(float x)", .desc = NULL, },
    { .id = HGL_SV_LIT("asin"),       .type = TYPE_FLOAT, .impl = builtin_asin_,       .argtypes = {TYPE_FLOAT, TYPE_NIL},                                                 .synopsis = "float asin(float x)", .desc = NULL, },
    { .id = HGL_SV_LIT("acos"),       .type = TYPE_FLOAT, .impl = builtin_acos_,       .argtypes = {TYPE_FLOAT, TYPE_NIL},                                                 .synopsis = "float acos(float x)", .desc = NULL, },
    { .id = HGL_SV_LIT("atan"),       .type = TYPE_FLOAT, .impl = builtin_atan_,       .argtypes = {TYPE_FLOAT, TYPE_NIL},                                                 .synopsis = "float atan(float x)", .desc = NULL, },
    { .id = HGL_SV_LIT("atan2"),      .type = TYPE_FLOAT, .impl = builtin_atan2_,      .argtypes = {TYPE_FLOAT, TYPE_FLOAT, TYPE_NIL},                                     .synopsis = "float atan2(float y, float x)", .desc = NULL, },
    { .id = HGL_SV_LIT("round"),      .type = TYPE_FLOAT, .impl = builtin_round_,      .argtypes = {TYPE_FLOAT, TYPE_NIL},                                                 .synopsis = "float round(float x)", .desc = NULL, },
    { .id = HGL_SV_LIT("floor"),      .type = TYPE_FLOAT, .impl = builtin_floor_,      .argtypes = {TYPE_FLOAT, TYPE_NIL},                                                 .synopsis = "float floor(float x)", .desc = NULL, },
    { .id = HGL_SV_LIT("ceil"),       .type = TYPE_FLOAT, .impl = builtin_ceil_,       .argtypes = {TYPE_FLOAT, TYPE_NIL},                                                 .synopsis = "float ceil(float x)", .desc = NULL, },
    { .id = HGL_SV_LIT("fract"),      .type = TYPE_FLOAT, .impl = builtin_fract_,      .argtypes = {TYPE_FLOAT, TYPE_NIL},                                                 .synopsis = "float fract(float x)", .desc = NULL, },
    { .id = HGL_SV_LIT("min"),        .type = TYPE_FLOAT, .impl = builtin_min_,        .argtypes = {TYPE_FLOAT, TYPE_FLOAT, TYPE_NIL},                                     .synopsis = "float min(float a, float b)", .desc = NULL, },
    { .id = HGL_SV_LIT("max"),        .type = TYPE_FLOAT, .impl = builtin_max_,        .argtypes = {TYPE_FLOAT, TYPE_FLOAT, TYPE_NIL},                                     .synopsis = "float max(float a, float b)", .desc = NULL, },
    { .id = HGL_SV_LIT("clamp"),      .type = TYPE_FLOAT, .impl = builtin_clamp_,      .argtypes = {TYPE_FLOAT, TYPE_FLOAT, TYPE_FLOAT, TYPE_NIL},                         .synopsis = "float clamp(float min, float max, float x)", .desc = "Returns x clamped to [`min`,`max`]", },
    { .id = HGL_SV_LIT("lerp"),       .type = TYPE_FLOAT, .impl = builtin_lerp_,       .argtypes = {TYPE_FLOAT, TYPE_FLOAT, TYPE_FLOAT, TYPE_NIL},                         .synopsis = "float lerp(float a, float b, float t)", .desc = "Linear interpolation", },
    { .id = HGL_SV_LIT("ilerp"),      .type = TYPE_FLOAT, .impl = builtin_ilerp_,      .argtypes = {TYPE_FLOAT, TYPE_FLOAT, TYPE_FLOAT, TYPE_NIL},                         .synopsis = "float ilerp(float a, float b, float x)", .desc = "Inverse linear interpolation", },
    { .id = HGL_SV_LIT("remap"),      .type = TYPE_FLOAT, .impl = builtin_remap_,      .argtypes = {TYPE_FLOAT, TYPE_FLOAT, TYPE_FLOAT, TYPE_FLOAT, TYPE_FLOAT, TYPE_NIL}, .synopsis = "float remap(float in_min, float in_max, float out_min, float out_max, float x)", .desc = "See Freya Holmér's talks :-)", },
    { .id = HGL_SV_LIT("lerpsmooth"), .type = TYPE_FLOAT, .impl = builtin_lerpsmooth_, .argtypes = {TYPE_FLOAT, TYPE_FLOAT, TYPE_FLOAT, TYPE_FLOAT, TYPE_NIL},             .synopsis = "float lerpsmooth(float a, float b, float dt, float omega)", .desc = "See Freya Holmér's talks :-)" , },
    { .id = HGL_SV_LIT("smoothstep"), .type = TYPE_FLOAT, .impl = builtin_smoothstep_, .argtypes = {TYPE_FLOAT, TYPE_NIL},                                                 .synopsis = "float smoothstep(float t)", .desc = "Steps, smoothly. :3", },
    { .id = HGL_SV_LIT("radians"),    .type = TYPE_FLOAT, .impl = builtin_radians_,    .argtypes = {TYPE_FLOAT, TYPE_NIL},                                                 .synopsis = "float radians(float degrees)", .desc = "Converts degrees into radians", },

    { .id = HGL_SV_LIT("vec2"),       .type = TYPE_VEC2,  .impl = builtin_vec2_,       .argtypes = {TYPE_FLOAT, TYPE_FLOAT, TYPE_NIL},                                     .synopsis = "vec2 vec2(float x, float y)", .desc = NULL, },
    { .id = HGL_SV_LIT("vec3"),       .type = TYPE_VEC3,  .impl = builtin_vec3_,       .argtypes = {TYPE_FLOAT, TYPE_FLOAT, TYPE_FLOAT, TYPE_NIL},                         .synopsis = "vec3 vec3(float x, float y, float z)", .desc = NULL, },
    { .id = HGL_SV_LIT("vec4"),       .type = TYPE_VEC4,  .impl = builtin_vec4_,       .argtypes = {TYPE_FLOAT, TYPE_FLOAT, TYPE_FLOAT, TYPE_FLOAT, TYPE_NIL},             .synopsis = "vec4 vec4(float x, float y, float z, float w)", .desc = NULL, },

    { .id = HGL_SV_LIT("ivec2"),      .type = TYPE_IVEC2, .impl = builtin_ivec2_,      .argtypes = {TYPE_INT, TYPE_INT, TYPE_NIL},                                         .synopsis = "ivec2 ivec2(int x, int y)", .desc = NULL, },
    { .id = HGL_SV_LIT("ivec3"),      .type = TYPE_IVEC3, .impl = builtin_ivec3_,      .argtypes = {TYPE_INT, TYPE_INT, TYPE_INT, TYPE_NIL},                               .synopsis = "ivec3 ivec3(int x, int y, int z)", .desc = NULL, },
    { .id = HGL_SV_LIT("ivec4"),      .type = TYPE_IVEC4, .impl = builtin_ivec4_,      .argtypes = {TYPE_INT, TYPE_INT, TYPE_INT, TYPE_INT, TYPE_NIL},                     .synopsis = "ivec4 ivec4(int x, int y, int z, int w)", .desc = NULL, },

    { .id = HGL_SV_LIT("mat2"),       .type = TYPE_MAT2,  .impl = builtin_mat2_,       .argtypes = {TYPE_VEC2, TYPE_VEC2, TYPE_NIL},                                       .synopsis = "mat2 mat2(vec2 c0, vec2 c1)", .desc = NULL, },
    { .id = HGL_SV_LIT("mat3"),       .type = TYPE_MAT3,  .impl = builtin_mat3_,       .argtypes = {TYPE_VEC3, TYPE_VEC3, TYPE_VEC3, TYPE_NIL},                            .synopsis = "mat3 mat3(vec3 c0, vec3 c1, vec3 c2)", .desc = NULL, },
    { .id = HGL_SV_LIT("mat4"),       .type = TYPE_MAT4,  .impl = builtin_mat4_,       .argtypes = {TYPE_VEC4, TYPE_VEC4, TYPE_VEC4, TYPE_VEC4, TYPE_NIL},                 .synopsis = "mat4 mat4(vec4 c0, vec4 c1, vec4 c2, vec4 c3)", .desc = NULL, },

    { .id = HGL_SV_LIT("mat2_id"),    .type = TYPE_MAT2,  .impl = builtin_mat2_id_,    .argtypes = {TYPE_NIL},                                                             .synopsis = "mat2 mat2_id()", .desc = NULL, },
    { .id = HGL_SV_LIT("mat3_id"),    .type = TYPE_MAT3,  .impl = builtin_mat3_id_,    .argtypes = {TYPE_NIL},                                                             .synopsis = "mat3 mat3_id()", .desc = NULL, },
    { .id = HGL_SV_LIT("mat4_id"),    .type = TYPE_MAT4,  .impl = builtin_mat4_id_,    .argtypes = {TYPE_NIL},                                                             .synopsis = "mat4 mat4_id()", .desc = NULL, },

};
static const size_t N_BUILTIN_FUNCTIONS = sizeof(BUILTIN_FUNCTIONS) / sizeof(BUILTIN_FUNCTIONS[0]);

#endif /* BUILTINS_H */


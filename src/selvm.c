/*--- Include files ---------------------------------------------------------------------*/

#include "sel.h"
#include "shaq_core.h"
#include "renderer.h"
#include "user_input.h"
#include "gui.h"
#include "log.h" // ???

#include <time.h>

/*--- Private macros --------------------------------------------------------------------*/

#define SVM_STACK_SIZE (16*1024)

/*--- Private type definitions ----------------------------------------------------------*/

/*--- Private function prototypes -------------------------------------------------------*/

static void svm_run(void);
static void svm_reset(void);

static inline void *svm_next_bytes(u32 size);
static inline Op *svm_next_op(void);
static inline void svm_stack_push(void *data, u32 size);
static inline void svm_stack_push_selvalue(SelValue v, Type t);
static inline void *svm_stack_pop(u32 size);
static inline i32 addi(i32 *lhs, i32 *rhs);
static inline u32 addu(u32 *lhs, u32 *rhs);
static inline f32 addf(f32 *lhs, f32 *rhs);
static inline Vec2 addv2(Vec2 *lhs, Vec2 *rhs);
static inline Vec3 addv3(Vec3 *lhs, Vec3 *rhs);
static inline Vec4 addv4(Vec4 *lhs, Vec4 *rhs);
static inline i32 subi(i32 *lhs, i32 *rhs);
static inline u32 subu(u32 *lhs, u32 *rhs);
static inline f32 subf(f32 *lhs, f32 *rhs);
static inline Vec2 subv2(Vec2 *lhs, Vec2 *rhs);
static inline Vec3 subv3(Vec3 *lhs, Vec3 *rhs);
static inline Vec4 subv4(Vec4 *lhs, Vec4 *rhs);
static inline i32 muli(i32 *lhs, i32 *rhs);
static inline u32 mulu(u32 *lhs, u32 *rhs);
static inline f32 mulf(f32 *lhs, f32 *rhs);
static inline Vec2 mulv2(Vec2 *lhs, Vec2 *rhs);
static inline Vec3 mulv3(Vec3 *lhs, Vec3 *rhs);
static inline Vec4 mulv4(Vec4 *lhs, Vec4 *rhs);
static inline i32 divi(i32 *lhs, i32 *rhs);
static inline u32 divu(u32 *lhs, u32 *rhs);
static inline f32 divf(f32 *lhs, f32 *rhs);
static inline Vec2 divv2(Vec2 *lhs, Vec2 *rhs);
static inline Vec3 divv3(Vec3 *lhs, Vec3 *rhs);
static inline Vec4 divv4(Vec4 *lhs, Vec4 *rhs);
static inline i32 remi(i32 *lhs, i32 *rhs);
static inline u32 remu(u32 *lhs, u32 *rhs);
static inline i32 negi(i32 *val);
static inline f32 negf(f32 *val);

static SelValue fn_load_image_(void *args);
static SelValue fn_output_of_(void *args);
static SelValue fn_last_output_of_(void *args);

static SelValue fn_left_mouse_button_is_down_(void *args);
static SelValue fn_right_mouse_button_is_down_(void *args);
static SelValue fn_left_mouse_button_was_clicked_(void *args);
static SelValue fn_right_mouse_button_was_clicked_(void *args);
static SelValue fn_key_is_down_(void *args);
static SelValue fn_key_was_pressed_(void *args);

static SelValue fn_int_(void *args);
static SelValue fn_unsigned_(void *args);
static SelValue fn_mini_(void *args);
static SelValue fn_maxi_(void *args);
static SelValue fn_randi_(void *args);
static SelValue fn_iota_(void *args);
static SelValue fn_frame_count_(void *args);

static SelValue fn_signed_(void *args);
static SelValue fn_xor_(void *args);
static SelValue fn_and_(void *args);
static SelValue fn_or_(void *args);
static SelValue fn_not_(void *args);
static SelValue fn_lshift_(void *args);
static SelValue fn_rshift_(void *args);
static SelValue fn_rol_(void *args);
static SelValue fn_ror_(void *args);

static SelValue fn_float_(void *args);
static SelValue fn_time_(void *args);
static SelValue fn_deltatime_(void *args);
static SelValue fn_rand_(void *args);
static SelValue fn_sqrt_(void *args);
static SelValue fn_pow_(void *args);
static SelValue fn_exp_(void *args);
static SelValue fn_log_(void *args);
static SelValue fn_exp2_(void *args);
static SelValue fn_log2_(void *args);
static SelValue fn_sin_(void *args);
static SelValue fn_cos_(void *args);
static SelValue fn_tan_(void *args);
static SelValue fn_asin_(void *args);
static SelValue fn_acos_(void *args);
static SelValue fn_atan_(void *args);
static SelValue fn_atan2_(void *args);
static SelValue fn_round_(void *args);
static SelValue fn_floor_(void *args);
static SelValue fn_ceil_(void *args);
static SelValue fn_fract_(void *args);
static SelValue fn_min_(void *args);
static SelValue fn_max_(void *args);
static SelValue fn_clamp_(void *args);
static SelValue fn_lerp_(void *args);
static SelValue fn_ilerp_(void *args);
static SelValue fn_remap_(void *args);
static SelValue fn_lerpsmooth_(void *args);
static SelValue fn_smoothstep_(void *args);
static SelValue fn_radians_(void *args);
static SelValue fn_perlin3D_(void *args);
static SelValue fn_aspect_ratio_(void *args);

static SelValue fn_vec2_(void *args);
static SelValue fn_vec2_from_polar_(void *args);
static SelValue fn_vec2_distance_(void *args);
static SelValue fn_vec2_length_(void *args);
static SelValue fn_vec2_normalize_(void *args);
static SelValue fn_vec2_dot_(void *args);
static SelValue fn_vec2_mul_scalar_(void *args);
static SelValue fn_vec2_lerp_(void *args);
static SelValue fn_vec2_slerp_(void *args);
static SelValue fn_mouse_position_(void *args);
static SelValue fn_mouse_drag_position_(void *args);

static SelValue fn_vec3_(void *args);
static SelValue fn_vec3_from_spherical_(void *args);
static SelValue fn_vec3_distance_(void *args);
static SelValue fn_vec3_length_(void *args);
static SelValue fn_vec3_normalize_(void *args);
static SelValue fn_vec3_dot_(void *args);
static SelValue fn_vec3_mul_scalar_(void *args);
static SelValue fn_vec3_lerp_(void *args);
static SelValue fn_vec3_slerp_(void *args);
static SelValue fn_vec3_cross_(void *args);

static SelValue fn_vec4_(void *args);
static SelValue fn_vec4_distance_(void *args);
static SelValue fn_vec4_length_(void *args);
static SelValue fn_vec4_normalize_(void *args);
static SelValue fn_vec4_dot_(void *args);
static SelValue fn_vec4_mul_scalar_(void *args);
static SelValue fn_vec4_lerp_(void *args);
static SelValue fn_vec4_xyz_(void *args);
static SelValue fn_rgba_(void *args);

static SelValue fn_ivec2_(void *args);
static SelValue fn_iresolution_(void *args);

static SelValue fn_ivec3_(void *args);

static SelValue fn_ivec4_(void *args);

static SelValue fn_mat2_(void *args);
static SelValue fn_mat2_id_(void *args);

static SelValue fn_mat3_(void *args);
static SelValue fn_mat3_id_(void *args);

static SelValue fn_mat4_(void *args);
static SelValue fn_mat4_id_(void *args);
static SelValue fn_mat4_make_scale_(void *args);
static SelValue fn_mat4_make_rotation_(void *args);
static SelValue fn_mat4_make_translation_(void *args);
static SelValue fn_mat4_look_at_(void *args);
static SelValue fn_mat4_scale_(void *args);
static SelValue fn_mat4_rotate_(void *args);
static SelValue fn_mat4_translate_(void *args);
static SelValue fn_mat4_mul_mat4_(void *args);
static SelValue fn_mat4_mul_vec4_(void *args);
static SelValue fn_mat4_mul_scalar_(void *args);

static SelValue fn_input_float_(void *args);
static SelValue fn_checkbox_(void *args);
static SelValue fn_drag_int_(void *args);
static SelValue fn_slider_float_(void *args);
static SelValue fn_slider_float_log_(void *args);
static SelValue fn_input_int_(void *args);
static SelValue fn_input_vec2_(void *args);
static SelValue fn_input_vec3_(void *args);
static SelValue fn_input_vec4_(void *args);
static SelValue fn_color_picker_(void *args);

/*--- Public variables ------------------------------------------------------------------*/

const Func BUILTIN_FUNCTIONS[] = 
{
    { .id = SV_LIT("load_image"), .type = TYPE_TEXTURE, .qualifier = QUALIFIER_PURE, .impl = fn_load_image_, .argtypes = {TYPE_STR, TYPE_NIL}, .synopsis = "texture load_image(str filepath)", .desc = "Returns a reference to a texture loaded from `filepath`", },
    //{ .id = SV_LIT("load_image_detailed"),  /* ... */ }, // TODO (specify min/mag filter, wrapping, etc.).
    { .id = SV_LIT("output_of"),  .type = TYPE_TEXTURE, .qualifier = QUALIFIER_PURE, .impl = fn_output_of_, .argtypes = {TYPE_STR, TYPE_NIL}, .synopsis = "texture output_of(str shader)", .desc = "Returns a reference to a texture rendered to by the shader `shader` in this frame. Calling this function implicitly defines the render order.", },
    { .id = SV_LIT("last_output_of"),  .type = TYPE_TEXTURE, .qualifier = QUALIFIER_PURE, .impl = fn_last_output_of_, .argtypes = {TYPE_STR, TYPE_NIL}, .synopsis = "texture last_output_of(str shader)", .desc = "Returns a reference to a texture rendered to by the shader `shader` in the last frame.", },

    { .id = SV_LIT("left_mouse_button_is_down"),      .type = TYPE_BOOL, .qualifier = QUALIFIER_NONE, .impl = fn_left_mouse_button_is_down_, .argtypes = {TYPE_NIL},      .synopsis = "bool left_mouse_button_is_down()", .desc = "Returns true if the left mouse button is currently down", },
    { .id = SV_LIT("right_mouse_button_is_down"),     .type = TYPE_BOOL, .qualifier = QUALIFIER_NONE, .impl = fn_right_mouse_button_is_down_, .argtypes = {TYPE_NIL},     .synopsis = "bool right_mouse_button_is_down()", .desc = "Returns true if the right mouse button is currently down", },
    { .id = SV_LIT("left_mouse_button_was_clicked"),  .type = TYPE_BOOL, .qualifier = QUALIFIER_NONE, .impl = fn_left_mouse_button_was_clicked_, .argtypes = {TYPE_NIL},  .synopsis = "bool left_mouse_button_was_clicked()", .desc = "Returns true if the left mouse button was pressed in the last frame.", },
    { .id = SV_LIT("right_mouse_button_was_clicked"), .type = TYPE_BOOL, .qualifier = QUALIFIER_NONE, .impl = fn_right_mouse_button_was_clicked_, .argtypes = {TYPE_NIL}, .synopsis = "bool right_mouse_button_was_clicked()", .desc = "Returns true if the right mouse button was pressed in the last frame.", },
    { .id = SV_LIT("key_is_down"),                    .type = TYPE_BOOL, .qualifier = QUALIFIER_NONE, .impl = fn_key_is_down_, .argtypes = {TYPE_STR, TYPE_NIL}, .synopsis = "bool key_is_down(str key)", .desc = "Returns true if `key` is down. `key` can be any letter in the English alphabet.", },
    { .id = SV_LIT("key_was_pressed"),                .type = TYPE_BOOL, .qualifier = QUALIFIER_NONE, .impl = fn_key_was_pressed_, .argtypes = {TYPE_STR, TYPE_NIL}, .synopsis = "bool key_was_pressed(str key)", .desc = "Returns true if `key` was pressed . `key` can be any letter in the English alphabet.", },

    { .id = SV_LIT("int"),         .type = TYPE_INT,  .qualifier = QUALIFIER_PURE, .impl = fn_int_,         .argtypes = {TYPE_FLOAT, TYPE_NIL},          .synopsis = "int int(float x)", .desc = "Typecast float to int.", },
    { .id = SV_LIT("unsigned"),    .type = TYPE_UINT, .qualifier = QUALIFIER_PURE, .impl = fn_unsigned_,    .argtypes = {TYPE_INT, TYPE_NIL},            .synopsis = "uint unsigned(int x)", .desc = "Typecast int to uint.", },
    { .id = SV_LIT("mini"),        .type = TYPE_INT,  .qualifier = QUALIFIER_PURE, .impl = fn_mini_,        .argtypes = {TYPE_INT, TYPE_INT, TYPE_NIL},  .synopsis = "int mini(int a, int b)", .desc = "Returns the minimum of `a` and `b`.", },
    { .id = SV_LIT("maxi"),        .type = TYPE_INT,  .qualifier = QUALIFIER_PURE, .impl = fn_maxi_,        .argtypes = {TYPE_INT, TYPE_INT, TYPE_NIL},  .synopsis = "int maxi(int a, int b)", .desc = "Returns the maximum of `a` and `b`.", },
    { .id = SV_LIT("randi"),       .type = TYPE_INT,  .qualifier = QUALIFIER_NONE, .impl = fn_randi_,       .argtypes = {TYPE_INT, TYPE_INT, TYPE_NIL},  .synopsis = "int randi(int min, int max)", .desc = "Returns a random number in [`min`, `max`].", },
    { .id = SV_LIT("iota"),        .type = TYPE_INT,  .qualifier = QUALIFIER_NONE, .impl = fn_iota_,        .argtypes = {TYPE_NIL},                      .synopsis = "int iota()", .desc = "Returns the number of times it's been called. See the `iota` in golang.", },
    { .id = SV_LIT("frame_count"), .type = TYPE_INT,  .qualifier = QUALIFIER_NONE, .impl = fn_frame_count_, .argtypes = {TYPE_NIL},                      .synopsis = "int frame_count()", .desc = "Returns the frame count.", },

    { .id = SV_LIT("signed"), .type = TYPE_INT,  .qualifier = QUALIFIER_PURE, .impl = fn_signed_, .argtypes = {TYPE_UINT, TYPE_NIL},             .synopsis = "int signed(uint x)", .desc = "Typecast uint to int.", },
    { .id = SV_LIT("xor"),    .type = TYPE_UINT, .qualifier = QUALIFIER_PURE, .impl = fn_xor_,    .argtypes = {TYPE_UINT, TYPE_UINT, TYPE_NIL},  .synopsis = "uint xor(uint a, uint b)", .desc = "bitwise XOR of `a` and `b`.", },
    { .id = SV_LIT("and"),    .type = TYPE_UINT, .qualifier = QUALIFIER_PURE, .impl = fn_and_,    .argtypes = {TYPE_UINT, TYPE_UINT, TYPE_NIL},  .synopsis = "uint and(uint a, uint b)", .desc = "bitwise AND of `a` and `b`.", },
    { .id = SV_LIT("or"),     .type = TYPE_UINT, .qualifier = QUALIFIER_PURE, .impl = fn_or_,     .argtypes = {TYPE_UINT, TYPE_UINT, TYPE_NIL},  .synopsis = "uint or(uint a, uint b)", .desc = "bitwise OR of `a` and `b`.", },
    { .id = SV_LIT("not"),    .type = TYPE_UINT, .qualifier = QUALIFIER_PURE, .impl = fn_not_,    .argtypes = {TYPE_UINT, TYPE_NIL},             .synopsis = "uint not(uint x)", .desc = "bitwise NOT of `x`.", },
    { .id = SV_LIT("lshift"), .type = TYPE_UINT, .qualifier = QUALIFIER_PURE, .impl = fn_lshift_, .argtypes = {TYPE_UINT, TYPE_UINT, TYPE_NIL},  .synopsis = "uint lshift(uint x, uint n)", .desc = "left shift of `x` by `n`.", },
    { .id = SV_LIT("rshift"), .type = TYPE_UINT, .qualifier = QUALIFIER_PURE, .impl = fn_rshift_, .argtypes = {TYPE_UINT, TYPE_UINT, TYPE_NIL},  .synopsis = "uint rshift(uint x, uint n)", .desc = "right shift of `x` by `n`.", },
    { .id = SV_LIT("rol"),    .type = TYPE_UINT, .qualifier = QUALIFIER_PURE, .impl = fn_rol_,    .argtypes = {TYPE_UINT, TYPE_UINT, TYPE_NIL},  .synopsis = "uint rol(uint x, uint n)", .desc = "left rotate of `x` by `n`.", },
    { .id = SV_LIT("ror"),    .type = TYPE_UINT, .qualifier = QUALIFIER_PURE, .impl = fn_ror_,    .argtypes = {TYPE_UINT, TYPE_UINT, TYPE_NIL},  .synopsis = "uint ror(uint x, uint n)", .desc = "right rotate of `x` by `n`.", },

    { .id = SV_LIT("float"),        .type = TYPE_FLOAT, .qualifier = QUALIFIER_PURE, .impl = fn_float_,        .argtypes = {TYPE_INT, TYPE_NIL},                                                   .synopsis = "float float(int x)", .desc = "Typecast int to float.", },
    { .id = SV_LIT("time"),         .type = TYPE_FLOAT, .qualifier = QUALIFIER_NONE, .impl = fn_time_,         .argtypes = {TYPE_NIL},                                                             .synopsis = "float time()", .desc = "Returns the program runtime in seconds.", },
    { .id = SV_LIT("deltatime"),    .type = TYPE_FLOAT, .qualifier = QUALIFIER_NONE, .impl = fn_deltatime_,    .argtypes = {TYPE_NIL},                                                             .synopsis = "float deltatime()", .desc = "Returns the frame delta time in seconds.", },
    { .id = SV_LIT("rand"),         .type = TYPE_FLOAT, .qualifier = QUALIFIER_NONE, .impl = fn_rand_,         .argtypes = {TYPE_FLOAT, TYPE_FLOAT, TYPE_NIL},                                     .synopsis = "float rand(float min, float max)", .desc = "Returns a random number in [`min`, `max`].", },
    { .id = SV_LIT("sqrt"),         .type = TYPE_FLOAT, .qualifier = QUALIFIER_PURE, .impl = fn_sqrt_,         .argtypes = {TYPE_FLOAT, TYPE_NIL},                                                 .synopsis = "float sqrt(float x)", .desc = "Returns the square root of `x`.", },
    { .id = SV_LIT("pow"),          .type = TYPE_FLOAT, .qualifier = QUALIFIER_PURE, .impl = fn_pow_,          .argtypes = {TYPE_FLOAT, TYPE_FLOAT, TYPE_NIL},                                     .synopsis = "float pow(float x, float y)", .desc = "Returns the result of `x` raised to the power `y`", },
    { .id = SV_LIT("exp"),          .type = TYPE_FLOAT, .qualifier = QUALIFIER_PURE, .impl = fn_exp_,          .argtypes = {TYPE_FLOAT, TYPE_NIL},                                                 .synopsis = "float exp(float x)", .desc = "Returns the result of `e` raised to the power `x`", },
    { .id = SV_LIT("log"),          .type = TYPE_FLOAT, .qualifier = QUALIFIER_PURE, .impl = fn_log_,          .argtypes = {TYPE_FLOAT, TYPE_NIL},                                                 .synopsis = "float log(float x)", .desc = "Returns the natural logarithm of `x`", },
    { .id = SV_LIT("exp2"),         .type = TYPE_FLOAT, .qualifier = QUALIFIER_PURE, .impl = fn_exp2_,         .argtypes = {TYPE_FLOAT, TYPE_NIL},                                                 .synopsis = "float exp2(float x)", .desc = "Returns the result of 2 raised to the power `x`", },
    { .id = SV_LIT("log2"),         .type = TYPE_FLOAT, .qualifier = QUALIFIER_PURE, .impl = fn_log2_,         .argtypes = {TYPE_FLOAT, TYPE_NIL},                                                 .synopsis = "float log2(float x)", .desc = "Returns the base-2 logarithm of `x`", },
    { .id = SV_LIT("sin"),          .type = TYPE_FLOAT, .qualifier = QUALIFIER_PURE, .impl = fn_sin_,          .argtypes = {TYPE_FLOAT, TYPE_NIL},                                                 .synopsis = "float sin(float x)", .desc = "Returns the sine of `x`", },
    { .id = SV_LIT("cos"),          .type = TYPE_FLOAT, .qualifier = QUALIFIER_PURE, .impl = fn_cos_,          .argtypes = {TYPE_FLOAT, TYPE_NIL},                                                 .synopsis = "float cos(float x)", .desc = "Returns the cosine of `x`", },
    { .id = SV_LIT("tan"),          .type = TYPE_FLOAT, .qualifier = QUALIFIER_PURE, .impl = fn_tan_,          .argtypes = {TYPE_FLOAT, TYPE_NIL},                                                 .synopsis = "float tan(float x)", .desc = "Returns the tangent of `x`", },
    { .id = SV_LIT("asin"),         .type = TYPE_FLOAT, .qualifier = QUALIFIER_PURE, .impl = fn_asin_,         .argtypes = {TYPE_FLOAT, TYPE_NIL},                                                 .synopsis = "float asin(float x)", .desc = "Returns the principal value of the arc sine of `x`", },
    { .id = SV_LIT("acos"),         .type = TYPE_FLOAT, .qualifier = QUALIFIER_PURE, .impl = fn_acos_,         .argtypes = {TYPE_FLOAT, TYPE_NIL},                                                 .synopsis = "float acos(float x)", .desc = "Returns the arc cosine of `x`", },
    { .id = SV_LIT("atan"),         .type = TYPE_FLOAT, .qualifier = QUALIFIER_PURE, .impl = fn_atan_,         .argtypes = {TYPE_FLOAT, TYPE_NIL},                                                 .synopsis = "float atan(float x)", .desc = "Returns the principal value of the arc tangent of `x`", },
    { .id = SV_LIT("atan2"),        .type = TYPE_FLOAT, .qualifier = QUALIFIER_PURE, .impl = fn_atan2_,        .argtypes = {TYPE_FLOAT, TYPE_FLOAT, TYPE_NIL},                                     .synopsis = "float atan2(float y, float x)", .desc = "Returns the principal value of the arc tangent of `y` / `x`, using the sine of the two arguments to determine the quadrant of the result", },
    { .id = SV_LIT("round"),        .type = TYPE_FLOAT, .qualifier = QUALIFIER_PURE, .impl = fn_round_,        .argtypes = {TYPE_FLOAT, TYPE_NIL},                                                 .synopsis = "float round(float x)", .desc = "Returns the integer value closest to `x`, as a float", },
    { .id = SV_LIT("floor"),        .type = TYPE_FLOAT, .qualifier = QUALIFIER_PURE, .impl = fn_floor_,        .argtypes = {TYPE_FLOAT, TYPE_NIL},                                                 .synopsis = "float floor(float x)", .desc = "Returns the integer part of `x`, as a float", },
    { .id = SV_LIT("ceil"),         .type = TYPE_FLOAT, .qualifier = QUALIFIER_PURE, .impl = fn_ceil_,         .argtypes = {TYPE_FLOAT, TYPE_NIL},                                                 .synopsis = "float ceil(float x)", .desc = "Returns the smallest integer that is larger than `x`, as a float", },
    { .id = SV_LIT("fract"),        .type = TYPE_FLOAT, .qualifier = QUALIFIER_PURE, .impl = fn_fract_,        .argtypes = {TYPE_FLOAT, TYPE_NIL},                                                 .synopsis = "float fract(float x)", .desc = "Returns the fractional part of `x`", },
    { .id = SV_LIT("min"),          .type = TYPE_FLOAT, .qualifier = QUALIFIER_PURE, .impl = fn_min_,          .argtypes = {TYPE_FLOAT, TYPE_FLOAT, TYPE_NIL},                                     .synopsis = "float min(float a, float b)", .desc = "Returns the minimum of `a` and `b`", },
    { .id = SV_LIT("max"),          .type = TYPE_FLOAT, .qualifier = QUALIFIER_PURE, .impl = fn_max_,          .argtypes = {TYPE_FLOAT, TYPE_FLOAT, TYPE_NIL},                                     .synopsis = "float max(float a, float b)", .desc = "Returns the maximum of `a` and `b`", },
    { .id = SV_LIT("clamp"),        .type = TYPE_FLOAT, .qualifier = QUALIFIER_PURE, .impl = fn_clamp_,        .argtypes = {TYPE_FLOAT, TYPE_FLOAT, TYPE_FLOAT, TYPE_NIL},                         .synopsis = "float clamp(float min, float max, float x)", .desc = "Returns x clamped to the range [`min`,`max`]", },
    { .id = SV_LIT("lerp"),         .type = TYPE_FLOAT, .qualifier = QUALIFIER_PURE, .impl = fn_lerp_,         .argtypes = {TYPE_FLOAT, TYPE_FLOAT, TYPE_FLOAT, TYPE_NIL},                         .synopsis = "float lerp(float a, float b, float t)", .desc = "Linearly interpolates between `a` and `b` for values of `t` in [0, 1]. I.e. lerp(a,b,t) = a*(1-t)+b*t", },
    { .id = SV_LIT("ilerp"),        .type = TYPE_FLOAT, .qualifier = QUALIFIER_PURE, .impl = fn_ilerp_,        .argtypes = {TYPE_FLOAT, TYPE_FLOAT, TYPE_FLOAT, TYPE_NIL},                         .synopsis = "float ilerp(float a, float b, float x)", .desc = "Calculates the inverse of lerp(a,b,t). I.e. solves the equation x = a*(1-t)+b*t for t.", },
    { .id = SV_LIT("remap"),        .type = TYPE_FLOAT, .qualifier = QUALIFIER_PURE, .impl = fn_remap_,        .argtypes = {TYPE_FLOAT, TYPE_FLOAT, TYPE_FLOAT, TYPE_FLOAT, TYPE_FLOAT, TYPE_NIL}, .synopsis = "float remap(float in_min, float in_max, float out_min, float out_max, float x)", .desc = "See Freya Holmér's talks :-)", },
    { .id = SV_LIT("lerpsmooth"),   .type = TYPE_FLOAT, .qualifier = QUALIFIER_PURE, .impl = fn_lerpsmooth_,   .argtypes = {TYPE_FLOAT, TYPE_FLOAT, TYPE_FLOAT, TYPE_FLOAT, TYPE_NIL},             .synopsis = "float lerpsmooth(float a, float b, float dt, float omega)", .desc = "See Freya Holmér's talks :-)" , },
    { .id = SV_LIT("smoothstep"),   .type = TYPE_FLOAT, .qualifier = QUALIFIER_PURE, .impl = fn_smoothstep_,   .argtypes = {TYPE_FLOAT, TYPE_NIL},                                                 .synopsis = "float smoothstep(float t)", .desc = "Steps, smoothly. :3", },
    { .id = SV_LIT("radians"),      .type = TYPE_FLOAT, .qualifier = QUALIFIER_PURE, .impl = fn_radians_,      .argtypes = {TYPE_FLOAT, TYPE_NIL},                                                 .synopsis = "float radians(float degrees)", .desc = "Converts degrees into radians", },
    { .id = SV_LIT("perlin3D"),     .type = TYPE_FLOAT, .qualifier = QUALIFIER_PURE, .impl = fn_perlin3D_,     .argtypes = {TYPE_FLOAT, TYPE_FLOAT, TYPE_FLOAT, TYPE_NIL},                         .synopsis = "float perlin3D(float x, float y, float z)", .desc = "Perlin noise at (x,y,z)", },
    { .id = SV_LIT("aspect_ratio"), .type = TYPE_FLOAT, .qualifier = QUALIFIER_PURE, .impl = fn_aspect_ratio_, .argtypes = {TYPE_NIL},                                                             .synopsis = "float aspect_ratio()", .desc = "Returns the current window aspect ratio (width/height)", },

    { .id = SV_LIT("vec2"),                .type = TYPE_VEC2,  .qualifier = QUALIFIER_PURE, .impl = fn_vec2_,                .argtypes = {TYPE_FLOAT, TYPE_FLOAT, TYPE_NIL},           .synopsis = "vec2 vec2(float x, float y)", .desc = "Creates a 2D vector with components `x` and `y`", },
    { .id = SV_LIT("vec2_from_polar"),     .type = TYPE_VEC2,  .qualifier = QUALIFIER_PURE, .impl = fn_vec2_from_polar_,     .argtypes = {TYPE_FLOAT, TYPE_FLOAT, TYPE_NIL},           .synopsis = "vec2 vec2_from_polar(float r, float phi)", .desc = "Creates a 2D vector from the polar coordinates `r` and `phi`", },
    { .id = SV_LIT("vec2_distance"),       .type = TYPE_FLOAT, .qualifier = QUALIFIER_PURE, .impl = fn_vec2_distance_,       .argtypes = {TYPE_VEC2, TYPE_VEC2, TYPE_NIL},             .synopsis = "float vec2_distance(vec2 a, vec2 b)", .desc = "Returns the absolute distance between `a` and `b`", },
    { .id = SV_LIT("vec2_length"),         .type = TYPE_FLOAT, .qualifier = QUALIFIER_PURE, .impl = fn_vec2_length_,         .argtypes = {TYPE_VEC2, TYPE_NIL},                        .synopsis = "float vec2_length(vec2 v)", .desc = "Returns the absolute length of `v`", },
    { .id = SV_LIT("vec2_normalize"),      .type = TYPE_VEC2,  .qualifier = QUALIFIER_PURE, .impl = fn_vec2_normalize_,      .argtypes = {TYPE_VEC2, TYPE_NIL},                        .synopsis = "vec2 vec2_normalize(vec2 v)", .desc = "Returns the normalized vector of `v`", },
    { .id = SV_LIT("vec2_dot"),            .type = TYPE_FLOAT, .qualifier = QUALIFIER_PURE, .impl = fn_vec2_dot_,            .argtypes = {TYPE_VEC2, TYPE_VEC2, TYPE_NIL},             .synopsis = "float vec2_dot(vec2 a, vec2 b)", .desc = "Returns the dot product of `a` and `b`", },
    { .id = SV_LIT("vec2_mul_scalar"),     .type = TYPE_VEC2,  .qualifier = QUALIFIER_PURE, .impl = fn_vec2_mul_scalar_,     .argtypes = {TYPE_VEC2, TYPE_FLOAT, TYPE_NIL},            .synopsis = "vec2 vec2_mul_scalar(vec2 v, float s)", .desc = "Calculates the scalar-vector multiplication `s`*`v`", },
    { .id = SV_LIT("vec2_lerp"),           .type = TYPE_VEC2,  .qualifier = QUALIFIER_PURE, .impl = fn_vec2_lerp_,           .argtypes = {TYPE_VEC2, TYPE_VEC2, TYPE_FLOAT, TYPE_NIL}, .synopsis = "vec2 vec2_lerp(vec2 a, vec2 b, float t)", .desc = "Linearly interpolates between `a` and `b` for values of `t` in [0, 1]. I.e. lerp(a,b,t) = a*(1-t)+b*t", },
    { .id = SV_LIT("vec2_slerp"),          .type = TYPE_VEC2,  .qualifier = QUALIFIER_PURE, .impl = fn_vec2_slerp_,          .argtypes = {TYPE_VEC2, TYPE_VEC2, TYPE_FLOAT, TYPE_NIL}, .synopsis = "vec2 vec2_slerp(vec2 a, vec2 b, float t)", .desc = "Interpolates between `a` and `b` for values of `t` in [0, 1] with constant speed along an arc on the unit circle.", },
    { .id = SV_LIT("mouse_position"),      .type = TYPE_VEC2,  .qualifier = QUALIFIER_NONE, .impl = fn_mouse_position_,      .argtypes = {TYPE_NIL},                                   .synopsis = "vec2 mouse_position()", .desc = "Returns the current mouse position, in pixel coordinates.", },
    { .id = SV_LIT("mouse_drag_position"), .type = TYPE_VEC2,  .qualifier = QUALIFIER_NONE, .impl = fn_mouse_drag_position_, .argtypes = {TYPE_NIL},                                   .synopsis = "vec2 mouse_drag_position()", .desc = "Returns the mouse position from when the left mouse button was last held, in pixel coordinates.", },

    { .id = SV_LIT("vec3"),                .type = TYPE_VEC3,  .qualifier = QUALIFIER_PURE, .impl = fn_vec3_,                .argtypes = {TYPE_FLOAT, TYPE_FLOAT, TYPE_FLOAT, TYPE_NIL}, .synopsis = "vec3 vec3(float x, float y, float z)",                      . desc = "Creates a 3D vector with components `x`, `y`, and `z`", },
    { .id = SV_LIT("vec2_from_spherical"), .type = TYPE_VEC3,  .qualifier = QUALIFIER_PURE, .impl = fn_vec3_from_spherical_, .argtypes = {TYPE_FLOAT, TYPE_FLOAT, TYPE_FLOAT, TYPE_NIL}, .synopsis = "vec3 vec3_from_spherical(float r, float phi, float theta)", . desc = "Creates a 2D vector from the spherical coordinates `r`, `phi`, and `theta`", },
    { .id = SV_LIT("vec3_distance"),       .type = TYPE_FLOAT, .qualifier = QUALIFIER_PURE, .impl = fn_vec3_distance_,       .argtypes = {TYPE_VEC3, TYPE_VEC3, TYPE_NIL},               .synopsis = "float vec3_distance(vec3 a, vec3 b)",                        . desc = "Returns the absolute distance between `a` and `b`", },
    { .id = SV_LIT("vec3_length"),         .type = TYPE_FLOAT, .qualifier = QUALIFIER_PURE, .impl = fn_vec3_length_,         .argtypes = {TYPE_VEC3, TYPE_NIL},                          .synopsis = "float vec3_length(vec3 v)",                                  . desc = "Returns the absolute length of `v`", },
    { .id = SV_LIT("vec3_normalize"),      .type = TYPE_VEC3,  .qualifier = QUALIFIER_PURE, .impl = fn_vec3_normalize_,      .argtypes = {TYPE_VEC3, TYPE_NIL},                          .synopsis = "vec3 vec3_normalize(vec3 v)",                               . desc = "Returns the normalized vector of `v`", },
    { .id = SV_LIT("vec3_dot"),            .type = TYPE_FLOAT, .qualifier = QUALIFIER_PURE, .impl = fn_vec3_dot_,            .argtypes = {TYPE_VEC3, TYPE_VEC3, TYPE_NIL},               .synopsis = "float vec3_dot(vec3 a, vec3 b)",                             . desc = "Returns the dot product of `a` and `b`", },
    { .id = SV_LIT("vec3_mul_scalar"),     .type = TYPE_VEC3,  .qualifier = QUALIFIER_PURE, .impl = fn_vec3_mul_scalar_,     .argtypes = {TYPE_VEC3, TYPE_FLOAT, TYPE_NIL},              .synopsis = "vec3 vec3_mul_scalar(vec3 v, float s)",                     . desc = "Calculates the scalar-vector multiplication `s`*`v`", },
    { .id = SV_LIT("vec3_lerp"),           .type = TYPE_VEC3,  .qualifier = QUALIFIER_PURE, .impl = fn_vec3_lerp_,           .argtypes = {TYPE_VEC3, TYPE_VEC3, TYPE_FLOAT, TYPE_NIL},   .synopsis = "vec3 vec3_lerp(vec3 a, vec3 b, float t)",                   . desc = "Linearly interpolates between `a` and `b` for values of `t` in [0, 1]. I.e. lerp(a,b,t) = a*(1-t)+b*t", },
    { .id = SV_LIT("vec3_slerp"),          .type = TYPE_VEC3,  .qualifier = QUALIFIER_PURE, .impl = fn_vec3_slerp_,          .argtypes = {TYPE_VEC3, TYPE_VEC3, TYPE_FLOAT, TYPE_NIL},   .synopsis = "vec3 vec3_slerp(vec3 a, vec3 b, float t)",                  . desc = "Interpolates between `a` and `b` for values of `t` in [0, 1] with constant speed along an arc on the unit sphere.", },
    { .id = SV_LIT("vec3_cross"),          .type = TYPE_VEC3,  .qualifier = QUALIFIER_PURE, .impl = fn_vec3_cross_,          .argtypes = {TYPE_VEC3, TYPE_VEC3, TYPE_NIL},               .synopsis = "vec3 vec3_cross(vec3 a, vec3 b)",                           . desc = "Returns the cross product of `a` and `b`", },

    { .id = SV_LIT("vec4"),            .type = TYPE_VEC4,  .qualifier = QUALIFIER_PURE, .impl = fn_vec4_,            .argtypes = {TYPE_FLOAT, TYPE_FLOAT, TYPE_FLOAT, TYPE_FLOAT, TYPE_NIL}, .synopsis = "vec4 vec4(float x, float y, float z, float w)", .desc = "Creates a 4D vector with components `x`, `y`, `z`, and `w`", },
    { .id = SV_LIT("vec4_distance"),   .type = TYPE_FLOAT, .qualifier = QUALIFIER_PURE, .impl = fn_vec4_distance_,   .argtypes = {TYPE_VEC4, TYPE_VEC4, TYPE_NIL},                           .synopsis = "float vec4_distance(vec4 a, vec4 b)",            .desc = "Returns the absolute distance between `a` and `b`", },
    { .id = SV_LIT("vec4_length"),     .type = TYPE_FLOAT, .qualifier = QUALIFIER_PURE, .impl = fn_vec4_length_,     .argtypes = {TYPE_VEC4, TYPE_NIL},                                      .synopsis = "float vec4_length(vec4 v)",                      .desc = "Returns the absolute length of `v`", },
    { .id = SV_LIT("vec4_normalize"),  .type = TYPE_VEC4,  .qualifier = QUALIFIER_PURE, .impl = fn_vec4_normalize_,  .argtypes = {TYPE_VEC4, TYPE_NIL},                                      .synopsis = "vec4 vec4_normalize(vec4 v)",                   .desc = "Returns the normalized vector of `v`", },
    { .id = SV_LIT("vec4_dot"),        .type = TYPE_FLOAT, .qualifier = QUALIFIER_PURE, .impl = fn_vec4_dot_,        .argtypes = {TYPE_VEC4, TYPE_VEC4, TYPE_NIL},                           .synopsis = "float vec4_dot(vec4 a, vec4 b)",                 .desc = "Returns the dot product of `a` and `b`", },
    { .id = SV_LIT("vec4_mul_scalar"), .type = TYPE_VEC4,  .qualifier = QUALIFIER_PURE, .impl = fn_vec4_mul_scalar_, .argtypes = {TYPE_VEC4, TYPE_FLOAT, TYPE_NIL},                          .synopsis = "vec4 vec4_mul_scalar(vec4 v, float s)",         .desc = "Calculates the scalar-vector multiplication `s`*`v`", },
    { .id = SV_LIT("vec4_lerp"),       .type = TYPE_VEC4,  .qualifier = QUALIFIER_PURE, .impl = fn_vec4_lerp_,       .argtypes = {TYPE_VEC4, TYPE_VEC4, TYPE_FLOAT, TYPE_NIL},               .synopsis = "vec4 vec4_lerp(vec4 a, vec4 b, float t)",       .desc = "Linearly interpolates between `a` and `b` for values of `t` in [0, 1]. I.e. lerp(a,b,t) = a*(1-t)+b*t", },
    { .id = SV_LIT("vec4_xyz"),        .type = TYPE_VEC3,  .qualifier = QUALIFIER_PURE, .impl = fn_vec4_xyz_,        .argtypes = {TYPE_VEC4, TYPE_NIL},                                      .synopsis = "vec3 vec3_xyz(vec4 v)",                         .desc = "Returns the x,y, and z components of `v` as a vec3", },
    { .id = SV_LIT("rgba"),            .type = TYPE_VEC4,  .qualifier = QUALIFIER_PURE, .impl = fn_rgba_,            .argtypes = {TYPE_INT, TYPE_NIL},                                       .synopsis = "vec4 rgba(int hexcode)",                        .desc = "Returns a vector with R, G, B, and A components normalized to 0.0 - 1.0 given a color hexcode", },

    { .id = SV_LIT("ivec2"),           .type = TYPE_IVEC2, .qualifier = QUALIFIER_PURE, .impl = fn_ivec2_,       .argtypes = {TYPE_INT, TYPE_INT, TYPE_NIL}, .synopsis = "ivec2 ivec2(int x, int y)", .desc = "Creates a 2D integer vector with components `x` and `y`", },
    { .id = SV_LIT("iresolution"),     .type = TYPE_IVEC2, .qualifier = QUALIFIER_PURE, .impl = fn_iresolution_, .argtypes = {TYPE_NIL},                     .synopsis = "ivec2 iresolution()",       .desc = "Returns the current window resolution", },

    { .id = SV_LIT("ivec3"),      .type = TYPE_IVEC3, .qualifier = QUALIFIER_PURE, .impl = fn_ivec3_,      .argtypes = {TYPE_INT, TYPE_INT, TYPE_INT, TYPE_NIL},           .synopsis = "ivec3 ivec3(int x, int y, int z)", .desc = "Creates a 3D integer vector with components `x`, `y`, and `z`", },

    { .id = SV_LIT("ivec4"),      .type = TYPE_IVEC4, .qualifier = QUALIFIER_PURE, .impl = fn_ivec4_,      .argtypes = {TYPE_INT, TYPE_INT, TYPE_INT, TYPE_INT, TYPE_NIL}, .synopsis = "ivec4 ivec4(int x, int y, int z, int w)", .desc = "Creates a 4D integer vector with components `x`, `y`, `z`, and `w`", },

    { .id = SV_LIT("mat2"),       .type = TYPE_MAT2,  .qualifier = QUALIFIER_PURE, .impl = fn_mat2_,       .argtypes = {TYPE_VEC2, TYPE_VEC2, TYPE_NIL},                   .synopsis = "mat2 mat2(vec2 c0, vec2 c1)", .desc = "Creates a 2x2 matrix with column vectors `c0` and `c1`", },
    { .id = SV_LIT("mat2_id"),    .type = TYPE_MAT2,  .qualifier = QUALIFIER_PURE, .impl = fn_mat2_id_,    .argtypes = {TYPE_NIL},                                         .synopsis = "mat2 mat2_id()", .desc = "Creates a 2x2 identity matrix", },

    { .id = SV_LIT("mat3"),       .type = TYPE_MAT3,  .qualifier = QUALIFIER_PURE, .impl = fn_mat3_,       .argtypes = {TYPE_VEC3, TYPE_VEC3, TYPE_VEC3, TYPE_NIL},        .synopsis = "mat3 mat3(vec3 c0, vec3 c1, vec3 c2)", .desc = "Creates a 3x3 matrix with column vectors `c0`, `c1`, and `c2`", },
    { .id = SV_LIT("mat3_id"),    .type = TYPE_MAT3,  .qualifier = QUALIFIER_PURE, .impl = fn_mat3_id_,    .argtypes = {TYPE_NIL},                                         .synopsis = "mat3 mat3_id()", .desc = "Creates a 3x3 identity matrix", },

    { .id = SV_LIT("mat4"),                  .type = TYPE_MAT4, .qualifier = QUALIFIER_PURE, .impl = fn_mat4_,                  .argtypes = {TYPE_VEC4, TYPE_VEC4, TYPE_VEC4, TYPE_VEC4, TYPE_NIL}, .synopsis = "mat4 mat4(vec4 c0, vec4 c1, vec4 c2, vec4 c3)",        .desc = "Creates a 4x4 matrix with column vectors `c0`, `c1`, `c2`, and `c3`.", },
    { .id = SV_LIT("mat4_id"),               .type = TYPE_MAT4, .qualifier = QUALIFIER_PURE, .impl = fn_mat4_id_,               .argtypes = {TYPE_NIL},                                             .synopsis = "mat4 mat4_id()",                                       .desc = "Creates a 4x4 identity matrix.", },
    { .id = SV_LIT("mat4_make_scale"),       .type = TYPE_MAT4, .qualifier = QUALIFIER_PURE, .impl = fn_mat4_make_scale_,       .argtypes = {TYPE_VEC3, TYPE_NIL},                                  .synopsis = "mat4 mat4_make_scale(vec3 v)",                         .desc = "Creates a 4x4 scaling matrix for 3D vectors with scaling coefficients for the x, y, and z-axes given by `v`.", },
    { .id = SV_LIT("mat4_make_rotation"),    .type = TYPE_MAT4, .qualifier = QUALIFIER_PURE, .impl = fn_mat4_make_rotation_,    .argtypes = {TYPE_FLOAT, TYPE_VEC3, TYPE_NIL},                      .synopsis = "mat4 mat4_make_rotation(float angle, vec3 axis)",      .desc = "Creates a 4x4 rotation matrix for 3D vectors where the rotation operation is given by `angle` and `axis`.", },
    { .id = SV_LIT("mat4_make_translation"), .type = TYPE_MAT4, .qualifier = QUALIFIER_PURE, .impl = fn_mat4_make_translation_, .argtypes = {TYPE_VEC3, TYPE_NIL},                                  .synopsis = "mat4 mat4_make_translation(vec3 v)",                   .desc = "Creates a 4x4 translation matrix for 3D vectors where translation components for the x, y, and x-axes is given by `v`.", },
    { .id = SV_LIT("mat4_look_at"),          .type = TYPE_MAT4, .qualifier = QUALIFIER_PURE, .impl = fn_mat4_look_at_,          .argtypes = {TYPE_VEC3, TYPE_VEC3, TYPE_VEC3, TYPE_NIL},            .synopsis = "mat4 mat4_look_at(vec3 camera, vec3 target, vec3 up)", .desc = "Creates a 4x4 \"look-at\" view matrix, given a camera position `camera`, a target position `target`, and up-vector `up`.", },
    { .id = SV_LIT("mat4_scale"),            .type = TYPE_MAT4, .qualifier = QUALIFIER_PURE, .impl = fn_mat4_scale_,            .argtypes = {TYPE_MAT4, TYPE_VEC3, TYPE_NIL},                       .synopsis = "mat4 mat4_scale(mat4 m, vec3 v)",                      .desc = "Applies a scale-operation on `m` given scaling coefficients in `v`.", },
    { .id = SV_LIT("mat4_rotate"),           .type = TYPE_MAT4, .qualifier = QUALIFIER_PURE, .impl = fn_mat4_rotate_,           .argtypes = {TYPE_MAT4, TYPE_FLOAT, TYPE_VEC3, TYPE_NIL},           .synopsis = "mat4 mat4_rotate(mat4 m, float angle, vec3 axis)",     .desc = "Applies a rotation-operation on `m` given `angle` and `axis`.", },
    { .id = SV_LIT("mat4_translate"),        .type = TYPE_MAT4, .qualifier = QUALIFIER_PURE, .impl = fn_mat4_translate_,        .argtypes = {TYPE_MAT4, TYPE_VEC3, TYPE_NIL},                       .synopsis = "mat4 mat4_translate(mat4 m, vec3 v)",                  .desc = "Applies a translation-operation on `m` given translation components in `v`.", },
    { .id = SV_LIT("mat4_mul_mat4"),         .type = TYPE_MAT4, .qualifier = QUALIFIER_PURE, .impl = fn_mat4_mul_mat4_,         .argtypes = {TYPE_MAT4, TYPE_MAT4, TYPE_NIL},                       .synopsis = "mat4 mat4_mul_mat4(mat4 lhs, mat4 rhs)",               .desc = "Calculates the matrix-matrix multiplication `lhs`*`rhs`", },
    { .id = SV_LIT("mat4_mul_vec4"),         .type = TYPE_VEC4, .qualifier = QUALIFIER_PURE, .impl = fn_mat4_mul_vec4_,         .argtypes = {TYPE_MAT4, TYPE_VEC4, TYPE_NIL},                       .synopsis = "vec4 mat4_mul_vec4(mat4 m, vec4 v)",                   .desc = "Calculates the matrix-vector multiplication `m`*`v`", },
    { .id = SV_LIT("mat4_mul_scalar"),       .type = TYPE_MAT4, .qualifier = QUALIFIER_PURE, .impl = fn_mat4_mul_scalar_,       .argtypes = {TYPE_MAT4, TYPE_FLOAT, TYPE_NIL},                      .synopsis = "mat4 mat4_mul_scalar(mat4 m, float s)",                .desc = "Calculates the matrix-scalar multiplication `m`*`s`", },

    { .id = SV_LIT("input_float"),      .type = TYPE_FLOAT, .qualifier = QUALIFIER_NONE, .impl = fn_input_float_,      .argtypes = {TYPE_STR, TYPE_FLOAT, TYPE_NIL}, .synopsis = "float input_float(str label, float default)", .desc = "Creates an input widget for floats with the label `label` and default value `default`", },
    { .id = SV_LIT("checkbox"),         .type = TYPE_BOOL,  .qualifier = QUALIFIER_NONE, .impl = fn_checkbox_,         .argtypes = {TYPE_STR, TYPE_BOOL, TYPE_NIL}, .synopsis = "bool checkbox(str label, bool default)", .desc = "Creates an checkbox widget with the label `label` and default value `default`", },
    { .id = SV_LIT("drag_int"),         .type = TYPE_INT,   .qualifier = QUALIFIER_NONE, .impl = fn_drag_int_,         .argtypes = {TYPE_STR, TYPE_INT, TYPE_INT, TYPE_INT, TYPE_NIL}, .synopsis = "int drag_int(str label, int min, int max, int default)", .desc = "Creates an integer slider widget with the label `label`, minimum and maximum allow values `min` and `max`, and default value `default`", },
    { .id = SV_LIT("slider_float"),     .type = TYPE_FLOAT, .qualifier = QUALIFIER_NONE, .impl = fn_slider_float_,     .argtypes = {TYPE_STR, TYPE_FLOAT, TYPE_FLOAT, TYPE_FLOAT, TYPE_NIL}, .synopsis = "float slider_float(str label, float min, float max, float default)", .desc = "Creates an float slider widget with the label `label`, minimum and maximum allow values `min` and `max`, and default value `default`", },
    { .id = SV_LIT("slider_float_log"), .type = TYPE_FLOAT, .qualifier = QUALIFIER_NONE, .impl = fn_slider_float_log_, .argtypes = {TYPE_STR, TYPE_FLOAT, TYPE_FLOAT, TYPE_FLOAT, TYPE_NIL}, .synopsis = "float slider_float_log(str label, float min, float max, float default)", .desc = "Creates an float slider widget, with logarithmic scaling, with the label `label`, minimum and maximum allow values `min` and `max`, and default value `default`", },
    { .id = SV_LIT("input_int"),        .type = TYPE_INT,   .qualifier = QUALIFIER_NONE, .impl = fn_input_int_,        .argtypes = {TYPE_STR, TYPE_INT, TYPE_NIL}, .synopsis = "int input_int(str label, int default)", .desc = "Creates an input widget for integers with the label `label` and default value `default`", },
    { .id = SV_LIT("input_vec2"),       .type = TYPE_VEC2,  .qualifier = QUALIFIER_NONE, .impl = fn_input_vec2_,       .argtypes = {TYPE_STR, TYPE_VEC2, TYPE_NIL}, .synopsis = "vec2 input_vec2(str label, vec2 default)", .desc = "Creates an input widget for 2D vectors with the label `label` and default value `default`", },
    { .id = SV_LIT("input_vec3"),       .type = TYPE_VEC3,  .qualifier = QUALIFIER_NONE, .impl = fn_input_vec3_,       .argtypes = {TYPE_STR, TYPE_VEC3, TYPE_NIL}, .synopsis = "vec3 input_vec3(str label, vec3 default)", .desc = "Creates an input widget for 3D vectors with the label `label` and default value `default`", },
    { .id = SV_LIT("input_vec4"),       .type = TYPE_VEC4,  .qualifier = QUALIFIER_NONE, .impl = fn_input_vec4_,       .argtypes = {TYPE_STR, TYPE_VEC4, TYPE_NIL}, .synopsis = "vec4 input_vec4(str label, vec4 default)", .desc = "Creates an input widget for 4D vectors with the label `label` and default value `default`", },
    { .id = SV_LIT("color_picker"),     .type = TYPE_VEC4,  .qualifier = QUALIFIER_NONE, .impl = fn_color_picker_,     .argtypes = {TYPE_STR, TYPE_VEC4, TYPE_NIL}, .synopsis = "vec4 color_picker(str label, vec4 default)", .desc = "Creates a color picker widget with the label `label` and default value `default`", },
};
const size_t N_BUILTIN_FUNCTIONS = sizeof(BUILTIN_FUNCTIONS) / sizeof(BUILTIN_FUNCTIONS[0]);

/*--- Private variables -----------------------------------------------------------------*/

/* Simple Expression Language Virtual Machine */
static struct SVM {
    const ExeExpr *exe;
    u8 stack[SVM_STACK_SIZE];
    u32 pc;
    u32 sp;
} svm = {0};

/*--- Public functions ------------------------------------------------------------------*/

SelValue sel_eval(ExeExpr *exe, b8 force_recompute)
{
    if (exe == NULL) {
        //return (SelValue) {.val_i32 = -1};
        return (SelValue) {0};
    }

    if ((exe->qualifier & QUALIFIER_CONST) && 
        (exe->has_been_computed_once) &&
        (!force_recompute)) {
        return exe->cached_computed_value;
    }

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
    exe->cached_computed_value = result;
    exe->has_been_computed_once = true;

    return result;
}

/*--- Private functions -----------------------------------------------------------------*/

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
                    case TYPE_UINT: {u32 tmp = addu(lhs, rhs); svm_stack_push(&tmp, sizeof(tmp));} break;
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
                    case TYPE_UINT: {u32 tmp = subu(lhs, rhs); svm_stack_push(&tmp, sizeof(tmp));} break;
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
                    case TYPE_UINT: {u32 tmp = mulu(lhs, rhs); svm_stack_push(&tmp, sizeof(tmp));} break;
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
                    case TYPE_UINT: {u32 tmp = divu(lhs, rhs); svm_stack_push(&tmp, sizeof(tmp));} break;
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
                    case TYPE_UINT: {u32 tmp = remu(lhs, rhs); svm_stack_push(&tmp, sizeof(tmp));} break;
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
                u32 func_id = *(u32*)svm_next_bytes(sizeof(u32));
                const Func *func = &BUILTIN_FUNCTIONS[func_id];
                for (i32 i = 0; i < SEL_FUNC_MAX_N_ARGS; i++) {
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

/* ----------------------- Basic operators -------------------- */

static inline i32 addi(i32 *lhs, i32 *rhs) { return (*lhs) + (*rhs); }
static inline u32 addu(u32 *lhs, u32 *rhs) { return (*lhs) + (*rhs); }
static inline f32 addf(f32 *lhs, f32 *rhs) { return (*lhs) + (*rhs); }
static inline Vec2 addv2(Vec2 *lhs, Vec2 *rhs) {return vec2_add(*lhs, *rhs);}
static inline Vec3 addv3(Vec3 *lhs, Vec3 *rhs) {return vec3_add(*lhs, *rhs);}
static inline Vec4 addv4(Vec4 *lhs, Vec4 *rhs) {return vec4_add(*lhs, *rhs);}

static inline i32 subi(i32 *lhs, i32 *rhs) { return (*lhs) - (*rhs); }
static inline u32 subu(u32 *lhs, u32 *rhs) { return (*lhs) - (*rhs); }
static inline f32 subf(f32 *lhs, f32 *rhs) { return (*lhs) - (*rhs); }
static inline Vec2 subv2(Vec2 *lhs, Vec2 *rhs) {return vec2_sub(*lhs, *rhs);}
static inline Vec3 subv3(Vec3 *lhs, Vec3 *rhs) {return vec3_sub(*lhs, *rhs);}
static inline Vec4 subv4(Vec4 *lhs, Vec4 *rhs) {return vec4_sub(*lhs, *rhs);}

static inline i32 muli(i32 *lhs, i32 *rhs) { return (*lhs) * (*rhs); }
static inline u32 mulu(u32 *lhs, u32 *rhs) { return (*lhs) * (*rhs); }
static inline f32 mulf(f32 *lhs, f32 *rhs) { return (*lhs) * (*rhs); }
static inline Vec2 mulv2(Vec2 *lhs, Vec2 *rhs) {return vec2_hadamard(*lhs, *rhs);}
static inline Vec3 mulv3(Vec3 *lhs, Vec3 *rhs) {return vec3_hadamard(*lhs, *rhs);}
static inline Vec4 mulv4(Vec4 *lhs, Vec4 *rhs) {return vec4_hadamard(*lhs, *rhs);}

static inline i32 divi(i32 *lhs, i32 *rhs) { return (*lhs) / (*rhs); }
static inline u32 divu(u32 *lhs, u32 *rhs) { return (*lhs) / (*rhs); }
static inline f32 divf(f32 *lhs, f32 *rhs) { return (*lhs) / (*rhs); }
static inline Vec2 divv2(Vec2 *lhs, Vec2 *rhs) {return vec2_hadamard(*lhs, vec2_recip(*rhs));}
static inline Vec3 divv3(Vec3 *lhs, Vec3 *rhs) {return vec3_hadamard(*lhs, vec3_recip(*rhs));}
static inline Vec4 divv4(Vec4 *lhs, Vec4 *rhs) {return vec4_hadamard(*lhs, vec4_recip(*rhs));}

static inline i32 remi(i32 *lhs, i32 *rhs) { return (*lhs) % (*rhs); }
static inline u32 remu(u32 *lhs, u32 *rhs) { return (*lhs) % (*rhs); }
static inline i32 negi(i32 *val) { return -(*val); }
static inline f32 negf(f32 *val) { return -(*val); }

/* --------------------- TEXTURE functions ------------------ */

static SelValue fn_load_image_(void *args)
{
    StringView filepath = *(StringView *)args;
    i32 index = shaq_load_texture_if_necessary(filepath);
    if (index == -1) {
        return (SelValue) { .val_tex = {.error = 1}};
    }
    return (SelValue) { .val_tex = {.kind = LOADED_TEXTURE_INDEX, .loaded_texture_index = (u32) index}};
}

static SelValue fn_output_of_(void *args)
{
    StringView name = *(StringView *)args;
    i32 index = shaq_find_shader_id_by_name(name);
    if (index == -1) {
        return (SelValue) { .val_tex = {.error = 1}};
    }
    return (SelValue) { .val_tex = {.kind = SHADER_CURRENT_RENDER_TEXTURE_INDEX, .render_texture_index = (u32) index}};
}

static SelValue fn_last_output_of_(void *args)
{
    StringView name = *(StringView *)args;
    i32 index = shaq_find_shader_id_by_name(name);
    if (index == -1) {
        return (SelValue) { .val_tex = {.error = 1}};
    }
    return (SelValue) { .val_tex = {.kind = SHADER_LAST_RENDER_TEXTURE_INDEX, .render_texture_index = (u32) index}};
}



/* ---------------------- BOOL functions -------------------- */

static SelValue fn_left_mouse_button_is_down_(void *args)
{
    (void) args;
    return (SelValue) {.val_bool = user_input_left_mouse_button_is_down()};
}

static SelValue fn_right_mouse_button_is_down_(void *args)
{
    (void) args;
    return (SelValue) {.val_bool = user_input_right_mouse_button_is_down()};
}

static SelValue fn_left_mouse_button_was_clicked_(void *args)
{
    (void) args;
    return (SelValue) {.val_bool = user_input_left_mouse_button_was_clicked()};
}

static SelValue fn_right_mouse_button_was_clicked_(void *args)
{
    (void) args;
    return (SelValue) {.val_bool = user_input_right_mouse_button_was_clicked()};
}

static SelValue fn_key_is_down_(void *args)
{
    StringView key = *(StringView *)args;
    if (key.length != 1) {
        return (SelValue) {.val_bool = false};
    }
    char c = key.start[0];
    if (!(c >= 'a' && c <= 'z') && !(c >= 'A' && c <= 'Z')) {
        return (SelValue) {.val_bool = false};
    }
    return (SelValue) {.val_bool = user_input_key_is_down(c)};
}

static SelValue fn_key_was_pressed_(void *args)
{
    StringView key = *(StringView *)args;
    if (key.length != 1) {
        return (SelValue) {.val_bool = false};
    }
    char c = key.start[0];
    if (!(c >= 'a' && c <= 'z') && !(c >= 'A' && c <= 'Z')) {
        return (SelValue) {.val_bool = false};
    }
    return (SelValue) {.val_bool = user_input_key_was_pressed(c)};
}

/* ----------------------- INT functions -------------------- */

static SelValue fn_int_(void *args)
{
    f32 a0 = *(f32*)args;
    return (SelValue) {.val_i32 = (i32)a0}; 
}

static SelValue fn_unsigned_(void *args)
{
    i32 *args_i32 = (i32 *) args;
    return (SelValue) {.val_u32 = (u32)args_i32[0]}; 
}

static SelValue fn_mini_(void *args)
{
    i32 *args_i32 = (i32 *) args;
    return (SelValue) {.val_i32 = (args_i32[0] < args_i32[1]) ? args_i32[0] : args_i32[1]}; 
}

static SelValue fn_maxi_(void *args)
{
    i32 *args_i32 = (i32 *) args;
    return (SelValue) {.val_i32 = (args_i32[0] > args_i32[1]) ? args_i32[0] : args_i32[1]}; 
}

static SelValue fn_randi_(void *args)
{
    i32 *args_i32 = (i32 *) args;
    i32 min = args_i32[0];
    i32 max = args_i32[1];
    return (SelValue) {.val_i32 = rand() % (max + 1 - min) + min}; 
}

static SelValue fn_iota_(void *args)
{
    (void) args;
    static i32 iota = 0;
    return (SelValue) {.val_i32 = iota++}; 
}

static SelValue fn_frame_count_(void *args)
{
    (void) args;
    return (SelValue) {.val_i32 = shaq_frame_count()}; // TODO
}


/* ----------------------- UINT functions --------------------- */

static SelValue fn_signed_(void *args)
{
    u32 a0 = *(u32*)args;
    return (SelValue) {.val_i32 = (i32) a0}; 
}

static SelValue fn_xor_(void *args)
{
    u32 *args_u32 = (u32 *) args;
    return (SelValue) {.val_u32 = (args_u32[0] ^ args_u32[1])}; 
}

static SelValue fn_and_(void *args)
{
    u32 *args_u32 = (u32 *) args;
    return (SelValue) {.val_u32 = (args_u32[0] & args_u32[1])}; 
}

static SelValue fn_or_(void *args)
{
    u32 *args_u32 = (u32 *) args;
    return (SelValue) {.val_u32 = (args_u32[0] | args_u32[1])}; 
}

static SelValue fn_not_(void *args)
{
    u32 a0 = *(u32*)args;
    return (SelValue) {.val_u32 = ~a0}; 
}

static SelValue fn_lshift_(void *args)
{
    u32 *args_u32 = (u32 *) args;
    return (SelValue) {.val_u32 = (args_u32[0] << args_u32[1])}; 
}

static SelValue fn_rshift_(void *args)
{
    u32 *args_u32 = (u32 *) args;
    return (SelValue) {.val_u32 = (args_u32[0] >> args_u32[1])}; 
}

static SelValue fn_rol_(void *args)
{
    u32 *args_u32 = (u32 *) args;
    u32 x = args_u32[0];
    u32 n = args_u32[1] & (32 - 1);
    u32 b = x >> (32 - n);
    x <<= n;
    x |= b;
    //x |= (0xFFFFFFFFu >> (32 - n)) & upper;
    return (SelValue) {.val_u32 = x}; 
}

static SelValue fn_ror_(void *args)
{
    u32 *args_u32 = (u32 *) args;
    u32 x = args_u32[0];
    u32 n = args_u32[1] & (32 - 1);
    u32 b = x << (32 - n);
    x >>= n;
    x |= b;
    return (SelValue) {.val_u32 = x}; 
}


/* ----------------------- FLOAT functions -------------------- */

static SelValue fn_float_(void *args)
{
    i32 a0 = *(i32*)args;
    return (SelValue) {.val_f32 = (f32)a0}; 
}

static SelValue fn_time_(void *args)
{
    (void) args;
    return (SelValue) {.val_f32 = shaq_time()};
}

static SelValue fn_deltatime_(void *args)
{
    (void) args;
    return (SelValue) {.val_f32 = shaq_deltatime()};
}

static SelValue fn_rand_(void *args)
{
    f32 *args_f32 = (f32 *) args;
    f32 min = args_f32[0];
    f32 max = args_f32[1];
    f32 range = max - min;
    return (SelValue) {.val_f32 = ((f32)rand()/(f32)RAND_MAX)*range + min}; 
}

static SelValue fn_sqrt_(void *args)
{
    f32 a0 = *(f32*)args;
    return (SelValue) {.val_f32 = sqrtf(a0)}; 
}

static SelValue fn_pow_(void *args)
{
    f32 *args_f32 = (f32 *) args;
    return (SelValue) {.val_f32 = powf(args_f32[0], args_f32[1])}; 
}

static SelValue fn_exp_(void *args)
{
    f32 a0 = *(f32*)args;
    return (SelValue) {.val_f32 = expf(a0)}; 
}

static SelValue fn_log_(void *args)
{
    f32 a0 = *(f32*)args;
    return (SelValue) {.val_f32 = logf(a0)}; 
}

static SelValue fn_exp2_(void *args)
{
    f32 a0 = *(f32*)args;
    return (SelValue) {.val_f32 = exp2f(a0)}; 
}

static SelValue fn_log2_(void *args)
{
    f32 a0 = *(f32*)args;
    return (SelValue) {.val_f32 = log2f(a0)}; 
}

static SelValue fn_sin_(void *args)
{
    f32 a0 = *(f32*)args;
    return (SelValue) {.val_f32 = sinf(a0)}; 
}

static SelValue fn_cos_(void *args)
{
    f32 a0 = *(f32*)args;
    return (SelValue) {.val_f32 = cosf(a0)}; 
}

static SelValue fn_tan_(void *args)
{
    f32 a0 = *(f32*)args;
    return (SelValue) {.val_f32 = tanf(a0)}; 
}

static SelValue fn_asin_(void *args)
{
    f32 a0 = *(f32*)args;
    return (SelValue) {.val_f32 = asinf(a0)}; 
}

static SelValue fn_acos_(void *args)
{
    f32 a0 = *(f32*)args;
    return (SelValue) {.val_f32 = acosf(a0)}; 
}

static SelValue fn_atan_(void *args)
{
    f32 a0 = *(f32*)args;
    return (SelValue) {.val_f32 = atanf(a0)}; 
}

static SelValue fn_atan2_(void *args)
{
    f32 *args_f32 = (f32 *) args;
    return (SelValue) {.val_f32 = atan2f(args_f32[0], args_f32[1])}; 
}

static SelValue fn_round_(void *args)
{
    f32 a0 = *(f32*)args;
    return (SelValue) {.val_f32 = roundf(a0)}; 
}

static SelValue fn_floor_(void *args)
{
    f32 a0 = *(f32*)args;
    return (SelValue) {.val_f32 = floorf(a0)}; 
}

static SelValue fn_ceil_(void *args)
{
    f32 a0 = *(f32*)args;
    return (SelValue) {.val_f32 = ceilf(a0)}; 
}

static SelValue fn_fract_(void *args)
{
    f32 a0 = *(f32*)args;
    return (SelValue) {.val_f32 = a0 - floorf(a0)}; 
}

static SelValue fn_min_(void *args)
{
    f32 *args_f32 = (f32 *) args;
    return (SelValue) {.val_f32 = (args_f32[0] < args_f32[1]) ? args_f32[0] : args_f32[1]}; 
}

static SelValue fn_max_(void *args)
{
    f32 *args_f32 = (f32 *) args;
    return (SelValue) {.val_f32 = (args_f32[0] > args_f32[1]) ? args_f32[0] : args_f32[1]}; 
}

static SelValue fn_clamp_(void *args)
{
    f32 *args_f32 = (f32 *) args;
    return (SelValue) {.val_f32 = hglm_clamp(args_f32[0], args_f32[1], args_f32[2])}; 
}

static SelValue fn_lerp_(void *args)
{
    f32 *args_f32 = (f32 *) args;
    return (SelValue) {.val_f32 = hglm_lerp(args_f32[0], args_f32[1], args_f32[2])}; 
}

static SelValue fn_ilerp_(void *args)
{
    f32 *args_f32 = (f32 *) args;
    return (SelValue) {.val_f32 = hglm_ilerp(args_f32[0], args_f32[1], args_f32[2])}; 
}

static SelValue fn_remap_(void *args)
{
    f32 *args_f32 = (f32 *) args;
    return (SelValue) {.val_f32 = hglm_remap(args_f32[0], args_f32[1], args_f32[2], args_f32[3], args_f32[4])}; 
}

static SelValue fn_lerpsmooth_(void *args)
{
    f32 *args_f32 = (f32 *) args;
    return (SelValue) {.val_f32 = hglm_lerpsmooth(args_f32[0], args_f32[1], args_f32[2], args_f32[3])}; 
}

static SelValue fn_smoothstep_(void *args)
{
    f32 a0 = *(f32*)args;
    return (SelValue) {.val_f32 = hglm_smoothstep(a0)}; 
}

static SelValue fn_radians_(void *args)
{
    f32 a0 = *(f32*)args;
    return (SelValue) {.val_f32 = (f32)((2.0*HGLM_PI/360.0)) * a0}; 
}

static SelValue fn_perlin3D_(void *args)
{
    f32 *args_f32 = (f32 *) args;
    return (SelValue) {.val_f32 = hglm_perlin3D(args_f32[0], args_f32[1], args_f32[2])}; 
}

static SelValue fn_aspect_ratio_(void *args)
{
    (void) args;
    IVec2 ires = renderer_shader_viewport_size();
    return (SelValue) {.val_f32 = (f32)ires.x / (f32)ires.y}; 
}


/* ----------------------- VEC2 functions -------------------- */

static SelValue fn_vec2_(void *args)
{
    f32 *args_f32 = (f32 *) args;
    return (SelValue) {.val_vec2 = hglm_vec2_make(args_f32[0], args_f32[1])};
}

static SelValue fn_vec2_from_polar_(void *args)
{
    f32 *args_f32 = (f32 *) args;
    return (SelValue) {.val_vec2 = hglm_vec2_from_polar(args_f32[0], args_f32[1])};
}

static SelValue fn_vec2_distance_(void *args)
{
    Vec2 *args_v2 = (Vec2 *) args;
    return (SelValue) {.val_f32 = hglm_vec2_distance(args_v2[0], args_v2[1])};
}

static SelValue fn_vec2_length_(void *args)
{
    Vec2 *args_v2 = (Vec2 *) args;
    return (SelValue) {.val_f32 = hglm_vec2_len(args_v2[0])};
}

static SelValue fn_vec2_normalize_(void *args)
{
    Vec2 *args_v2 = (Vec2 *) args;
    return (SelValue) {.val_vec2 = hglm_vec2_normalize(args_v2[0])};
}

static SelValue fn_vec2_dot_(void *args)
{
    Vec2 *args_v2 = (Vec2 *) args;
    return (SelValue) {.val_f32 = hglm_vec2_dot(args_v2[0], args_v2[1])};
}

static SelValue fn_vec2_mul_scalar_(void *args)
{
    Vec2 *args_v2 = (Vec2 *) args;
    return (SelValue) {.val_vec2 = hglm_vec2_mul_scalar(args_v2[0], *(f32*)&args_v2[1])};
}

static SelValue fn_vec2_lerp_(void *args)
{
    Vec2 *args_v2 = (Vec2 *) args;
    return (SelValue) {.val_vec2 = hglm_vec2_lerp(args_v2[0], args_v2[1], *(f32*)&args_v2[2])};
}

static SelValue fn_vec2_slerp_(void *args)
{
    Vec2 *args_v2 = (Vec2 *) args;
    return (SelValue) {.val_vec2 = hglm_vec2_slerp(args_v2[0], args_v2[1], *(f32*)&args_v2[2])};
}

static SelValue fn_mouse_position_(void *args)
{
    (void) args;
    return (SelValue) {.val_vec2 = user_input_mouse_position()};
}

static SelValue fn_mouse_drag_position_(void *args)
{
    (void) args;
    return (SelValue) {.val_vec2 = user_input_mouse_drag_position()};
}


/* ----------------------- VEC3 functions -------------------- */

static SelValue fn_vec3_(void *args)
{
    f32 *args_f32 = (f32 *) args;
    return (SelValue) {.val_vec3 = hglm_vec3_make(args_f32[0], args_f32[1], args_f32[2])};
}

static SelValue fn_vec3_from_spherical_(void *args)
{
    f32 *args_f32 = (f32 *) args;
    return (SelValue) {.val_vec3 = hglm_vec3_from_spherical(args_f32[0], args_f32[1], args_f32[1])};
} 
  
static SelValue fn_vec3_distance_(void *args)
{
    Vec3 *args_v3 = (Vec3 *) args;
    return (SelValue) {.val_f32 = hglm_vec3_distance(args_v3[0], args_v3[1])};
} 
  
static SelValue fn_vec3_length_(void *args)
{
    Vec3 *args_v3 = (Vec3 *) args;
    return (SelValue) {.val_f32 = hglm_vec3_len(args_v3[0])};
} 
  
static SelValue fn_vec3_normalize_(void *args)
{
    Vec3 *args_v3 = (Vec3 *) args;
    return (SelValue) {.val_vec3 = hglm_vec3_normalize(args_v3[0])};
} 
  
static SelValue fn_vec3_dot_(void *args)
{
    Vec3 *args_v3 = (Vec3 *) args;
    return (SelValue) {.val_f32 = hglm_vec3_dot(args_v3[0], args_v3[1])};
} 
  
static SelValue fn_vec3_mul_scalar_(void *args)
{
    Vec3 *args_v3 = (Vec3 *) args;
    return (SelValue) {.val_vec3 = hglm_vec3_mul_scalar(args_v3[0], *(f32*)&args_v3[1])};
} 
  
static SelValue fn_vec3_lerp_(void *args)
{
    Vec3 *args_v3 = (Vec3 *) args;
    return (SelValue) {.val_vec3 = hglm_vec3_lerp(args_v3[0], args_v3[1], *(f32*)&args_v3[2])};
} 
  
static SelValue fn_vec3_slerp_(void *args)
{
    Vec3 *args_v3 = (Vec3 *) args;
    return (SelValue) {.val_vec3 = hglm_vec3_lerp(args_v3[0], args_v3[1], *(f32*)&args_v3[2])};
} 
  
static SelValue fn_vec3_cross_(void *args)
{
    Vec3 *args_v3 = (Vec3 *) args;
    return (SelValue) {.val_vec3 = hglm_vec3_cross(args_v3[0], args_v3[1])};
} 


/* ----------------------- VEC4 functions -------------------- */

static SelValue fn_vec4_(void *args)
{
    f32 *args_f32 = (f32 *) args;
    return (SelValue) {.val_vec4 = hglm_vec4_make(args_f32[0], args_f32[1], args_f32[2], args_f32[3])};
}

static SelValue fn_vec4_distance_(void *args)
{
    Vec4 *args_v4 = (Vec4 *) args;
    return (SelValue) {.val_f32 = hglm_vec4_distance(args_v4[0], args_v4[1])};
} 
  
static SelValue fn_vec4_length_(void *args)
{
    Vec4 *args_v4 = (Vec4 *) args;
    return (SelValue) {.val_f32 = hglm_vec4_len(args_v4[0])};
} 
  
static SelValue fn_vec4_normalize_(void *args)
{
    Vec4 *args_v4 = (Vec4 *) args;
    return (SelValue) {.val_vec4 = hglm_vec4_normalize(args_v4[0])};
} 
  
static SelValue fn_vec4_dot_(void *args)
{
    Vec4 *args_v4 = (Vec4 *) args;
    return (SelValue) {.val_f32 = hglm_vec4_dot(args_v4[0], args_v4[1])};
} 
  
static SelValue fn_vec4_mul_scalar_(void *args)
{
    Vec4 *args_v4 = (Vec4 *) args;
    return (SelValue) {.val_vec4 = hglm_vec4_mul_scalar(args_v4[0], *(f32*)&args_v4[1])};
} 
  
static SelValue fn_vec4_lerp_(void *args)
{
    Vec4 *args_v4 = (Vec4 *) args;
    return (SelValue) {.val_vec4 = hglm_vec4_lerp(args_v4[0], args_v4[1], *(f32*)&args_v4[2])};
} 
  
static SelValue fn_vec4_xyz_(void *args)
{
    Vec4 *args_v4 = (Vec4 *) args;
    return (SelValue) {.val_vec3 = args_v4[0].xyz};
} 
 
static SelValue fn_rgba_(void *args)
{
    u32 argu32 = *(u32 *)args;
    f32 r = (f32)(argu32 >> 24 & 0xFF) / 255.0f;
    f32 g = (f32)(argu32 >> 16 & 0xFF) / 255.0f;
    f32 b = (f32)(argu32 >>  8 & 0xFF) / 255.0f;
    f32 a = (f32)(argu32 >>  0 & 0xFF) / 255.0f;
    return (SelValue) {.val_vec4 = hglm_vec4_make(r, g, b, a)};
}


/* ---------------------- IVEC2 functions -------------------- */

static SelValue fn_ivec2_(void *args)
{
    i32 *args_i32 = (i32 *) args;
    return (SelValue) {.val_ivec2 = hglm_ivec2_make(args_i32[0], args_i32[1])};
}

static SelValue fn_iresolution_(void *args)
{
    (void) args;
    return (SelValue) {.val_ivec2 = renderer_shader_viewport_size()};
}


/* ---------------------- IVEC4 functions -------------------- */

static SelValue fn_ivec3_(void *args)
{
    i32 *args_i32 = (i32 *) args;
    return (SelValue) {.val_ivec3 = hglm_ivec3_make(args_i32[0], args_i32[1], args_i32[2])};
}


/* ---------------------- IVEC4 functions -------------------- */

static SelValue fn_ivec4_(void *args)
{
    i32 *args_i32 = (i32 *) args;
    return (SelValue) {.val_ivec4 = hglm_ivec4_make(args_i32[0], args_i32[1], args_i32[2], args_i32[3])};
}


/* ---------------------- MAT2 functions -------------------- */

static SelValue fn_mat2_(void *args)
{
    Vec2 *args_v2 = (Vec2 *) args;
    return (SelValue) {.val_mat2 = hglm_mat2_make(args_v2[0], args_v2[1])};
}

static SelValue fn_mat2_id_(void *args)
{
    (void) args;
    return (SelValue) {.val_mat2 = HGLM_MAT2_IDENTITY};
}


/* ---------------------- MAT3 functions -------------------- */

static SelValue fn_mat3_(void *args)
{
    Vec3 *args_v3 = (Vec3 *) args;
    return (SelValue) {.val_mat3 = hglm_mat3_make(args_v3[0], args_v3[1], args_v3[2])};
}

static SelValue fn_mat3_id_(void *args)
{
    (void) args;
    return (SelValue) {.val_mat3 = HGLM_MAT3_IDENTITY};
}


/* ---------------------- MAT4 functions -------------------- */

static SelValue fn_mat4_(void *args)
{
    Vec4 *args_v4 = (Vec4 *) args;
    return (SelValue) {.val_mat4 = hglm_mat4_make(args_v4[0], args_v4[1], args_v4[2], args_v4[3])};
}

static SelValue fn_mat4_id_(void *args)
{
    (void) args;
    return (SelValue) {.val_mat4 = HGLM_MAT4_IDENTITY};
}

static SelValue fn_mat4_make_scale_(void *args)
{ 
    Vec3 *args_v3 = (Vec3 *) args;
    return (SelValue) {.val_mat4 = hglm_mat4_make_scale(args_v3[0])};
}  

static SelValue fn_mat4_make_rotation_(void *args)
{ 
    f32 *args_f32 = (f32 *) args;
    return (SelValue) {.val_mat4 = hglm_mat4_make_rotation(args_f32[0], *(Vec3*)&args_f32[1])};
}  

static SelValue fn_mat4_make_translation_(void *args)
{ 
    Vec3 *args_v3 = (Vec3 *) args;
    return (SelValue) {.val_mat4 = hglm_mat4_make_translation(args_v3[0])};
}  

static SelValue fn_mat4_look_at_(void *args)
{ 
    Vec3 *args_v3 = (Vec3 *) args;
    return (SelValue) {.val_mat4 = hglm_mat4_look_at(args_v3[0], args_v3[1], args_v3[2])};
}  

static SelValue fn_mat4_scale_(void *args)
{ 
    Mat4 *args_m4 = (Mat4 *) args;
    return (SelValue) {.val_mat4 = hglm_mat4_scale(args_m4[0], *(Vec3*)&args_m4[1])};
}  

static SelValue fn_mat4_rotate_(void *args)
{ 
    u8 *args8 = (u8 *) args;
    Mat4 *args_m4 = (Mat4 *) args;
    return (SelValue) {.val_mat4 = hglm_mat4_rotate(args_m4[0], 
                                                   *(f32*)(args8 + sizeof(Mat4)), 
                                                   *(Vec3*)(args8 + sizeof(Mat4) + sizeof(Vec3)))};
}  

static SelValue fn_mat4_translate_(void *args)
{ 
    Mat4 *args_m4 = (Mat4 *) args;
    return (SelValue) {.val_mat4 = hglm_mat4_translate(args_m4[0], *(Vec3*)&args_m4[1])};
}  

static SelValue fn_mat4_mul_mat4_(void *args)
{ 
    Mat4 *args_m4 = (Mat4 *) args;
    return (SelValue) {.val_mat4 = hglm_mat4_mul_mat4(args_m4[0], args_m4[1])};
}  

static SelValue fn_mat4_mul_vec4_(void *args)
{ 
    Mat4 *args_m4 = (Mat4 *) args;
    return (SelValue) {.val_vec4 = hglm_mat4_mul_vec4(args_m4[0], *(Vec4*)&args_m4[1])};
}  

static SelValue fn_mat4_mul_scalar_(void *args)
{ 
    Mat4 *args_m4 = (Mat4 *) args;
    return (SelValue) {.val_mat4 = hglm_mat4_mul_scalar(args_m4[0], *(f32*)&args_m4[1])};
}  


/* ---------------------- GUI functions --------------------- */

static SelValue fn_input_float_(void *args)
{
    u8 *args8 = (u8 *) args;
    StringView label = *(StringView *)args;
    void *secondary_args = (void *)(args8 + sizeof(StringView));
    return gui_get_dynamic_item_value(label, INPUT_FLOAT, secondary_args, 1*sizeof(f32));
}

static SelValue fn_checkbox_(void *args)
{
    u8 *args8 = (u8 *) args;
    StringView label = *(StringView *)args;
    void *secondary_args = (void *)(args8 + sizeof(StringView));
    return gui_get_dynamic_item_value(label, CHECKBOX, secondary_args, 1*sizeof(f32));
}

static SelValue fn_drag_int_(void *args)
{
    u8 *args8 = (u8 *) args;
    StringView label = *(StringView *)args;
    void *secondary_args = (void *)(args8 + sizeof(StringView));
    return gui_get_dynamic_item_value(label, DRAG_INT, secondary_args, 3*sizeof(i32));
}

static SelValue fn_slider_float_(void *args)
{
    u8 *args8 = (u8 *) args;
    StringView label = *(StringView *)args;
    void *secondary_args = (void *)(args8 + sizeof(StringView));
    return gui_get_dynamic_item_value(label, SLIDER_FLOAT, secondary_args, 3*sizeof(f32));
}

static SelValue fn_slider_float_log_(void *args)
{
    u8 *args8 = (u8 *) args;
    StringView label = *(StringView *)args;
    void *secondary_args = (void *)(args8 + sizeof(StringView));
    return gui_get_dynamic_item_value(label, SLIDER_FLOAT_LOG, secondary_args, 3*sizeof(f32));
}

static SelValue fn_input_int_(void *args)
{
    u8 *args8 = (u8 *) args;
    StringView label = *(StringView *)args;
    void *secondary_args = (void *)(args8 + sizeof(StringView));
    return gui_get_dynamic_item_value(label, INPUT_INT, secondary_args, sizeof(i32));
}

static SelValue fn_input_vec2_(void *args)
{
    u8 *args8 = (u8 *) args;
    StringView label = *(StringView *)args;
    void *secondary_args = (void *)(args8 + sizeof(StringView));
    return gui_get_dynamic_item_value(label, INPUT_VEC2, secondary_args, sizeof(Vec2));
}

static SelValue fn_input_vec3_(void *args)
{
    u8 *args8 = (u8 *) args;
    StringView label = *(StringView *)args;
    void *secondary_args = (void *)(args8 + sizeof(StringView));
    return gui_get_dynamic_item_value(label, INPUT_VEC3, secondary_args, sizeof(Vec3));
}

static SelValue fn_input_vec4_(void *args)
{
    u8 *args8 = (u8 *) args;
    StringView label = *(StringView *)args;
    void *secondary_args = (void *)(args8 + sizeof(StringView));
    return gui_get_dynamic_item_value(label, INPUT_VEC4, secondary_args, sizeof(Vec4));
}

static SelValue fn_color_picker_(void *args)
{
    u8 *args8 = (u8 *) args;
    StringView label = *(StringView *)args;
    void *secondary_args = (void *)(args8 + sizeof(StringView));
    return gui_get_dynamic_item_value(label, COLOR_PICKER, secondary_args, sizeof(Vec4));
}






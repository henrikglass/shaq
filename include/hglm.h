/**
 * LICENSE:
 *
 * MIT License
 *
 * Copyright (c) 2024 Henrik A. Glass
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * MIT License
 *
 *
 * ABOUT:
 *
 * hglm.h is a (mostly) vector math library (with some SIMD support).
 *
 * hglm_aliases.h contains aliases for types and functions inside hglm.h that
 * omit the `Hglm` and `hglm_` prefixes.
 *
 *
 * USAGE:
 *
 * Include `hglm.h` like this:
 *
 *     #include "hglm.h"
 *
 * Optionally include `hglm_aliases.h`:
 *
 *     #include "hglm_aliases.h"
 *
 *
 * EXAMPLE:
 *
 * project vector a onto b (using aliases!):
 *
 *     Vec2 a = vec2_make(10, 5);
 *     Vec2 b = vec2_make(20, 0);
 *     Vec2 projb_a = vec2_mul_scalar(b, (vec2_dot(a, b) / vec2_dot(b, b)));
 *     vec2_print(projb_a);
 *
 * spherical linear interpolation between a and b (using aliases!):
 *
 *     Vec2 a = vec2_make(10, 0);
 *     Vec2 b = vec2_make(0, 10);
 *     for (int i = 0; i <= 8; i++) {
 *         vec2_print(vec2_slerp(a, b, (float)i/8));
 *     }
 *     
 *
 * AUTHOR: Henrik A. Glass
 *
 */

#ifndef HGLM_H
#define HGLM_H

#include <math.h>
#include <stdint.h>

#include <assert.h> // DEBUG

#define HGL_INLINE inline __attribute__((always_inline))

#if !defined(HGLM_ALLOC) && !defined(HGLM_FREE)
#   include <stdlib.h>
#   define HGLM_ALLOC malloc
#   define HGLM_FREE  free
#endif

#define HGLM_PI 3.14159265358979

#define HGLM_DEG_TO_RAD(deg) ((deg)*(HGLM_PI/180.0f))
#define HGLM_RAD_TO_DEG(rad) ((rad)*(180.0f/HGLM_PI))

#ifdef HGLM_USE_SIMD
#   include <smmintrin.h>
#   include <immintrin.h>
#endif

#define HGLM_MAT3_IDENTITY ((HglmMat3) {   \
    .m00 = 1.0f, .m01 = 0.0f, .m02 = 0.0f, \
    .m10 = 0.0f, .m11 = 1.0f, .m12 = 0.0f, \
    .m20 = 0.0f, .m21 = 0.0f, .m22 = 1.0f,})

#define HGLM_MAT4_IDENTITY ((HglmMat4) {                \
    .m00 = 1.0f, .m01 = 0.0f, .m02 = 0.0f, .m03 = 0.0f, \
    .m10 = 0.0f, .m11 = 1.0f, .m12 = 0.0f, .m13 = 0.0f, \
    .m20 = 0.0f, .m21 = 0.0f, .m22 = 1.0f, .m23 = 0.0f, \
    .m30 = 0.0f, .m31 = 0.0f, .m32 = 0.0f, .m33 = 1.0f,})

typedef struct
{
    int x;
    int y;
} HglmIVec2;

typedef struct
{
    float x;
    float y;
} HglmVec2;

typedef struct
{
    union {
        struct {
            float x;
            float y;
        };
        HglmVec2 xy;
    };
    float z;
} HglmVec3;

typedef union __attribute__ ((aligned(16)))
{
    struct {
        union {
            struct {
                union {
                    struct {
                        float x;
                        float y;
                    };
                    HglmVec2 xy;
                };
                float z;
            };
            HglmVec3 xyz;
        };
        float w;
    };
#ifdef HGLM_USE_SIMD
    __m128 v;
#endif
    float f[4];
} HglmVec4;

typedef struct __attribute__ ((aligned(16)))
{
    union {
        struct {
            HglmVec3 c0;
            HglmVec3 c1;
            HglmVec3 c2;
        };
        struct {
            float m00;
            float m10;
            float m20;
            float m01;
            float m11;
            float m21;
            float m02;
            float m12;
            float m22;
        };
        float f[9];
    };
} HglmMat3;

typedef struct __attribute__ ((aligned(16)))
{
    union {
        struct {
            HglmVec4 c0;
            HglmVec4 c1;
            HglmVec4 c2;
            HglmVec4 c3;
        };
        struct {
            float m00;
            float m10;
            float m20;
            float m30;
            float m01;
            float m11;
            float m21;
            float m31;
            float m02;
            float m12;
            float m22;
            float m32;
            float m03;
            float m13;
            float m23;
            float m33;
        };
        float f[16];
    };
} HglmMat4;

typedef struct
{
    float *data;
    union {
        uint32_t M;
        uint32_t rows;
    };
    union {
        uint32_t N;
        uint32_t cols;
    };
} HglmMat;

static HGL_INLINE HglmIVec2 hglm_ivec2_make(int x, int y);
static HGL_INLINE HglmIVec2 hglm_ivec2_add(HglmIVec2 a, HglmIVec2 b);
static HGL_INLINE HglmIVec2 hglm_ivec2_sub(HglmIVec2 a, HglmIVec2 b);
static HGL_INLINE float hglm_ivec2_distance(HglmIVec2 a, HglmIVec2 b);
static HGL_INLINE float hglm_ivec2_len(HglmIVec2 v);
static HGL_INLINE HglmIVec2 hglm_ivec2_mul_scalar(HglmIVec2 v, float s);
static HGL_INLINE HglmIVec2 hglm_ivec2_lerp(HglmIVec2 a, HglmIVec2 b, float amount);

static HGL_INLINE HglmVec2 hglm_vec2_make(float x, float y);
static HGL_INLINE HglmVec2 hglm_vec2_from_polar(float r, float phi);
static HGL_INLINE HglmVec2 hglm_vec2_add(HglmVec2 a, HglmVec2 b);
static HGL_INLINE HglmVec2 hglm_vec2_sub(HglmVec2 a, HglmVec2 b);
static HGL_INLINE float hglm_vec2_distance(HglmVec2 a, HglmVec2 b);
static HGL_INLINE float hglm_vec2_len(HglmVec2 v);
static HGL_INLINE HglmVec2 hglm_vec2_normalize(HglmVec2 v);
static HGL_INLINE float hglm_vec2_dot(HglmVec2 a, HglmVec2 b);
static HGL_INLINE HglmVec2 hglm_vec2_hadamard(HglmVec2 a, HglmVec2 b);
static HGL_INLINE HglmVec2 hglm_vec2_mul_scalar(HglmVec2 v, float s);
static HGL_INLINE HglmVec2 hglm_vec2_reflect(HglmVec2 v, HglmVec2 normal);
static HGL_INLINE HglmVec2 hglm_vec2_lerp(HglmVec2 a, HglmVec2 b, float t);
static HGL_INLINE HglmVec2 hglm_vec2_slerp(HglmVec2 a, HglmVec2 b, float t);
static HGL_INLINE HglmVec2 hglm_vec2_bezier3(HglmVec2 v0, HglmVec2 v1, HglmVec2 v2, HglmVec2 v3, float t);

static HGL_INLINE HglmVec3 hglm_vec3_make(float x, float y, float z);
static HGL_INLINE HglmVec3 hglm_vec3_from_spherical(float r, float phi, float theta);
static HGL_INLINE HglmVec3 hglm_vec3_add(HglmVec3 a, HglmVec3 b);
static HGL_INLINE HglmVec3 hglm_vec3_sub(HglmVec3 a, HglmVec3 b);
static HGL_INLINE float hglm_vec3_distance(HglmVec3 a, HglmVec3 b);
static HGL_INLINE float hglm_vec3_len(HglmVec3 v);
static HGL_INLINE HglmVec3 hglm_vec3_normalize(HglmVec3 v);
static HGL_INLINE float hglm_vec3_dot(HglmVec3 a, HglmVec3 b);
static HGL_INLINE HglmVec3 hglm_vec3_cross(HglmVec3 a, HglmVec3 b);
static HGL_INLINE HglmVec3 hglm_vec3_hadamard(HglmVec3 a, HglmVec3 b);
static HGL_INLINE HglmVec3 hglm_vec3_mul_scalar(HglmVec3 v, float s);
static HGL_INLINE HglmVec3 hglm_vec3_reflect(HglmVec3 v, HglmVec3 normal);
static HGL_INLINE HglmVec3 hglm_vec3_lerp(HglmVec3 a, HglmVec3 b, float t);
static HGL_INLINE HglmVec3 hglm_vec3_slerp(HglmVec3 a, HglmVec3 b, float t);
static HGL_INLINE HglmVec3 hglm_vec3_bezier3(HglmVec3 v0, HglmVec3 v1, HglmVec3 v2, HglmVec3 v3, float t);

static HGL_INLINE HglmVec4 hglm_vec4_make(float x, float y, float z, float w);
static HGL_INLINE HglmVec4 hglm_vec4_add(HglmVec4 a, HglmVec4 b);
static HGL_INLINE HglmVec4 hglm_vec4_sub(HglmVec4 a, HglmVec4 b);
static HGL_INLINE float hglm_vec4_distance(HglmVec4 a, HglmVec4 b);
static HGL_INLINE float hglm_vec4_len(HglmVec4 v);
static HGL_INLINE HglmVec4 hglm_vec4_normalize(HglmVec4 v);
static HGL_INLINE float hglm_vec4_dot(HglmVec4 a, HglmVec4 b);
static HGL_INLINE HglmVec4 hglm_vec4_hadamard(HglmVec4 a, HglmVec4 b);
static HGL_INLINE HglmVec4 hglm_vec4_mul_scalar(HglmVec4 v, float s);
static HGL_INLINE HglmVec4 hglm_vec4_perspective_divide(HglmVec4 v);
static HGL_INLINE HglmVec4 hglm_vec4_lerp(HglmVec4 a, HglmVec4 b, float t);
static HGL_INLINE HglmVec4 hglm_vec4_bezier3(HglmVec4 v0, HglmVec4 v1, HglmVec4 v2, HglmVec4 v3, float t);

__attribute__ ((const, unused)) static HGL_INLINE HglmMat3 hglm_mat3_make(HglmVec3 c0, HglmVec3 c1, HglmVec3 c2);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat3 hglm_mat3_make_identity(void);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat3 hglm_mat3_make_from_mat4(HglmMat4 mat4);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat3 hglm_mat3_transpose(HglmMat3 m);
__attribute__ ((const, unused)) static HGL_INLINE HglmVec3 hglm_mat3_mul_vec3(HglmMat3 m, HglmVec3 v);

__attribute__ ((const, unused)) static HGL_INLINE HglmMat4 hglm_mat4_make(HglmVec4 c0, HglmVec4 c1, HglmVec4 c2, HglmVec4 c3);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat4 hglm_mat4_make_zero(void);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat4 hglm_mat4_make_identity(void);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat4 hglm_mat4_make_scale(HglmVec3 v);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat4 hglm_mat4_make_rotation(float angle, HglmVec3 axis);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat4 hglm_mat4_make_translation(HglmVec3 v);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat4 hglm_mat4_make_ortho(float left, float right, float bottom, float top,  float near,  float far);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat4 hglm_mat4_make_perspective(float fov, float aspect, float znear, float zfar);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat4 hglm_mat4_look_at(HglmVec3 camera, HglmVec3 target, HglmVec3 up);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat4 hglm_mat4_look_to(HglmVec3 camera, HglmVec3 dir, HglmVec3 up);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat4 hglm_mat4_add(HglmMat4 a, HglmMat4 b);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat4 hglm_mat4_sub(HglmMat4 a, HglmMat4 b);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat4 hglm_mat4_transpose(HglmMat4 m);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat4 hglm_mat4_mul_scalar(HglmMat4 m, float s);
__attribute__ ((const, unused)) static HGL_INLINE HglmVec4 hglm_mat4_mul_vec4(HglmMat4 m, HglmVec4 v);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat4 hglm_mat4_mul_mat4(HglmMat4 a, HglmMat4 b);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat4 hglm_mat4_scale(HglmMat4 m, HglmVec3 v);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat4 hglm_mat4_rotate(HglmMat4 m, float angle, HglmVec3 axis);
__attribute__ ((const, unused)) static HGL_INLINE HglmMat4 hglm_mat4_translate(HglmMat4 m, HglmVec3 v);
__attribute__ ((const, unused)) static HGL_INLINE HglmVec4 hglm_mat4_perspective_project(HglmMat4 proj, HglmVec4 v);

static HGL_INLINE HglmMat hglm_mat_make(uint32_t M /* rows */, uint32_t N /* cols */);
static HGL_INLINE HglmMat hglm_mat_make_identity(uint32_t N);
static HGL_INLINE void hglm_mat_free(HglmMat m);
static HGL_INLINE void hglm_mat_fill(HglmMat m, float value);
static HGL_INLINE void hglm_mat_add(HglmMat res, HglmMat a, HglmMat b);
static HGL_INLINE void hglm_mat_sub(HglmMat res, HglmMat a, HglmMat b);
static HGL_INLINE void hglm_mat_mul_scalar(HglmMat m, float s);
static HGL_INLINE void hglm_mat_mul_mat(HglmMat res, HglmMat a, HglmMat b);
static HGL_INLINE void hglm_mat_transpose_in_place(HglmMat m);
static HGL_INLINE void hglm_mat_transpose(HglmMat res, HglmMat m);

static HGL_INLINE float hglm_pid(float error, float last_error, float *i, 
                                 float Kp, float Ki, float Kd, float dt);
static HGL_INLINE float hglm_lerp(float a, float b, float t);
static HGL_INLINE float hglm_ilerp(float a, float b, float value);
static HGL_INLINE float hglm_clamp(float min, float max, float value);
static HGL_INLINE float hglm_remap(float in_min, float in_max, float out_min, float out_max, float value);
static HGL_INLINE float hglm_smoothstep(float t);
static HGL_INLINE float hglm_sinstep(float t);
static HGL_INLINE float hglm_lerpsmooth(float a, float b, float dt, float omega);
static HGL_INLINE float hglm_smoothmin_quadratic(float a, float b, float k);
static HGL_INLINE float hglm_smoothmin_sigmoid(float a, float b, float k);
static HGL_INLINE HglmVec4 hglm_bezier3(float t);
static HGL_INLINE HglmVec4 hglm_hermite3(float t);
static HGL_INLINE float hglm_perlin3D(float x, float y, float z);

/* ========== HglmIVec2 ======================================================*/

#define hglm_ivec2_print(v) (printf("%s = {%d, %d}\n", #v , (v).x, (v).y))

static HGL_INLINE HglmIVec2 hglm_ivec2_make(int x, int y)
{
    return (HglmIVec2) {.x = x, .y = y};
}

static HGL_INLINE HglmIVec2 hglm_ivec2_add(HglmIVec2 a, HglmIVec2 b)
{
    return (HglmIVec2) {.x = a.x + b.x, .y = a.y + b.y};
}

static HGL_INLINE HglmIVec2 hglm_ivec2_sub(HglmIVec2 a, HglmIVec2 b)
{
    return (HglmIVec2) {.x = a.x - b.x, .y = a.y - b.y};
}

static HGL_INLINE float hglm_ivec2_distance(HglmIVec2 a, HglmIVec2 b)
{
    int dx = b.x - a.x;
    int dy = b.y - a.y;
    return (int) sqrtf(dx*dx + dy*dy);
}

static HGL_INLINE float hglm_ivec2_len(HglmIVec2 v)
{
    return (int) sqrtf(v.x * v.x + v.y * v.y);
}

static HGL_INLINE HglmIVec2 hglm_ivec2_mul_scalar(HglmIVec2 v, float s)
{
    return (HglmIVec2) {.x = s * v.x, .y = s * v.y};
}

static HGL_INLINE HglmIVec2 hglm_ivec2_lerp(HglmIVec2 a, HglmIVec2 b, float amount)
{
    return hglm_ivec2_make(
        (int)hglm_lerp(a.x, b.x, amount),
        (int)hglm_lerp(a.y, b.y, amount)
    );
}

/* ========== HglmVec2 =======================================================*/

#define hglm_vec2_print(v) (printf("%s = {%f, %f}\n", #v , (v).x, (v).y))

static HGL_INLINE HglmVec2 hglm_vec2_make(float x, float y)
{
    return (HglmVec2){.x = x, .y = y};
}

static HGL_INLINE HglmVec2 hglm_vec2_from_polar(float r, float phi)
{
    return (HglmVec2) {
        .x = r * cosf(phi),
        .y = r * sinf(phi),
    };
}

static HGL_INLINE HglmVec2 hglm_vec2_add(HglmVec2 a, HglmVec2 b)
{
    return (HglmVec2){.x = a.x + b.x, .y = a.y + b.y};
}

static HGL_INLINE HglmVec2 hglm_vec2_sub(HglmVec2 a, HglmVec2 b)
{
    return (HglmVec2){.x = a.x - b.x, .y = a.y - b.y};
}

static HGL_INLINE float hglm_vec2_distance(HglmVec2 a, HglmVec2 b)
{
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    return sqrtf(dx*dx + dy*dy);
}

static HGL_INLINE float hglm_vec2_len(HglmVec2 v)
{
    return sqrtf(v.x * v.x + v.y * v.y);
}

static HGL_INLINE HglmVec2 hglm_vec2_normalize(HglmVec2 v)
{
    float ilen = 1.0f / hglm_vec2_len(v);
    return (HglmVec2) {.x = v.x * ilen, .y = v.y * ilen};
}

static HGL_INLINE float hglm_vec2_dot(HglmVec2 a, HglmVec2 b)
{
    return a.x * b.x + a.y *b.y;
}

static HGL_INLINE HglmVec2 hglm_vec2_hadamard(HglmVec2 a, HglmVec2 b)
{
    return (HglmVec2) {.x = a.x * b.x, .y = a.y * b.y};
}

static HGL_INLINE HglmVec2 hglm_vec2_mul_scalar(HglmVec2 v, float s)
{
    return (HglmVec2) {.x = s * v.x, .y = s * v.y};
}

static HGL_INLINE HglmVec2 hglm_vec2_reflect(HglmVec2 v, HglmVec2 normal)
{
    return hglm_vec2_sub(v, hglm_vec2_mul_scalar(normal, 2*hglm_vec2_dot(v, normal)));
}

static HGL_INLINE HglmVec2 hglm_vec2_lerp(HglmVec2 a, HglmVec2 b, float t)
{
    return hglm_vec2_add(
        hglm_vec2_mul_scalar(a, 1.0f - t),
        hglm_vec2_mul_scalar(b, t)
    );
}

static HGL_INLINE HglmVec2 hglm_vec2_slerp(HglmVec2 a, HglmVec2 b, float t)
{
    float omega = acosf(hglm_vec2_dot(a, b));
    return hglm_vec2_add(
        hglm_vec2_mul_scalar(a, sinf((1.0f - t)*omega)/sinf(omega)),
        hglm_vec2_mul_scalar(b, sinf(t*omega)/sinf(omega))
    );
}

static HGL_INLINE HglmVec2 hglm_vec2_bezier3(HglmVec2 v0, HglmVec2 v1, HglmVec2 v2, HglmVec2 v3, float t)
{
    HglmVec4 bezier3 = hglm_bezier3(t);
    return hglm_vec2_add(
        hglm_vec2_add(
            hglm_vec2_mul_scalar(v0, bezier3.x),
            hglm_vec2_mul_scalar(v1, bezier3.y)
        ),
        hglm_vec2_add(
            hglm_vec2_mul_scalar(v2, bezier3.z),
            hglm_vec2_mul_scalar(v3, bezier3.w)
        )
    );
}


/* ========== HglmVec3 =======================================================*/

#define hglm_vec3_print(v) (printf("%s = {%f, %f, %f}\n", #v , (v).x, (v).y, (v).z))

static HGL_INLINE HglmVec3 hglm_vec3_make(float x, float y, float z)
{
    return (HglmVec3){.x = x, .y = y, .z = z};
}

static HGL_INLINE HglmVec3 hglm_vec3_from_spherical(float r, float phi, float theta)
{
    return (HglmVec3) {
        .x = r * cosf(theta) * sinf(phi), 
        .y = r * sinf(theta) * sinf(phi), 
        .z = r * cosf(phi),
    };
}

static HGL_INLINE HglmVec3 hglm_vec3_add(HglmVec3 a, HglmVec3 b)
{
    return (HglmVec3){.x = a.x + b.x, .y = a.y + b.y, .z = a.z + b.z};
}

static HGL_INLINE HglmVec3 hglm_vec3_sub(HglmVec3 a, HglmVec3 b)
{
    return (HglmVec3){.x = a.x - b.x, .y = a.y - b.y, .z = a.z - b.z};
}

static HGL_INLINE float hglm_vec3_distance(HglmVec3 a, HglmVec3 b)
{
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float dz = b.z - a.z;
    return sqrtf(dx*dx + dy*dy + dz*dz);
}

static HGL_INLINE float hglm_vec3_len(HglmVec3 v)
{
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

static HGL_INLINE HglmVec3 hglm_vec3_normalize(HglmVec3 v)
{
    float ilen = 1.0f / hglm_vec3_len(v);
    return (HglmVec3) {.x = v.x * ilen, .y = v.y * ilen, .z = v.z * ilen};
}

static HGL_INLINE float hglm_vec3_dot(HglmVec3 a, HglmVec3 b)
{
    return a.x * b.x + a.y *b.y + a.z * b.z;
}

static HGL_INLINE HglmVec3 hglm_vec3_cross(HglmVec3 a, HglmVec3 b)
{
    return (HglmVec3) {
        .x = (a.y * b.z) - (a.z * b.y),
        .y = (a.z * b.x) - (a.x * b.z),
        .z = (a.x * b.y) - (a.y * b.x)
    };
}

static HGL_INLINE HglmVec3 hglm_vec3_hadamard(HglmVec3 a, HglmVec3 b)
{
    return (HglmVec3) {.x = a.x * b.x, .y = a.y * b.y, .z = a.z * b.z};
}

static HGL_INLINE HglmVec3 hglm_vec3_mul_scalar(HglmVec3 v, float s)
{
    return (HglmVec3) {.x = s * v.x, .y = s * v.y, .z = s * v.z};
}

static HGL_INLINE HglmVec3 hglm_vec3_reflect(HglmVec3 v, HglmVec3 normal)
{
    return hglm_vec3_sub(v, hglm_vec3_mul_scalar(normal, 2*hglm_vec3_dot(v, normal)));
}

static HGL_INLINE HglmVec3 hglm_vec3_lerp(HglmVec3 a, HglmVec3 b, float t)
{
    return hglm_vec3_add(
        hglm_vec3_mul_scalar(a, 1.0f - t),
        hglm_vec3_mul_scalar(b, t)
    );
}

static HGL_INLINE HglmVec3 hglm_vec3_slerp(HglmVec3 a, HglmVec3 b, float t)
{
    float omega = acosf(hglm_vec3_dot(a, b));
    return hglm_vec3_add(
        hglm_vec3_mul_scalar(a, sinf((1.0f - t)*omega)/sinf(omega)),
        hglm_vec3_mul_scalar(b, sinf(t*omega)/sinf(omega))
    );
}

static HGL_INLINE HglmVec3 hglm_vec3_bezier3(HglmVec3 v0, HglmVec3 v1, HglmVec3 v2, HglmVec3 v3, float t)
{
    HglmVec4 bezier3 = hglm_bezier3(t);
    return hglm_vec3_add(
        hglm_vec3_add(
            hglm_vec3_mul_scalar(v0, bezier3.x),
            hglm_vec3_mul_scalar(v1, bezier3.y)
        ),
        hglm_vec3_add(
            hglm_vec3_mul_scalar(v2, bezier3.z),
            hglm_vec3_mul_scalar(v3, bezier3.w)
        )
    );
}


/* ========== HglmVec4 =======================================================*/

#define hglm_vec4_print(v) (printf("%s = {%f, %f, %f, %f}\n", #v , (v).x, (v).y, (v).z, (v).w))

static HGL_INLINE HglmVec4 hglm_vec4_make(float x, float y, float z, float w)
{
#ifdef HGLM_USE_SIMD
    return (HglmVec4){.v = _mm_set_ps(w, z, y, x)};
#else
    return (HglmVec4){.x = x, .y = y, .z = z, .w = w};
#endif
}

static HGL_INLINE HglmVec4 hglm_vec4_add(HglmVec4 a, HglmVec4 b)
{
#ifdef HGLM_USE_SIMD
    return (HglmVec4){.v = _mm_add_ps(a.v, b.v)};
#else
    return (HglmVec4){.x = a.x + b.x, .y = a.y + b.y, .z = a.z + b.z, .w = a.w + b.w};
#endif
}

static HGL_INLINE HglmVec4 hglm_vec4_sub(HglmVec4 a, HglmVec4 b)
{
#ifdef HGLM_USE_SIMD
    return (HglmVec4){.v = _mm_sub_ps(a.v, b.v)};
#else
    return (HglmVec4){.x = a.x - b.x, .y = a.y - b.y, .z = a.z - b.z, .w = a.w - b.w};
#endif
}

static HGL_INLINE float hglm_vec4_distance(HglmVec4 a, HglmVec4 b)
{
#ifdef HGLM_USE_SIMD
    __m128 d = _mm_sub_ps(b.v, a.v);
    d = _mm_mul_ps(d, d);
    d = _mm_hadd_ps(d,d);
    d = _mm_hadd_ps(d,d);
    return sqrtf(_mm_cvtss_f32(d));

    /* SSE1 */
    //__m128 shuf   = _mm_shuffle_ps(d, d, _MM_SHUFFLE(2, 3, 0, 1));  // [ C D | A B ]
    //__m128 sums   = _mm_add_ps(d, shuf);      // sums = [ D+C C+D | B+A A+B ]
    //shuf          = _mm_movehl_ps(shuf, sums);      //  [   C   D | D+C C+D ]  // let the compiler avoid a mov by reusing shuf
    //sums          = _mm_add_ss(sums, shuf);
    //return sqrtf(_mm_cvtss_f32(sums));
#else
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float dz = b.z - a.z;
    float dw = b.w - a.w;
    return sqrtf(dx*dx + dy*dy + dz*dz + dw*dw);
#endif
}

static HGL_INLINE float hglm_vec4_len(HglmVec4 v)
{
#ifdef HGLM_USE_SIMD
    __m128 d = _mm_mul_ps(v.v, v.v);
    d = _mm_hadd_ps(d,d);
    d = _mm_hadd_ps(d,d);
    return sqrtf(_mm_cvtss_f32(d));
#else
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
#endif
}

static HGL_INLINE HglmVec4 hglm_vec4_normalize(HglmVec4 v)
{
#ifdef HGLM_USE_SIMD
    float rlen = 1.0f / hglm_vec4_len(v);
    __m128 vrlen = _mm_broadcast_ss(&rlen);
    return (HglmVec4) {.v = _mm_mul_ps(v.v, vrlen)};
#else
    float len = hglm_vec4_len(v);
    return (HglmVec4) {.x = v.x / len, .y = v.y / len, .z = v.z / len, .w = v.w / len};
#endif
}

static HGL_INLINE float hglm_vec4_dot(HglmVec4 a, HglmVec4 b)
{
#ifdef HGLM_USE_SIMD
    __m128 v = _mm_mul_ps(a.v, b.v);
    v = _mm_hadd_ps(v,v);
    v = _mm_hadd_ps(v,v);
    return _mm_cvtss_f32(v);
#else
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
#endif
}

static HGL_INLINE HglmVec4 hglm_vec4_hadamard(HglmVec4 a, HglmVec4 b)
{
#ifdef HGLM_USE_SIMD
    return (HglmVec4) {.v = _mm_mul_ps(a.v, b.v)};
#else
    return (HglmVec4) {.x = a.x * b.x, .y = a.y * b.y, .z = a.z * b.z, .w = a.w * b.w};
#endif
}

static HGL_INLINE HglmVec4 hglm_vec4_mul_scalar(HglmVec4 v, float s)
{
#ifdef HGLM_USE_SIMD
    return (HglmVec4) {.v = _mm_mul_ps(v.v, _mm_broadcast_ss(&s))};
#else
    return (HglmVec4) {.x = s * v.x, .y = s * v.y, .z = s * v.z, .w = s * v.w};
#endif
}

static HGL_INLINE HglmVec4 hglm_vec4_swizzle(HglmVec4 v, int a, int b, int c, int d)
{
#ifdef HGLM_USE_SIMD
    return (HglmVec4) {.v = _mm_shuffle_ps(v.v, v.v, _MM_SHUFFLE(d, c, b, a))};
#else
    return (HglmVec4) {.x = v.f[a], .y = v.f[b], .z = v.f[c], .w = v.f[d]};
#endif
}

static HGL_INLINE HglmVec4 hglm_vec4_perspective_divide(HglmVec4 v)
{
    HglmVec4 u;
    u.x = v.x / v.w;
    u.y = v.y / v.w;
    u.z = v.z / v.w;
    u.w = 1.0f;
    return u;
}

static HGL_INLINE HglmVec4 hglm_vec4_lerp(HglmVec4 a, HglmVec4 b, float t)
{
    return hglm_vec4_add(
        hglm_vec4_mul_scalar(a, 1.0f - t),
        hglm_vec4_mul_scalar(b, t)
    );
}

static HGL_INLINE HglmVec4 hglm_vec4_bezier3(HglmVec4 v0, HglmVec4 v1, HglmVec4 v2, HglmVec4 v3, float t)
{
    HglmVec4 bezier3 = hglm_bezier3(t);
    return hglm_vec4_add(
        hglm_vec4_add(
            hglm_vec4_mul_scalar(v0, bezier3.x),
            hglm_vec4_mul_scalar(v1, bezier3.y)
        ),
        hglm_vec4_add(
            hglm_vec4_mul_scalar(v2, bezier3.z),
            hglm_vec4_mul_scalar(v3, bezier3.w)
        )
    );
}


/* ========== HglmMat3 =======================================================*/

#define hglm_mat3_print(m)                                           \
(                                                                    \
    printf("%s = \n"                                                 \
           "    |%14.5f %14.5f %14.5f |\n"                           \
           "    |%14.5f %14.5f %14.5f |\n"                           \
           "    |%14.5f %14.5f %14.5f |\n", #m ,                     \
            (double) (m).c0.x, (double) (m).c1.x, (double) (m).c2.x, \
            (double) (m).c0.y, (double) (m).c1.y, (double) (m).c2.y, \
            (double) (m).c0.z, (double) (m).c1.z, (double) (m).c2.z) \
)

__attribute__ ((const, unused)) static HGL_INLINE HglmMat3 hglm_mat3_make(HglmVec3 c0, HglmVec3 c1, HglmVec3 c2)
{
    return (HglmMat3){.c0 = c0, .c1 = c1, .c2 = c2};
}

__attribute__ ((const, unused)) static HGL_INLINE HglmMat3 hglm_mat3_make_identity(void)
{
    return HGLM_MAT3_IDENTITY;
}

__attribute__ ((const, unused)) static HGL_INLINE HglmMat3 hglm_mat3_make_from_mat4(HglmMat4 mat4)
{
    return (HglmMat3){.c0 = mat4.c0.xyz, .c1 = mat4.c1.xyz, .c2 = mat4.c2.xyz};
}

__attribute__ ((const, unused)) static HGL_INLINE HglmMat3 hglm_mat3_transpose(HglmMat3 m)
{
    return (HglmMat3) {
        .c0 = {.x = m.c0.x, .y = m.c1.x, .z = m.c2.x},
        .c1 = {.x = m.c0.y, .y = m.c1.y, .z = m.c2.y},
        .c2 = {.x = m.c0.z, .y = m.c1.z, .z = m.c2.z},
    };
}

__attribute__ ((const, unused)) static HGL_INLINE HglmVec3 hglm_mat3_mul_vec3(HglmMat3 m, HglmVec3 v)
{
    return (HglmVec3) {
        .x = m.c0.x * v.x + m.c1.x * v.y + m.c2.x * v.z,
        .y = m.c0.y * v.x + m.c1.y * v.y + m.c2.y * v.z,
        .z = m.c0.z * v.x + m.c1.z * v.y + m.c2.z * v.z,
    };
}


/* ========== HglmMat4 =======================================================*/

#define hglm_mat4_print(m)                                                              \
(                                                                                       \
    printf("%s = \n"                                                                    \
           "    |%14.5f %14.5f %14.5f %14.5f |\n"                                       \
           "    |%14.5f %14.5f %14.5f %14.5f |\n"                                       \
           "    |%14.5f %14.5f %14.5f %14.5f |\n"                                       \
           "    |%14.5f %14.5f %14.5f %14.5f |\n", #m ,                                 \
            (double) (m).c0.x, (double) (m).c1.x, (double) (m).c2.x, (double) (m).c3.x, \
            (double) (m).c0.y, (double) (m).c1.y, (double) (m).c2.y, (double) (m).c3.y, \
            (double) (m).c0.z, (double) (m).c1.z, (double) (m).c2.z, (double) (m).c3.z, \
            (double) (m).c0.w, (double) (m).c1.w, (double) (m).c2.w, (double) (m).c3.w) \
)

__attribute__ ((const, unused))
static HGL_INLINE HglmMat4 hglm_mat4_make(HglmVec4 c0,
                                          HglmVec4 c1,
                                          HglmVec4 c2,
                                          HglmVec4 c3)
{
    return (HglmMat4){.c0 = c0, .c1 = c1, .c2 = c2, .c3 = c3};
}

__attribute__ ((const, unused))
static HGL_INLINE HglmMat4 hglm_mat4_make_zero()
{
    return (HglmMat4){0};
}

__attribute__ ((const, unused))
static HGL_INLINE HglmMat4 hglm_mat4_make_identity(void)
{
    return HGLM_MAT4_IDENTITY;
}

__attribute__ ((const, unused))
static HGL_INLINE HglmMat4 hglm_mat4_make_scale(HglmVec3 v)
{
    HglmMat4 s = HGLM_MAT4_IDENTITY;
    s.c0.x = v.x;
    s.c1.y = v.y;
    s.c2.z = v.z;
    return s;
}

__attribute__ ((const, unused))
static HGL_INLINE HglmMat4 hglm_mat4_make_rotation(float angle, HglmVec3 axis)
{
    float O = angle;
    float ux = axis.x;
    float uy = axis.y;
    float uz = axis.z;
    float c0x = cosf(O) + ux*ux * (1 - cosf(O));
    float c1x = ux*uy * (1 - cosf(O)) - uz * sinf(O);
    float c2x = ux*uz * (1 - cosf(O)) + uy * sinf(O);
    float c0y = uy*ux * (1 - cosf(O)) + uz * sinf(O);
    float c1y = cosf(O) + uy*uy * (1 - cosf(O));
    float c2y = uy*uz * (1 - cosf(O)) - ux * sinf(O);
    float c0z = uz*ux * (1 - cosf(O)) - uy * sinf(O);
    float c1z = uz*uy * (1 - cosf(O)) + ux * sinf(O);
    float c2z = cosf(O) + uz*uz * (1 - cosf(O));
    return (HglmMat4) {
        .c0 = {.x =  c0x, .y =  c0y, .z =  c0z, .w = 0.0f},
        .c1 = {.x =  c1x, .y =  c1y, .z =  c1z, .w = 0.0f},
        .c2 = {.x =  c2x, .y =  c2y, .z =  c2z, .w = 0.0f},
        .c3 = {.x = 0.0f, .y = 0.0f, .z = 0.0f, .w = 1.0f},
    };
}

__attribute__ ((const, unused))
static HGL_INLINE HglmMat4 hglm_mat4_make_translation(HglmVec3 v)
{
    HglmMat4 t = HGLM_MAT4_IDENTITY;
    t.c3.x = v.x;
    t.c3.y = v.y;
    t.c3.z = v.z;
    return t;
}

__attribute__ ((const, unused))
static HGL_INLINE HglmMat4 hglm_mat4_make_ortho(float left, float right, float bottom,
                                                float top,  float near,  float far)
{
#if 1
    HglmMat4 m = HGLM_MAT4_IDENTITY;
    m.c0.x = 2 / (right - left);
    m.c1.y = 2 / (top - bottom);
    m.c2.z = -1 / (far - near);
    m.c3.x = -((left + right) / (right - left));
    m.c3.y = -((bottom + top) / (top - bottom));
    m.c3.z = -((near)   / (far - near));
#else
    HglmMat4 m = HGLM_MAT4_IDENTITY;
    m.c0.x =  2 / (right - left);
    m.c1.y =  2 / (top - bottom);
    m.c2.z = -2 / (far - near); // Note: inversion
    m.c3.x = -((right + left) / (right - left));
    m.c3.y = -((top + bottom) / (top - bottom));
    m.c3.z = -((far + near)   / (far - near));
#endif
    return m;
}

__attribute__ ((const, unused))
static HGL_INLINE HglmMat4 hglm_mat4_make_perspective(float fov, float aspect, float znear, float zfar)
{
#if 1
    /* x: [-1,1], y: [-1,1], z = [0, 1] */
    float a = 1.0f / aspect;
    float f = 1.0f / tanf(fov/2);
    float d0 = -(zfar + znear) / (zfar - znear);
    float d1 = -(2 * zfar * znear) / (zfar - znear);
    return (HglmMat4) {
        .m00 =   a*f, .m01 =  0.0f, .m02 =  0.0f, .m03 =  0.0f,
        .m10 =  0.0f, .m11 =     f, .m12 =  0.0f, .m13 =  0.0f,
        .m20 =  0.0f, .m21 =  0.0f, .m22 =    d0, .m23 =    d1,
        .m30 =  0.0f, .m31 =  0.0f, .m32 = -1.0f, .m33 =  0.0f,
    };
#else
    /* x: [-1,1], y: [-1,1], z = [-1, 1] */
    float a = 1.0f / aspect;
    float f = 1.0f / tanf(fov/2);
    float d0 = (-znear - zfar) / (znear - zfar);
    float d1 = 2*(-(2 * zfar * znear) / (znear - zfar)) - 1;
    return (HglmMat4) {
        .m00 =   a*f, .m01 =  0.0f, .m02 =  0.0f, .m03 =  0.0f,
        .m10 =  0.0f, .m11 =     f, .m12 =  0.0f, .m13 =  0.0f,
        .m20 =  0.0f, .m21 =  0.0f, .m22 =    d0, .m23 =    d1,
        .m30 =  0.0f, .m31 =  0.0f, .m32 =  1.0f, .m33 =  0.0f,
    };
#endif
}

__attribute__ ((const, unused)) static HGL_INLINE HglmMat4 hglm_mat4_look_at(HglmVec3 camera, HglmVec3 target, HglmVec3 up)
{
    HglmVec3 f = hglm_vec3_normalize(hglm_vec3_sub(target, camera));
    HglmVec3 u = hglm_vec3_normalize(up);
    HglmVec3 s = hglm_vec3_normalize(hglm_vec3_cross(f, u));
    u = hglm_vec3_cross(s, f);
    HglmMat4 m = HGLM_MAT4_IDENTITY;
    m.c0.x =  s.x;
    m.c1.x =  s.y;
    m.c2.x =  s.z;
    m.c0.y =  u.x;
    m.c1.y =  u.y;
    m.c2.y =  u.z;
    m.c0.z = -f.x;
    m.c1.z = -f.y;
    m.c2.z = -f.z;
    m.c3.x = -hglm_vec3_dot(s, camera);
    m.c3.y = -hglm_vec3_dot(u, camera);
    m.c3.z = hglm_vec3_dot(f, camera); // this is a little odd..
    return m;
}

__attribute__ ((const, unused)) static HGL_INLINE HglmMat4 hglm_mat4_look_to(HglmVec3 camera, HglmVec3 dir, HglmVec3 up)
{
    HglmVec3 f = dir;
    HglmVec3 u = hglm_vec3_normalize(up);
    HglmVec3 s = hglm_vec3_normalize(hglm_vec3_cross(f, u));
    u = hglm_vec3_cross(s, f);
    HglmMat4 m = HGLM_MAT4_IDENTITY;
    m.c0.x =  s.x;
    m.c1.x =  s.y;
    m.c2.x =  s.z;
    m.c0.y =  u.x;
    m.c1.y =  u.y;
    m.c2.y =  u.z;
    m.c0.z = -f.x;
    m.c1.z = -f.y;
    m.c2.z = -f.z;
    m.c3.x = -hglm_vec3_dot(s, camera);
    m.c3.y = -hglm_vec3_dot(u, camera);
    m.c3.z = hglm_vec3_dot(f, camera); // this is a little odd..
    return m;
}


__attribute__ ((const, unused))
static HGL_INLINE HglmMat4 hglm_mat4_add(HglmMat4 a, HglmMat4 b)
{
#ifdef HGLM_USE_SIMD
    HglmMat4 m;
    m.c0.v = _mm_add_ps(a.c0.v, b.c0.v);
    m.c1.v = _mm_add_ps(a.c1.v, b.c1.v);
    m.c2.v = _mm_add_ps(a.c2.v, b.c2.v);
    m.c3.v = _mm_add_ps(a.c3.v, b.c3.v);
    return m;
#else
    return (HglmMat4) {
        .c0 = hglm_vec4_add(a.c0, b.c0),
        .c1 = hglm_vec4_add(a.c1, b.c1),
        .c2 = hglm_vec4_add(a.c2, b.c2),
        .c3 = hglm_vec4_add(a.c3, b.c3)
    };
#endif
}

__attribute__ ((const, unused))
static HGL_INLINE HglmMat4 hglm_mat4_sub(HglmMat4 a, HglmMat4 b)
{
#ifdef HGLM_USE_SIMD
    HglmMat4 m;
    m.c0.v = _mm_sub_ps(a.c0.v, b.c0.v);
    m.c1.v = _mm_sub_ps(a.c1.v, b.c1.v);
    m.c2.v = _mm_sub_ps(a.c2.v, b.c2.v);
    m.c3.v = _mm_sub_ps(a.c3.v, b.c3.v);
    return m;
#else
    return (HglmMat4) {
        .c0 = hglm_vec4_sub(a.c0, b.c0),
        .c1 = hglm_vec4_sub(a.c1, b.c1),
        .c2 = hglm_vec4_sub(a.c2, b.c2),
        .c3 = hglm_vec4_sub(a.c3, b.c3)
    };
#endif
}

__attribute__ ((const, unused))
static HGL_INLINE HglmMat4 hglm_mat4_transpose(HglmMat4 m)
{
#ifdef HGLM_USE_SIMD
    _MM_TRANSPOSE4_PS(m.c0.v, m.c1.v, m.c2.v, m.c3.v);
    return m;
#else
    return (HglmMat4) {
        .c0 = {.x = m.c0.x, .y = m.c1.x, .z = m.c2.x, .w = m.c3.x},
        .c1 = {.x = m.c0.y, .y = m.c1.y, .z = m.c2.y, .w = m.c3.y},
        .c2 = {.x = m.c0.z, .y = m.c1.z, .z = m.c2.z, .w = m.c3.z},
        .c3 = {.x = m.c0.w, .y = m.c1.w, .z = m.c2.w, .w = m.c3.w},
    };
#endif
}

__attribute__ ((const, unused))
static HGL_INLINE HglmMat4 hglm_mat4_mul_scalar(HglmMat4 m, float s)
{
#ifdef HGLM_USE_SIMD
    __m128 vec_s = _mm_broadcast_ss(&s);
    return (HglmMat4) {
        .c0 = {.v = _mm_mul_ps(vec_s, m.c0.v)},
        .c1 = {.v = _mm_mul_ps(vec_s, m.c1.v)},
        .c2 = {.v = _mm_mul_ps(vec_s, m.c2.v)},
        .c3 = {.v = _mm_mul_ps(vec_s, m.c3.v)}
    };
#else
    return (HglmMat4) {
        .c0 = {.x = s * m.c0.x, .y = s * m.c0.y, .z = s * m.c0.z, .w = s * m.c0.w}, // c0
        .c1 = {.x = s * m.c1.x, .y = s * m.c1.y, .z = s * m.c1.z, .w = s * m.c1.w}, // c1
        .c2 = {.x = s * m.c2.x, .y = s * m.c2.y, .z = s * m.c2.z, .w = s * m.c2.w}, // c2
        .c3 = {.x = s * m.c3.x, .y = s * m.c3.y, .z = s * m.c3.z, .w = s * m.c3.w}  // c3
    };
#endif
}

__attribute__ ((const, unused))
static HGL_INLINE HglmVec4 hglm_mat4_mul_vec4(HglmMat4 m, HglmVec4 v)
{
#ifdef HGLM_USE_SIMD
    //(void) m;
    //(void) v;
    //return hglm_vec4_make(0,0,0,0);

    //__m128 vec_s = _mm_set_ps1(s);



    HglmVec4 res;
    __m128 t0 = _mm_mul_ps(_mm_broadcast_ss(&v.x), m.c0.v);
    __m128 t1 = _mm_mul_ps(_mm_broadcast_ss(&v.y), m.c1.v);
    __m128 t2 = _mm_mul_ps(_mm_broadcast_ss(&v.z), m.c2.v);
    __m128 t3 = _mm_mul_ps(_mm_broadcast_ss(&v.w), m.c3.v);
    res.v = _mm_add_ps(_mm_add_ps(t0, t1),
                       _mm_add_ps(t2, t3));
    return res;

    // hmmmm
    //__m128 r = _mm_mul_ps(_mm_set1_ps(v.x), m.c0.v);
    //r = _mm_fmadd_ps(_mm_set1_ps(v.y), m.c1.v, r);
    //r = _mm_fmadd_ps(_mm_set1_ps(v.z), m.c2.v, r);
    //r = _mm_fmadd_ps(_mm_set1_ps(v.w), m.c3.v, r);
    //return (HglmVec4) {.v = r};

#else
    return (HglmVec4) {
        .x = m.c0.x * v.x + m.c1.x * v.y + m.c2.x * v.z + m.c3.x * v.w,
        .y = m.c0.y * v.x + m.c1.y * v.y + m.c2.y * v.z + m.c3.y * v.w,
        .z = m.c0.z * v.x + m.c1.z * v.y + m.c2.z * v.z + m.c3.z * v.w,
        .w = m.c0.w * v.x + m.c1.w * v.y + m.c2.w * v.z + m.c3.w * v.w,
    };
#endif
}

__attribute__ ((const, unused))
static HGL_INLINE HglmMat4 hglm_mat4_mul_mat4(HglmMat4 a, HglmMat4 b)
{
#ifdef HGLM_USE_SIMD
    //HglmMat4 res;
    //__m128 row[4], sum[4];
    //for (int i = 0; i < 4; i++) row[i] = _mm_load_ps(&a.f[4*i]);
    //for (int i = 0; i < 4; i++) {
    //    sum[i] = _mm_setzero_ps();
    //    for (int j = 0; j < 4; j++) {
    //        sum[i] = _mm_add_ps(_mm_mul_ps(_mm_set1_ps(b.f[i*4 + j]), row[j]), sum[i]);
    //    }
    //}
    //_mm_store_ps((float *)&res.c0.v, sum[0]);
    //_mm_store_ps((float *)&res.c1.v, sum[1]);
    //_mm_store_ps((float *)&res.c2.v, sum[2]);
    //_mm_store_ps((float *)&res.c3.v, sum[3]);
    //return res;


    //HglmMat4 m;
    //__m256 t0, t1, t2;

    //__m256 c01 = _mm256_load_ps((float *)&a.c0);
    //__m256 c23 = _mm256_load_ps((float *)&a.c2);

    //t0     = _mm256_mul_ps(c01, _mm256_set_m128(_mm_broadcast_ss(&b.c0.y), _mm_broadcast_ss(&b.c0.x)));
    //t1     = _mm256_mul_ps(c23, _mm256_set_m128(_mm_broadcast_ss(&b.c0.w), _mm_broadcast_ss(&b.c0.z)));
    //t2     = _mm256_add_ps(t0, t1);
    //m.c0.v = _mm_add_ps(_mm256_castps256_ps128(t2), _mm256_extractf128_ps(t2, 1));

    //t0     = _mm256_mul_ps(c01, _mm256_set_m128(_mm_broadcast_ss(&b.c1.y), _mm_broadcast_ss(&b.c1.x)));
    //t1     = _mm256_mul_ps(c23, _mm256_set_m128(_mm_broadcast_ss(&b.c1.w), _mm_broadcast_ss(&b.c1.z)));
    //t2     = _mm256_add_ps(t0, t1);
    //m.c1.v = _mm_add_ps(_mm256_castps256_ps128(t2), _mm256_extractf128_ps(t2, 1));

    //t0     = _mm256_mul_ps(c01, _mm256_set_m128(_mm_broadcast_ss(&b.c2.y), _mm_broadcast_ss(&b.c2.x)));
    //t1     = _mm256_mul_ps(c23, _mm256_set_m128(_mm_broadcast_ss(&b.c2.w), _mm_broadcast_ss(&b.c2.z)));
    //t2     = _mm256_add_ps(t0, t1);
    //m.c2.v = _mm_add_ps(_mm256_castps256_ps128(t2), _mm256_extractf128_ps(t2, 1));

    //t0     = _mm256_mul_ps(c01, _mm256_set_m128(_mm_broadcast_ss(&b.c3.y), _mm_broadcast_ss(&b.c3.x)));
    //t1     = _mm256_mul_ps(c23, _mm256_set_m128(_mm_broadcast_ss(&b.c3.w), _mm_broadcast_ss(&b.c3.z)));
    //t2     = _mm256_add_ps(t0, t1);
    //m.c3.v = _mm_add_ps(_mm256_castps256_ps128(t2), _mm256_extractf128_ps(t2, 1));

    //return m;

    //HglmMat4 m;
    //__m256 t0, t1, t2;

    //__m256 c01 = _mm256_load_ps((float *)&a.c0);
    //__m256 c23 = _mm256_load_ps((float *)&a.c2);

    //__m256 n01;
    //n01 = _mm256_mul_ps();

    //return m;

    HglmMat4 m;

    __m128 t0, t1, t2, t3;
    __m128 c0, c1, c2, c3;

    /* c0 */
    t0 = _mm_mul_ps(a.c0.v, _mm_broadcast_ss(&b.c0.x));
    t1 = _mm_mul_ps(a.c1.v, _mm_broadcast_ss(&b.c0.y));
    t2 = _mm_mul_ps(a.c2.v, _mm_broadcast_ss(&b.c0.z));
    t3 = _mm_mul_ps(a.c3.v, _mm_broadcast_ss(&b.c0.w));
    c0 = _mm_add_ps(_mm_add_ps(t0, t1), _mm_add_ps(t2, t3));

    /* c1 */
    t0 = _mm_mul_ps(a.c0.v, _mm_broadcast_ss(&b.c1.x));
    t1 = _mm_mul_ps(a.c1.v, _mm_broadcast_ss(&b.c1.y));
    t2 = _mm_mul_ps(a.c2.v, _mm_broadcast_ss(&b.c1.z));
    t3 = _mm_mul_ps(a.c3.v, _mm_broadcast_ss(&b.c1.w));
    c1 = _mm_add_ps(_mm_add_ps(t0, t1), _mm_add_ps(t2, t3));

    /* c2 */
    t0 = _mm_mul_ps(a.c0.v, _mm_broadcast_ss(&b.c2.x));
    t1 = _mm_mul_ps(a.c1.v, _mm_broadcast_ss(&b.c2.y));
    t2 = _mm_mul_ps(a.c2.v, _mm_broadcast_ss(&b.c2.z));
    t3 = _mm_mul_ps(a.c3.v, _mm_broadcast_ss(&b.c2.w));
    c2 = _mm_add_ps(_mm_add_ps(t0, t1), _mm_add_ps(t2, t3));

    /* c3 */
    t0 = _mm_mul_ps(a.c0.v, _mm_broadcast_ss(&b.c3.x));
    t1 = _mm_mul_ps(a.c1.v, _mm_broadcast_ss(&b.c3.y));
    t2 = _mm_mul_ps(a.c2.v, _mm_broadcast_ss(&b.c3.z));
    t3 = _mm_mul_ps(a.c3.v, _mm_broadcast_ss(&b.c3.w));
    c3 = _mm_add_ps(_mm_add_ps(t0, t1), _mm_add_ps(t2, t3));

    //c0 = _mm_mul_ps(a.c0.v, _mm_broadcast_ss(&b.c0.x));
    //c0 = _mm_fmadd_ps(a.c1.v, _mm_broadcast_ss(&b.c0.y), c0);
    //c0 = _mm_fmadd_ps(a.c2.v, _mm_broadcast_ss(&b.c0.z), c0);
    //c0 = _mm_fmadd_ps(a.c3.v, _mm_broadcast_ss(&b.c0.w), c0);

    //c1 = _mm_mul_ps(a.c0.v, _mm_broadcast_ss(&b.c1.x));
    //c1 = _mm_fmadd_ps(a.c1.v, _mm_broadcast_ss(&b.c1.y), c1);
    //c1 = _mm_fmadd_ps(a.c2.v, _mm_broadcast_ss(&b.c1.z), c1);
    //c1 = _mm_fmadd_ps(a.c3.v, _mm_broadcast_ss(&b.c1.w), c1);

    //c2 = _mm_mul_ps(a.c0.v, _mm_broadcast_ss(&b.c2.x));
    //c2 = _mm_fmadd_ps(a.c1.v, _mm_broadcast_ss(&b.c2.y), c2);
    //c2 = _mm_fmadd_ps(a.c2.v, _mm_broadcast_ss(&b.c2.z), c2);
    //c2 = _mm_fmadd_ps(a.c3.v, _mm_broadcast_ss(&b.c2.w), c2);

    //c3 = _mm_mul_ps(a.c0.v, _mm_broadcast_ss(&b.c3.x));
    //c3 = _mm_fmadd_ps(a.c1.v, _mm_broadcast_ss(&b.c3.y), c3);
    //c3 = _mm_fmadd_ps(a.c2.v, _mm_broadcast_ss(&b.c3.z), c3);
    //c3 = _mm_fmadd_ps(a.c3.v, _mm_broadcast_ss(&b.c3.w), c3);

    _mm_store_ps((float *)&m.c0, c0);
    _mm_store_ps((float *)&m.c1, c1);
    _mm_store_ps((float *)&m.c2, c2);
    _mm_store_ps((float *)&m.c3, c3);

    return m;

#else
    return (HglmMat4) {
        /* c0 */
        .m00 = a.c0.x * b.c0.x + a.c1.x * b.c0.y + a.c2.x * b.c0.z + a.c3.x * b.c0.w,
        .m10 = a.c0.y * b.c0.x + a.c1.y * b.c0.y + a.c2.y * b.c0.z + a.c3.y * b.c0.w,
        .m20 = a.c0.z * b.c0.x + a.c1.z * b.c0.y + a.c2.z * b.c0.z + a.c3.z * b.c0.w,
        .m30 = a.c0.w * b.c0.x + a.c1.w * b.c0.y + a.c2.w * b.c0.z + a.c3.w * b.c0.w,
        /* c1 */
        .m01 = a.c0.x * b.c1.x + a.c1.x * b.c1.y + a.c2.x * b.c1.z + a.c3.x * b.c1.w,
        .m11 = a.c0.y * b.c1.x + a.c1.y * b.c1.y + a.c2.y * b.c1.z + a.c3.y * b.c1.w,
        .m21 = a.c0.z * b.c1.x + a.c1.z * b.c1.y + a.c2.z * b.c1.z + a.c3.z * b.c1.w,
        .m31 = a.c0.w * b.c1.x + a.c1.w * b.c1.y + a.c2.w * b.c1.z + a.c3.w * b.c1.w,
        /* c2 */
        .m02 = a.c0.x * b.c2.x + a.c1.x * b.c2.y + a.c2.x * b.c2.z + a.c3.x * b.c2.w,
        .m12 = a.c0.y * b.c2.x + a.c1.y * b.c2.y + a.c2.y * b.c2.z + a.c3.y * b.c2.w,
        .m22 = a.c0.z * b.c2.x + a.c1.z * b.c2.y + a.c2.z * b.c2.z + a.c3.z * b.c2.w,
        .m32 = a.c0.w * b.c2.x + a.c1.w * b.c2.y + a.c2.w * b.c2.z + a.c3.w * b.c2.w,
        /* c3 */
        .m03 = a.c0.x * b.c3.x + a.c1.x * b.c3.y + a.c2.x * b.c3.z + a.c3.x * b.c3.w,
        .m13 = a.c0.y * b.c3.x + a.c1.y * b.c3.y + a.c2.y * b.c3.z + a.c3.y * b.c3.w,
        .m23 = a.c0.z * b.c3.x + a.c1.z * b.c3.y + a.c2.z * b.c3.z + a.c3.z * b.c3.w,
        .m33 = a.c0.w * b.c3.x + a.c1.w * b.c3.y + a.c2.w * b.c3.z + a.c3.w * b.c3.w,
    };
#endif
}

__attribute__ ((const, unused))
static HGL_INLINE HglmMat4 hglm_mat4_scale(HglmMat4 m, HglmVec3 v)
{
    return hglm_mat4_mul_mat4(m, hglm_mat4_make_scale(v));
}

__attribute__ ((const, unused))
static HGL_INLINE HglmMat4 hglm_mat4_rotate(HglmMat4 m, float angle, HglmVec3 axis)
{
    return hglm_mat4_mul_mat4(m, hglm_mat4_make_rotation(angle, axis));
}

__attribute__ ((const, unused))
static HGL_INLINE HglmMat4 hglm_mat4_translate(HglmMat4 m, HglmVec3 v)
{
    HglmVec4 c3 = m.c3;
    c3.x += v.x;
    c3.y += v.y;
    c3.z += v.z;
    return (HglmMat4) {.c0 = m.c0, .c1 = m.c1, .c2 = m.c2, .c3 = c3};
}

__attribute__ ((const, unused))
static HGL_INLINE HglmVec4 hglm_mat4_perspective_project(HglmMat4 proj, HglmVec4 v)
{
    HglmVec4 u = hglm_mat4_mul_vec4(proj, v);
    u.x /= u.w;
    u.y /= u.w;
    u.z /= u.w;
    return u;
}


/* ========== Arbitrary size Matrix funtions =================================*/

#define hglm_mat_at(m, y, x) ((m).data[(y)*(m).N + (x)])
#define hglm_mat_print(m) \
    do { \
        printf("%s = \n", #m ); \
        for (uint32_t row = 0; row < (m).M; row++) {\
            printf("  | "); \
            for (uint32_t col = 0; col < (m).N; col++) {\
                printf("%16f ", (double) hglm_mat_at((m), row, col)); \
            } \
            printf(" |\n"); \
        } \
    } while(0)

static HGL_INLINE HglmMat hglm_mat_make(uint32_t M /* rows */, uint32_t N /* cols */)
{
    HglmMat m = {
        .data = HGLM_ALLOC(M * N * sizeof(*m.data)),
        .M = M,
        .N = N,
    };
    assert(m.data != NULL);
    return m;
}

static HGL_INLINE HglmMat hglm_mat_make_identity(uint32_t N)
{
    HglmMat m = hglm_mat_make(N, N);
    hglm_mat_fill(m, 0);
    for (uint32_t i = 0; i < N; i++) {
        hglm_mat_at(m, i, i) = 1.0f;
    }
    return m;
}

static HGL_INLINE void hglm_mat_free(HglmMat m)
{
    HGLM_FREE(m.data);
}

static HGL_INLINE void hglm_mat_fill(HglmMat m, float value)
{
    for (uint32_t row = 0; row < m.M; row++) {
        for (uint32_t col = 0; col < m.N; col++) {
            hglm_mat_at(m, row, col) = value;
        }
    }
}

static HGL_INLINE void hglm_mat_add(HglmMat res, HglmMat a, HglmMat b)
{
    assert(a.M == b.M);
    assert(a.N == b.N);
    assert(res.N == a.N);
    assert(res.M == a.M);
    for (uint32_t row = 0; row < res.M; row++) {
        for (uint32_t col = 0; col < res.N; col++) {
            hglm_mat_at(res, row, col) = hglm_mat_at(a, row, col) + hglm_mat_at(b, row, col);
        }
    }
}

static HGL_INLINE void hglm_mat_sub(HglmMat res, HglmMat a, HglmMat b)
{
    assert(a.M == b.M);
    assert(a.N == b.N);
    assert(res.N == a.N);
    assert(res.M == a.M);
    for (uint32_t row = 0; row < res.M; row++) {
        for (uint32_t col = 0; col < res.N; col++) {
            hglm_mat_at(res, row, col) = hglm_mat_at(a, row, col) - hglm_mat_at(b, row, col);
        }
    }
}

static HGL_INLINE void hglm_mat_mul_scalar(HglmMat m, float s)
{
    for (uint32_t row = 0; row < m.M; row++) {
        for (uint32_t col = 0; col < m.N; col++) {
            hglm_mat_at(m, row, col) *= s;
        }
    }
}

static HGL_INLINE void hglm_mat_mul_mat(HglmMat res, HglmMat a, HglmMat b)
{
    /* AxB x BxC ==> AxC*/
    assert(a.N == b.M);
    assert(res.M == a.M);
    assert(res.N == b.N);
    assert(res.data != a.data);
    assert(res.data != b.data);
    for (uint32_t row = 0; row < res.M; row++) {
        for (uint32_t col = 0; col < res.N; col++) {
            hglm_mat_at(res, row, col) = 0.0f;
            for (uint32_t i = 0; i < res.N; i++) {
                hglm_mat_at(res, row, col) += hglm_mat_at(a, row, i) * hglm_mat_at(b, i, col);
            }
        }
    }
}

static HGL_INLINE void hglm_mat_transpose_in_place(HglmMat m)
{
    assert((m.M == m.N) && "In-place transpose only supports square matrices");
    for (uint32_t row = 0; row < m.M - 1; row++) {
        for (uint32_t col = row + 1; col < m.N; col++) {
            float temp = hglm_mat_at(m, row, col);
            hglm_mat_at(m, row, col) = hglm_mat_at(m, col, row);
            hglm_mat_at(m, col, row) = temp;
        }
    }
}

static HGL_INLINE void hglm_mat_transpose(HglmMat res, HglmMat m)
{
    assert(res.M == m.N);
    assert(res.N == m.M);
    for (uint32_t row = 0; row < res.M; row++) {
        for (uint32_t col = 0; col < res.N; col++) {
            hglm_mat_at(res, row, col) = hglm_mat_at(m, col, row);
        }
    }
}


/* ========== scalar & misc. math functions ==================================*/

static HGL_INLINE float hglm_pid(float error, float last_error, float *i, 
                                 float Kp, float Ki, float Kd, float dt)
{
    *i += error * dt;
    float d = (error - last_error) / dt;
    float p = error;

    return Kp*p + Ki*(*i) + Kd*d;
}

static HGL_INLINE float hglm_lerp(float a, float b, float t)
{
    return (1.0f - t) * a + t * b; // value
}

static HGL_INLINE float hglm_ilerp(float a, float b, float value)
{
    return (value - a) / (b - a); // t
}

static HGL_INLINE float hglm_clamp(float min, float max, float value)
{
    //return fminf(fmaxf(min, value), max);
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

static HGL_INLINE float hglm_remap(float in_min,
                                   float in_max,
                                   float out_min,
                                   float out_max,
                                   float value)
{
    float t = hglm_ilerp(in_min, in_max, value);
    return hglm_lerp(out_min, out_max, t);
}

static HGL_INLINE float hglm_smoothstep(float t)
{
    return t * t * (3.0f - 2.0f * t);
}

static HGL_INLINE float hglm_smootherstep(float t)
{
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

static HGL_INLINE float hglm_sinstep(float t)
{
    return -0.5f * cosf(t * (float)HGLM_PI) + 0.5f;
}

static HGL_INLINE float hglm_lerpsmooth(float a, float b, float dt, float omega)
{
    return b + (a - b) * exp2f(-dt/omega);
}

static HGL_INLINE float hglm_smoothmin_quadratic(float a, float b, float k)
{
    k *= 4.0f;
    float h = fmaxf(k - fabsf(a - b), 0.0) / k;
    return fminf(a, b) - h*h*k*0.25f;
}

static HGL_INLINE float hglm_smoothmin_sigmoid(float a, float b, float k)
{
    k *= logf(2.0);
    float x = b - a;
    return a + x / (1.0f - exp2f(x / k));
}

static HGL_INLINE HglmVec4 hglm_bezier3(float t)
{
    HglmMat4 bezier3 = ((HglmMat4) {.m00 =  1.0f, .m01 =  0.0f, .m02 =  0.0f, .m03 =  0.0f,
                                    .m10 = -3.0f, .m11 =  3.0f, .m12 =  0.0f, .m13 =  0.0f,
                                    .m20 =  3.0f, .m21 = -6.0f, .m22 =  3.0f, .m23 =  0.0f,
                                    .m30 = -1.0f, .m31 =  3.0f, .m32 = -3.0f, .m33 =  1.0f});
    HglmVec4 ts = (HglmVec4) {.x = 1.0f, .y = t, .z = t*t, .w = t*t*t};
    return (HglmVec4) {
        .x = hglm_vec4_dot(ts, bezier3.c0),
        .y = hglm_vec4_dot(ts, bezier3.c1),
        .z = hglm_vec4_dot(ts, bezier3.c2),
        .w = hglm_vec4_dot(ts, bezier3.c3),
    };
}

static HGL_INLINE HglmVec4 hglm_hermite3(float t)
{
    HglmMat4 hermite3 = ((HglmMat4) {.m00 =  1.0f, .m01 =  0.0f, .m02 =  0.0f, .m03 =  0.0f,
                                     .m10 =  0.0f, .m11 =  1.0f, .m12 =  0.0f, .m13 =  0.0f,
                                     .m20 = -3.0f, .m21 = -2.0f, .m22 =  3.0f, .m23 = -1.0f,
                                     .m30 =  2.0f, .m31 =  1.0f, .m32 = -2.0f, .m33 =  1.0f});
    HglmVec4 ts = (HglmVec4) {.x = 1.0f, .y = t, .z = t*t, .w = t*t*t};
    return (HglmVec4) {
        .x = hglm_vec4_dot(ts, hermite3.c0),
        .y = hglm_vec4_dot(ts, hermite3.c1),
        .z = hglm_vec4_dot(ts, hermite3.c2),
        .w = hglm_vec4_dot(ts, hermite3.c3),
    };
}

static HGL_INLINE float hglm_grad(int hash, float x, float y, float z)
{
    int h = hash & 15;
    float u = (h < 8) ? x : y;
    float v = (h < 4) ? y : ((h == 12) || (h == 14)) ? x : z;
    return (((h & 1) == 0) ? u : -u) +
           (((h & 2) == 0) ? v : -v);
}

static HGL_INLINE float hglm_perlin3D(float x, float y, float z)
{
    static const int P[512] = {
        151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140,
        36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148, 247, 120,
        234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,
        88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71,
        134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133,
        230, 220, 105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54, 65, 25, 63, 161,
        1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196, 135, 130, 116,
        188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124,
        123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16,
        58, 17, 182, 189, 28, 42, 223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163,
        70, 221, 153, 101, 155, 167, 43, 172, 9, 129, 22, 39, 253, 19, 98, 108, 110,
        79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228, 251, 34, 242, 193,
        238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107,
        49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45,
        127, 4, 150, 254, 138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141,
        128, 195, 78, 66, 215, 61, 156, 180
    };

    int X = (int)floorf(x) & 255;
    int Y = (int)floorf(y) & 255;
    int Z = (int)floorf(z) & 255;
    x -= floorf(x);
    y -= floorf(y);
    z -= floorf(z);
    float u = hglm_smootherstep(x);
    float v = hglm_smootherstep(y);
    float w = hglm_smootherstep(z);
    int A  = P[X] + Y;
    int AA = P[A] + Z;
    int AB = P[A+1] + Z;
    int B  = P[X+1] + Y;
    int BA = P[B] + Z;
    int BB = P[B+1] + Z;
    return 0.5f + hglm_lerp(hglm_lerp(hglm_lerp(hglm_grad(P[AA  ], x    , y    , z    ),
                                                hglm_grad(P[BA  ], x - 1, y    , z    ), u),
                                      hglm_lerp(hglm_grad(P[AB  ], x    , y - 1, z    ),
                                                hglm_grad(P[BB  ], x - 1, y - 1, z    ), u), v),
                            hglm_lerp(hglm_lerp(hglm_grad(P[AA+1], x    , y    , z - 1),
                                                hglm_grad(P[BA+1], x - 1, y    , z - 1), u),
                                      hglm_lerp(hglm_grad(P[AB+1], x    , y - 1, z - 1),
                                                hglm_grad(P[BB+1], x - 1, y - 1, z - 1), u), v), w);
}

#endif /* HGLM_H */


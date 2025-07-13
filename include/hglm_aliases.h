/**
 * LICENSE:
 *
 * MIT License
 *
 * Copyright (c) 2025 Henrik A. Glass
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
 
#ifndef HGLM_ALIASES_H
#define HGLM_ALIASES_H

#ifndef PI
#define PI HGLM_PI
#endif

#ifndef DEG_TO_RAD
#define DEG_TO_RAD HGLM_DEG_TO_RAD
#endif
#ifndef RAD_TO_DEG
#define RAD_TO_DEG HGLM_RAD_TO_DEG
#endif

#ifndef MAT2_IDENTITY
#define MAT2_IDENTITY HGLM_MAT1_IDENTITY
#endif

#ifndef MAT3_IDENTITY
#define MAT3_IDENTITY HGLM_MAT3_IDENTITY
#endif

#ifndef MAT4_IDENTITY
#define MAT4_IDENTITY HGLM_MAT4_IDENTITY
#endif

typedef HglmIVec2 IVec2;
typedef HglmIVec3 IVec3;
typedef HglmIVec4 IVec4;
typedef HglmVec2   Vec2;
typedef HglmVec3   Vec3;
typedef HglmVec4   Vec4;
typedef HglmMat2   Mat2;
typedef HglmMat3   Mat3;
typedef HglmMat4   Mat4;
typedef HglmMat    Mat;

#define ivec2_print              hglm_ivec2_print
#define ivec2_make               hglm_ivec2_make
#define ivec2_add                hglm_ivec2_add
#define ivec2_sub                hglm_ivec2_sub
#define ivec2_distance           hglm_ivec2_distance
#define ivec2_len                hglm_ivec2_len
#define ivec2_mul_scalar         hglm_ivec2_mul_scalar
#define ivec2_lerp               hglm_ivec2_lerp

#define ivec3_print              hglm_ivec3_print
#define ivec3_make               hglm_ivec3_make

#define ivec4_print              hglm_ivec4_print
#define ivec4_make               hglm_ivec4_make

#define vec2_print               hglm_vec2_print
#define vec2_make                hglm_vec2_make
#define vec2_from_polar          hglm_vec2_from_polar
#define vec2_add                 hglm_vec2_add
#define vec2_sub                 hglm_vec2_sub
#define vec2_distance            hglm_vec2_distance
#define vec2_len                 hglm_vec2_len
#define vec2_normalize           hglm_vec2_normalize
#define vec2_dot                 hglm_vec2_dot
#define vec2_recip               hglm_vec2_recip
#define vec2_hadamard            hglm_vec2_hadamard
#define vec2_mul_scalar          hglm_vec2_mul_scalar
#define vec2_reflect             hglm_vec2_reflect
#define vec2_lerp                hglm_vec2_lerp
#define vec2_slerp               hglm_vec2_slerp
#define vec2_bezier3             hglm_vec2_bezier3

#define vec3_print               hglm_vec3_print
#define vec3_make                hglm_vec3_make
#define vec3_from_spherical      hglm_vec3_from_spherical
#define vec3_add                 hglm_vec3_add
#define vec3_sub                 hglm_vec3_sub
#define vec3_distance            hglm_vec3_distance
#define vec3_len                 hglm_vec3_len
#define vec3_normalize           hglm_vec3_normalize
#define vec3_dot                 hglm_vec3_dot
#define vec3_cross               hglm_vec3_cross
#define vec3_recip               hglm_vec3_recip
#define vec3_hadamard            hglm_vec3_hadamard
#define vec3_mul_scalar          hglm_vec3_mul_scalar
#define vec3_reflect             hglm_vec3_reflect
#define vec3_lerp                hglm_vec3_lerp
#define vec3_slerp               hglm_vec3_slerp
#define vec3_bezier3             hglm_vec3_bezier3

#define vec4_print               hglm_vec4_print
#define vec4_make                hglm_vec4_make
#define vec4_add                 hglm_vec4_add
#define vec4_sub                 hglm_vec4_sub
#define vec4_distance            hglm_vec4_distance
#define vec4_len                 hglm_vec4_len
#define vec4_normalize           hglm_vec4_normalize
#define vec4_dot                 hglm_vec4_dot
#define vec4_recip               hglm_vec4_recip
#define vec4_hadamard            hglm_vec4_hadamard
#define vec4_mul_scalar          hglm_vec4_mul_scalar
#define vec4_swizzle             hglm_vec4_swizzle
#define vec4_perspective_divide  hglm_vec4_perspective_divide
#define vec4_lerp                hglm_vec4_lerp
#define vec4_bezier3             hglm_vec4_bezier3

#define mat2_print               hglm_mat2_print
#define mat2_make                hglm_mat2_make
#define mat2_make_identity       hglm_mat2_make_identity

#define mat3_print               hglm_mat3_print
#define mat3_make                hglm_mat3_make
#define mat3_make_identity       hglm_mat3_make_identity
#define mat3_make_from_mat4      hglm_mat3_make_from_mat4
#define mat3_transpose           hglm_mat3_transpose
#define mat3_mul_vec3            hglm_mat3_mul_vec3

#define mat4_print               hglm_mat4_print
#define mat4_make                hglm_mat4_make
#define mat4_make_zero           hglm_mat4_make_zero
#define mat4_make_identity       hglm_mat4_make_identity
#define mat4_make_scale          hglm_mat4_make_scale
#define mat4_make_rotation       hglm_mat4_make_rotation
#define mat4_make_translation    hglm_mat4_make_translation
#define mat4_make_ortho          hglm_mat4_make_ortho
#define mat4_make_perspective    hglm_mat4_make_perspective
#define mat4_look_at             hglm_mat4_look_at
#define mat4_look_to             hglm_mat4_look_to
#define mat4_add                 hglm_mat4_add
#define mat4_sub                 hglm_mat4_sub
#define mat4_transpose           hglm_mat4_transpose
#define mat4_mul_scalar          hglm_mat4_mul_scalar
#define mat4_mul_vec4            hglm_mat4_mul_vec4
#define mat4_mul_mat4            hglm_mat4_mul_mat4
#define mat4_scale               hglm_mat4_scale
#define mat4_rotate              hglm_mat4_rotate
#define mat4_translate           hglm_mat4_translate
#define mat4_perspective_project hglm_mat4_perspective_project

#define mat_print                hglm_mat_print
#define mat_at                   hglm_mat_at
#define mat_make                 hglm_mat_make
#define mat_make_identity        hglm_mat_make_identity
#define mat_free                 hglm_mat_free
#define mat_fill                 hglm_mat_fill
#define mat_add                  hglm_mat_add
#define mat_sub                  hglm_mat_sub
#define mat_mul_scalar           hglm_mat_mul_scalar
#define mat_mul_mat              hglm_mat_mul_mat
#define mat_transpose_in_place   hglm_mat_transpose_in_place
#define mat_transpose            hglm_mat_transpose

#define pid                      hglm_pid
#define lerp                     hglm_lerp
#define ilerp                    hglm_ilerp
#define clamp                    hglm_clamp
#define remap                    hglm_remap
#define smoothstep               hglm_smoothstep
#define smootherstep             hglm_smootherstep
#define sinstep                  hglm_sinstep
#define lerpsmooth               hglm_lerpsmooth
#define smoothmin_quadratic      hglm_smoothmin_quadratic
#define smoothmin_sigmoid        hglm_smoothmin_sigmoid
#define bezier3                  hglm_bezier3
#define hermite3                 hglm_hermite3
#define perlin3D                 hglm_perlin3D

#endif /* HGLM_ALIASES_H */


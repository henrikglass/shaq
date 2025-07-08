#ifndef HGL_FLT_H
#define HGL_FLT_H

/*--- Include files ---------------------------------------------------------------------*/

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

/*--- Public macros ---------------------------------------------------------------------*/

/*--- Public type definitions -----------------------------------------------------------*/

typedef struct __attribute__((packed)) {uint16_t bits;} bf16; /* brain floating point (bfloat16) */
typedef struct __attribute__((packed)) {uint16_t bits;} f16;  /* IEEE-754 half-precision floating point (binary16) */
typedef float  f32;
typedef double f64;

/* just in case... */
static_assert(sizeof(float) == 4, "");
static_assert(sizeof(double) == 8, "");

/*--- Public variables ------------------------------------------------------------------*/

/*--- Public function prototypes --------------------------------------------------------*/

/* float bit functions */
static inline uint32_t f32_sign(f32 v);
static inline uint64_t f64_sign(f64 v);
static inline uint32_t f32_exponent(f32 v);
static inline uint64_t f64_exponent(f64 v);
static inline uint32_t f32_mantissa(f32 v);
static inline uint64_t f64_mantissa(f64 v);
static inline uint32_t f32_to_bits(f32 v);
static inline uint64_t f64_to_bits(f64 v);
static inline f32 f32_from_bits(uint32_t v);
static inline f64 f64_from_bits(uint64_t v);

/* Fixed point numbers */
static inline f64 fixed_radix_to_epsilon(int radix);
static inline f64 f64_from_fixed_radix(uint64_t v, int n_bits, int radix, bool is_signed);
static inline f64 f64_from_fixed_scale(uint64_t v, int n_bits, f64 scale, bool is_signed);
static inline uint64_t f64_to_fixed_radix(f64 v, int n_bits, int radix);
static inline uint64_t f64_to_fixed_scale(f64 v, int n_bits, f64 scale);
static inline f64 f64_clamp_to_fixed_range_radix(f64 v, int n_bits, int radix, bool is_signed);
static inline f64 f64_clamp_to_fixed_range_scale(f64 v, int n_bits, f64 scale, bool is_signed);

/* brain floating point (bfloat16) */
static inline bf16 bf16_from_f32(f32 v);
static inline f32 bf16_to_f32(bf16 v);

/* IEEE-754 half-precision floating point (binary16) */
static inline f16 f16_from_f32(f32 v);
static inline f32 f16_to_f32(f16 v);

static inline uint32_t f32_sign(f32 v)
{
    return (f32_to_bits(v) >> 31);
}

static inline uint64_t f64_sign(f64 v)
{
    return (f64_to_bits(v) >> 63);
}

static inline uint32_t f32_exponent(f32 v)
{
    return (f32_to_bits(v) >> 23) & 0xFF;
}

static inline uint64_t f64_exponent(f64 v)
{
    return (f64_to_bits(v) >> 52) & 0x7FF;
}

static inline uint32_t f32_mantissa(f32 v)
{
    return f32_to_bits(v) & 0x007FFFFF;
}

static inline uint64_t f64_mantissa(f64 v)
{
    return f64_to_bits(v) & 0x000FFFFFFFFFFFFF;
}

static inline uint32_t f32_to_bits(f32 v)
{
    union {
        uint32_t bits32_;
        f32 float32_;
    } u;
    u.float32_ = v;
    return u.bits32_;
}

static inline uint64_t f64_to_bits(f64 v)
{
    union {
        uint64_t bits64_;
        f64 float64_;
    } u;
    u.float64_ = v;
    return u.bits64_;
}

static inline f32 f32_from_bits(uint32_t v)
{
    union {
        uint32_t bits32_;
        f32 float32_;
    } u;
    u.bits32_ = v;
    return u.float32_;
}

static inline f64 f64_from_bits(uint64_t v)
{
    union {
        uint64_t bits64_;
        f64 float64_;
    } u;
    u.bits64_ = v;
    return u.float64_;
}

static inline f64 fixed_radix_to_epsilon(int radix)
{
    return (f64) (1.0 / (1lu << radix));
}

static inline f64 f64_from_fixed_radix(uint64_t v, int n_bits, int radix, bool is_signed)
{
    assert(n_bits <= 64);
    assert(radix <= n_bits);
    f64 s = 1.0 / (f64)(1lu << radix);
    uint64_t m = n_bits != 64 ? (((uint64_t)-1) << n_bits) ^ ((uint64_t)-1) 
                              : (uint64_t)-1;
    v = v & m;
    if (is_signed) {
        m = 1lu << (n_bits - 1);
        i64 i = (v ^ m) - m;
        return (f64)i * s;
    } else {
        return (f64)v * s;
    }
}

static inline f64 f64_from_fixed_scale(uint64_t v, int n_bits, f64 scale, bool is_signed)
{
    assert(n_bits <= 64);
    f64 s = scale;
    uint64_t m = n_bits != 64 ? (((uint64_t)-1) << n_bits) ^ ((uint64_t)-1) 
                              : (uint64_t)-1;
    v = v & m;
    if (is_signed) {
        m = 1lu << (n_bits - 1);
        i64 i = (v ^ m) - m;
        return (f64)i * s;
    } else {
        return (f64)v * s;
    }
}

static inline uint64_t f64_to_fixed_radix(f64 v, int n_bits, int radix)
{
    assert(n_bits <= 64);
    uint64_t m = n_bits != 64 ? (((uint64_t)-1) << n_bits) ^ ((uint64_t)-1) 
                              : (uint64_t)-1;
    uint64_t u = ((uint64_t)(v * (1lu << radix))) & m;
    return u;
}

static inline uint64_t f64_to_fixed_scale(f64 v, int n_bits, f64 scale)
{
    assert(n_bits <= 64);
    uint64_t m = n_bits != 64 ? (((uint64_t)-1) << n_bits) ^ ((uint64_t)-1) 
                              : (uint64_t)-1;
    uint64_t u = ((uint64_t)(v / scale)) & m;
    return u;
}

static inline f64 f64_clamp_to_fixed_range_radix(f64 v, int n_bits, int radix, bool is_signed)
{
    uint64_t m = n_bits != 64 ? (((uint64_t)-1) << n_bits) ^ ((uint64_t)-1) 
                              : (uint64_t)-1;
    f64 min = is_signed ? f64_from_fixed_radix((m >> 1) ^ m, n_bits, radix, true) : 0;
    f64 max = is_signed ? f64_from_fixed_radix((m >> 1), n_bits, radix, true) :
                          f64_from_fixed_radix(m, n_bits, radix, false);
    if (v > max) return max;
    if (v < min) return min;
    return v;
}

static inline f64 f64_clamp_to_fixed_range_scale(f64 v, int n_bits, f64 scale, bool is_signed)
{
    uint64_t m = n_bits != 64 ? (((uint64_t)-1) << n_bits) ^ ((uint64_t)-1) 
                              : (uint64_t)-1;
    f64 min = is_signed ? f64_from_fixed_scale((m >> 1) ^ m, n_bits, scale, true) : 0;
    f64 max = is_signed ? f64_from_fixed_scale((m >> 1), n_bits, scale, true) :
                          f64_from_fixed_scale(m, n_bits, scale, false);
    if (v > max) return max;
    if (v < min) return min;
    return v;
}

static inline bf16 bf16_from_f32(f32 v)
{
    bf16 r;
    r.bits = (uint16_t)(f32_to_bits(v) >> 16);
    return r;
}

static inline f32 bf16_to_f32(bf16 v)
{
    return f32_from_bits(((uint32_t)v.bits) << 16);
}

static inline f16 f16_from_f32(f32 v)
{
    // Credits: https://stackoverflow.com/a/60047308/5350029
    f16 r = {0};
    const uint32_t b = f32_to_bits(v) + 0x00001000; // round-to-nearest-even: add last bit after truncated mantissa
    const uint32_t e = (b & 0x7F800000) >> 23; // exponent
    const uint32_t m = b & 0x007FFFFF; // mantissa; in line below: 0x007FF000 = 0x00800000-0x00001000 = decimal indicator flag - initial rounding
    r.bits = (b&0x80000000)>>16 | (e>112)*((((e-112)<<10)&0x7C00)|m>>13) | ((e<113)&(e>101))*((((0x007FF000+m)>>(125-e))+1)>>1) | (e>143)*0x7FFF; // sign : normalized : denormalized : saturate
    return r;
}

static inline f32 f16_to_f32(f16 v)
{
    // Credits: https://stackoverflow.com/a/60047308/5350029
    uint16_t b = v.bits;
    const uint32_t e = (b&0x7C00)>>10; // exponent
    const uint32_t m = (b&0x03FF)<<13; // mantissa
    const uint32_t x = f32_to_bits((float)m)>>23; // evil log2 bit hack to count leading zeros in denormalized format
    return f32_from_bits((b&0x8000)<<16 | (e!=0)*((e+112)<<23|m) | ((e==0)&(m!=0))*((x-37)<<23|((m<<(150-x))&0x007FE000))); // sign : normalized : denormalized
}


#endif /* HGL_FLT_H */


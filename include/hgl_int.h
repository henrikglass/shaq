#ifndef HGL_INT_H
#define HGL_INT_H

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

typedef bool           b8;
typedef uint8_t        u8;
typedef uint16_t      u16;
typedef uint32_t      u32;
typedef uint64_t      u64;
typedef int8_t         i8;
typedef int16_t       i16;
typedef int32_t       i32;
typedef int64_t       i64;
typedef unsigned int uint;

static_assert(sizeof(bool) == 1, "");

#endif /* HGL_INT_H */

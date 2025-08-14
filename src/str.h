#ifndef STR_H
#define STR_H

#include "alloc.h"

#include <stddef.h>
#include <stdbool.h>

#define HGL_STRING_ALLOC dummy_alloc
#define HGL_STRING_REALLOC dummy_realloc
#define HGL_STRING_FREE dummy_free
#define HGL_STRING_STRIP_PREFIX
#include "hgl_string.h"

#endif /* STR_H */


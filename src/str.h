#ifndef STR_H
#define STR_H

#include "alloc.h"

#include <stddef.h>
#include <stdbool.h>

#define HGL_STRING_ALLOC dummy_alloc
#define HGL_STRING_REALLOC dummy_realloc
#define HGL_STRING_FREE dummy_free
#include "hgl_string.h"
#include "hgl_string_aliases.h"

#endif /* STR_H */


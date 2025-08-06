#ifndef STR_H
#define STR_H

#include "alloc.h"

#include <stddef.h>
#include <stdbool.h>

static inline void *str_alloc(size_t size);
static inline void *str_realloc(void *ptr, size_t size);
static inline void str_free(void *ptr);
static inline void *str_alloc(size_t size){ return arena_alloc(g_session_arena, size);}
static inline void *str_realloc(void *ptr, size_t size){ (void) ptr; (void) size; assert(false && "disallow realloc"); return NULL;}
static inline void str_free(void *ptr){ (void) ptr; }

#define HGL_STRING_ALLOC str_alloc
#define HGL_STRING_REALLOC str_realloc
#define HGL_STRING_FREE str_free
#include "hgl_string.h"
#include "hgl_string_aliases.h"

#endif /* STR_H */


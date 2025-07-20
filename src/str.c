#include "str.h"

#include "alloc.h"

void *str_alloc(size_t size);
void *str_realloc(void *ptr, size_t size);
void str_free(void *ptr);
void *str_alloc(size_t size){ return arena_alloc(g_longterm_arena, size);}
void *str_realloc(void *ptr, size_t size){ (void) ptr; (void) size; assert(false && "disallow realloc"); return NULL;}
void str_free(void *ptr){ (void) ptr; }

#define HGL_STRING_ALLOC str_alloc
#define HGL_STRING_REALLOC str_realloc
#define HGL_STRING_FREE str_free
#define HGL_STRING_IMPLEMENTATION
#include "hgl_string.h"


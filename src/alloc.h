#ifndef ALLOC_H
#define ALLOC_H

#include <stddef.h>
#include "hgl_arena_alloc.h"

#define alloc hgl_arena_alloc
#define free_all hgl_arena_free_all
#define print_usage hgl_arena_print_usage

extern HglArena *temp_allocator;

#endif /* ALLOC_H */


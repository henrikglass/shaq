#ifndef ALLOC_H
#define ALLOC_H

#include <stddef.h>
#include "hgl_arena_alloc.h"
#include "hgl_fs_alloc.h"

#define arena_alloc hgl_arena_alloc
#define arena_free_all hgl_arena_free_all
#define arena_print_usage hgl_arena_print_usage
#define arena_realloc hgl_arena_realloc

#define fs_alloc hgl_fs_alloc
#define fs_realloc hgl_fs_realloc
#define fs_free hgl_fs_free
#define fs_free_all hgl_fs_free_all

//#define stack_alloc hgl_stack_alloc
//#define stack_realloc hgl_stack_realloc
//#define stack_free hgl_stack_free

#endif /* ALLOC_H */


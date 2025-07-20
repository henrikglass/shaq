#ifndef ALLOC_H
#define ALLOC_H

#include <stddef.h>
#include "hgl_arena_alloc.h"
#include "hgl_fs_alloc.h"

#define Arena HglArena
#define FsAllocator HglFsAllocator

#define arena_alloc hgl_arena_alloc
#define arena_free_last hgl_arena_free_last
#define arena_free_all hgl_arena_free_all
#define arena_print_usage hgl_arena_print_usage
#define arena_realloc hgl_arena_realloc

#define stack_alloc hgl_stack_alloc
#define stack_realloc hgl_stack_realloc
#define stack_free hgl_stack_free

#define fs_alloc hgl_fs_alloc
#define fs_realloc hgl_fs_realloc
#define fs_free hgl_fs_free
#define fs_free_all hgl_fs_free_all

extern Arena *g_frame_arena;
extern Arena *g_longterm_arena;
extern FsAllocator *g_longterm_fs_allocator;

void alloc_init(void);

#endif /* ALLOC_H */


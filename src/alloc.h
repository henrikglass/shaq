#ifndef ALLOC_H
#define ALLOC_H

#include <stddef.h>
#include "hgl_alloc.h"

#define Allocator HglAllocator

extern Allocator *g_temp_allocator;
extern Allocator *g_frame_arena;
extern Allocator *g_session_arena;
extern Allocator *g_session_fs_allocator;

void alloc_init(void);
void alloc_final(void);

void *tmp_alloc(size_t size);
void *frame_arena_alloc(size_t size);
void *session_arena_alloc(size_t size);
void *session_arena_realloc(void *ptr, size_t size);
void  session_arena_free(void *ptr);
void *session_fs_alloc(size_t size);
void *session_fs_realloc(void *ptr, size_t size);
void  session_fs_free(void *ptr);
void *dummy_alloc(size_t size);
void *dummy_realloc(void *ptr, size_t size);
void  dummy_free(void *ptr);

#endif /* ALLOC_H */


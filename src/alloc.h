#ifndef ALLOC_H
#define ALLOC_H

#include <stddef.h>
#include "hgl_alloc.h"

#define Allocator HglAllocator

extern Allocator *g_frame_arena;
extern Allocator *g_session_arena;
extern Allocator *g_session_fs_allocator;

void alloc_init(void);
void *alloc_temp(size_t size);

#endif /* ALLOC_H */


#ifndef ALLOC_H
#define ALLOC_H

#include <stddef.h>
#include "hgl_alloc.h"

#define Allocator HglAllocator

extern Allocator *g_temp_allocator;   // for temporary allocations
extern Allocator *g_frame_arena;      // for allocations that can be freed at the end of the frame
extern Allocator *g_r2r_arena;        // for allocations that can be freed at the next reload
extern Allocator *g_r2r_fs_allocator; // for allocations that can be freed at the next reload
extern Allocator *g_image_allocator;  // for image allocations (freed opon opening a new project file)

void alloc_init(void);
void alloc_final(void);

void *tmp_alloc(size_t size);
void *frame_arena_alloc(size_t size);
void *r2r_arena_alloc(size_t size);
void *r2r_arena_realloc(void *ptr, size_t size);
void  r2r_arena_free(void *ptr);
void *r2r_fs_alloc(size_t size);
void *r2r_fs_realloc(void *ptr, size_t size);
void  r2r_fs_free(void *ptr);
void *image_alloc(size_t size);
void *image_realloc(void *ptr, size_t size);
void  image_free(void *ptr);

/* These are NO-OP */
void *dummy_alloc(size_t size);
void *dummy_realloc(void *ptr, size_t size);
void  dummy_free(void *ptr);

#endif /* ALLOC_H */


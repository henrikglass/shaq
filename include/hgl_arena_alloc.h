
/**
 * LICENSE:
 *
 * MIT License
 *
 * Copyright (c) 2023 Henrik A. Glass
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * MIT License
 *
 *
 * ABOUT:
 *
 * hgl_arena_alloc.h implements a simple to use arena (a.k.a. linear) allocator.
 *
 *
 * USAGE:
 *
 * Include hgl_arena_alloc.h file like this:
 *
 *     #define HGL_ARENA_ALLOC_IMPLEMENTATION
 *     #include "hgl_arena_alloc.h"
 *
 * This will create the implementation of hgl_arena_alloc in the current compilation unit. To
 * include hgl_arena_alloc.h without creating the implementation, simply ommit the #define
 * of HGL_ARENA_ALLOC_IMPLEMENTATION.
 *
 * Below is a complet listing of the API:
 *
 * HglArena hgl_arena_make(size_t arena_size)
 * void *hgl_arena_alloc(HglArena *arena, size_t alloc_size)
 * void hgl_arena_free_all(HglArena *arena)
 * void hgl_arena_destroy(HglArena *arena)
 *
 * hgl_arena_alloc allows the user to define the alignment of allocations from an arena by
 * redefining HGL_ARENA_ALIGNMENT, as such:
 *
 *     #define HGL_ARENA_ALIGNMENT 64
 *
 * The default alignment is 16.
 *
 *
 * AUTHOR: Henrik A. Glass
 *
 */

#ifndef HGL_ARENA_ALLOC_H
#define HGL_ARENA_ALLOC_H

/*--- Arena alloc-specific macros -------------------------------------------------------*/

//#define HGL_ARENA_ALLOC_DEBUG_PRINTS

#ifndef HGL_ARENA_ALIGNMENT
#define HGL_ARENA_ALIGNMENT 16
#endif


#define HGL_ARENA_INITIALIZER(s)     \
    {                                \
        .memory = (uint8_t[(s)]){0}, \
        .head = 0,                   \
        .size = (s),                 \
        .backend = HGL_ARENA_STATIC, \
        .last_alloc = 0,             \
    }

/*--- Include files ---------------------------------------------------------------------*/

#include <assert.h>
#include <stdint.h>

/*--- Public type definitions -----------------------------------------------------------*/

static_assert(HGL_ARENA_ALIGNMENT % 2 == 0);

typedef enum
{
    HGL_ARENA_MALLOC,
    HGL_ARENA_MMAP,
    HGL_ARENA_STATIC,
} HglArenaBackend;

typedef struct
{
    uint8_t *memory;
    size_t head;
    size_t size;
    HglArenaBackend backend;
    void *last_alloc;
} HglArena;

/**
 * Create an arena of size `arena_size`.
 */
HglArena hgl_arena_make(size_t arena_size);

#ifdef _DEFAULT_SOURCE
/**
 * Create an arena of size `arena_size` using an anonymous private mmap-mapping.
 */
HglArena hgl_arena_make_mmap(size_t arena_size);
#endif

/**
 * Create an arena of `buf_size` from a preallocated buffer `buf`.
 */
HglArena hgl_arena_make_from_buffer(void *buf, size_t buf_size);

/**
 * Allocate a chunk of `alloc_size` bytes from `arena`.
 */
void *hgl_arena_alloc(HglArena *arena, size_t alloc_size);

/**
 * Reallocate `ptr` in the arena. `ptr` MUST be the result of the last 
 * call to `hgl_arena_alloc()`, otherwise the program aborts. 
 *
 * This is really only here in case you have a single dynamic array living
 * inside an arena and you wish to grow it.
 */
void *hgl_arena_realloc(HglArena *arena, void *ptr, size_t alloc_size);

/**
 * Free all allocations in `arena`.
 */
void hgl_arena_free_all(HglArena *arena);

/**
 * Print arena memory usage.
 */
void hgl_arena_print_usage(HglArena *arena);

/**
 * Destroy the arena (Free the arena itself).
 */
void hgl_arena_destroy(HglArena *arena);

#endif /* HGL_ARENA_ALLOC_H */

/*--- Public variables ------------------------------------------------------------------*/

/*--- Arena Alloc functions -------------------------------------------------------------*/

#ifdef HGL_ARENA_ALLOC_IMPLEMENTATION

#ifdef HGL_ARENA_ALLOC_DEBUG_PRINTS
#  include <stdio.h>
#endif

#include <stdlib.h>
#include <sys/mman.h>

HglArena hgl_arena_make(size_t arena_size)
{
    void *mem = aligned_alloc(HGL_ARENA_ALIGNMENT, arena_size);
    return (HglArena) {
        .memory = mem,
        .head = 0,
        .size = (mem == NULL) ? 0 : arena_size,
        .backend = HGL_ARENA_MALLOC,
        .last_alloc = 0,
    };
}

#ifdef _DEFAULT_SOURCE
HglArena hgl_arena_make_mmap(size_t arena_size)
{
    void *mem = mmap(NULL, arena_size, 
                     PROT_READ | PROT_WRITE, 
                     MAP_PRIVATE | MAP_ANONYMOUS,
                     -1, 0);
    if (mem == MAP_FAILED) {
        mem = NULL;
    }
    return (HglArena) {
        .memory = mem,
        .head = 0,
        .size = (mem == NULL) ? 0 : arena_size,
        .backend = HGL_ARENA_MMAP,
        .last_alloc = 0,
    };
}
#endif

HglArena hgl_arena_make_from_buffer(void *buf, size_t buf_size)
{
    return (HglArena) {
        .memory = (uint8_t *) buf,
        .head = 0,
        .size = buf_size,
    };
}

void *hgl_arena_alloc(HglArena *arena, size_t alloc_size)
{
    void *ptr = arena->memory + arena->head;

    /* Allocation too small: Return NULL */
    if (alloc_size == 0) {
        return NULL;
    }

    /* Allocation too big: Return NULL */
    if (arena->head + alloc_size > arena->size) {
#ifdef HGL_ARENA_ALLOC_DEBUG_PRINTS
        printf("Arena alloc failed. Requested %lu bytes, but only %lu remain in arena.\n",
                alloc_size, arena->size - arena->head);
#endif /* HGL_ARENA_ALLOC_DEBUG_PRINTS */
        return NULL;
    }

    /* save this as the last allocation */
    arena->last_alloc = ptr;

    /* Move head to nearest multiple of alignment after `head` + `alloc_size` */
    arena->head += (alloc_size + HGL_ARENA_ALIGNMENT - 1) &
                  ~(HGL_ARENA_ALIGNMENT - 1);
    return ptr;
}

void *hgl_arena_realloc(HglArena *arena, void *ptr, size_t alloc_size)
{
    /* check pointer */
    if (ptr != arena->last_alloc) {
        fprintf(stderr, "hgl_arena_realloc(): invalid pointer.\n");
        abort();
    }

    /* restore old head */
    arena->head = (uint8_t *)ptr - arena->memory;

    /* allocate anew */
    return hgl_arena_alloc(arena, alloc_size);
}

void hgl_arena_free_all(HglArena *arena)
{
    arena->head = 0;
}

void hgl_arena_print_usage(HglArena *arena)
{
    printf("Arena usage: %f%% (%lu/%lu bytes).\n",
           100.0 * ((double) arena->head / (double) arena->size),
           arena->head, arena->size);
}

void hgl_arena_destroy(HglArena *arena)
{
    switch (arena->backend) {
        case HGL_ARENA_MALLOC: free(arena->memory); break;
        case HGL_ARENA_MMAP:   munmap(arena->memory, arena->size); break;
        case HGL_ARENA_STATIC: break;
    }
}

#endif /* HGL_ARENA_ALLOC_IMPLEMENTATION */

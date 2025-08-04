
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
 *
 * Example:
 *
 * See examples/ directory.
 *
 *
 * AUTHOR: Henrik A. Glass
 *
 */

#ifndef HGL_ARENA_ALLOC_H
#define HGL_ARENA_ALLOC_H

/*--- Arena alloc-specific macros -------------------------------------------------------*/

//#define HGL_ARENA_ALLOC_DEBUG_PRINTS

#define HGL_ARENA_INITIALIZER(k, s)             \
    {                                           \
        .memory = (uint8_t[(s)]){0},            \
        .head = 0,                              \
        .config =  {                            \
            .kind = (k),                        \
            .backend = HGL_ARENA_BUFFER_BACKED, \
            .size = (s),                        \
        },                                      \
    }

/*--- Include files ---------------------------------------------------------------------*/

#include <assert.h>
#include <stdint.h>

/*--- Public type definitions -----------------------------------------------------------*/

typedef enum
{
    HGL_ARENA_MALLOC,
    HGL_ARENA_MMAP,
    HGL_ARENA_MMAP_HUGEPAGE,
    HGL_ARENA_MMAP_HUGEPAGE_2MB,
    HGL_ARENA_MMAP_HUGEPAGE_1GB,
    HGL_ARENA_BUFFER_BACKED,
} HglArenaBackend;

typedef enum
{
    HGL_ARENA_BUMP_ALLOCATOR,
    HGL_ARENA_STACK_ALLOCATOR,
} HglArenaKind;

typedef struct
{
    HglArenaKind kind;
    HglArenaBackend backend;
    uint8_t *optional_backing_buffer;
    uint32_t alignment;
    size_t size;
} HglArenaConfig;

typedef struct
{
    HglArenaConfig config;
    uint8_t *memory;
    size_t head;
} HglArena;

/**
 * Create an arena with the given parameters in `config`.
 */
#define hgl_arena_make(...) hgl_arena_make_((HglArenaConfig){.kind = HGL_ARENA_BUMP_ALLOCATOR,  \
                                                             .backend = HGL_ARENA_MMAP,         \
                                                             .optional_backing_buffer = NULL,   \
                                                             .alignment = 16,                   \
                                                             .size = 0,                         \
                                                             __VA_ARGS__})
HglArena hgl_arena_make_(HglArenaConfig config);

/**
 * Allocate a chunk of `alloc_size` bytes from `arena`.
 */
#define hgl_arena_push hgl_arena_alloc
void *hgl_arena_alloc(HglArena *arena, size_t alloc_size);

/**
 * Reallocate `ptr` in the arena. `ptr` MUST be the result of the last 
 * call to `hgl_arena_alloc()`, otherwise the program aborts.
 *
 * This is really only here in case you have a single dynamic array living
 * inside an arena and you wish to grow it.
 */
#define hgl_arena_grow_inplace hgl_arena_realloc
void *hgl_arena_realloc(HglArena *arena, void *ptr, size_t alloc_size);

/**
 * Free `ptr`. `ptr` MUST be the result of the last call to `hgl_arena_alloc()`, 
 * otherwise the program aborts.
 */
void hgl_arena_free(HglArena *arena, void *ptr);

/**
 * Like `hgl_arena_free` but `ptr` is inferred.
 */
void hgl_arena_pop(HglArena *arena);

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

#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <linux/mman.h>

typedef struct
{
    uintptr_t alloc_offset;
} HglArenaAllocationFooter;

HglArena hgl_arena_make_(HglArenaConfig config)
{
    assert(config.alignment % 2 == 0);

    HglArena arena = {
        .head = 0,
        .config = config
    };

    switch (config.backend) {
        case HGL_ARENA_MALLOC: {
            arena.memory = aligned_alloc(config.alignment, config.size);
            if (arena.memory == NULL) {
                fprintf(stderr, "hgl_arena_alloc.h] Call to `aligned_alloc()` failed.\n");
                arena.memory = NULL;
            }
        } break;

        case HGL_ARENA_MMAP:
        case HGL_ARENA_MMAP_HUGEPAGE:
        case HGL_ARENA_MMAP_HUGEPAGE_2MB:
        case HGL_ARENA_MMAP_HUGEPAGE_1GB: {
            int mmap_flags = MAP_PRIVATE | MAP_ANONYMOUS; 
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
            switch (config.backend) {
                case HGL_ARENA_MMAP_HUGEPAGE:     mmap_flags |= MAP_HUGETLB; break;
                case HGL_ARENA_MMAP_HUGEPAGE_2MB: mmap_flags |= MAP_HUGETLB | MAP_HUGE_2MB; break;
                case HGL_ARENA_MMAP_HUGEPAGE_1GB: mmap_flags |= MAP_HUGETLB | MAP_HUGE_1GB; break;
                default: break;
            }
#pragma GCC diagnostic pop
            arena.memory = mmap(NULL, config.size, PROT_READ | PROT_WRITE, mmap_flags, -1, 0);
            if (arena.memory == MAP_FAILED) {
                fprintf(stderr, "hgl_arena_alloc.h] Call to `mmap()` failed.\n");
                arena.memory = NULL;
            }
        } break;

        case HGL_ARENA_BUFFER_BACKED: {
            assert(config.optional_backing_buffer != NULL);
            arena.memory = config.optional_backing_buffer;
        } break;
    }

    return arena;
}

void *hgl_arena_alloc(HglArena *arena, size_t alloc_size)
{
    void *ptr = arena->memory + arena->head;

    /* Allocation too small: Return NULL */
    if (alloc_size == 0) {
        return NULL;
    }

    size_t aligned_size = alloc_size + arena->config.alignment - 1;
    if (arena->config.kind == HGL_ARENA_STACK_ALLOCATOR) {
        aligned_size += sizeof(HglArenaAllocationFooter);
    }
    aligned_size &= ~(arena->config.alignment - 1);

    /* Allocation too big: Return NULL */
    if (arena->head + aligned_size > arena->config.size) {
#ifdef HGL_ARENA_ALLOC_DEBUG_PRINTS
        printf("Arena alloc failed. Requested %lu bytes (%lu including padding + maybe footer), but "
               "only %lu remain in arena.\n", alloc_size, aligned_size, arena->config.size - arena->head);
#endif /* HGL_ARENA_ALLOC_DEBUG_PRINTS */
        return NULL;
    }

    if (arena->config.kind == HGL_ARENA_STACK_ALLOCATOR) {
        /* Write footer */
        HglArenaAllocationFooter footer = {
            .alloc_offset = arena->head
        };
        size_t footer_offset = arena->head + aligned_size - sizeof(HglArenaAllocationFooter);
        memcpy(&arena->memory[footer_offset], &footer, sizeof(HglArenaAllocationFooter));
    }

    /* Move head to nearest multiple of alignment after `head` + `alloc_size` */
    arena->head += aligned_size;

    return ptr;
}

void *hgl_arena_realloc(HglArena *arena, void *ptr, size_t alloc_size)
{
    if (arena->config.kind == HGL_ARENA_BUMP_ALLOCATOR) {
        fprintf(stderr, "hgl_arena_realloc(): invalid operation on bump-style (PUSH ONLY) arena.\n");
        abort();
    }

    hgl_arena_free(arena, ptr);
    return hgl_arena_alloc(arena, alloc_size);
}

void hgl_arena_free(HglArena *arena, void *ptr)
{
    if (arena->config.kind == HGL_ARENA_BUMP_ALLOCATOR) {
        fprintf(stderr, "hgl_arena_free(): invalid operation on bump-style (PUSH ONLY) arena.\n");
        abort();
    }

    if (arena->head == 0) {
        fprintf(stderr, "hgl_arena_free(): Arena is empty.\n"); // "except for one man"
        abort();
    }

    HglArenaAllocationFooter *last_footer = (HglArenaAllocationFooter *) 
                                            &arena->memory[arena->head - sizeof(HglArenaAllocationFooter)];

    /* ptr was not at the top of the stack */
    if (ptr != arena->memory + last_footer->alloc_offset) {
        fprintf(stderr, "hgl_arena_free(): invalid pointer.\n");
        abort();
    }

    arena->head = last_footer->alloc_offset;
}

void hgl_arena_pop(HglArena *arena)
{
    if (arena->config.kind == HGL_ARENA_BUMP_ALLOCATOR) {
        fprintf(stderr, "hgl_arena_pop(): invalid operation on bump-style (PUSH ONLY) arena.\n");
        abort();
    }

    if (arena->head == 0) {
        fprintf(stderr, "hgl_arena_pop(): Arena is empty.\n"); // "except for one man"
        abort();
    }

    HglArenaAllocationFooter *last_footer = (HglArenaAllocationFooter *) 
                                            &arena->memory[arena->head - sizeof(HglArenaAllocationFooter)];
    arena->head = last_footer->alloc_offset;
}

void hgl_arena_free_all(HglArena *arena)
{
    arena->head = 0;
}

void hgl_arena_print_usage(HglArena *arena)
{
    printf("usage: %f%% (%lu/%lu bytes).\n",
           100.0 * ((double) arena->head / (double) arena->config.size),
           arena->head, arena->config.size);
}

void hgl_arena_destroy(HglArena *arena)
{
    switch (arena->config.backend) {
        case HGL_ARENA_MALLOC: free(arena->memory); break;
        case HGL_ARENA_MMAP:
        case HGL_ARENA_MMAP_HUGEPAGE:
        case HGL_ARENA_MMAP_HUGEPAGE_2MB:
        case HGL_ARENA_MMAP_HUGEPAGE_1GB: munmap(arena->memory, arena->config.size); break;
        case HGL_ARENA_BUFFER_BACKED: break;
    }
}

#endif /* HGL_ARENA_ALLOC_IMPLEMENTATION */

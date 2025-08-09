
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
 * hgl_alloc.h implements a collection of simple to use allocators, through a common
 * API. hgl_alloc.h supports the following types of allocators:
 *
 *     - Bump allocator (arena allocator with push-only semantics)
 *     - Stack allocator (arena allocator with push-and-pop semantics)
 *     - Free stack allocator (General purpose allocator similar to a free-list allocator) 
 *     - Pool allocator (Allocator with fixed size chunks)
 *
 * Planned:
 *
 *     - Free list allocator
 *
 *
 * USAGE:
 *
 * Include hgl_alloc.h file like this:
 *
 *     #define HGL_ALLOC_IMPLEMENTATION
 *     #include "hgl_alloc.h"
 *
 * This will create the implementation of hgl_alloc in the current compilation unit. To
 * include hgl_alloc.h without creating the implementation, simply ommit the #define
 * of HGL_ALLOC_IMPLEMENTATION.
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

#ifndef HGL_ALLOC_H
#define HGL_ALLOC_H

/*--- Arena alloc-specific macros -------------------------------------------------------*/

//#define HGL_ALLOC_DEBUG_PRINTS

#define HGL_ALLOC_BUMP_ARENA_INITIALIZER(s, ...)  \
    {                                             \
        .config =  {                              \
            .kind = HGL_BUMP_ARENA,               \
            .backend = HGL_BUFFER_BACKED,         \
            .alignment = 16,                      \
            .size = (s),                          \
            __VA_ARGS__                           \
        },                                        \
        .memory = (uint8_t[(s)]){0},              \
        .head = 0,                                \
    }

#define HGL_ALLOC_STACK_ARENA_INITIALIZER(s, ...) \
    {                                             \
        .config =  {                              \
            .kind = HGL_STACK_ARENA,              \
            .backend = HGL_BUFFER_BACKED,         \
            .alignment = 16,                      \
            .size = (s),                          \
            __VA_ARGS__                           \
        },                                        \
        .memory = (uint8_t[(s)]){0},              \
        .head = 0,                                \
    }

/*--- Include files ---------------------------------------------------------------------*/

#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

/*--- Public type definitions -----------------------------------------------------------*/

typedef enum
{
    HGL_ALIGNED_ALLOC,
    HGL_MMAP,
    HGL_MMAP_HUGEPAGE,
    HGL_MMAP_HUGEPAGE_2MB,
    HGL_MMAP_HUGEPAGE_1GB,
    HGL_BUFFER_BACKED,
} HglAllocBackend;

typedef enum
{
    HGL_BUMP_ARENA,       // push-only arena
    HGL_STACK_ARENA,      // push-and-pop arena
    HGL_FREE_STACK_ALLOCATOR, // Like a free list allocator but with more fragmentation
    HGL_POOL_ALLOCATOR,       // Allocator with fixed size chunks
    HGL_N_ALLOCATOR_KINDS,
} HglAllocKind;

typedef enum
{
    HGL_OP_ALLOC             = (1u << 0),
    HGL_OP_REALLOC_IN_PLACE  = (1u << 1),
    HGL_OP_REALLOC_ARBITRARY = (1u << 2),
    HGL_OP_REALLOC           = HGL_OP_REALLOC_IN_PLACE | HGL_OP_REALLOC_ARBITRARY,
    HGL_OP_FREE              = (1u << 3),
    HGL_OP_FREE_ALL          = (1u << 4),
    HGL_OP_FREE_LAST         = (1u << 5),
} HglAllocOperation;

typedef struct
{
    struct HglFsChunkDescriptor *arr;
    int count;
    int capacity;
} HglAllocFreeStack;

typedef struct
{
    void **chunks;
    int head;
    int n_chunks;
} HglAllocPool;

typedef struct
{
    HglAllocKind kind;
    HglAllocBackend backend;
    uint8_t *optional_backing_buffer;
    uint32_t alignment;
    size_t size;
    ssize_t free_stack_capacity;
    ssize_t pool_chunk_size;
} HglAllocConfig;

typedef struct
{
    HglAllocConfig config;
    uint8_t *memory;
    union {
        size_t head;
        HglAllocFreeStack free_stack;
        HglAllocPool pool;
    };
} HglAllocator;

#define hgl_alloc_make(...) hgl_alloc_make_((HglAllocConfig){.kind = HGL_BUMP_ARENA,            \
                                                             .backend = HGL_MMAP,               \
                                                             .optional_backing_buffer = NULL,   \
                                                             .alignment = 16,                   \
                                                             .size = 0,                         \
                                                             .free_stack_capacity = -1,         \
                                                             .pool_chunk_size = -1,             \
                                                             __VA_ARGS__})
HglAllocator hgl_alloc_make_(HglAllocConfig config);
#define hgl_alloc_from_pool(allocator) hgl_alloc((allocator), (allocator)->config.pool_chunk_size)
void *hgl_alloc(HglAllocator *allocator, size_t alloc_size);
void *hgl_realloc(HglAllocator *allocator, void *ptr, size_t alloc_size);
void hgl_free(HglAllocator *allocator, void *ptr);
void hgl_free_last(HglAllocator *allocator);
void hgl_free_all(HglAllocator *allocator);
void hgl_alloc_destroy(HglAllocator *allocator);
bool hgl_alloc_supports(HglAllocator *allocator, uint32_t ops);
void hgl_alloc_print_usage(HglAllocator *allocator);
size_t hgl_alloc_usage(HglAllocator *allocator);

#endif /* HGL_ALLOC_H */

/*--- Public variables ------------------------------------------------------------------*/

/*--- Arena Alloc functions -------------------------------------------------------------*/

#ifdef HGL_ALLOC_IMPLEMENTATION

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <linux/mman.h>

typedef struct
{
    uint8_t *next_chunk;
} HglFsChunkHeader;

typedef struct HglFsChunkDescriptor
{
    uint8_t *start;
    uint8_t *end;
} HglFsChunkDescriptor;

typedef struct
{
    uintptr_t alloc_offset;
} HglAllocStackFooter;

#include <stdarg.h>
#define HGL_ALLOC_ERROR(...) hgl_alloc_print_("[hgl_alloc.h] Error: " __VA_ARGS__); abort();
#define HGL_ALLOC_WARNING(...) hgl_alloc_print_("[hgl_alloc.h] Warning: "__VA_ARGS__);
static inline void hgl_alloc_print_(const char *fmt, ...);
static inline void hgl_alloc_print_(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}

HglAllocator hgl_alloc_make_(HglAllocConfig config)
{
    HglAllocator allocator = {
        .head = 0,
        .memory = NULL,
        .config = config
    };

    if (config.alignment % 2 != 0) {
        HGL_ALLOC_WARNING("hgl_alloc_make_(): `alignment` must be a power of 2.\n");
        return allocator;
    }

    if (config.size < 1) {
        HGL_ALLOC_WARNING("hgl_alloc_make_():  `size` must be greater than 0.\n");
        return allocator;
    }

    switch (config.kind) {
        case HGL_BUMP_ARENA: break;
        case HGL_STACK_ARENA: break;

        case HGL_FREE_STACK_ALLOCATOR: {
            if (config.free_stack_capacity < 1) {
                HGL_ALLOC_WARNING("hgl_alloc_make_(): `free_stack_capacity` must be specified for HGL_FREE_STACK_ALLOCATOR.\n");
                return allocator;
            }
            if ((config.size & (config.alignment - 1)) != 0) {
                HGL_ALLOC_WARNING("hgl_alloc_make_(): `size` must be a multiple of the chosen alignment for HGL_FREE_STACK_ALLOCATOR.\n");
                return allocator;
            }
        } break;

        case HGL_POOL_ALLOCATOR: {
            if (config.pool_chunk_size < 1) {
                HGL_ALLOC_WARNING("hgl_alloc_make_(): `pool_chunk_size` must be specified for HGL_POOL_ALLOCATOR.\n");
                return allocator;
            }
            if ((config.size % config.pool_chunk_size) != 0) {
                HGL_ALLOC_WARNING("hgl_alloc_make_(): `size` must be a multiple of the chosen chunk size for HGL_POOL_ALLOCATOR.\n");
                return allocator;
            }
        } break;

        case HGL_N_ALLOCATOR_KINDS: assert(0);
    }

    switch (config.backend) {
        case HGL_ALIGNED_ALLOC: {
            allocator.memory = aligned_alloc(config.alignment, config.size);
            if (allocator.memory == NULL) {
                HGL_ALLOC_WARNING("hgl_alloc_make_(): Call to `aligned_alloc()` failed.\n");
                allocator.memory = NULL;
            }
        } break;

        case HGL_MMAP:
        case HGL_MMAP_HUGEPAGE:
        case HGL_MMAP_HUGEPAGE_2MB:
        case HGL_MMAP_HUGEPAGE_1GB: {
            int mmap_flags = MAP_PRIVATE | MAP_ANONYMOUS;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
            switch (config.backend) {
                case HGL_MMAP_HUGEPAGE:     mmap_flags |= MAP_HUGETLB; break;
                case HGL_MMAP_HUGEPAGE_2MB: mmap_flags |= MAP_HUGETLB | MAP_HUGE_2MB; break;
                case HGL_MMAP_HUGEPAGE_1GB: mmap_flags |= MAP_HUGETLB | MAP_HUGE_1GB; break;
                default: break;
            }
#pragma GCC diagnostic pop
            allocator.memory = mmap(NULL, config.size, PROT_READ | PROT_WRITE, mmap_flags, -1, 0);
            if (allocator.memory == MAP_FAILED) {
                HGL_ALLOC_WARNING("hgl_alloc_make_(): Call to `mmap()` failed.\n");
                allocator.memory = NULL;
            }
        } break;

        case HGL_BUFFER_BACKED: {
            if (config.optional_backing_buffer == NULL) {
                HGL_ALLOC_WARNING("hgl_alloc_make_(): optional_backing_buffer must be defined for allocators"
                                  "declared as HGL_BUFFER_BACKED\n");
                allocator.memory = NULL;
                return allocator;
            }
            if (((size_t)config.optional_backing_buffer & (config.alignment - 1)) != 0) {
                HGL_ALLOC_WARNING("hgl_alloc_make_(): `optional_backing_buffer` does not lie on a boundary that "
                                  "is a multiple of the chosen alignment (%d).\n", config.alignment);
                allocator.memory = NULL;
                return allocator;
            }
            allocator.memory = config.optional_backing_buffer;
        } break;
    }

    switch (config.kind) {
        case HGL_BUMP_ARENA: break;
        case HGL_STACK_ARENA: break;

        case HGL_FREE_STACK_ALLOCATOR: {
            if (config.backend == HGL_BUFFER_BACKED) {
                size_t allocatable_size = config.size - (config.free_stack_capacity *
                                                         sizeof(HglFsChunkDescriptor));
                allocator.config.size = allocatable_size;
                allocator.free_stack.arr = (HglFsChunkDescriptor *) (allocator.memory + allocatable_size);
            } else {
                allocator.free_stack.arr = malloc(config.free_stack_capacity * sizeof(HglFsChunkDescriptor));
            }

            allocator.free_stack.count = 1;
            allocator.free_stack.capacity = config.free_stack_capacity;
            allocator.free_stack.arr[0] = (HglFsChunkDescriptor) {
                .start = allocator.memory,
                .end   = allocator.memory + allocator.config.size
            };
        } break;

        case HGL_POOL_ALLOCATOR: {
            size_t n_chunks;

            if (config.backend == HGL_BUFFER_BACKED) {
                n_chunks = config.size / (config.pool_chunk_size + sizeof(void *));
                allocator.pool.chunks = (void *) (allocator.memory + (n_chunks * config.pool_chunk_size));
            } else {
                n_chunks = config.size / config.pool_chunk_size;
                allocator.pool.chunks = malloc(n_chunks * sizeof(void *));
            }

            allocator.pool.head = n_chunks - 1;
            allocator.pool.n_chunks = n_chunks;

            /* init (reset) pool */
            hgl_free_all(&allocator);
        } break;

        case HGL_N_ALLOCATOR_KINDS: assert(0);
    }

    return allocator;
}

void *hgl_alloc(HglAllocator *allocator, size_t alloc_size)
{
    /* Allocation too small: Return NULL */
    if (alloc_size == 0) {
        return NULL;
    }

    switch (allocator->config.kind) {
        case HGL_BUMP_ARENA: {
            void *ptr = allocator->memory + allocator->head;
            size_t aligned_size = alloc_size + allocator->config.alignment - 1;
            aligned_size &= ~(allocator->config.alignment - 1);

            /* Allocation too big: Return NULL */
            if (allocator->head + aligned_size > allocator->config.size) {
#ifdef HGL_ALLOC_DEBUG_PRINTS
                HGL_ALLOC_WARNING("hgl_alloc(): Allocation failed. Requested %lu bytes (%lu including "
                                  "padding), but only %lu remain in allocator.\n", alloc_size,
                                  aligned_size, allocator->config.size - allocator->head);
#endif
                return NULL;
            }

            allocator->head += aligned_size;
            return ptr;
        } break;

        case HGL_STACK_ARENA: {
            void *ptr = allocator->memory + allocator->head;

            size_t aligned_size = alloc_size + allocator->config.alignment - 1;
            aligned_size += sizeof(HglAllocStackFooter);
            aligned_size &= ~(allocator->config.alignment - 1);

            /* Allocation too big: Return NULL */
            if (allocator->head + aligned_size > allocator->config.size) {
#ifdef HGL_ALLOC_DEBUG_PRINTS
                HGL_ALLOC_WARNING("hgl_alloc(): Allocation failed. Requested %lu bytes (%lu including "
                                  "padding + footer), but only %lu remain in allocator.\n", alloc_size,
                                  aligned_size, allocator->config.size - allocator->head);
#endif
                return NULL;
            }

            /* Write footer */
            HglAllocStackFooter footer = {
                .alloc_offset = allocator->head
            };
            size_t footer_offset = allocator->head + aligned_size - sizeof(HglAllocStackFooter);
            memcpy(&allocator->memory[footer_offset], &footer, sizeof(HglAllocStackFooter));

            /* Move head to nearest multiple of alignment after `head` + `alloc_size` */
            allocator->head += aligned_size;

            return ptr;
        } break;

        case HGL_FREE_STACK_ALLOCATOR: {
            //size_t aligned_size = allocator->config.alignment +  // header + padding
            //                      (((alloc_size - 1) / allocator->config.alignment) + 1) * allocator->config.alignment;
            size_t aligned_size = alloc_size + 2*allocator->config.alignment - 1;
            aligned_size &= ~(allocator->config.alignment - 1);

            HglFsChunkDescriptor *chunk = NULL;
            int i = allocator->free_stack.count;
            while (i-- > 0) {
                chunk = &allocator->free_stack.arr[i];
                if ((i == allocator->free_stack.count - 1) && (chunk->end == chunk->start)) {
                    allocator->free_stack.count--;
                }
                if ((uintptr_t)(chunk->end - chunk->start) >= aligned_size) {
                    break;
                }
            }

            /* found no chunk large enough */
            if (i < 0) {
#ifdef HGL_ALLOC_DEBUG_PRINTS
                HGL_ALLOC_WARNING("hgl_alloc(): Allocation of size %zu failed. No large enough "
                                  "chunk of contigous memory found.\n", alloc_size);
#endif
                return NULL;
            }

            HglFsChunkHeader header = {
                .next_chunk = chunk->start + aligned_size,
            };

            uint8_t *ptr8 = chunk->start;
            memcpy(ptr8, &header, sizeof(header));
            ptr8 += allocator->config.alignment;
            chunk->start += aligned_size;

            return (void *) ptr8;
        } break;

        case HGL_POOL_ALLOCATOR: {
            if ((ssize_t)alloc_size != allocator->config.pool_chunk_size) {
#ifdef HGL_ALLOC_DEBUG_PRINTS
                HGL_ALLOC_WARNING("hgl_alloc(): `alloc_size` must be equal to "
                                  "`pool_chunk_size` for HGL_POOL_ALLOCATOR.\n", alloc_size);
#endif
                return NULL;
            }

            /* No free chunks: return NULL */
            if (allocator->pool.head < 0) {
#ifdef HGL_ALLOC_DEBUG_PRINTS
                HGL_ALLOC_WARNING("hgl_alloc(): Allocation of chunk failed. "
                                  "No free chunks left\n", alloc_size);
#endif
                return NULL;
            }

            /* Pop free chunk from stack */
            return allocator->pool.chunks[allocator->pool.head--];
        } break;

        case HGL_N_ALLOCATOR_KINDS: assert(0);
    }

    return NULL;
}

void *hgl_realloc(HglAllocator *allocator, void *ptr, size_t alloc_size)
{
    switch (allocator->config.kind) {
        case HGL_BUMP_ARENA: {
            HGL_ALLOC_ERROR("hgl_realloc(): invalid operation on HGL_BUMP_ARENA.\n");
        } break;

        case HGL_STACK_ARENA: {
            hgl_free(allocator, ptr);
            return hgl_alloc(allocator, alloc_size);
        } break;

        case HGL_FREE_STACK_ALLOCATOR: {
            // TODO I haven't really though about this too hard..
            uint8_t *ptr8 = (uint8_t *) ptr;

            if (ptr8 == NULL) {
                return hgl_alloc(allocator, alloc_size);        
            }

            if ((ptr8 < allocator->memory) || (ptr8 > (allocator->memory + allocator->config.size))) {
                HGL_ALLOC_ERROR("hgl_realloc(): Invalid pointer. Pointer ptr=%p is outside the valid range [%p, %p].\n", 
                        ptr, allocator->memory, allocator->memory + allocator->config.size);
            }

            if (((uintptr_t)ptr8 & (allocator->config.alignment - 1)) != 0) {
                HGL_ALLOC_ERROR("hgl_realloc(): Invalid pointer. Pointer ptr=%p has incorrect alignment (%d).\n", 
                        ptr, allocator->config.alignment);
            }

            ptr8 -= allocator->config.alignment;
            HglFsChunkHeader *header = (HglFsChunkHeader *) ptr8;
            size_t current_allocation_size = header->next_chunk - (uint8_t *)ptr;

            /* alloc new & copy data there */
            void *newptr = hgl_alloc(allocator, alloc_size);
            memcpy(newptr, ptr, current_allocation_size);

            /* free old chunk */
            hgl_free(allocator, ptr);

            return newptr; 
        } break;

        case HGL_POOL_ALLOCATOR: {
            HGL_ALLOC_ERROR("hgl_realloc(): invalid operation on HGL_POOL_ALLOCATOR.\n");
        } break;

        case HGL_N_ALLOCATOR_KINDS: assert(0);
    }

    return NULL;
}

void hgl_free(HglAllocator *allocator, void *ptr)
{
    if (ptr == NULL) {
        return;
    }

    switch (allocator->config.kind) {
        case HGL_BUMP_ARENA: {
            HGL_ALLOC_ERROR("hgl_free(): invalid operation on HGL_BUMP_ARENA.\n");
        } break;

        case HGL_STACK_ARENA: {
            if (allocator->head == 0) {
                HGL_ALLOC_ERROR("hgl_free(): Invalid pointer. Arena is empty.\n"); // "except for one man"
            }

            HglAllocStackFooter *last_footer = (HglAllocStackFooter *)
                                                &allocator->memory[allocator->head - sizeof(HglAllocStackFooter)];

            /* ptr was not at the top of the stack */
            if (ptr != allocator->memory + last_footer->alloc_offset) {
                HGL_ALLOC_ERROR("hgl_free(): invalid pointer. `ptr` is not at the top of the stack.\n");
            }

            allocator->head = last_footer->alloc_offset;
        } break;

        case HGL_FREE_STACK_ALLOCATOR: {
            uint8_t *ptr8 = (uint8_t *) ptr;

            if ((ptr8 < allocator->memory) || (ptr8 > (allocator->memory + allocator->config.size))) {
                HGL_ALLOC_ERROR("hgl_free(): Invalid pointer. Pointer ptr=%p is outside the valid range [%p, %p].\n",
                        ptr, allocator->memory, allocator->memory + allocator->config.size);
            }

            if (((uintptr_t)ptr8 & (allocator->config.alignment - 1)) != 0) {
                HGL_ALLOC_ERROR("hgl_free(): Invalid pointer. Pointer ptr=%p has incorrect alignment (%d).\n",
                        ptr, allocator->config.alignment);
            }

            ptr8 -= allocator->config.alignment;
            HglFsChunkHeader *header = (HglFsChunkHeader *) ptr8;

            // TODO coalesce in a while-loop?
            if (allocator->free_stack.arr[allocator->free_stack.count - 1].start == header->next_chunk) {
                allocator->free_stack.arr[allocator->free_stack.count - 1].start = ptr8;
            } else if (allocator->free_stack.arr[allocator->free_stack.count - 1].end == ptr8) {
                allocator->free_stack.arr[allocator->free_stack.count - 1].end = header->next_chunk;
            } else {
                if (allocator->free_stack.count + 1 > allocator->free_stack.capacity) {
                    HGL_ALLOC_ERROR("hgl_free(): Free stack ran out of space. Consider increasing `free_stack_capacity`\n");
                }

                allocator->free_stack.arr[allocator->free_stack.count] = (HglFsChunkDescriptor) {
                    .start = ptr8,
                    .end   = header->next_chunk,
                };

                allocator->free_stack.count++;
            }
        } break;

        case HGL_POOL_ALLOCATOR: {
            uint8_t *ptr8 = (uint8_t *) ptr;

            /* Invalid ptr */
            if ((ptr8 < allocator->memory) ||
                (ptr8 > allocator->memory + (allocator->pool.n_chunks - 1) * allocator->config.pool_chunk_size)) {
                HGL_ALLOC_ERROR("hgl_free(): Invalid ptr (not in range of pool addresses).\n");
            }

            /* Nothing to free */
            if (allocator->pool.head > (ssize_t)allocator->pool.n_chunks - 1) {
                HGL_ALLOC_ERROR("hgl_free(): Invalid ptr (No unfreed chunks in pool).\n");
            }

            /* Push free chunk onto stack */
            allocator->pool.chunks[++(allocator->pool.head)] = ptr;
        } break;

        case HGL_N_ALLOCATOR_KINDS: assert(0);
    }
}

void hgl_free_last(HglAllocator *allocator)
{
    switch (allocator->config.kind) {
        case HGL_BUMP_ARENA: {
            HGL_ALLOC_ERROR("hgl_free_last(): invalid operation on HGL_BUMP_ARENA.\n");
        } break;

        case HGL_STACK_ARENA: {
            if (allocator->head == 0) {
                HGL_ALLOC_ERROR("hgl_free_last(): Arena is empty.\n"); // "except for one man"
            }
            HglAllocStackFooter *last_footer = (HglAllocStackFooter *)
                                                &allocator->memory[allocator->head - sizeof(HglAllocStackFooter)];
            allocator->head = last_footer->alloc_offset;
        } break;

        case HGL_FREE_STACK_ALLOCATOR: {
            HGL_ALLOC_ERROR("hgl_free_last(): invalid operation on HGL_FREE_STACK_ALLOCATOR.\n");
        } break;

        case HGL_POOL_ALLOCATOR: {
            HGL_ALLOC_ERROR("hgl_free_last(): invalid operation on HGL_POOL_ALLOCATOR.\n");
        } break;

        case HGL_N_ALLOCATOR_KINDS: assert(0);
    }

}

void hgl_free_all(HglAllocator *allocator)
{
    switch (allocator->config.kind) {
        case HGL_BUMP_ARENA:
        case HGL_STACK_ARENA: {
            allocator->head = 0;
        } break;

        case HGL_FREE_STACK_ALLOCATOR: {
            allocator->free_stack.count = 1;
            allocator->free_stack.arr[0].start = allocator->memory;
            allocator->free_stack.arr[0].end   = allocator->memory + allocator->config.size;
        } break;

        case HGL_POOL_ALLOCATOR: {
            int idx = 0;
            allocator->pool.head = allocator->pool.n_chunks - 1;
            for (int i = allocator->pool.head; i >= 0; i--) {
                allocator->pool.chunks[idx++] = allocator->memory + (i * allocator->config.pool_chunk_size);
            }
        } break;

        case HGL_N_ALLOCATOR_KINDS: assert(0);
    }
}

void hgl_alloc_destroy(HglAllocator *allocator)
{
    switch (allocator->config.backend) {
        case HGL_ALIGNED_ALLOC: free(allocator->memory); break;
        case HGL_MMAP:
        case HGL_MMAP_HUGEPAGE:
        case HGL_MMAP_HUGEPAGE_2MB:
        case HGL_MMAP_HUGEPAGE_1GB: munmap(allocator->memory, allocator->config.size); break;
        case HGL_BUFFER_BACKED: break;
    }

    switch (allocator->config.kind) {
        case HGL_BUMP_ARENA: break;
        case HGL_STACK_ARENA: break;
        case HGL_FREE_STACK_ALLOCATOR: {
            if (allocator->config.backend != HGL_BUFFER_BACKED) {
                free(allocator->free_stack.arr);
            }
        } break;
        case HGL_POOL_ALLOCATOR: {
            if (allocator->config.backend != HGL_BUFFER_BACKED) {
                free(allocator->pool.chunks);
            }
        } break;
        case HGL_N_ALLOCATOR_KINDS: assert(0);
    }
}

bool hgl_alloc_supports(HglAllocator *allocator, uint32_t ops)
{
    static const uint32_t SUPPORTED_OPS[HGL_N_ALLOCATOR_KINDS] = {
        [HGL_BUMP_ARENA]           = HGL_OP_ALLOC | HGL_OP_FREE_ALL,
        [HGL_STACK_ARENA]          = HGL_OP_ALLOC | HGL_OP_FREE_ALL | HGL_OP_REALLOC_IN_PLACE | HGL_OP_FREE_LAST,
        [HGL_FREE_STACK_ALLOCATOR] = HGL_OP_ALLOC | HGL_OP_FREE_ALL | HGL_OP_REALLOC | HGL_OP_FREE,
        [HGL_POOL_ALLOCATOR]       = HGL_OP_ALLOC | HGL_OP_FREE_ALL | HGL_OP_FREE,
    };

    return (ops & SUPPORTED_OPS[allocator->config.kind]) == ops;
}

void hgl_alloc_print_usage(HglAllocator *allocator)
{
    switch (allocator->config.kind) {
        case HGL_BUMP_ARENA:
        case HGL_STACK_ARENA: {
            printf("usage: %f%% (%lu/%lu bytes).\n",
                   100.0 * ((double) allocator->head / (double) allocator->config.size),
                   allocator->head, allocator->config.size);
        } break;

        case HGL_FREE_STACK_ALLOCATOR: {
            size_t total = allocator->config.size;
            size_t used = total;
            HglFsChunkDescriptor *chunk = NULL;
            int i = allocator->free_stack.count;
            while (i-- > 0) {
                chunk = &allocator->free_stack.arr[i];
                used -= chunk->end - chunk->start;
            }
            printf("usage: %f%% (%zu/%zu bytes). Free stack: %f%% (%d/%ld entries).\n",
                   100.0 * ((double)used / (double)total), used, total,
                   100.0 * ((double)allocator->free_stack.count / (double)allocator->config.free_stack_capacity),
                   allocator->free_stack.count, allocator->config.free_stack_capacity);
        } break;

        case HGL_POOL_ALLOCATOR: {
            int used = allocator->pool.n_chunks - allocator->pool.head - 1;
            int total = allocator->pool.n_chunks;
            printf("usage: %f%% (%d/%d chunks á %ld bytes).\n",
                   100.0 * ((double)used / (double)total), used, total,
                   allocator->config.pool_chunk_size);

        } break;

        case HGL_N_ALLOCATOR_KINDS: assert(0);
    }
}

size_t hgl_alloc_usage(HglAllocator *allocator)
{
    switch (allocator->config.kind) {
        case HGL_BUMP_ARENA:
        case HGL_STACK_ARENA: {
            return allocator->head;
        } break;

        case HGL_FREE_STACK_ALLOCATOR: {
            size_t used = allocator->config.size;
            HglFsChunkDescriptor *chunk = NULL;
            int i = allocator->free_stack.count;
            while (i-- > 0) {
                chunk = &allocator->free_stack.arr[i];
                used -= chunk->end - chunk->start;
            }
            return used;
        } break;

        case HGL_POOL_ALLOCATOR: {
            return allocator->pool.n_chunks - allocator->pool.head - 1;
        } break;

        case HGL_N_ALLOCATOR_KINDS: assert(0);
    }

    return 0;
}


#endif /* HGL_ALLOC_IMPLEMENTATION */

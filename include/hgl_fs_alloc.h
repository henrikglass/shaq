
/**
 * LICENSE:
 *
 * MIT License
 *
 * Copyright (c) 2024 Henrik A. Glass
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
 * hgl_fs_alloc.h implements a variant on the free list allocator. 
 *
 * Instead of a linked list or red-black tree, hgl_fs_alloc.h uses a stack to 
 * keep track of free chunks. Coalescing of neighbouring chunks is done lazily 
 * upon calls to hgl_fs_free (or hgl_fs_realloc) - and only if they occur 
 * consecutively in the "free stack". The use of a stack should provide some 
 * benefits to performance. However, the lazy coalescing behavior means that 
 * there is a risk that some neighbouring chunks may never be coalesced. To get
 * descent coalescing behavior, it is recommended to to free chunks of memory in
 * a similar order to how they were allocated.
 *
 * The indended use of this memory allocator is in cases where a regular arena 
 * or stack allocator would be used, but where reallocing is required.
 *
 *
 * USAGE:
 *
 * Include hgl_fs_alloc.h file like this:
 *
 *     #define HGL_FS_ALLOC_IMPLEMENTATION
 *     #include "hgl_fs_alloc.h"
 *
 * hgl_fs_alloc.h allows the user to define the alignment of allocations from its 
 * internal slice of memory by redefining HGL_FS_ALLOC_ALIGNMENT, as such:
 *
 *     #define HGL_FS_ALLOC_ALIGNMENT 64
 *
 * Below is a code example:
 *
 *     HglFsAllocator allocator = hgl_fs_make(16*1024, 16);
 *
 *         /.../
 *     
 *     int *a = hgl_fs_alloc(&allocator, sizeof(int));
 *     int *b = hgl_fs_alloc(&allocator, 10 * sizeof(int));
 *     int *c = hgl_fs_alloc(&allocator, 20 * sizeof(int));
 *     hgl_fs_free(&allocator, b);
 *     b = hgl_fs_alloc(&allocator, 8);
 *
 *         /.../
 *     
 *     hgl_fs_destroy(&allocator);
 *
 *
 * AUTHOR: Henrik A. Glass
 *
 */


#ifndef HGL_FS_ALLOC_H
#define HGL_FS_ALLOC_H

/*--- Include files ---------------------------------------------------------------------*/

#include <stdint.h>
#include <stddef.h>

/*--- Public macros ---------------------------------------------------------------------*/

/*--- Public type definitions -----------------------------------------------------------*/

typedef struct
{
    uint8_t *memory;
    size_t size;
    struct HglFsChunkDescriptor *free_stack;
    int free_count;
    int free_capacity;
} HglFsAllocator;

/*--- Public variables ------------------------------------------------------------------*/

/*--- Public function prototypes --------------------------------------------------------*/

/**
 * Creates a new HglFsAllocator with `size` bytes of allocatable memory, and 
 * a free stack capacity of `free_stack_capacity`.
 */
HglFsAllocator hgl_fs_make(size_t size, int free_stack_capacity);

/**
 * creates a new hglfsallocator from a buffer `buf` of size `size`, and a free
 * stack capactiy of `free_stack_capacity`. note: the free stack is stored at 
 * the end of the buffer, so not all of `size` will be allocatable.
 */
HglFsAllocator hgl_fs_make_from_buffer(void *buf, size_t size, int free_stack_capacity);

/**
 * Frees the memory allocated for `allocator`. This function may only be called 
 * on HglFsAllocator instances that has been created through calls to 
 * `hgl_fs_make`.
 */
void hgl_fs_destroy(HglFsAllocator *allocator);

/**
 * Allocates `size` bytes of memory from `allocator`.
 */
void *hgl_fs_alloc(HglFsAllocator *allocator, size_t size);

/**
 * Reallocates `ptr` to a chunk of size `size` from `allocator`.
 */
void *hgl_fs_realloc(HglFsAllocator *allocator, void *ptr, size_t size);

/**
 * Frees `ptr` from `allocator`. Note: `ptr` must be the result of a call to
 * `hgl_fs_alloc`.
 */
void hgl_fs_free(HglFsAllocator *allocator, void *ptr);

/**
 * Frees all allocations in `allocator`.
 */
void hgl_fs_free_all(HglFsAllocator *allocator);

/**
 * Print fs allocator memory usage.
 */
void hgl_fs_print_usage(HglFsAllocator *allocator);

#endif /* HGL_FS_ALLOC_H */

#ifdef HGL_FS_ALLOC_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#ifndef HGL_FS_ALLOC_ALIGNMENT
#define HGL_FS_ALLOC_ALIGNMENT 16
#endif

typedef struct HglFsChunkDescriptor
{
    uint8_t *start;
    uint8_t *end;
} HglFsChunkDescriptor;

typedef struct
{
    uint8_t *next_chunk;
} HglFsChunkHeader;


static_assert(sizeof(HglFsChunkHeader) == 8, "");
static_assert(sizeof(HglFsChunkHeader) <= HGL_FS_ALLOC_ALIGNMENT,
              "[hgl_fs_alloc.h] Alignment too small.");
static_assert((HGL_FS_ALLOC_ALIGNMENT & (HGL_FS_ALLOC_ALIGNMENT - 1)) == 0,
              "[hgl_fs_alloc.h] Alignment not a power of 2.");


HglFsAllocator hgl_fs_make(size_t size, int free_stack_capacity)
{

    if (free_stack_capacity < 1) {
        fprintf(stderr, "[hgl_fs_make] `free_stack_capacity` is too small (%d).\n",
                free_stack_capacity);
        return (HglFsAllocator) {0};
    }

    if ((size & (HGL_FS_ALLOC_ALIGNMENT - 1)) != 0) {
        fprintf(stderr, "[hgl_fs_make] `size` is not a multiple of the chosen alignment (%d).\n",
                HGL_FS_ALLOC_ALIGNMENT);
        return (HglFsAllocator) {0};
    }

    HglFsAllocator allocator = {
        .memory        = aligned_alloc(HGL_FS_ALLOC_ALIGNMENT, size),
        .size          = size,
        .free_stack    = malloc(free_stack_capacity * sizeof(HglFsChunkDescriptor)),
        .free_count    = 1,
        .free_capacity = free_stack_capacity,
    };

    assert(allocator.memory != NULL);
    assert(allocator.free_stack != NULL);

    allocator.free_stack[0] = (HglFsChunkDescriptor) {
        .start = allocator.memory,
        .end   = allocator.memory + size,
    };

    return allocator;
}

HglFsAllocator hgl_fs_make_from_buffer(void *buf, size_t size, int free_stack_capacity)
{
    uint8_t *buf8 = (uint8_t *) buf;

    if (((size_t)buf8 & (HGL_FS_ALLOC_ALIGNMENT - 1)) != 0) {
        fprintf(stderr, "[hgl_fs_make] `buf` does not lie on a boundary that "
                "is a multiple of the chosen alignment (%d).\n", HGL_FS_ALLOC_ALIGNMENT);
        return (HglFsAllocator) {0};
    }

    assert((free_stack_capacity * sizeof(HglFsChunkDescriptor)) < size && "Free stack capacity too big");
    size_t allocable_size = size - (free_stack_capacity * sizeof(HglFsChunkDescriptor));

    HglFsAllocator allocator = {
        .memory        = buf8,
        .size          = allocable_size,
        .free_stack    = (HglFsChunkDescriptor *) (buf8 + allocable_size),
        .free_count    = 1,
        .free_capacity = free_stack_capacity,
    };

    allocator.free_stack[0] = (HglFsChunkDescriptor) {
        .start = allocator.memory,
        .end   = allocator.memory + allocable_size,
    };

    return allocator;
}

void hgl_fs_destroy(HglFsAllocator *allocator)
{
    free(allocator->memory);
    free(allocator->free_stack);
}

void *hgl_fs_alloc(HglFsAllocator *allocator, size_t size)
{
    if (size == 0) {
        return NULL;
    }

    size_t total_size = HGL_FS_ALLOC_ALIGNMENT +  // header + padding
                        (((size - 1) / HGL_FS_ALLOC_ALIGNMENT) + 1) * HGL_FS_ALLOC_ALIGNMENT;

    HglFsChunkDescriptor *chunk = NULL;
    int i = allocator->free_count;
    while (i-- > 0) {
        chunk = &allocator->free_stack[i];
        if ((i == allocator->free_count - 1) && (chunk->end == chunk->start)) {
            allocator->free_count--;
        }
        if ((uintptr_t)(chunk->end - chunk->start) >= total_size) {
            break;
        }
    }

    /* found no chunk large enough */
    if (i < 0) {
        fprintf(stderr, "[hgl_fs_alloc] allocation of size %zu failed. No large enough "
                "chunk of contigous memory found.\n", size);
        return NULL;
    }

    HglFsChunkHeader header = {
        .next_chunk = chunk->start + total_size,
    };

    uint8_t *ptr8 = chunk->start;
    memcpy(ptr8, &header, sizeof(header));
    ptr8 += HGL_FS_ALLOC_ALIGNMENT;
    chunk->start += total_size;

    return (void *) ptr8;
}

void *hgl_fs_realloc(HglFsAllocator *allocator, void *ptr, size_t size)
{
    // TODO I haven't really though about this too hard..
    uint8_t *ptr8 = (uint8_t *) ptr;

    if (ptr8 == NULL) {
        return hgl_fs_alloc(allocator, size);        
    }

    if ((ptr8 < allocator->memory) || (ptr8 > (allocator->memory + allocator->size))) {
        fprintf(stderr, "[hgl_fs_free] Invalid pointer. Pointer ptr=%p is outside the valid range [%p, %p].\n", 
                ptr, allocator->memory, allocator->memory + allocator->size);
        assert(0);
    }
    
    if (((uintptr_t)ptr8 & (HGL_FS_ALLOC_ALIGNMENT - 1)) != 0) {
        fprintf(stderr, "[hgl_fs_free] Invalid pointer. Pointer ptr=%p has incorrect alignment (%d).\n", 
                ptr, HGL_FS_ALLOC_ALIGNMENT);
        assert(0);
    }

    ptr8 -= HGL_FS_ALLOC_ALIGNMENT;
    HglFsChunkHeader *header = (HglFsChunkHeader *) ptr8;
    size_t current_allocation_size = header->next_chunk - (uint8_t *)ptr;
    //printf("current_allocation_size = %zu\n", current_allocation_size);

    /* alloc new & copy data there */
    void *newptr = hgl_fs_alloc(allocator, size);
    memcpy(newptr, ptr, current_allocation_size);

    /* free old chunk */
    hgl_fs_free(allocator, ptr);

    return newptr;
}

void hgl_fs_free(HglFsAllocator *allocator, void *ptr)
{
    uint8_t *ptr8 = (uint8_t *) ptr;

    if (ptr8 == NULL) {
        return;
    }

    if ((ptr8 < allocator->memory) || (ptr8 > (allocator->memory + allocator->size))) {
        fprintf(stderr, "[hgl_fs_free] Invalid pointer. Pointer ptr=%p is outside the valid range [%p, %p].\n", 
                ptr, allocator->memory, allocator->memory + allocator->size);
        assert(0);
    }
    
    if (((uintptr_t)ptr8 & (HGL_FS_ALLOC_ALIGNMENT - 1)) != 0) {
        fprintf(stderr, "[hgl_fs_free] Invalid pointer. Pointer ptr=%p has incorrect alignment (%d).\n", 
                ptr, HGL_FS_ALLOC_ALIGNMENT);
        assert(0);
    }

    ptr8 -= HGL_FS_ALLOC_ALIGNMENT;
    HglFsChunkHeader *header = (HglFsChunkHeader *) ptr8;

    // TODO coalesce in a while-loop? 
    if (allocator->free_stack[allocator->free_count - 1].start == header->next_chunk) {
        allocator->free_stack[allocator->free_count - 1].start = ptr8;
    } else if (allocator->free_stack[allocator->free_count - 1].end == ptr8) {
        allocator->free_stack[allocator->free_count - 1].end = header->next_chunk;
    } else {
        if (allocator->free_count + 1 > allocator->free_capacity) {
            assert(0 && "Not pessimistic enough");
        }

        allocator->free_stack[allocator->free_count] = (HglFsChunkDescriptor) {
            .start = ptr8,
            .end   = header->next_chunk,
        };

        allocator->free_count++;
    }
}

void hgl_fs_free_all(HglFsAllocator *allocator)
{
    allocator->free_count = 1;
    allocator->free_stack[0].start = allocator->memory;
    allocator->free_stack[0].end = allocator->memory + allocator->size;
}

void hgl_fs_print_usage(HglFsAllocator *allocator)
{
    size_t usage = allocator->size;
    int i = allocator->free_count;
    while (i-- > 0) {
        HglFsChunkDescriptor *chunk = &allocator->free_stack[i];
        usage -= (chunk->end - chunk->start);
    }
    printf("usage: %f%% (%lu/%lu bytes).\n",
           100.0 * ((double) usage / (double) allocator->size),
           usage, allocator->size);
}

#endif






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
 * hgl_stack_alloc.h implements a simple to use stack allocator (ontop of hgl_arena_alloc).
 *
 *
 * USAGE:
 *
 * Include hgl_stack_alloc.h file like this:
 *
 *     #define HGL_STACK_ALLOC_IMPLEMENTATION
 *     #include "hgl_stack_alloc.h"
 *
 * This will create the implementation of hgl_stack_alloc in the current compilation unit. To
 * include hgl_stack_alloc.h without creating the implementation, simply ommit the #define
 * of HGL_STACK_ALLOC_IMPLEMENTATION. Note that hgl_stack_alloc.h includes hgl_arena_alloc.h
 * internally, since it can be considered as an extension of hgl_arena_alloc.h and utilizes
 * some of its constructs. Make sure to copy both files into your project. When including
 * hgl_arena_alloc.h, hgl_stack_alloc will try to be clever about whether
 * HGL_ARENA_ALLOC_IMPLEMENTATION is defined or not.
 *
 * Below is a complet listing of the API:
 *
 * void *hgl_stack_alloc(HglArena *arena, size_t alloc_size);
 * void *hgl_stack_realloc(HglArena *arena, void *ptr, size_t alloc_size);
 * void hgl_stack_free(HglArena *arena, void *ptr);
 * void hgl_stack_assert_canaries_alive(HglArena *arena); // requires HGL_STACK_ALLOC_ENABLE_CANARIES
 *
 * Since hgl_stack_alloc builds ontop of hgl_arena_alloc, the alignment can be defined in the
 * same way as for hgl_arena_alloc:
 *
 *     #define HGL_ARENA_ALIGNMENT 64
 *
 * A few other useful things that may be defined by the user:
 *
 *     #define HGL_STACK_ALLOC_DEBUG_PRINTS
 *     #define HGL_STACK_ALLOC_ENABLE_CANARIES
 *
 * HGL_STACK_ALLOC_DEBUG_PRINTS Enables a few useful debug prints.
 *
 * HGL_STACK_ALLOC_ENABLE_CANARIES Places canaries at the boundaries between allocations;
 * more precisely, inside each allocation footer. Some rudimentary integrity checks of
 * the canaries are performed for calls to hgl_stack_alloc, *_realloc, and *_free.
 * hgl_stack_assert_canaries_alive may be called manually to check the integrity of all
 * allocation boundaries.
 *
 * AUTHOR: Henrik A. Glass
 *
 */

#ifndef HGL_STACK_ALLOC_H
#define HGL_STACK_ALLOC_H

/*--- stack alloc-specific macros -------------------------------------------------------*/

//#define HGL_STACK_ALLOC_DEBUG_PRINTS
//#define HGL_STACK_ALLOC_ENABLE_CANARIES

/*--- Include files ---------------------------------------------------------------------*/

#include "hgl_arena_alloc.h"

/*--- Public type definitions -----------------------------------------------------------*/

/**
 * Allocate a chunk of `alloc_size` bytes from `arena` using .
 */
void *hgl_stack_alloc(HglArena *arena, size_t alloc_size);

/**
 * Reallocate `ptr` with a new size `alloc_size`. `ptr` must be the result of
 * the last allocation from `hgl_stack_alloc`.
 */
void *hgl_stack_realloc(HglArena *arena, void *ptr, size_t alloc_size);

/**
 * Free `ptr` from `arena`. `ptr` must be the result of the last allocation
 * from `hgl_stack_alloc`.
 */
void hgl_stack_free(HglArena *arena, void *ptr);

/**
 * Check the integrity of all allocation boundary canaries. Aborts the program if a
 * corrupted canary is detected.
 */
void hgl_stack_assert_canaries_alive(HglArena *arena);

#endif /* HGL_STACK_ALLOC_H */

/*--- Public variables ------------------------------------------------------------------*/

/*--- Stack Alloc functions -------------------------------------------------------------*/

#ifdef HGL_STACK_ALLOC_IMPLEMENTATION

#include <string.h>
#include <stdio.h>

#ifdef HGL_STACK_ALLOC_ENABLE_CANARIES
#include <execinfo.h>
#include <endian.h>
#include <stdbool.h>
#endif

static_assert(sizeof(void *) == 8);
#define HGL_STACK_ALLOC_CANARY_ htobe64(0xBEEFD00DBEEFBABEu)

typedef struct
{
#ifdef HGL_STACK_ALLOC_ENABLE_CANARIES
    uintptr_t canary;
#endif
    uintptr_t alloc_offset;
} HglStackFooter;

#ifdef HGL_STACK_ALLOC_ENABLE_CANARIES
void hgl_stack_assert_canary_alive_(uintptr_t canary, const char *error_msg);
void hgl_stack_assert_canary_alive_(uintptr_t canary, const char *error_msg)
{
    if (canary == HGL_STACK_ALLOC_CANARY_) {
        return; // canary alive
    }

    fprintf(stderr, "%s.\n", error_msg);
    fprintf(stderr, "expected canary value: %016zX\n", be64toh(HGL_STACK_ALLOC_CANARY_));
    fprintf(stderr, "actual canary value:   %016zX\n", be64toh(canary));
    fprintf(stderr, "                       ^^            ^^\n");
    fprintf(stderr, "                    low byte     high byte\n");

    void *array[32];
    char **strings;

    int size = backtrace (array, 32);
    strings = backtrace_symbols (array, size);
    //backtrace_symbols_fd(array, size, STDERR_FILENO);
    if (strings != NULL) {
        printf ("Stack trace (compile with -rdynamic to enable symbols): \n");
        for (int i = 0; i < size; i++) {
            printf ("  [%d] %s\n", i, strings[i]);
        }
    }

    free(strings);
    abort();
}
#endif

void *hgl_stack_alloc(HglArena *arena, size_t alloc_size)
{
    void *ptr = arena->memory + arena->head;
    size_t size_with_footer = alloc_size + sizeof(HglStackFooter);
    size_t aligned_size     = (size_with_footer + HGL_ARENA_ALIGNMENT - 1)
                              & ~(HGL_ARENA_ALIGNMENT - 1);

    /* Allocation too small: Return NULL */
    if (alloc_size == 0) {
        return NULL;
    }

    /* Allocation too big: Return NULL */
    if (arena->head + aligned_size > arena->size) {
#ifdef HGL_STACK_ALLOC_DEBUG_PRINTS
        printf("Stack alloc failed. Requested %lu bytes (including footer + padding),"
               "but only %lu remain in arena.\n", aligned_size, arena->size - arena->head);
#endif /* HGL_ARENA_ALLOC_DEBUG_PRINTS */
        return NULL;
    }

#ifdef HGL_STACK_ALLOC_ENABLE_CANARIES
    /* Check if footer of last allocation is intact */
    if (arena->head != 0) {
        HglStackFooter *last_footer = (HglStackFooter *) &arena->memory[arena->head - sizeof(HglStackFooter)];
        hgl_stack_assert_canary_alive_(last_footer->canary, "hgl_stack_alloc(): "
                                       "Detected corrupted canary in last allocation footer");
    }
#endif

    /* Write footer */
    HglStackFooter footer = {
#ifdef HGL_STACK_ALLOC_ENABLE_CANARIES
        .canary = HGL_STACK_ALLOC_CANARY_,
#endif
        .alloc_offset = arena->head
    };
    size_t footer_offset = arena->head + aligned_size - sizeof(HglStackFooter);
    memcpy(&arena->memory[footer_offset], &footer, sizeof(HglStackFooter));

    /* Update head */
    arena->head += aligned_size;

#ifdef HGL_STACK_ALLOC_DEBUG_PRINTS
    printf("Arena usage: %f%% of %lu KiB\n",
           100.0 * ((double) arena->head / (double) arena->size),
           (arena->size / 1024));
#endif /* HGL_ARENA_ALLOC_DEBUG_PRINTS */

    /* return ptr to chunk */
    return ptr;
}

void *hgl_stack_realloc(HglArena *arena, void *ptr, size_t alloc_size)
{
    hgl_stack_free(arena, ptr);
    return hgl_stack_alloc(arena, alloc_size);
}

void hgl_stack_free(HglArena *arena, void *ptr)
{

    if (arena->head == 0) {
        fprintf(stderr, "hgl_stack_free(): Arena is empty.\n"); // "except for one man"
        abort();
    }

    HglStackFooter *last_footer = (HglStackFooter *) &arena->memory[arena->head - sizeof(HglStackFooter)];

#ifdef HGL_STACK_ALLOC_ENABLE_CANARIES
    hgl_stack_assert_canary_alive_(last_footer->canary, "hgl_stack_free(): "
                                   "Detected corrupted canary in allocation footer");
#endif

    /* ptr was not the last thing alloced */
    if (ptr != arena->memory + last_footer->alloc_offset) {
        fprintf(stderr, "hgl_stack_free(): invalid pointer.\n");
        abort();
    }

    arena->head = last_footer->alloc_offset;
}

void hgl_stack_assert_canaries_alive(HglArena *arena)
{
#ifdef HGL_STACK_ALLOC_ENABLE_CANARIES
    if (arena->head == 0) {
        return;
    }

    HglStackFooter *last_footer = (HglStackFooter *) &arena->memory[arena->head - sizeof(HglStackFooter)];
    while (true) {
        hgl_stack_assert_canary_alive_(last_footer->canary, "hgl_stack_assert_canaries_alive(): "
                                       "Detected corrupted canary in allocation footer");

        if (last_footer->alloc_offset == 0) {
            return;
        }

        last_footer = (HglStackFooter *) &arena->memory[last_footer->alloc_offset - sizeof(HglStackFooter)];
    }
#else
#ifdef HGL_STACK_ALLOC_DEBUG_PRINTS
    printf("hgl_stack_assert_canaries_alive(): Canaries are disabled.\n");
#endif /* HGL_ARENA_ALLOC_DEBUG_PRINTS */
    (void) arena;
    return;
#endif
}

#endif /* HGL_STACK_ALLOC_IMPLEMENTATION */


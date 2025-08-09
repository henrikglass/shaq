/*--- Include files ---------------------------------------------------------------------*/

#include "alloc.h"

#include "hgl_int.h"

#define HGL_ALLOC_IMPLEMENTATION
#include "hgl_alloc.h"

#include <assert.h>

static HglAllocator frame_arena_internal_ = {0};
static HglAllocator session_arena_internal_ = {0};
static HglAllocator session_fs_allocator_internal_ = {0};

HglAllocator *g_frame_arena          = NULL;
HglAllocator *g_session_arena        = NULL;
HglAllocator *g_session_fs_allocator = NULL;

void alloc_init()
{
    frame_arena_internal_          = hgl_alloc_make(.kind = HGL_BUMP_ARENA, 
                                                    .size = 1024*1024);    //   1 MiB
    session_arena_internal_        = hgl_alloc_make(.kind = HGL_STACK_ARENA, 
                                                    .size = 16*1024*1024); //  16 MiB
    session_fs_allocator_internal_ = hgl_alloc_make(.kind = HGL_FREE_STACK_ALLOCATOR,
                                                    .size = 256*1024*1024, // 256 MiB
                                                    .free_stack_capacity = 1024);

    assert(session_fs_allocator_internal_.memory != NULL);

    g_frame_arena = &frame_arena_internal_;
    g_session_arena = &session_arena_internal_;
    g_session_fs_allocator = &session_fs_allocator_internal_;
}

void alloc_final()
{
    hgl_free_all(g_frame_arena);
    hgl_free_all(g_session_arena);
    hgl_free_all(g_session_fs_allocator);
    hgl_alloc_destroy(g_frame_arena);
    hgl_alloc_destroy(g_session_arena);
    hgl_alloc_destroy(g_session_fs_allocator);
}

void *frame_arena_alloc(size_t size)                { return hgl_alloc(g_frame_arena, size); }
void *session_arena_alloc(size_t size)              { return hgl_alloc(g_session_arena, size); }
void *session_arena_realloc(void *ptr, size_t size) { return hgl_realloc(g_session_arena, ptr, size); }
void  session_arena_free(void *ptr)                 { hgl_free(g_session_arena, ptr); }
void *session_fs_alloc(size_t size)                 { return hgl_alloc(g_session_fs_allocator, size); }
void *session_fs_realloc(void *ptr, size_t size)    { return hgl_realloc(g_session_fs_allocator, ptr, size); }
void  session_fs_free(void *ptr)                    { hgl_free(g_session_fs_allocator, ptr); }
void *dummy_alloc(size_t size)                      { (void) size; assert(0); return NULL; }
void *dummy_realloc(void *ptr, size_t size)         { (void) size; (void) ptr; assert(0); return NULL; }
void  dummy_free(void *ptr)                         { (void) ptr; assert(0); }

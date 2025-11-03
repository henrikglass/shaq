/*--- Include files ---------------------------------------------------------------------*/

#include "alloc.h"

#include "hgl_int.h"

#define HGL_ALLOC_IMPLEMENTATION
#include "hgl_alloc.h"

#include <assert.h>

static HglAllocator temp_allocator_internal_       = {0};
static HglAllocator frame_arena_internal_          = {0};
static HglAllocator session_arena_internal_        = {0};
static HglAllocator session_fs_allocator_internal_ = {0};
static HglAllocator image_allocator_internal_      = {0};

HglAllocator *g_temp_allocator       = NULL;
HglAllocator *g_frame_arena          = NULL;
HglAllocator *g_session_arena        = NULL; // TODO rename
HglAllocator *g_session_fs_allocator = NULL;
HglAllocator *g_image_allocator      = NULL;

void alloc_init()
{
    temp_allocator_internal_       = hgl_alloc_make(.kind = HGL_SCRATCH_ALLOCATOR,
                                                    .size = 4096);         //   4 KiB
    frame_arena_internal_          = hgl_alloc_make(.kind = HGL_ARENA_ALLOCATOR, 
                                                    .size = 1024*1024);    //   1 MiB
    session_arena_internal_        = hgl_alloc_make(.kind = HGL_STACK_ALLOCATOR, 
                                                    .size = 16*1024*1024); //  16 MiB
    session_fs_allocator_internal_ = hgl_alloc_make(.kind = HGL_FREE_STACK_ALLOCATOR,
                                                    .size = 32*1024*1024,  //  32 MiB
                                                    .free_stack_capacity = 1024);
    image_allocator_internal_      = hgl_alloc_make(.kind = HGL_FREE_STACK_ALLOCATOR,
                                                    .size = 512*1024*1024, // 512 MiB
                                                    .free_stack_capacity = 1024);

    assert(session_fs_allocator_internal_.memory != NULL);
    assert(image_allocator_internal_.memory != NULL);

    g_temp_allocator       = &temp_allocator_internal_;
    g_frame_arena          = &frame_arena_internal_;
    g_session_arena        = &session_arena_internal_;
    g_session_fs_allocator = &session_fs_allocator_internal_;
    g_image_allocator      = &image_allocator_internal_;
}

void alloc_final()
{
    hgl_free_all(g_frame_arena);
    hgl_free_all(g_session_arena);
    hgl_free_all(g_session_fs_allocator);
    hgl_free_all(g_image_allocator);
    hgl_alloc_destroy(g_temp_allocator);
    hgl_alloc_destroy(g_frame_arena);
    hgl_alloc_destroy(g_session_arena);
    hgl_alloc_destroy(g_session_fs_allocator);
    hgl_alloc_destroy(g_image_allocator);
}

void *tmp_alloc(size_t size)                        { return hgl_alloc(g_temp_allocator, size); }
void *frame_arena_alloc(size_t size)                { return hgl_alloc(g_frame_arena, size); }
void *session_arena_alloc(size_t size)              { return hgl_alloc(g_session_arena, size); }
void *session_arena_realloc(void *ptr, size_t size) { return hgl_realloc(g_session_arena, ptr, size); }
void  session_arena_free(void *ptr)                 { hgl_free(g_session_arena, ptr); }
void *session_fs_alloc(size_t size)                 { return hgl_alloc(g_session_fs_allocator, size); }
void *session_fs_realloc(void *ptr, size_t size)    { return hgl_realloc(g_session_fs_allocator, ptr, size); }
void  session_fs_free(void *ptr)                    { hgl_free(g_session_fs_allocator, ptr); }
void *dummy_alloc(size_t size)                      { (void) size; return NULL; }
void *dummy_realloc(void *ptr, size_t size)         { (void) size; (void) ptr; return NULL; }
void  dummy_free(void *ptr)                         { (void) ptr; }
void *image_alloc(size_t size)                      { return hgl_alloc(g_image_allocator, size); }
void *image_realloc(void *ptr, size_t size)         { return hgl_realloc(g_image_allocator, ptr, size); }
void  image_free(void *ptr)                         { hgl_free(g_image_allocator, ptr); }

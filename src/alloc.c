/*--- Include files ---------------------------------------------------------------------*/

#include "alloc.h"

#include "hgl_int.h"

#define HGL_ARENA_ALLOC_DEBUG_PRINTS
#define HGL_ARENA_ALLOC_IMPLEMENTATION
#include "hgl_arena_alloc.h"

#define HGL_FS_ALLOC_IMPLEMENTATION
#include "hgl_fs_alloc.h"

#define SESSION_FS_ALLOCATOR_BUFFER_SIZE          (256*1024*1024) // 256 MiB
#define SESSION_FS_ALLOCATOR_FREE_STACK_CAPACITY         (4*1024)

static u8 session_fs_allocator_buffer_[SESSION_FS_ALLOCATOR_BUFFER_SIZE];

static Arena frame_arena_internal_ = {0};
static Arena session_arena_internal_ = {0};
static FsAllocator session_fs_allocator_internal_ = {0};

Arena *g_frame_arena                = NULL;
Arena *g_session_arena              = NULL;
FsAllocator *g_session_fs_allocator = NULL;

void alloc_init()
{
    frame_arena_internal_          = hgl_arena_make(.kind = HGL_ARENA_BUMP_ALLOCATOR,  .size = 1024*1024);    //   1 MiB
    session_arena_internal_        = hgl_arena_make(.kind = HGL_ARENA_STACK_ALLOCATOR, .size = 16*1024*1024); //  16 MiB
    session_fs_allocator_internal_ = hgl_fs_make_from_buffer(session_fs_allocator_buffer_,
                                                             SESSION_FS_ALLOCATOR_BUFFER_SIZE,
                                                             SESSION_FS_ALLOCATOR_FREE_STACK_CAPACITY);

    assert(session_fs_allocator_internal_.memory != NULL);

    g_frame_arena = &frame_arena_internal_;
    g_session_arena = &session_arena_internal_;
    g_session_fs_allocator = &session_fs_allocator_internal_;
}


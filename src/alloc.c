/*--- Include files ---------------------------------------------------------------------*/

#include "alloc.h"

#include "hgl_int.h"

#define HGL_ARENA_ALLOC_DEBUG_PRINTS
#define HGL_ARENA_ALLOC_IMPLEMENTATION
#include "hgl_arena_alloc.h"

#define HGL_FS_ALLOC_IMPLEMENTATION
#include "hgl_fs_alloc.h"

//#define HGL_STACK_ALLOC_DEBUG_PRINTS
//#define HGL_STACK_ALLOC_IMPLEMENTATION
//#include "hgl_stack_alloc.h"

#define FRAME_ARENA_SIZE                              (1024*1024) //   1 MiB
#define LONGTERM_ARENA_SIZE                        (16*1024*1024) //  16 MiB
#define LONGTERM_FS_ALLOCATOR_BUFFER_SIZE         (256*1024*1024) // 256 MiB
#define LONGTERM_FS_ALLOCATOR_FREE_STACK_CAPACITY        (4*1024)

static Arena frame_arena_internal_ = HGL_ARENA_STATIC(FRAME_ARENA_SIZE);  // TODO figure out something better... 
Arena *g_frame_arena = &frame_arena_internal_;

static Arena longterm_arena_internal_ = HGL_ARENA_STATIC(LONGTERM_ARENA_SIZE);  // TODO figure out something better... 
Arena *g_longterm_arena = &longterm_arena_internal_;

static u8 longterm_fs_allocator_buffer_[LONGTERM_FS_ALLOCATOR_BUFFER_SIZE];
static FsAllocator longterm_fs_allocator_internal_ = {0};
FsAllocator *g_longterm_fs_allocator = NULL;

void alloc_init()
{
    longterm_fs_allocator_internal_ = hgl_fs_make_from_buffer(longterm_fs_allocator_buffer_,
                                                              LONGTERM_FS_ALLOCATOR_BUFFER_SIZE,
                                                              LONGTERM_FS_ALLOCATOR_FREE_STACK_CAPACITY);
    assert(longterm_fs_allocator_internal_.memory != NULL);
    g_longterm_fs_allocator = &longterm_fs_allocator_internal_;
}


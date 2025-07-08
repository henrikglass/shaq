/*--- Include files ---------------------------------------------------------------------*/

#include "alloc.h"

#define HGL_ARENA_ALLOC_DEBUG_PRINTS
#define HGL_ARENA_ALLOC_IMPLEMENTATION
#include "hgl_arena_alloc.h"

static HglArena temp_allocator_internal_ = HGL_ARENA_INITIALIZER(64*1024);
HglArena *temp_allocator = &temp_allocator_internal_;




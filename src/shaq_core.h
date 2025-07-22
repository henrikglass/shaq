#ifndef SHAQ_CORE_H
#define SHAQ_CORE_H

/*--- Include files ---------------------------------------------------------------------*/

#include "hgl_int.h"
#include "hgl_float.h"
#include "vecmath.h"
#include "str.h"

/*--- Public macros ---------------------------------------------------------------------*/

/*--- Public type definitions -----------------------------------------------------------*/

/*--- Public variables ------------------------------------------------------------------*/

/*--- Public function prototypes --------------------------------------------------------*/

void shaq_begin(const char *ini_filepath);
b8 shaq_needs_reload(void);
b8 shaq_should_close(void);
void shaq_reload(void);
void shaq_new_frame(void);
void shaq_end(void);

f32 shaq_time(void);
f32 shaq_deltatime(void);
//IVec2 shaq_iresolution(void);
i32 shaq_find_shader_id_by_name(StringView name);
i32 shaq_load_texture_if_necessary(StringView filepath);
u32 shaq_get_shader_render_texture_by_index(u32 index);
u32 shaq_get_loaded_texture_by_index(u32 index);


#endif /* SHAQ_CORE_H */


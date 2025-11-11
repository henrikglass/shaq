#ifndef SHAQ_CORE_H
#define SHAQ_CORE_H

/*--- Include files ---------------------------------------------------------------------*/

#include "hgl_int.h"
#include "hgl_float.h"
#include "vecmath.h"
#include "str.h"
#include "shader.h"

/*--- Public macros ---------------------------------------------------------------------*/

/*--- Public type definitions -----------------------------------------------------------*/

/*--- Public variables ------------------------------------------------------------------*/

/*--- Public function prototypes --------------------------------------------------------*/

void shaq_begin(const char *ini_filepath, bool quiet);
b8 shaq_should_close(void);
void shaq_new_frame(void);
void shaq_end(void);

void shaq_reset_time(void);
f32 shaq_time(void);
f32 shaq_deltatime(void);
i32 shaq_frame_count(void);
b8 shaq_reloaded_this_frame(void);
b8 shaq_reloaded_last_frame(void);

b8 shaq_has_loaded_project(void);
Shader *shaq_find_shader_by_name(StringView name);
i32 shaq_find_shader_id_by_name(StringView name);
i32 shaq_load_texture_if_necessary(StringView filepath);
u32 shaq_get_shader_current_render_texture_by_shader_id(u32 id);
u32 shaq_get_shader_last_render_texture_by_shader_id(u32 id);
u32 shaq_get_loaded_texture_by_texture_id(u32 id);


#endif /* SHAQ_CORE_H */


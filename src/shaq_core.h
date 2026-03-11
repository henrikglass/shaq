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

void shaq_reset_time(void);        // Reset all internal timekeeping (except wall time)
f32 shaq_time(void);               // Returns the time in seconds since the last reset
f32 shaq_deltatime(void);          // Returns the last frame deltatime in seconds
u64 shaq_timestamp_ns(void);       // Returns the current wall time in nanoseconds
i32 shaq_frame_count(void);        // Returns the current frame count since the last reset
void shaq_toggle_time_pause(void); // pauses/unpauses timekeeping functions, except for wall time

b8 shaq_reloaded_this_frame(void); // Returns true iff a reload occured this frame
b8 shaq_reloaded_last_frame(void); // Returns true iff a reload occured in the previous frame

void shaq_toggle_widgets_overlay(void);
b8 shaq_has_loaded_project(void);
Shader *shaq_find_shader_by_name(StringView name);
i32 shaq_find_shader_id_by_name(StringView name);
i32 shaq_find_texture_id_by_name(StringView filepath, b8 load_if_necessary);
Shader *shaq_get_shader_by_id(i32 id);
Texture *shaq_get_texture_by_id(i32 id);


#endif /* SHAQ_CORE_H */


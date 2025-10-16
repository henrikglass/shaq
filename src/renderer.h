#ifndef RENDERER_H
#define RENDERER_H

/*--- Include files ---------------------------------------------------------------------*/

#include "shader.h"
#include "vecmath.h"

/*--- Public macros ---------------------------------------------------------------------*/

/*--- Public type definitions -----------------------------------------------------------*/

/*--- Public variables ------------------------------------------------------------------*/

/*--- Public function prototypes --------------------------------------------------------*/

void renderer_init(void);
void renderer_final(void);
void renderer_reload(void);
void renderer_begin_frame(void);
void renderer_do_shader_pass(Shader *s);
void renderer_do_final_pass(Shader *s);
void renderer_end_frame(void);
b8 renderer_should_close(void);
b8 renderer_should_reload(void);
b8 renderer_shader_view_is_maximized(void);
IVec2 renderer_window_size(void);
Vec2 renderer_mouse_position(void);

#endif /* RENDERER_H */


#ifndef RENDERER_H
#define RENDERER_H

/*--- Include files ---------------------------------------------------------------------*/

#include "shader.h"
#include "vecmath.h"
#include "glad/glad.h"
#include <GLFW/glfw3.h>

/*--- Public macros ---------------------------------------------------------------------*/

/*--- Public type definitions -----------------------------------------------------------*/

/*--- Public variables ------------------------------------------------------------------*/

/*--- Public function prototypes --------------------------------------------------------*/

void renderer_init(void);
void renderer_final(void);
void renderer_reload(void);
void renderer_clear_current_framebuffer(void);
void renderer_do_shader_pass(Shader *s);
void renderer_draw_fullscreen_shader(Shader *s);
void renderer_begin_final_pass(void);
void renderer_end_final_pass(void);
void renderer_toggle_fullscreen(void);
void renderer_toggle_maximized_shader_view(void);
GLFWwindow *renderer_get_glfw_window(void);
b8 renderer_should_close(void);
b8 renderer_should_reload(void);
b8 renderer_shader_view_is_maximized(void);
b8 renderer_is_fullscreen(void);
IVec2 renderer_window_size(void);
IVec2 renderer_shader_viewport_size(void);

#endif /* RENDERER_H */


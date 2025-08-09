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
void renderer_begin_frame(void);
void renderer_do_shader_pass(Shader *s);
void renderer_do_final_pass(Shader *s);
void renderer_end_frame(void);
b8 renderer_should_close(void);
b8 renderer_should_hide_gui(void);
b8 renderer_window_was_resized(void);
IVec2 renderer_iresolution(void);
//GLFWwindow *renderer_get_window(void);

#endif /* RENDERER_H */


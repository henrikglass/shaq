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
void renderer_begin_new_frame(void);
void renderer_do_shader_pass(Shader *s);
void renderer_begin_final_pass(void);
void renderer_display_output_of_shader(Shader *s);
void renderer_end_final_pass(void);
b8 renderer_should_close(void);
b8 renderer_window_was_resized(void);
IVec2 renderer_iresolution(void);

#endif /* RENDERER_H */


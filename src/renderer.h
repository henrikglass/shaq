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
void renderer_draw_shader(Shader *s);
void renderer_display(Shader *s);
b8 renderer_should_close(void);
IVec2 renderer_iresolution(void);

#endif /* RENDERER_H */


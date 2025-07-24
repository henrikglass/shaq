#ifndef GUI_H
#define GUI_H

/*--- Include files ---------------------------------------------------------------------*/

#include <GLFW/glfw3.h>

/*--- Public macros ---------------------------------------------------------------------*/

/*--- Public type definitions -----------------------------------------------------------*/

/*--- Public variables ------------------------------------------------------------------*/

/*--- Public function prototypes --------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

void gui_init(GLFWwindow *window);
void gui_draw_test(void);

#ifdef __cplusplus
}
#endif

#endif /* GUI_H */


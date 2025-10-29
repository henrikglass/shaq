#ifndef USER_INPUT_H
#define USER_INPUT_H

/*--- Include files ---------------------------------------------------------------------*/
        
#include "hgl_int.h"
#include "vecmath.h"
#include "glad/glad.h"
#include <GLFW/glfw3.h>

/*--- Public macros ---------------------------------------------------------------------*/

/*--- Public type definitions -----------------------------------------------------------*/

/*--- Public variables ------------------------------------------------------------------*/

/*--- Public function prototypes --------------------------------------------------------*/

void user_input_poll(void);
b8 user_input_should_reload(void);
Vec2 user_input_mouse_position(void);
Vec2 user_input_mouse_drag_position(void);
Vec2 user_input_mouse_position_last(void);
b8 user_input_left_mouse_button_is_down(void);
b8 user_input_right_mouse_button_is_down(void);
b8 user_input_left_mouse_button_was_clicked(void);
b8 user_input_right_mouse_button_was_clicked(void);
b8 user_input_key_is_down(char c);
b8 user_input_key_was_pressed(char c);
void user_input_glfw_key_callback(GLFWwindow *window, i32 key, i32 scancode, i32 action, i32 mods);

#endif /* USER_INPUT_H */


/*--- Include files ---------------------------------------------------------------------*/

#include "user_input.h"

#include "renderer.h"
#include "shaq_core.h"
#include "gui.h"
#include "imguic.h"

/*--- Private macros --------------------------------------------------------------------*/

/*--- Private type definitions ----------------------------------------------------------*/

/*--- Private function prototypes -------------------------------------------------------*/

/*--- Public variables ------------------------------------------------------------------*/

/*--- Private variables -----------------------------------------------------------------*/

static struct {
    b8 should_reload;

    Vec2 mouse_position;
    Vec2 mouse_drag_position;
    Vec2 mouse_position_last;
    b8 lmb_is_down;
    b8 rmb_is_down;
    b8 lmb_was_down_last_frame;
    b8 rmb_was_down_last_frame;
    u32 key_down_bitfield;
    u32 key_pressed_bitfield;
} user_input;

/*--- Public functions ------------------------------------------------------------------*/

void user_input_poll()
{
    user_input.should_reload = false;
    user_input.key_pressed_bitfield = 0u;
    user_input.mouse_position_last = user_input.mouse_position;

    glfwPollEvents();
    GLFWwindow *w = renderer_get_glfw_window();

    /* Update mouse position */
    f64 x, y;
    glfwGetCursorPos(w, &x, &y);
    user_input.mouse_position = vec2_make(x, y);
    if (!renderer_shader_view_is_maximized()) {
        IVec2 shader_window_pos = gui_shader_window_position();
        user_input.mouse_position.x -= shader_window_pos.x;
        user_input.mouse_position.y -= shader_window_pos.y;
        user_input.mouse_position.y = gui_shader_window_size().y - user_input.mouse_position.y;
    } else {
        user_input.mouse_position.y = renderer_window_size().y - user_input.mouse_position.y;
    }

    /* Update mouse button state */
    user_input.lmb_was_down_last_frame = user_input.lmb_is_down;
    user_input.rmb_was_down_last_frame = user_input.rmb_is_down;
    user_input.lmb_is_down = glfwGetMouseButton(w, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    user_input.rmb_is_down = glfwGetMouseButton(w, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

    /* 
     * Update mouse drag position
     *
     * TODO: Handle this in a better way. Currently, mouse_drag_position may be updated
     *       when, for instance, the shader view window is being resized
     */
    if (user_input.lmb_is_down /*&& !imgui_is_any_item_active()*/) {
        if (renderer_shader_view_is_maximized()) {
            user_input.mouse_drag_position = user_input.mouse_position;
        } else {
            IVec2 swsize = gui_shader_window_size();
            float mx = user_input.mouse_position.x;
            float my = user_input.mouse_position.y;
            if (((f32)mx >= 0) && ((f32)mx < swsize.x) &&
                ((f32)my >= 0) && ((f32)my < swsize.y)) {
                user_input.mouse_drag_position = user_input.mouse_position;
            }
        }
    }
}

b8 user_input_should_reload()
{
    return user_input.should_reload;
}

Vec2 user_input_mouse_position()
{
    return user_input.mouse_position;
}

Vec2 user_input_mouse_drag_position()
{
    return user_input.mouse_drag_position;
}

Vec2 user_input_mouse_position_last()
{
    return user_input.mouse_position_last;
}

b8 user_input_left_mouse_button_is_down()
{
    return user_input.lmb_is_down;
}

b8 user_input_right_mouse_button_is_down()
{
    return user_input.rmb_is_down;
}

b8 user_input_left_mouse_button_was_clicked()
{
    return user_input.lmb_is_down && !user_input.lmb_was_down_last_frame;
}

b8 user_input_right_mouse_button_was_clicked()
{
    return user_input.rmb_is_down && !user_input.rmb_was_down_last_frame;
}

b8 user_input_key_is_down(char c)
{
    u32 bit_pos = 0;
    if (c >= 'a' && c <= 'z') {
        bit_pos = c - 'a'; 
    } else if (c >= 'A' && c <= 'Z') {
        bit_pos = c - 'A'; 
    } else {
        return false;
    }
    return 0 != (user_input.key_down_bitfield & (1u << bit_pos));
}

b8 user_input_key_was_pressed(char c)
{
    u32 bit_pos = 0;
    if (c >= 'a' && c <= 'z') {
        bit_pos = c - 'a'; 
    } else if (c >= 'A' && c <= 'Z') {
        bit_pos = c - 'A'; 
    } else {
        return false;
    }
    return 0 != (user_input.key_pressed_bitfield & (1u << bit_pos));
}

void user_input_glfw_key_callback(GLFWwindow *window, i32 key, i32 scancode, i32 action, i32 mods)
{
    (void) window;
    (void) scancode;

    if (mods == 0) {
        switch (key) {
            case GLFW_KEY_ESCAPE: {
                if (action == GLFW_PRESS && imgui_file_dialog_is_open()) {
                    imgui_close_file_dialog();
                }
                if (action == GLFW_PRESS && renderer_is_fullscreen()) {
                    renderer_toggle_fullscreen();
                }
            } break;

            case GLFW_KEY_F11: {
                if (action == GLFW_PRESS) {
                    renderer_toggle_fullscreen();
                }
            } break;

            case GLFW_KEY_A ... GLFW_KEY_Z: {
                u32 bit_pos = key - GLFW_KEY_A;
                if (action == GLFW_PRESS) {
                    user_input.key_down_bitfield |= (1u << bit_pos);
                    user_input.key_pressed_bitfield |= (1u << bit_pos);
                } else if (action == GLFW_RELEASE) {
                    user_input.key_down_bitfield &= ~(1u << bit_pos);
                }
            } break;
        }
    }

    if (action == GLFW_PRESS && mods == GLFW_MOD_ALT) {
        switch (key) {
            case GLFW_KEY_ENTER: {
                renderer_toggle_fullscreen();
            } break;
        }
    }

    if (action == GLFW_PRESS && mods == GLFW_MOD_CONTROL) {
        switch (key) {
            case GLFW_KEY_D: {
                gui_toggle_darkmode();
            } break;

            case GLFW_KEY_F: {
                if (shaq_has_loaded_project()) {
                    renderer_toggle_maximized_shader_view();
                }
            } break;

            case GLFW_KEY_R: {
                user_input.should_reload = true;
            } break;

            case GLFW_KEY_T: {
                shaq_reset_time();
            } break;

            case GLFW_KEY_P: {
                shaq_toggle_time_pause();
            } break;

            case GLFW_KEY_O: {
                imgui_open_file_dialog();
            } break;

            case GLFW_KEY_W: {
                glfwSetWindowShouldClose(renderer_get_glfw_window(), true);
            } break;
        }
    }
}

/*--- Private functions -----------------------------------------------------------------*/



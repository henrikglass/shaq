#ifndef GUI_HELPERS_H
#define GUI_HELPERS_H

/*--- Include files ---------------------------------------------------------------------*/

#include "shader.h"
#include "sel.h"

#include <GLFW/glfw3.h>

/*--- Public macros ---------------------------------------------------------------------*/

/*--- Public type definitions -----------------------------------------------------------*/

typedef enum {
    INPUT_INT,
    INPUT_FLOAT,
    INPUT_VEC2,
    INPUT_VEC3,
    INPUT_VEC4,
    CHECKBOX,
    DRAG_INT,
    SLIDER_FLOAT,
    SLIDER_FLOAT_LOG,
    COLOR_PICKER,
} WidgetKind;

/*--- Public variables ------------------------------------------------------------------*/

/*--- Public function prototypes --------------------------------------------------------*/

void gui_init(GLFWwindow *window, GLFWmonitor *monitor);
void gui_final(void);
void gui_reload(void);
void gui_clear_widgets(void);
void gui_begin_frame(void);
b8 gui_begin_main_window(void);
b8 gui_begin_shader_window(void);
void gui_draw_help(void);
i32 gui_draw_shader_display_selector(i32 current_idx, Shader *shaders, u32 n_shaders);
void gui_draw_shader_info(const Shader *s);
void gui_draw_shader(const Shader *s);
void gui_draw_widgets(void);
void gui_end_shader_window(void);
void gui_end_main_window(void);
void gui_end_frame(void);
void gui_draw_log_window(void);
void gui_draw_error_log_overlay(void);
void gui_draw_menu_bar(void);
b8 gui_file_dialog_is_open(void);
b8 gui_draw_file_dialog(char *ini_filepath);
b8 gui_darkmode_is_enabled(void);
void gui_toggle_darkmode(void);
SelValue gui_get_widget_value(StringView label,
                              WidgetKind kind,
                              void *secondary_args,
                              u32 secondary_args_size);
b8 gui_should_reload(void);
IVec2 gui_shader_window_position(void);
IVec2 gui_shader_window_size(void);

#endif /* GUI_HELPERS_H */


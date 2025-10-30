#ifndef IMGUIC_H
#define IMGUIC_H

/*--- Include files ---------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif
#include "str.h"
#ifdef __cplusplus
}
#endif

#include "hgl_int.h"
#include <stdarg.h>
#include <GLFW/glfw3.h>

/*--- Public macros ---------------------------------------------------------------------*/

/*--- Public type definitions -----------------------------------------------------------*/

/*--- Public variables ------------------------------------------------------------------*/

/*--- Public function prototypes --------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

void imgui_init(GLFWwindow *window, GLFWmonitor *monitor);
void imgui_final(void);
void imgui_set_darkmode(b8 enable);
b8 imgui_is_any_item_active(void);
void imgui_begin_frame(void);
b8 imgui_begin(const char *str);
void imgui_begin_child(const char *label, u32 color);
void imgui_begin_table(const char *label, i32 n_cols);
b8 imgui_begin_combo(const char *label, const char *preview_value);

b8 imgui_begin_main_menu_bar(void);
b8 imgui_begin_menu(const char *label);
b8 imgui_menu_item(const char *label, const char *shortcut);
void imgui_end_menu(void);
void imgui_end_main_menu_bar(void);
void imgui_open_file_dialog(void);
void imgui_close_file_dialog(void);
b8 imgui_file_dialog_is_open(void);
b8 imgui_display_file_dialog(char *filepath);

void imgui_show_dpi_override_setting(void);
void imgui_open_about_modal(void);
void imgui_show_about_modal(void);
void imgui_draw_texture(u32 gl_texture_id, int w, int h);
void imgui_table_next_row(void);
void imgui_table_next_col(void);
void imgui_text_unformatted(const char *str, size_t len);
void imgui_textf(const char *fmt, ...);
void imgui_text_color(const char *str, u32 color);
void imgui_separator(void);
void imgui_newline(void);
void imgui_checkbox(const char *label, b8 *b);
void imgui_drag_int(const char *label, int *v, int min, int max);
void imgui_input_int(const char *label, int *v);
void imgui_input_float(const char *label, float *v);
void imgui_input_float2(const char *label, float *v);
void imgui_input_float3(const char *label, float *v);
void imgui_input_float4(const char *label, float *v);
void imgui_slider_float(const char *label, float *v, float min, float max, b8 log);
void imgui_color_picker(const char *label, float *v);
b8 imgui_selectable(const char *label, b8 is_selected);
void imgui_set_item_default_focus(void);
void imgui_push_style_shader_window(void);
void imgui_pop_style_shader_window(void);
void imgui_get_current_window_dimensions(int *x, int *y, int *w, int *h);
void imgui_end_combo(void);
void imgui_end_table(void);
void imgui_end_child(void);
void imgui_end(void);
void imgui_end_frame(void);

#ifdef __cplusplus
}
#endif

#endif /* IMGUIC_H */


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

void imgui_init(GLFWwindow *window);
void imgui_final(void);
void imgui_set_darkmode(bool enable);
void imgui_begin_frame(void);
b8 imgui_begin(const char *str);
void imgui_begin_child(const char *label, u32 color);
void imgui_begin_table(const char *label, i32 n_cols);
bool imgui_begin_combo(const char *label, const char *preview_value);
void imgui_table_next_row(void);
void imgui_table_next_col(void);
void imgui_text_unformatted(const char *str, size_t len);
void imgui_textf(const char *fmt, ...);
void imgui_text_color(const char *str, u32 color);
void imgui_separator(void);
void imgui_newline(void);
void imgui_input_float(const char *label, float *v);
void imgui_input_float2(const char *label, float *v);
void imgui_input_float3(const char *label, float *v);
void imgui_input_float4(const char *label, float *v);
void imgui_slider_float(const char *label, float *v, float min, float max, bool log);
void imgui_color_picker(const char *label, float *v);
bool imgui_selectable(const char *label, bool is_selected);
void imgui_set_item_default_focus(void);
void imgui_end_combo(void);
void imgui_end_table(void);
void imgui_end_child(void);
void imgui_end(void);
void imgui_end_frame(void);

#ifdef __cplusplus
}
#endif

#endif /* IMGUIC_H */


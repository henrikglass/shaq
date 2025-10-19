#ifndef GUI_HELPERS_H
#define GUI_HELPERS_H

/*--- Include files ---------------------------------------------------------------------*/

#include "shader.h"
#include "sel.h"

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
} DynamicGuiItemKind;

/*--- Public variables ------------------------------------------------------------------*/

/*--- Public function prototypes --------------------------------------------------------*/

void gui_reload(void);
void gui_begin_frame(void);
b8 gui_begin_main_window(void);
b8 gui_begin_shader_window(void);
void gui_draw_help(void);
i32 gui_draw_shader_display_selector(i32 current_idx, Shader *shaders, u32 n_shaders);
void gui_draw_shader_info(const Shader *s);
void gui_draw_shader(const Shader *s);
void gui_draw_dymanic_gui_items(void);
void gui_end_shader_window(void);
void gui_end_main_window(void);
void gui_end_frame(void);
void gui_draw_log_window(void);
void gui_draw_menu_bar(void);
void gui_toggle_darkmode(void);
SelValue gui_get_dynamic_item_value(StringView label,
                                    DynamicGuiItemKind kind,
                                    void *secondary_args,
                                    u32 secondary_args_size);
b8 gui_should_reload(void);
IVec2 gui_shader_window_position(void);
IVec2 gui_shader_window_size(void);

#endif /* GUI_HELPERS_H */


/*--- Include files ---------------------------------------------------------------------*/

#include "gui.h"
#include "imguic.h"
#include "shaq_core.h"
#include "alloc.h"
#include "shaq_config.h"
#include "array.h"
#include "renderer.h"
#include "log.h"
#include "uniform.h"

/*--- Private macros --------------------------------------------------------------------*/

/*--- Private type definitions ----------------------------------------------------------*/

typedef struct
{
    StringView label;
    SelValue value;
    WidgetKind kind;
    u8 secondary_args[64];
    b8 touched_this_frame;
} Widget;

/*--- Private function prototypes -------------------------------------------------------*/

static inline void draw_and_update_widget(Widget *w);
static inline void draw_uniform(const Uniform *u);

/*--- Public variables ------------------------------------------------------------------*/

/*--- Private variables -----------------------------------------------------------------*/

static struct
{
    Array(Widget, SHAQ_MAX_N_DYNAMIC_GUI_ITEMS) widgets;
    b8 dark_mode;
    b8 should_reload;
    b8 shader_window_is_active;
    b8 shader_window_is_maximized;
    IVec2 shader_window_position;
    IVec2 shader_window_size;
    f32 smoothed_deltatime;
} gui;

/*--- Public functions ------------------------------------------------------------------*/

void gui_init(GLFWwindow *window, GLFWmonitor *monitor)
{
    imgui_init(window, monitor);
    gui.dark_mode = true;
    gui.shader_window_is_maximized = false;
    gui.shader_window_is_active = false;
    imgui_set_darkmode(gui.dark_mode);
}

void gui_final()
{
    imgui_final();
}

void gui_reload()
{
    gui.should_reload = false;
}

void gui_clear_widgets()
{
    array_clear(&gui.widgets);
}

void gui_begin_frame()
{
    imgui_begin_frame();
    if (gui.shader_window_is_maximized) {
        gui.shader_window_is_active = !imgui_any_window_is_hovered();
    }
    printf("%d\n", gui.shader_window_is_active);
}

void gui_toggle_maximized_shader_window()
{
    gui.shader_window_is_maximized = !gui.shader_window_is_maximized;
    gui.should_reload = true;
}

b8 gui_shader_window_is_maximized()
{
    return gui.shader_window_is_maximized;
}

b8 gui_shader_window_is_active()
{
    return gui.shader_window_is_active;
}

IVec2 gui_shader_window_position()
{
    return gui.shader_window_position; 
}

IVec2 gui_shader_window_size()
{
    if (gui.shader_window_is_maximized) {
        return renderer_window_size();
    }
    return gui.shader_window_size;
}

b8 gui_begin_main_window()
{
    gui.smoothed_deltatime = 0.90f*gui.smoothed_deltatime +
                             0.10f*shaq_deltatime();
    b8 ret = imgui_begin("Main Window");
    if (ret) {
        imgui_textf("Frame time: %3.1f ms", (f64)(1000.0f*gui.smoothed_deltatime)); imgui_newline();
        imgui_textf("FPS: %d", (i32)(1.0f/gui.smoothed_deltatime + 0.5f)); imgui_newline();
        imgui_separator();
    }
    return ret;
}

b8 gui_begin_shader_window()
{
    imgui_push_style_shader_window();
    b8 ret = imgui_begin("Shader View");
    if (ret) {
        int x, y, w, h;
        imgui_get_current_window_dimensions(&x, &y, &w, &h);

        gui.shader_window_position.x = x;
        gui.shader_window_position.y = y;

        if ((w != gui.shader_window_size.x) || 
            (h != gui.shader_window_size.y)) {
            gui.shader_window_size.x = w;
            gui.shader_window_size.y = h;
            gui.should_reload = true;
        }
    }

    /* 
     * Yes, I read the "IMPORTANT" comment in imgui.cpp for the definition
     * of ImGui::IsAnyItemHovered(). Using io.WantCaptureMouse will not work,
     * since unless the shader "window" is maximized (i.e. being drawn directly
     * to the framebuffer and not, ironically, to a window) it will report `true`
     * whenever the shader window is hovered.
     */
    gui.shader_window_is_active = imgui_current_window_is_hovered() && 
                                 !imgui_any_item_is_hovered();
    return ret;
}

i32 gui_draw_shader_display_selector(i32 current_idx, Shader *shaders, u32 n_shaders)
{
    Shader *current = &shaders[current_idx];
    char *current_name_cstr = hgl_sv_make_cstr_copy(current->name, tmp_alloc);

    if (imgui_begin_combo("Show", current_name_cstr)) {
        for (u32 i = 0; i < n_shaders; i++) {
            Shader *s = &shaders[i];
            b8 is_selected = (current == s);

            char *s_name_cstr = hgl_alloc(g_frame_arena, s->name.length + 1);
            memcpy(s_name_cstr, s->name.start, s->name.length);
            s_name_cstr[s->name.length] = '\0';

            if (imgui_selectable(s_name_cstr, is_selected)) {
                current_idx = (i32)i; 
            }
            if (is_selected) {
                imgui_set_item_default_focus();
            }
        }
        imgui_end_combo();
    }
    imgui_separator();
    return current_idx;
}

void gui_draw_shader_info(const Shader *s)
{
    /* TODO have this elsewhere - make linear search instead? */
    static const char *glint_to_str[] = {
        [GL_RGBA]         = "GL_RGBA",
        [GL_RGB]          = "GL_RGB",
        [GL_RG]           = "GL_RG",
        [GL_RED]          = "GL_RED",
        [GL_RGBA8]        = "GL_RGBA8",
        [GL_RGB8]         = "GL_RGB8",
        [GL_RG8]          = "GL_RG8",
        [GL_R8]           = "GL_R8",
        [GL_R32F]         = "GL_R32F",
        [GL_RGBA32F]      = "GL_RGBA32F",
        [GL_R3_G3_B2]     = "GL_R3_G3_B2",
        [GL_SRGB8]        = "GL_SRGB8",
        [GL_SRGB8_ALPHA8] = "GL_SRGB8_ALPHA8",
    };
    //imgui_textf("[" SV_FMT "]", SV_ARG(s->name));
    char *label_cstr = sv_make_cstr_copy(s->name, tmp_alloc);
    if (imgui_tree_node(label_cstr)) {
        if (imgui_tree_node("Attributes")) {
            imgui_begin_table(" ", 2);
            imgui_table_next_row();
            imgui_table_next_col();
            imgui_textf("source");
            imgui_table_next_col();
            imgui_textf(" = " SV_FMT, SV_ARG(s->attributes.source));
            imgui_table_next_row();
            imgui_table_next_col();
            imgui_textf("resolution");
            imgui_table_next_col();
            imgui_textf(" = {%d, %d}", s->attributes.resolution.x, 
                                    s->attributes.resolution.y);
            imgui_table_next_row();
            imgui_table_next_col();
            imgui_textf("format");
            imgui_table_next_col();
            imgui_textf(" = %s", glint_to_str[s->attributes.format]);
            imgui_end_table();
            imgui_tree_pop();
        }
        if (imgui_tree_node("Uniforms")) {
            imgui_begin_table(" ", 2);
            for (u32 j = 0; j < s->uniforms.count; j++) {
                draw_uniform(&s->uniforms.arr[j]);
            }
            imgui_end_table();
            imgui_tree_pop();
        }
        imgui_tree_pop();
    }
}

void gui_draw_shader(const Shader *s)
{
    if (!shader_is_ok(s)) {
        return;
    }

    imgui_draw_texture(s->render_texture_current->gl_texture_id,
                       gui.shader_window_size.x,
                       gui.shader_window_size.y);
}

void gui_draw_widgets()
{
    imgui_textf("Widgets:"); imgui_newline();
    for (u32 i = 0; i < gui.widgets.count; i++) {
        Widget *w = &gui.widgets.arr[i];
        draw_and_update_widget(w);
    }
    imgui_separator();
}

void gui_end_shader_window()
{
    imgui_end();
    imgui_pop_style_shader_window();
}

void gui_end_main_window()
{
    imgui_end();
}

void gui_end_frame()
{
    imgui_end_frame();
    for (u32 i = 0; i < gui.widgets.count; i++) {
        Widget *w = &gui.widgets.arr[i];
        if (w->touched_this_frame) {
            w->touched_this_frame = false;
        } else {
            array_delete(&gui.widgets, i);
        }
    }
}

void gui_draw_log_window()
{
    b8 ret = imgui_begin("Log##window");
    if (!ret) {
        imgui_end();
        return;
    }
    log_reset_iterators();
    imgui_begin_child("Log", gui.dark_mode ? 0x282828FF : 0xD1D1D1FF);
    while (true) {
        u32 msg_len;
        const char *msg = log_get_next_info_msg(&msg_len);
        if (msg == NULL) {
            break;
        }
        imgui_text_color("Info: ", gui.dark_mode ? 0xE1E1E1FF : 0x1E1E1EFF);
        imgui_text_unformatted(msg, msg_len); 
    }
    while (true) {
        u32 msg_len;
        const char *msg = log_get_next_error_msg(&msg_len);
        if (msg == NULL) {
            break;
        }
        imgui_text_color("Error: ", 0xE04050FF);
        imgui_text_unformatted(msg, msg_len); 
    }
    imgui_end_child();
    imgui_end();
}

void gui_draw_error_log_overlay()
{
    b8 ret = imgui_begin_overlay_bottom_left("Log##overlay");
    if (!ret) {
        imgui_end();
        return;
    }
    log_reset_iterators();
    while (true) {
        u32 msg_len;
        const char *msg = log_get_next_error_msg(&msg_len);
        if (msg == NULL) {
            break;
        }
        imgui_text_color("Error: ", 0xE04050FF);
        imgui_text_unformatted(msg, msg_len); 
    }
    imgui_end();
}

void gui_draw_menu_bar()
{
    b8 open_popup = false;
    if (imgui_begin_main_menu_bar()) {
        if (imgui_begin_menu("Menu")) {
            if (imgui_menu_item("Open", "Ctrl-O")) {
                imgui_open_file_dialog();
            }
            if (imgui_menu_item("Quit", "Alt-F4/Ctrl-W")) {
                exit(0);
            }
            imgui_end_menu();
        }
        if (imgui_begin_menu("Options")) {
            imgui_show_dpi_override_setting();
            imgui_checkbox("Enable darkmode", &gui.dark_mode);
            imgui_set_darkmode(gui.dark_mode);
            imgui_separator();
            if (imgui_menu_item("Force reload", "Ctrl-R")) {
                gui.should_reload = true;
            }
            if (imgui_menu_item("Reset time", "Ctrl-T")) {
                shaq_reset_time(); 
            }
            if (imgui_menu_item("Pause/Unpause time", "Ctrl-P")) {
                shaq_toggle_time_pause(); 
            }
            if (imgui_menu_item("Toggle darkmode", "Ctrl-D")) {
                gui_toggle_darkmode(); 
            }
            if (imgui_menu_item("Toggle maximized shader view", "Ctrl-F")) {
                gui_toggle_maximized_shader_window();
            }
            if (imgui_menu_item("Toggle fullscreen", "Alt-Enter/F11")) {
                renderer_toggle_fullscreen(); 
            }
            imgui_end_menu();
        }
        if (imgui_begin_menu("Help")) {
            if (imgui_menu_item("About", NULL)) {
                open_popup = true;
            }
            imgui_end_menu();
        }
    }
    if (open_popup) {
        imgui_open_about_modal();
    }
    imgui_show_about_modal(); // TODO FIX
    imgui_end_main_menu_bar();
}

b8 gui_file_dialog_is_open()
{
    return imgui_file_dialog_is_open();
}

b8 gui_draw_file_dialog(char *ini_filepath)
{
    return imgui_display_file_dialog(ini_filepath);
}

b8 gui_darkmode_is_enabled()
{
    return gui.dark_mode;
}

void gui_toggle_darkmode()
{
    gui.dark_mode = !gui.dark_mode;
    imgui_set_darkmode(gui.dark_mode);
}

SelValue gui_get_widget_value(StringView label,
                              WidgetKind kind,
                              void *secondary_args,
                              u32 secondary_args_size)
{
    // TODO cache items somehow?

    /* try lookup */
    for (u32 i = 0; i < gui.widgets.count; i++) {
        Widget *w = &gui.widgets.arr[i];
        if (w->kind != kind) {
            continue;
        }
        if (!sv_equals(w->label, label)) {
            continue;
        }
        if (0 != memcmp(w->secondary_args, secondary_args, secondary_args_size)) {
            continue;
        }
        w->touched_this_frame = true;
        return w->value;
    }
    
    /* not found - insert */
    i32 *args_i32   = (i32 *)secondary_args;
    f32 *args_f32   = (f32 *)secondary_args;
    Vec2 *args_vec2 = (Vec2 *)secondary_args;
    Vec3 *args_vec3 = (Vec3 *)secondary_args;
    Vec4 *args_vec4 = (Vec4 *)secondary_args;
    SelValue default_value;
    switch (kind) {
        case INPUT_INT:        default_value.val_i32  = args_i32[0];  break;
        case INPUT_FLOAT:      default_value.val_f32  = args_f32[0];  break;
        case INPUT_VEC2:       default_value.val_vec2 = args_vec2[0]; break;
        case INPUT_VEC3:       default_value.val_vec3 = args_vec3[0]; break;
        case INPUT_VEC4:       default_value.val_vec4 = args_vec4[0]; break;
        case CHECKBOX:         default_value.val_bool = args_i32[0];  break;
        case DRAG_INT:         default_value.val_i32  = args_i32[3];  break;
        case SLIDER_FLOAT:     default_value.val_f32  = args_f32[2];  break;
        case SLIDER_FLOAT_LOG: default_value.val_f32  = args_f32[2];  break;
        case COLOR_PICKER:     default_value.val_vec4 = args_vec4[0]; break;
    } 
    Widget w = (Widget) {
        .label              = label,
        .kind               = kind,
        .value              = default_value,
        .touched_this_frame = true,
    };
    memcpy(w.secondary_args, secondary_args, secondary_args_size);
    array_push(&gui.widgets, w);

    return (SelValue) {0};
}

b8 gui_should_reload()
{
#if SHAQ_RELOAD_DURING_RESIZE
    return gui.should_reload;
#else
    /* 
     * Hold off on signaling that a reload is needed until the user stops 
     * interacting with the GUI. By doing this we avoid having to perform
     * a reload every frame while the user is, for instance, actively 
     * resizing the shader view window. 
     *
     * TODO: Maybe remove this. If it turns out that reloading every 
     *       frame does not significantly impact performance, then we might
     *       as well do it for the sake of user experience.
     */
    if (imgui_is_any_item_active()) {
        return false;
    }
    return gui.should_reload;
#endif
}

/*--- Private functions -----------------------------------------------------------------*/

static inline void draw_and_update_widget(Widget *w)
{
    StringBuilder sb = sb_make(.initial_capacity = 256,
                               .mem_alloc        = tmp_alloc,
                               .mem_realloc      = dummy_realloc,
                               .mem_free         = dummy_free);
    sb_append_sv(&sb, &w->label);
    sb_append_cstr(&sb, "##widget");
    char *label_cstr = sb.cstr;

    /**
     * TODO: Figure out why this happens. It doesn't seem to cause any
     *       unexpected behaviors though.
     */
    if (strlen(label_cstr) == 0) {
        assert(false && "Huh?");
        return;
    }

    switch (w->kind) {
        case INPUT_INT: {
            imgui_input_int(label_cstr, &w->value.val_i32);
        } break;

        case INPUT_FLOAT: {
            imgui_input_float(label_cstr, &w->value.val_f32);
        } break;

        case INPUT_VEC2: {
            imgui_input_float2(label_cstr, (f32 *)&w->value.val_vec2);
        } break;

        case INPUT_VEC3: {
            imgui_input_float3(label_cstr, (f32 *)&w->value.val_vec3);
        } break;

        case INPUT_VEC4: {
            imgui_input_float4(label_cstr, (f32 *)&w->value.val_vec4);
        } break;

        case CHECKBOX: {
            imgui_checkbox(label_cstr, (bool *)&w->value.val_bool);
        } break;

        case DRAG_INT: {
            i32 *args_i32 = (i32*)w->secondary_args;
            f32 *args_f32 = (f32*)w->secondary_args;
            f32 v = args_f32[0];
            i32 min = args_i32[1];
            i32 max = args_i32[2];
            imgui_drag_int(label_cstr, &w->value.val_i32, v, min, max); 
        } break;

        case SLIDER_FLOAT:
        case SLIDER_FLOAT_LOG: {
            f32 *args_f32 = (f32*)w->secondary_args;
            f32 min = args_f32[0];
            f32 max = args_f32[1];
            imgui_slider_float(label_cstr, &w->value.val_f32, min, max, w->kind == SLIDER_FLOAT_LOG); 
        } break;

        case COLOR_PICKER: {
            imgui_color_picker(label_cstr, (f32 *)&w->value.val_vec4);
        } break;
    }
}

static inline void draw_uniform(const Uniform *u)
{
    imgui_table_next_row();
    imgui_table_next_col();
    imgui_textf("%s " SV_FMT, TYPE_TO_STR[u->type], SV_ARG(u->name));
    imgui_table_next_col();
    SelValue v = u->exe->cached_computed_value;
    switch (u->type) {
        case TYPE_BOOL:    imgui_textf(v.val_bool ? "= true" : "= false"); break;
        case TYPE_INT:     imgui_textf("= %d", v.val_i32);  break;
        case TYPE_UINT:    imgui_textf("= %u", v.val_u32);  break;
        case TYPE_FLOAT:   imgui_textf("= %f", (f64)v.val_f32);  break;
        case TYPE_VEC2:    imgui_textf("= {%f, %f}", (f64)v.val_vec2.x, (f64)v.val_vec2.y);  break;
        case TYPE_VEC3:    imgui_textf("= {%f, %f, %f}", (f64)v.val_vec3.x, (f64)v.val_vec3.y, (f64)v.val_vec3.y);  break;
        case TYPE_VEC4:    imgui_textf("= {%f, %f, %f, %f}", (f64)v.val_vec4.x, (f64)v.val_vec4.y, (f64)v.val_vec4.y, (f64)v.val_vec4.w);  break;
        case TYPE_IVEC2:   imgui_textf("= {%d, %d}", v.val_ivec2.x, v.val_ivec2.y);  break;
        case TYPE_IVEC3:   imgui_textf("= {%d, %d, %d}", v.val_ivec3.x, v.val_ivec3.y, v.val_ivec3.y);  break;
        case TYPE_IVEC4:   imgui_textf("= {%d, %d, %d, %d}", v.val_ivec4.x, v.val_ivec4.y, v.val_ivec4.y, v.val_ivec4.w);  break;
        case TYPE_MAT2:    imgui_textf("= mat2 TODO"); break;
        case TYPE_MAT3:    imgui_textf("= mat3 TODO"); break;
        case TYPE_MAT4:    imgui_textf("= mat4 TODO"); break;
        case TYPE_TEXTURE: imgui_textf("="); imgui_textf(u->exe->source_code); break;
        case TYPE_STR:     imgui_textf("str TODO"); break;
        case TYPE_NIL:
        case TYPE_AND_NAMECHECKER_ERROR_:
        case N_TYPES:
            log_error("Strange logic error that shouldn't happen<%s:%d>", __FILE__, __LINE__);
    }
    imgui_newline();
}


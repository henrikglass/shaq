/*--- Include files ---------------------------------------------------------------------*/

#include "gui.h"
#include "imguic.h"
#include "shaq_core.h"
#include "alloc.h"
#include "constants.h"
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
    DynamicGuiItemKind kind;
    u8 secondary_args[64];
    b8 touched_this_frame;
    b8 spawned_this_frame;
} DynamicGuiItem;

/*--- Private function prototypes -------------------------------------------------------*/

static inline void draw_and_update_dynamic_item(DynamicGuiItem *item);
static inline void draw_uniform(const Uniform *u);

/*--- Public variables ------------------------------------------------------------------*/

/*--- Private variables -----------------------------------------------------------------*/

static struct
{
    Array(DynamicGuiItem, SHAQ_MAX_N_DYNAMIC_GUI_ITEMS) dynamic_items;
    b8 dark_mode;
    float smoothed_deltatime;
} gui;

/*--- Public functions ------------------------------------------------------------------*/

void gui_begin_frame()
{
    imgui_begin_frame();
}

void gui_begin_main_window()
{
    gui.smoothed_deltatime = 0.97f*gui.smoothed_deltatime +
                             0.03f*shaq_deltatime();
    imgui_begin("##Main window");
    imgui_textf("Frame time: %3.1f ms", (f64)(1000.0f*gui.smoothed_deltatime)); imgui_newline();
    imgui_textf("FPS: %d", (i32)(1.0f/gui.smoothed_deltatime + 0.5f)); imgui_newline();
    imgui_separator();
    imgui_textf("Controls:"); imgui_newline();
    imgui_textf("d      -  toggle light/dark mode"); imgui_newline();
    imgui_textf("f      -  toggle fullscreen"); imgui_newline();
    imgui_textf("h      -  hide/show GUI "); imgui_newline();
    imgui_textf("esq/q  -  exit"); imgui_newline();
    imgui_separator();
}

i32 gui_draw_shader_display_selector(i32 current_idx, Shader *shaders, u32 n_shaders)
{
    Shader *current = &shaders[current_idx];
    char *current_name_cstr = arena_alloc(g_frame_arena, current->name.length + 1);
    memcpy(current_name_cstr, current->name.start, current->name.length);
    current_name_cstr[current->name.length] = '\0';

    if (imgui_begin_combo("Visible Shader", current_name_cstr)) {
        for (u32 i = 0; i < n_shaders; i++) {
            Shader *s = &shaders[i];
            b8 is_selected = (current == s);

            char *s_name_cstr = arena_alloc(g_frame_arena, s->name.length + 1);
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

void gui_draw_shader(const Shader *s)
{
    imgui_textf("[" SV_FMT "]\n    source = " SV_FMT , 
                SV_ARG(s->name), SV_ARG(s->filepath));
    imgui_newline();

    imgui_begin_table(" ", 2);
    for (u32 j = 0; j < s->uniforms.count; j++) {
        draw_uniform(&s->uniforms.arr[j]);
    }
    imgui_end_table();
    imgui_separator();
}

void gui_draw_dymanic_gui_items()
{
    imgui_textf("User-defined items:"); imgui_newline();
    for (u32 i = 0; i < gui.dynamic_items.count; i++) {
        DynamicGuiItem *item = &gui.dynamic_items.arr[i];
        draw_and_update_dynamic_item(item);
    }
    imgui_separator();
}

void gui_end_main_window()
{
    imgui_end();
}

void gui_end_frame()
{
    imgui_end_frame();
    for (u32 i = 0; i < gui.dynamic_items.count; i++) {
        DynamicGuiItem *item = &gui.dynamic_items.arr[i];
        if (item->touched_this_frame) {
            item->touched_this_frame = false;
            item->spawned_this_frame = false;
            continue;
        }
        array_delete(&gui.dynamic_items, i);
    }
}

void gui_draw_log()
{
    imgui_textf("Log:"); 
    imgui_newline();
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
    //u32 info_len;
    //const char *info = log_get_info_log(&info_len);
    //imgui_text_unformatted(info, info_len); 
    //imgui_textf("Error log:"); 
    //imgui_newline();
    //u32 error_len;
    //const char *error = log_get_error_log(&error_len);
    //imgui_text_unformatted(error, error_len); 
}

void gui_toggle_darkmode()
{
    gui.dark_mode = !gui.dark_mode;
    imgui_set_darkmode(gui.dark_mode);
}

SelValue gui_get_dynamic_item_value(StringView label,
                                    DynamicGuiItemKind kind,
                                    void *secondary_args,
                                    u32 secondary_args_size)
{
    // TODO cache items somehow?

    /* try lookup */
    for (u32 i = 0; i < gui.dynamic_items.count; i++) {
        DynamicGuiItem *item = &gui.dynamic_items.arr[i];
        if (item->kind != kind) {
            continue;
        }
        if (!sv_equals(item->label, label)) {
            continue;
        }
        if (0 != memcmp(item->secondary_args, secondary_args, secondary_args_size)) {
            continue;
        }
        item->touched_this_frame = true;
        return item->value;
    }
    
    /* not found - insert */
    DynamicGuiItem item = (DynamicGuiItem) {
        .label              = label,
        .kind               = kind,
        .touched_this_frame = true,
        .spawned_this_frame = true,
    };
    memcpy(item.secondary_args, secondary_args, secondary_args_size);
    array_push(&gui.dynamic_items, item);

    return (SelValue) {0};
}

/*--- Private functions -----------------------------------------------------------------*/

static inline void draw_and_update_dynamic_item(DynamicGuiItem *item)
{
    char *label_cstr = arena_alloc(g_frame_arena, item->label.length + 1);
    memcpy(label_cstr, item->label.start, item->label.length);
    label_cstr[item->label.length] = '\0';

    switch (item->kind) {
        case INPUT_FLOAT: {
            f32 *args_f32 = (f32*)item->secondary_args;
            float default_value = args_f32[0];
            if (item->spawned_this_frame) {
                item->value.val_f32 = default_value;
            }
            imgui_input_float(label_cstr, &item->value.val_f32);
        } break;

        case INPUT_VEC2: {
            Vec2 *args_vec2 = (Vec2*)item->secondary_args;
            Vec2 default_value = args_vec2[0];
            if (item->spawned_this_frame) {
                item->value.val_vec2 = default_value;
            }
            imgui_input_float2(label_cstr, (float *)&item->value.val_vec2);
        } break;

        case INPUT_VEC3: {
            Vec3 *args_vec4 = (Vec3*)item->secondary_args;
            Vec3 default_value = args_vec4[0];
            if (item->spawned_this_frame) {
                item->value.val_vec3 = default_value;
            }
            imgui_input_float3(label_cstr, (float *)&item->value.val_vec3);
        } break;

        case INPUT_VEC4: {
            Vec4 *args_vec4 = (Vec4*)item->secondary_args;
            Vec4 default_value = args_vec4[0];
            if (item->spawned_this_frame) {
                item->value.val_vec4 = default_value;
            }
            imgui_input_float4(label_cstr, (float *)&item->value.val_vec4);
        } break;

        case SLIDER_FLOAT:
        case SLIDER_FLOAT_LOG: {
            f32 *args_f32 = (f32*)item->secondary_args;
            float min = args_f32[0];
            float max = args_f32[1];
            float default_value = args_f32[2];
            if (item->spawned_this_frame) {
                item->value.val_f32 = default_value;
            }
            imgui_slider_float(label_cstr, &item->value.val_f32, min, max, item->kind == SLIDER_FLOAT_LOG); 
        } break;

        case COLOR_PICKER: {
            Vec4 *args_vec4 = (Vec4*)item->secondary_args;
            Vec4 default_value = args_vec4[0];
            if (item->spawned_this_frame) {
                item->value.val_vec4 = default_value;
            }
            imgui_color_picker(label_cstr, (float *)&item->value.val_vec4);
        } break;
    }
}

static inline void draw_uniform(const Uniform *u)
{
    imgui_table_next_row();
    imgui_table_next_col();
    imgui_textf("    uniform %s " SV_FMT, 
                TYPE_TO_STR[u->type], SV_ARG(u->name));
    imgui_table_next_col();
    SelValue v = u->exe->cached_computed_value;
    switch (u->type) {
        case TYPE_BOOL:   imgui_textf(v.val_bool ? " = true" : " = false"); break;
        case TYPE_INT:    imgui_textf(" = %d", v.val_i32);  break;
        case TYPE_UINT:   imgui_textf(" = %u", v.val_u32);  break;
        case TYPE_FLOAT:  imgui_textf(" = %f", (f64)v.val_f32);  break;
        case TYPE_VEC2:   imgui_textf(" = {%f, %f}", (f64)v.val_vec2.x, (f64)v.val_vec2.y);  break;
        case TYPE_VEC3:   imgui_textf(" = {%f, %f, %f}", (f64)v.val_vec3.x, (f64)v.val_vec3.y, (f64)v.val_vec3.y);  break;
        case TYPE_VEC4:   imgui_textf(" = {%f, %f, %f, %f}", (f64)v.val_vec4.x, (f64)v.val_vec4.y, (f64)v.val_vec4.y, (f64)v.val_vec4.w);  break;
        case TYPE_IVEC2:  imgui_textf(" = {%d, %d}", v.val_ivec2.x, v.val_ivec2.y);  break;
        case TYPE_IVEC3:  imgui_textf(" = {%d, %d, %d}", v.val_ivec3.x, v.val_ivec3.y, v.val_ivec3.y);  break;
        case TYPE_IVEC4:  imgui_textf(" = {%d, %d, %d, %d}", v.val_ivec4.x, v.val_ivec4.y, v.val_ivec4.y, v.val_ivec4.w);  break;
        case TYPE_MAT2:   imgui_textf(" = mat2 TODO"); break;
        case TYPE_MAT3:   imgui_textf(" = mat3 TODO"); break;
        case TYPE_MAT4:   imgui_textf(" = mat4 TODO"); break;
        case TYPE_TEXTURE: imgui_textf(" ="); imgui_textf(u->exe->source_code); break;
        case TYPE_STR: imgui_textf("str TODO"); break;
        case TYPE_NIL:
        case TYPE_AND_NAMECHECKER_ERROR_:
        case N_TYPES:
            log_error("Strange logic error that shouldn't happen<%s:%d>", __FILE__, __LINE__);
    }
    imgui_newline();
}



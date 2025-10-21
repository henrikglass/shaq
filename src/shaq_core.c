/*--- Include files ---------------------------------------------------------------------*/

#include "shaq_core.h"

#include "sel.h"
#include "alloc.h"
#include "constants.h"
#include "shader.h"
#include "uniform.h"
#include "texture.h"
#include "util.h"
#include "io.h"
#include "renderer.h"
#include "gui.h"
#include "log.h"

#define HGL_INI_ALLOC session_fs_alloc
#define HGL_INI_REALLOC session_fs_realloc
#define HGL_INI_FREE dummy_free /* handled by call to `hgl_free_all(g_session_fs_allocator)` */
#define HGL_INI_IMPLEMENTATION
#include "hgl_ini.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include <GLFW/glfw3.h>

/*--- Private macros --------------------------------------------------------------------*/

/*--- Private type definitions ----------------------------------------------------------*/

/*--- Private function prototypes -------------------------------------------------------*/

static b8 session_reload_needed(void);
static void reload_session(void);
static i32 satisfy_dependencies_for_shader(u32 index, u32 depth);
static void determine_render_order(void);
static i32 load_state_from_ini(HglIni *ini); // TODO better name
static void shaq_atexit_(void);

/*--- Public variables ------------------------------------------------------------------*/

/*--- Private variables -----------------------------------------------------------------*/

static struct {

    HglIni *ini; 
    char ini_filepath[SHAQ_FILEPATH_MAX_LEN];
    b8 has_ini_filepath;
    i64 ini_modifytime;
    Array(Shader, SHAQ_MAX_N_SHADERS) shaders;
    Array(u32, SHAQ_MAX_N_SHADERS) render_order;
    Array(Texture, SHAQ_MAX_N_LOADED_TEXTURES) loaded_textures;
    i32 visible_shader_idx;
    b8 quiet;
    b8 should_reload;

    u64 start_timestamp_ns;
    u64 last_frame_timestamp_ns;
    f32 last_frame_deltatime_s;
    f32 last_frame_time_s;
} shaq = {0};

/*--- Public functions ------------------------------------------------------------------*/

void shaq_begin(const char *ini_filepath, bool quiet)
{
    atexit(shaq_atexit_);

    alloc_init();
    renderer_init();

    if (ini_filepath != NULL) {
        strncpy(shaq.ini_filepath, ini_filepath, SHAQ_FILEPATH_MAX_LEN - 1);
        shaq.has_ini_filepath = true;
    }

    shaq.start_timestamp_ns = util_get_time_nanos();
    shaq.last_frame_timestamp_ns = shaq.start_timestamp_ns;
    shaq.visible_shader_idx = (u32) -1;
    shaq.quiet = quiet;

    reload_session();
}

b8 shaq_should_close()
{
    return renderer_should_close();
}

void shaq_new_frame()
{
    if (session_reload_needed()) {
        reload_session();
    }

    /* compute time */
    u64 now_ns = util_get_time_nanos();
    u64 dt_ns = now_ns - shaq.last_frame_timestamp_ns;
    u64 t_ns = now_ns - shaq.start_timestamp_ns;
    shaq.last_frame_timestamp_ns = now_ns;
    shaq.last_frame_deltatime_s = (f32)((f64)dt_ns / 1000000000.0);
    shaq.last_frame_time_s = (f32)((f64)t_ns / 1000000000.0);

    /* Draw individual shaders onto individual offscreen framebuffer textures */
    for (u32 i = 0; i < shaq.render_order.count; i++) {
        u32 index = shaq.render_order.arr[i];
        Shader *s  = &shaq.shaders.arr[index];
        assert(s != NULL);
        renderer_do_shader_pass(s);
    }

    /* Do final pass (render to framebuffer) */
    renderer_begin_final_pass();
    if (renderer_shader_view_is_maximized()) {
        /* No GUI: Draw visible shader directly onto the default frame buffer */
        renderer_draw_fullscreen_shader(&shaq.shaders.arr[shaq.visible_shader_idx]);
    } else {
        /* Show GUI */
        gui_begin_frame();

        /* Draw visible shader into shader window */
        if (gui_begin_shader_window()) {
            if (shaq.visible_shader_idx != -1) {
                gui_draw_shader(&shaq.shaders.arr[shaq.visible_shader_idx]);
            }
        }
        gui_end_shader_window();

        /* Draw main window */
        if (gui_begin_main_window()) {
            shaq.visible_shader_idx = gui_draw_shader_display_selector(shaq.visible_shader_idx, 
                                                                       shaq.shaders.arr, 
                                                                       shaq.shaders.count);
            gui_draw_dymanic_gui_items();
            for (u32 i = 0; i < shaq.render_order.count; i++) {
                Shader *s  = &shaq.shaders.arr[i];
                assert(s != NULL);
                gui_draw_shader_info(s);
            }
        }
        gui_end_main_window();

        /* Draw log window */
        gui_draw_log_window();

        /* Draw menu bar */
        gui_draw_menu_bar();

        /* Draw file dialog (maybe) */
        if (gui_draw_file_dialog(shaq.ini_filepath)) {
            shaq.has_ini_filepath = true;
            shaq.should_reload = true;
        }

        gui_end_frame();
    }
    renderer_end_final_pass();

    /* collect garbage */
    hgl_free_all(g_frame_arena);
}

void shaq_end()
{
    return; // TODO
}

f32 shaq_time()
{
    return shaq.last_frame_time_s;
}

f32 shaq_deltatime()
{
    return shaq.last_frame_deltatime_s;
}

IVec2 shaq_iresolution()
{
    if (renderer_shader_view_is_maximized()) {
        return renderer_window_size();
    } 
    return gui_shader_window_size();
}

Vec2 shaq_mouse_position()
{
    Vec2 mouse_pos = renderer_mouse_position();
    if (renderer_shader_view_is_maximized()) {
        return mouse_pos;
    } 
    IVec2 shader_window_pos = gui_shader_window_position();
    mouse_pos.x -= shader_window_pos.x;
    mouse_pos.y -= shader_window_pos.y;
    return mouse_pos;
}

Vec2 shaq_mouse_drag_position()
{
    Vec2 mouse_pos = renderer_mouse_drag_position();
    if (renderer_shader_view_is_maximized()) {
        return mouse_pos;
    } 
    IVec2 shader_window_pos = gui_shader_window_position();
    mouse_pos.x -= shader_window_pos.x;
    mouse_pos.y -= shader_window_pos.y;
    return mouse_pos;
}

b8 shaq_mouse_left_button_is_down()
{
    return renderer_mouse_left_button_is_down();
}

b8 shaq_mouse_right_button_is_down()
{
    return renderer_mouse_right_button_is_down();
}

b8 shaq_mouse_left_button_was_clicked()
{
    return renderer_mouse_left_button_was_clicked();
}

b8 shaq_mouse_right_button_was_clicked()
{
    return renderer_mouse_right_button_was_clicked();
}

void shaq_reset_time()
{
    shaq.start_timestamp_ns = util_get_time_nanos();
}

i32 shaq_find_shader_id_by_name(StringView name)
{
    /* look up shader id */
    for (u32 i = 0; i < shaq.shaders.count; i++) {
        Shader *s  = &shaq.shaders.arr[i];
        if (sv_equals(s->name, name)) {
            return i;
        }
    }

    /* no shader with name `name` found */
    log_error("No shader with name \"" SV_FMT "\" found.\n", SV_ARG(name));
    return -1;    
}

i32 shaq_load_texture_if_necessary(StringView filepath)
{
    /* look up texture id */
    for (u32 i = 0; i < shaq.loaded_textures.count; i++) {
        Texture *t  = &shaq.loaded_textures.arr[i];
        if (sv_equals(t->filepath, filepath)) {
            return i;
        }
    }

    /* not found? load it.*/
    Texture t = texture_load_from_file(filepath);
    if (t.data != NULL) {
        array_push(&shaq.loaded_textures, t);
        return shaq.loaded_textures.count - 1;
    }

    /* Unable to load texture */
    log_error("Unable to load texture from \"" SV_FMT "\".\n", SV_ARG(filepath));
    return -1; 
}

u32 shaq_get_shader_render_texture_by_index(u32 index)
{
    return shaq.shaders.arr[index].render_texture.gl_texture_id;
}

u32 shaq_get_loaded_texture_by_index(u32 index)
{
    return shaq.loaded_textures.arr[index].gl_texture_id;
}

/*--- Private functions -----------------------------------------------------------------*/

static b8 session_reload_needed()
{
    if (shaq.should_reload) {
        return true;
    }

    /* Return early if no filepath is set */
    if (!shaq.has_ini_filepath) {
        return false;
    }

    i64 ts = io_get_file_modify_time(shaq.ini_filepath, true);
    if (ts == -1) {
        return false; // File is probably in the process of being saved. Hold off for a bit.
    } else if (shaq.ini_modifytime != ts) {
        return true;
    }

    for (u32 i = 0; i < shaq.shaders.count; i++) {
        if (shader_was_modified(&shaq.shaders.arr[i])) {
            return true;
        }
    }

    if (renderer_should_reload()) {
        return true;
    }

    if (gui_should_reload()) {
        return true;
    }

    return false;
}

static void reload_session()
{
    shaq.should_reload = false;

    /* Manually free OpenGL resources */
    for (u32 i = 0; i < shaq.shaders.count; i++) {
        Shader *s = &shaq.shaders.arr[i];
        shader_free_opengl_resources(s);
    }
    for (u32 i = 0; i < shaq.loaded_textures.count; i++) {
        Texture *t = &shaq.loaded_textures.arr[i];
        texture_free_opengl_resources(t);
    }

    /* reset state */
    array_clear(&shaq.shaders);
    array_clear(&shaq.render_order);
    array_clear(&shaq.loaded_textures);
    log_clear_all_logs();

    /* "Reload" renderer & GUI */
    renderer_reload();
    gui_reload();

    /* collect garbage */
    hgl_free_all(g_session_arena);
    hgl_free_all(g_session_fs_allocator);

    /* Return early if no filepath is set */
    if (!shaq.has_ini_filepath) {
        return;
    }

    /* Reload ini file */
    shaq.ini_modifytime = io_get_file_modify_time(shaq.ini_filepath, false);
    shaq.ini = hgl_ini_open(shaq.ini_filepath);
    if (shaq.ini == NULL) {
        log_error("Failed to open or parse *.ini file.");
        goto out_error;
    }

    /* Parse ini file contents + recompile simple expressions */
    i32 err = load_state_from_ini(shaq.ini);
    if (err != 0) {
        log_error("Failed to load anything meaningful from *.ini file.");
        goto out_error;
    }

    /* Determine render order */
    determine_render_order(); // TODO return err?

    /* Reload shaders */
    for (u32 i = 0; i < shaq.shaders.count; i++) {
        Shader *s = &shaq.shaders.arr[i];
        shader_reload(s);
    }

    /* Reset visible shader idx if necessary */
    if ((shaq.visible_shader_idx >= (i32)shaq.shaders.count) ||
        (shaq.visible_shader_idx == -1)) {
        shaq.visible_shader_idx = shaq.render_order.arr[shaq.shaders.count - 1];
    }
    if (!shaq.quiet) {
        log_print_info_log();
        log_print_error_log();
    }

#if 0
    printf("frame arena          -- "); hgl_alloc_print_usage(g_frame_arena);
    printf("session arena        -- "); hgl_alloc_print_usage(g_session_arena);
    printf("session fs allocator -- "); hgl_alloc_print_usage(g_session_fs_allocator);
#endif

    log_info("Session reloaded successfully (%s)", io_get_timestamp_str());
    return;

out_error:
    shaq.visible_shader_idx = -1;
    if (!shaq.quiet) {
        log_print_info_log();
        log_print_error_log();
    }
    log_error("Session reload failed (%s)", io_get_timestamp_str());
}

static i32 satisfy_dependencies_for_shader(u32 index, u32 depth)
{
    Shader *s = &shaq.shaders.arr[index];

    /* recursed more times than there are shaders defined - guranteed cyclic dependency */
    if (depth > shaq.shaders.count + 1) {
        log_error("Cyclic dependency between shaders.");
        return -1;
    }

    /* recursively satisfy the dependencies */
    for (u32 i = 0; i < s->shader_depends.count; i++) {
        i32 err = satisfy_dependencies_for_shader(s->shader_depends.arr[i], depth + 1);
        if (err != 0) return err;
    }

    /* append shader to the end of the render order if not already present */
    for (u32 i = 0; i < shaq.render_order.count; i++) {
        if (shaq.render_order.arr[i] == index) return 0; 
    }
    array_push(&shaq.render_order, index);

    return 0;
}

static void determine_render_order()
{
    array_clear(&shaq.render_order);

    /* determine per-shader dependencies (on other shaders) */
    for (u32 i = 0; i < shaq.shaders.count; i++) {
        shader_determine_dependencies(&shaq.shaders.arr[i]);
    }

    /* satisfy dependencies (on other shaders) for each shader */
    for (u32 i = 0; i < shaq.shaders.count; i++) {
        i32 err = satisfy_dependencies_for_shader(i, 0);
        if (err != 0) {
            log_error("Could not determine a render order for shader: " 
                      SV_FMT ".", SV_ARG(shaq.shaders.arr[i].name));
        } else {
            log_info("Succesfully dermined render order for shader: " 
                     SV_FMT ".", SV_ARG(shaq.shaders.arr[i].name));
        }
    }
}

static i32 load_state_from_ini(HglIni *ini)
{
    hgl_ini_reset_section_iterator(ini);
    u32 shader_count = 0; 
    while (true) {
        HglIniSection *s = hgl_ini_next_section(ini);
        if(s == NULL) {
            break;
        }
        int err = shader_parse_from_ini_section(&shaq.shaders.arr[shader_count], s);
        if (err == 0) {
            shader_count++;
        }
    }
    shaq.shaders.count = shader_count;
    return (shaq.shaders.count != 0) ? 0 : -1;
}

static void shaq_atexit_()
{
    log_print_info_log();
    log_print_error_log();

    renderer_final();
    alloc_final();
}

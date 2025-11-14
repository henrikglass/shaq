/*--- Include files ---------------------------------------------------------------------*/

#include "shaq_core.h"

#include "sel.h"
#include "alloc.h"
#include "shaq_config.h"
#include "shader.h"
#include "uniform.h"
#include "texture.h"
#include "user_input.h"
#include "util.h"
#include "io.h"
#include "renderer.h"
#include "gui.h"
#include "log.h"
#include "image.h"

#define HGL_INI_ALLOC r2r_fs_alloc
#define HGL_INI_REALLOC r2r_fs_realloc
#define HGL_INI_FREE dummy_free /* handled by call to `hgl_free_all(g_session_fs_allocator)` */
#define HGL_INI_IMPLEMENTATION
#include "hgl_ini.h"

#if SHAQ_PROFILE
#define HGL_PROFILE_IMPLEMENTATION
#include "hgl_profile.h"
#endif

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include <GLFW/glfw3.h>

/*--- Private macros --------------------------------------------------------------------*/

/*--- Private type definitions ----------------------------------------------------------*/

/*--- Private function prototypes -------------------------------------------------------*/

static b8 session_reload_needed(void);
static i32 reload_session(void);
static i32 satisfy_dependencies_for_shader(u32 index, u32 depth);
static void determine_render_order(void);
static i32 load_state_from_project_ini(HglIni *project_ini); // TODO better name
static void shaq_atexit_(void);

/*--- Public variables ------------------------------------------------------------------*/

/*--- Private variables -----------------------------------------------------------------*/

static struct
{
    HglIni *project_ini; 
    char project_ini_filepath[SHAQ_FILEPATH_MAX_LEN];
    b8 project_ini_loaded;
    b8 project_ini_changed;
    i64 project_ini_modifytime;
    struct {
        const char *name;
        const char *desc;
    } project_info;

    Array(Shader, SHAQ_MAX_N_SHADERS) shaders;
    Array(u32, SHAQ_MAX_N_SHADERS) render_order;
    Array(Texture, SHAQ_MAX_N_LOADED_TEXTURES) textures;
    i32 visible_shader_idx;
    b8 quiet;
    b8 should_reload;
    b8 reloaded_this_frame;
    b8 reloaded_last_frame;

    i32 frame_count;
    b8 time_paused;
    u64 timestamp_ns;
    u64 time_ns;
    f32 deltatime_s;
    f32 time_s;
} shaq = {0};

/*--- Public functions ------------------------------------------------------------------*/

void shaq_begin(const char *project_ini_filepath, bool quiet)
{
    atexit(shaq_atexit_);

    alloc_init();
    renderer_init();

    if (project_ini_filepath != NULL) {
        strncpy(shaq.project_ini_filepath, project_ini_filepath, SHAQ_FILEPATH_MAX_LEN - 1);
        shaq.project_ini_loaded = true;
    }

    shaq.time_ns = 0;
    shaq.frame_count = 0;
    shaq.timestamp_ns = util_get_time_nanos();
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
    /* Reload if necessary */
    shaq.reloaded_last_frame = shaq.reloaded_this_frame;
    shaq.reloaded_this_frame = false;
    if (session_reload_needed()) {
        i32 err = reload_session();
        if (err == 0) {
            shaq.reloaded_this_frame = true;
        }
    }

    /* compute time */
    u64 now_ns = util_get_time_nanos();
    u64 dt_ns = now_ns - shaq.timestamp_ns;
    shaq.deltatime_s = (f32)((f64)dt_ns / 1000000000.0);
    shaq.timestamp_ns = now_ns;
    shaq.frame_count++;
    if (!shaq.time_paused) {
        shaq.time_ns += dt_ns;
        shaq.time_s = (f32)((f64)shaq.time_ns / 1000000000.0);
    }

    /* for all shaders: swap current and last frame render textures */
    for (u32 i = 0; i < shaq.render_order.count; i++) {
        u32 index = shaq.render_order.arr[i];
        Shader *s  = &shaq.shaders.arr[index];
        shader_swap_render_textures(s);
    }

    /* poll inputs */
    user_input_poll();

    /* Draw individual shaders onto individual offscreen framebuffer textures */
    for (u32 i = 0; i < shaq.render_order.count; i++) {
        u32 index = shaq.render_order.arr[i];
        Shader *s  = &shaq.shaders.arr[index];
        shader_update_uniforms(s);
        renderer_do_shader_pass(s);
    }

    /* Do final pass (render to framebuffer) */
    renderer_begin_final_pass();
    gui_begin_frame();
    if (renderer_shader_view_is_maximized()) {
        //renderer_clear_current_framebuffer();

        /* Draw visible shader directly onto the default frame buffer */
        if (shaq.visible_shader_idx != -1) {
            renderer_draw_fullscreen_shader(&shaq.shaders.arr[shaq.visible_shader_idx]);
        }

        /* Draw error log overlay if there are errors */
        if (!log_error_log_is_empty()) {
            gui_draw_error_log_overlay();
        }
    } else {

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
            gui_draw_widgets();
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
    }

    /* Draw file dialog (maybe) */
    if (gui_file_dialog_is_open()) {
        b8 file_selected = gui_draw_file_dialog(shaq.project_ini_filepath);
        if (file_selected) {
            shaq.project_ini_loaded  = true;
            shaq.should_reload       = true;
            shaq.project_ini_changed = true;
        }
    }

    /* end of frame */
    gui_end_frame();
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
    return shaq.time_s;
}

f32 shaq_deltatime()
{
    return shaq.deltatime_s;
}

i32 shaq_frame_count()
{
    return shaq.frame_count;
}

void shaq_reset_time()
{
    shaq.time_s      = 0.0f;
    shaq.time_ns     = 0;
    shaq.frame_count = 0;
}

void shaq_toggle_time_pause()
{
    shaq.time_paused = !shaq.time_paused;
}

b8 shaq_reloaded_this_frame()
{
    return shaq.reloaded_this_frame;
}

b8 shaq_reloaded_last_frame()
{
    return shaq.reloaded_last_frame;
}

b8 shaq_has_loaded_project()
{
    return shaq.project_ini_loaded;
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

i32 shaq_find_texture_id_by_name(StringView filepath, b8 load_if_necessary)
{
    /* look up texture id */
    for (u32 i = 0; i < shaq.textures.count; i++) {
        Texture *t  = &shaq.textures.arr[i];
        if (sv_equals(t->img->filepath, filepath)) {
            return i;
        }
    }

    if (!load_if_necessary) {
        return -1;
    }

    /* not found? load it.*/
    Texture t = texture_load_from_file(filepath);
    if (t.img->data != NULL) {
        array_push(&shaq.textures, t);
        return shaq.textures.count - 1;
    }

    /* Unable to load texture */
    log_error("Unable to load texture from \"" SV_FMT "\".\n", SV_ARG(filepath));
    return -1; 
}

Shader *shaq_get_shader_by_id(i32 id)
{
    if (id > (i32)shaq.shaders.count || id < 0) {
        return NULL;
    }
    return &shaq.shaders.arr[id];
}

Texture *shaq_get_texture_by_id(i32 id)
{
    if (id > (i32)shaq.textures.count || id < 0) {
        return NULL;
    }
    return &shaq.textures.arr[id];
}


/*--- Private functions -----------------------------------------------------------------*/

static b8 session_reload_needed()
{
    if (shaq.should_reload || user_input_should_reload()) {
        return true;
    }

    /* Return early if no filepath is set */
    if (!shaq.project_ini_loaded) {
        return false;
    }

    i64 ts = io_get_file_modify_time(shaq.project_ini_filepath, true);
    if (ts == -1) {
        /* 
         * Unable to get file modify time (after many tries.. -.-). Assume no
         * such file exists.
         */
        shaq.project_ini_loaded = false;
        return false;
    } else if (shaq.project_ini_modifytime != ts) {
        return true;
    }

    for (u32 i = 0; i < shaq.shaders.count; i++) {
        if (shader_was_modified(&shaq.shaders.arr[i])) {
            return true;
        }
    }

    return renderer_should_reload() ||
           gui_should_reload();
}

static i32 reload_session()
{
#if SHAQ_PROFILE
    hgl_profile_begin("reload session");
#endif

    shaq.should_reload = false;

    /* Manually free OpenGL resources */
    for (u32 i = 0; i < shaq.shaders.count; i++) {
        Shader *s = &shaq.shaders.arr[i];
        shader_free_opengl_resources(s);
    }
    for (u32 i = 0; i < shaq.textures.count; i++) {
        Texture *t = &shaq.textures.arr[i];
        texture_free(t);
    }

    /* Upon loading a new project clear some additional stuff */
    if (shaq.project_ini_changed) {
        image_free_all_cached_images();
        gui_clear_widgets();
        shaq_reset_time();
        shaq.visible_shader_idx = -1;
        shaq.project_info.name = NULL;
        shaq.project_info.desc = NULL;
        shaq.project_ini_changed = false;
    }

    /* reset state */
    array_clear(&shaq.shaders);
    array_clear(&shaq.render_order);
    array_clear(&shaq.textures);
    log_clear_all_logs();

    /* "Reload" renderer & GUI */
    renderer_reload();
    gui_reload();

    /* collect garbage */
    hgl_free_all(g_r2r_arena);
    hgl_free_all(g_r2r_fs_allocator);

    /* Return early if no filepath is set */
    if (!shaq.project_ini_loaded) {
#if SHAQ_PROFILE
        hgl_profile_end();
#endif
        return -1;
    }

    /* Reload project ini file */
    shaq.project_ini_modifytime = io_get_file_modify_time(shaq.project_ini_filepath, false);
    shaq.project_ini = hgl_ini_open(shaq.project_ini_filepath);
    if (shaq.project_ini == NULL) {
        log_error("Failed to open or parse *.ini file.");
        goto out_error;
    }

    /* Parse project ini file contents + recompile simple expressions */
    i32 err = load_state_from_project_ini(shaq.project_ini);
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
    printf("frame arena      -- "); hgl_alloc_print_usage(g_frame_arena);
    printf("r2r arena        -- "); hgl_alloc_print_usage(g_r2r_arena);
    printf("r2r fs allocator -- "); hgl_alloc_print_usage(g_r2r_fs_allocator);
    printf("image allocator  -- "); hgl_alloc_print_usage(g_image_allocator);
#endif

#if SHAQ_PROFILE
    hgl_profile_end();
    hgl_profile_report(HGL_PROFILE_TIME_ALL);
#endif
    log_info("name = \"%s\"", shaq.project_info.name);
    log_info("desc = \"%s\"", shaq.project_info.desc);
    log_info("Session reloaded successfully (%s)", io_get_timestamp_str());
    return 0;

out_error:
#if SHAQ_PROFILE
    hgl_profile_end();
#endif
    shaq.visible_shader_idx = -1;
    if (!shaq.quiet) {
        log_print_info_log();
        log_print_error_log();
    }
    log_error("Session reload failed (%s)", io_get_timestamp_str());
    return -1;
}

static i32 satisfy_dependencies_for_shader(u32 index, u32 depth)
{
    Shader *s = &shaq.shaders.arr[index];

    /* recursed more times than there are shaders defined - possible cyclic dependency */
    if (depth > shaq.shaders.count + 1) {
        log_error("Possible cyclic dependency between shaders.");
        return -1;
    }

    /* recursively satisfy the dependencies */
    for (u32 i = 0; i < s->shader_depends.count; i++) {
        i32 err = satisfy_dependencies_for_shader(s->shader_depends.arr[i], depth + 1);
        if (err != 0) {
            return err;
        }
    }

    /* append shader to the end of the render order if not already present */
    for (u32 i = 0; i < shaq.render_order.count; i++) {
        if (shaq.render_order.arr[i] == index) {
            return 0; 
        }
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
            log_info("Successfully dermined render order for shader: " 
                     SV_FMT ".", SV_ARG(shaq.shaders.arr[i].name));
        }
    }
}

static i32 load_state_from_project_ini(HglIni *project_ini)
{
    hgl_ini_reset_section_iterator(project_ini);
    u32 shader_count = 0; 
    while (true) {
        HglIniSection *s = hgl_ini_next_section(project_ini);
        if(s == NULL) {
            break;
        }

        /* Handle project info section */
        if (0 == strcasecmp(s->name, "Project")) {
            shaq.project_info.name = hgl_ini_get_in_section(s, "name");  
            shaq.project_info.desc = hgl_ini_get_in_section(s, "description");  
            continue;
        }

        /* Handle shader sections */
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

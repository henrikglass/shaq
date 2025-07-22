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

//#include "glad/glad.h"

void *ini_alloc(size_t size);
void *ini_realloc(void *ptr, size_t size);
void ini_free(void *ptr);
void *ini_alloc(size_t size){ return fs_alloc(g_longterm_fs_allocator, size);}
void *ini_realloc(void *ptr, size_t size){ return fs_realloc(g_longterm_fs_allocator, ptr, size);}
void ini_free(void *ptr){ (void) ptr; /*fs_free(g_longterm_fs_allocator, ptr); */}
#define HGL_INI_ALLOC ini_alloc
#define HGL_INI_REALLOC ini_realloc
#define HGL_INI_FREE ini_free
#define HGL_INI_IMPLEMENTATION
#include "hgl_ini.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include <GLFW/glfw3.h>

/*--- Private macros --------------------------------------------------------------------*/

/*--- Private type definitions ----------------------------------------------------------*/

/*--- Private function prototypes -------------------------------------------------------*/

static i32 satisfy_dependencies_for_shader(u32 index, u32 depth);
static void determine_render_order(void);
static void parse_ini_file(HglIni *ini);

/*--- Public variables ------------------------------------------------------------------*/

/*--- Private variables -----------------------------------------------------------------*/

//typedef Array(Shader, SHAQ_MAX_N_SHADERS) Shaders;
//typedef Array(u32, SHAQ_MAX_N_UNIFORMS) ShaderIndices;

static struct {

    HglIni *ini; 
    const char *ini_filepath;
    i64 ini_modifytime;
    Array(Shader, SHAQ_MAX_N_SHADERS) shaders;
    Array(u32, SHAQ_MAX_N_SHADERS) render_order;
    Array(Texture, SHAQ_MAX_N_LOADED_TEXTURES) loaded_textures;

    b8 should_close;

    u64 start_timestamp_ns;
    u64 last_frame_timestamp_ns;
    f32 last_frame_deltatime_s;
    f32 last_frame_time_s;
} shaq = {0};

/*--- Public functions ------------------------------------------------------------------*/

void shaq_begin(const char *ini_filepath)
{
    alloc_init();
    renderer_init();

    shaq.start_timestamp_ns = util_get_time_nanos();
    shaq.last_frame_timestamp_ns = shaq.start_timestamp_ns;
    shaq.ini_filepath = ini_filepath;

    shaq_reload();
}

b8 shaq_needs_reload(void)
{
    i64 ts = io_get_file_modify_time(shaq.ini_filepath);
    if (ts == -1) {
        return false; // File is probably in the process of being saved. Hold off for a bit.
    } else if (shaq.ini_modifytime != ts) {
        return true;
    }

    for (u32 i = 0; i < shaq.shaders.count; i++) {
        if (shader_needs_reload(&shaq.shaders.arr[i])) {
            return true;
        }
    }

    return false;
}

b8 shaq_should_close(void)
{
    //return shaq.should_close || glfwWindowShouldClose(shaq.renderer.window);
    return shaq.should_close || 
           renderer_should_close();
}

void shaq_reload(void)
{
    printf("shaq_reload()\n");

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

    /* collect garbage */
    arena_free_all(g_longterm_arena);
    fs_free_all(g_longterm_fs_allocator);

    /* Reload ini file */
    shaq.ini_modifytime = io_get_file_modify_time(shaq.ini_filepath);
    shaq.ini = hgl_ini_open(shaq.ini_filepath);
    if (shaq.ini == NULL) {
        shaq.should_close = true;
    }

    /* Parse ini file + recompile simple expressions */
    parse_ini_file(shaq.ini);

    /* Determine render order */
    determine_render_order();

    /* Reload shaders */
    for (u32 i = 0; i < shaq.shaders.count; i++) {
        Shader *s = &shaq.shaders.arr[i];
        shader_reload(s);
    }

}

void shaq_new_frame(void)
{
    //printf("shaq_new_frame()\n");
    if (shaq.should_close) {
        return;
    }

    /* compute time */
    u64 now_ns = util_get_time_nanos();
    u64 dt_ns = now_ns - shaq.last_frame_timestamp_ns;
    u64 t_ns = now_ns - shaq.start_timestamp_ns;
    shaq.last_frame_timestamp_ns = now_ns;
    shaq.last_frame_deltatime_s = (f32)((f64)dt_ns / 1000000000.0);
    shaq.last_frame_time_s = (f32)((f64)t_ns / 1000000000.0);

    /* DEBUG */
    //struct timespec ts = {.tv_sec = 1, .tv_nsec = 166666670};
    //nanosleep(&ts, &ts);
    /* END DEBUG */


    /* TODO ... */
    for (u32 i = 0; i < shaq.render_order.count; i++) {
        u32 index = shaq.render_order.arr[i];
        Shader *s  = &shaq.shaders.arr[index];
        assert(s != NULL);
        renderer_draw_shader(s);
//#if 1
//        printf("shader[%u] = \"" HGL_SV_FMT "\"\n", i, HGL_SV_ARG(s->name));
//        printf("  filepath = " HGL_SV_FMT "\n", HGL_SV_ARG(s->filepath));
//#endif
//
//        for (u32 j = 0; j < s->uniforms.count; j++) {
//            Uniform *u = &s->uniforms.arr[j];
//            SelValue r = sel_eval(u->exe, false);
//#if 1
//            printf("  uniform[%u] %d " HGL_SV_FMT " = ", j, u->type, HGL_SV_ARG(u->name));
//            sel_print_value(u->type, r);
//#else
//            (void) r;
//#endif
//        } 
    }

    /* TODO ... */
    renderer_display(&shaq.shaders.arr[shaq.shaders.count - 1]);

    /* DEBUG */
    //printf("frame arena           -- "); hgl_arena_print_usage(g_frame_arena);
    //printf("longterm arena        -- "); hgl_arena_print_usage(g_longterm_arena);
    //printf("longterm fs allocator -- "); hgl_fs_print_usage(g_longterm_fs_allocator);
    /* END DEBUG */

    /* collect garbage */
    arena_free_all(g_frame_arena);
}

void shaq_end(void)
{
    return; // TODO
}

f32 shaq_time(void)
{
    return shaq.last_frame_time_s;
}

f32 shaq_deltatime(void)
{
    return shaq.last_frame_deltatime_s;
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
    fprintf(stderr, "[SHAQ] Error: No shader with name \"" HGL_SV_FMT "\" found.\n", HGL_SV_ARG(name));
    return -1;    
}

i32 shaq_load_texture_if_necessary(StringView filepath)
{
    /* look up texture id */
    for (u32 i = 0; i < shaq.loaded_textures.count; i++) {
        Texture *t  = &shaq.loaded_textures.arr[i];
        if (sv_equals(t->filepath, filepath)) {
            //printf("FOUND TEXTURE: " HGL_SV_FMT "\n", HGL_SV_ARG(filepath));
            return i;
        }
    }

    /* not found? load it.*/
    Texture t = texture_load_from_file(filepath);
    if (t.data != NULL) {
        //printf("LOADED TEXTURE: " HGL_SV_FMT "\n", HGL_SV_ARG(filepath));
        array_push(&shaq.loaded_textures, t);
        return shaq.loaded_textures.count - 1;
    }

    /* Unable to load texture */
    fprintf(stderr, "[SHAQ] Error: Unable to load texture from \"" HGL_SV_FMT "\".\n", HGL_SV_ARG(filepath));
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

static i32 satisfy_dependencies_for_shader(u32 index, u32 depth)
{
    Shader *s = &shaq.shaders.arr[index];

    /* recursed more times than there are shaders defined - guranteed cyclic dependency */
    if (depth > shaq.shaders.count + 1) {
        fprintf(stderr, "[SHAQ] Error: cyclic dependency between shaders.\n");
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

static void determine_render_order(void)
{
    assert(shaq.shaders.count > 0);
    array_clear(&shaq.render_order);

    /* determine per-shader dependencies (on other shaders) */
    for (u32 i = 0; i < shaq.shaders.count; i++) {
        shader_determine_dependencies(&shaq.shaders.arr[i]);
    }

    /* satisfy dependencies (on other shaders) for each shader */
    for (u32 i = 0; i < shaq.shaders.count; i++) {
        i32 err = satisfy_dependencies_for_shader(i, 0);
        if (err != 0) {
            fprintf(stderr, "[SHAQ] Error: Could not determine a render order for shader \"" 
                    HGL_SV_FMT "\".\n", HGL_SV_ARG(shaq.shaders.arr[i].name));
        }
    }

    //assert(shaq.render_order.count == shaq.shaders.count);
   
    // DEBUG 
#if 0
    printf("render order: ");
    for (u32 j = 0; j < shaq.render_order.count; j++) {
        u32 idx = shaq.render_order.arr[j];
        printf(HGL_SV_FMT " ", HGL_SV_ARG(shaq.shaders.arr[idx].name));
    }
    printf("\n");
#endif
    // END DEBUG 
}

static void parse_ini_file(HglIni *ini)
{
    hgl_ini_reset_section_iterator(ini);
    u32 shader_idx = 0; 
    while (true) {
        HglIniSection *s = hgl_ini_next_section(ini);
        if(s == NULL) {
            break;
        }
        shader_parse_from_ini_section(&shaq.shaders.arr[shader_idx], s);
        shader_idx++;
    }
    shaq.shaders.count = shader_idx;
}


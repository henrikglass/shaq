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

#include "glad/glad.h"

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

#define RENDERER_ASSERT(cond_, ...)                                   \
    if (!(cond_)) {                                                   \
        fprintf(stderr, "[RENDERER] Error: " __VA_ARGS__);            \
        fprintf(stderr, "\n");                                        \
        abort();                                                      \
    }                                                                 \

/*--- Private type definitions ----------------------------------------------------------*/

/*--- Private function prototypes -------------------------------------------------------*/

static i32 satisfy_dependencies_for_shader(u32 index, u32 depth);
static void determine_render_order(void);

static void parse_ini_file(HglIni *ini);

static void opengl_init(void);
static void debug_draw_frame(void); // DEBUG
static void fb_resize_callback(GLFWwindow *window, i32 w, i32 h);

/*--- Public variables ------------------------------------------------------------------*/

/*--- Private variables -----------------------------------------------------------------*/

typedef Array(Shader, SHAQ_MAX_N_SHADERS) Shaders;
typedef Array(u32, SHAQ_MAX_N_UNIFORMS) ShaderIndices;

static struct ShaqState {

    HglIni *ini; 
    const char *ini_filepath;
    i64 ini_modifytime;
    Array(Shader, SHAQ_MAX_N_SHADERS) shaders;
    Array(u32, SHAQ_MAX_N_SHADERS) render_order;
    Array(Texture, SHAQ_MAX_N_LOADED_TEXTURES) loaded_textures;

    b8 should_close;
    b8 first_frame;

    u64 start_timestamp_ns;
    u64 last_frame_timestamp_ns;
    f32 last_frame_deltatime_s;
    f32 last_frame_time_s;
    
    struct {
        GLFWwindow *window;
        IVec2 resolution;
    } renderer;
} shaq_state = {0};

/*--- Public functions ------------------------------------------------------------------*/

void shaq_begin(const char *ini_filepath)
{
    alloc_init();
    opengl_init();

    shaq_state.start_timestamp_ns = util_get_time_nanos();
    shaq_state.last_frame_timestamp_ns = shaq_state.start_timestamp_ns;
    shaq_state.ini_filepath = ini_filepath;
    shaq_reload();

}

b8 shaq_needs_reload(void)
{
    i64 ts = io_get_file_modify_time(shaq_state.ini_filepath);
    if (ts == -1) {
        return false; // File is probably in the process of being saved. Hold off for a bit.
    } else if (shaq_state.ini_modifytime != ts) {
        return true;
    }

    for (u32 i = 0; i < shaq_state.shaders.count; i++) {
        if (shader_needs_reload(&shaq_state.shaders.arr[i])) {
            return true;
        }
    }

    return false;
}

b8 shaq_should_close(void)
{
    if (shaq_state.should_close) printf("should close...\n");
    return shaq_state.should_close || glfwWindowShouldClose(shaq_state.renderer.window);
}

void shaq_reload(void)
{
    printf("shaq_reload()\n");

    /* Manually free OpenGL resources */
    for (u32 i = 0; i < shaq_state.shaders.count; i++) {
        Shader *s = &shaq_state.shaders.arr[i];
        shader_free_opengl_resources(s);
    }
    for (u32 i = 0; i < shaq_state.loaded_textures.count; i++) {
        Texture *t = &shaq_state.loaded_textures.arr[i];
        texture_free_opengl_resources(t);
    }

    /* reset state */
    array_clear(&shaq_state.shaders);
    array_clear(&shaq_state.render_order);
    array_clear(&shaq_state.loaded_textures);

    /* collect garbage */
    arena_free_all(g_longterm_arena);
    fs_free_all(g_longterm_fs_allocator);

    /* Reload ini file */
    shaq_state.ini_modifytime = io_get_file_modify_time(shaq_state.ini_filepath);
    shaq_state.ini = hgl_ini_open(shaq_state.ini_filepath);
    if (shaq_state.ini == NULL) {
        shaq_state.should_close = true;
    }

    /* Parse ini file + recompile simple expressions */
    parse_ini_file(shaq_state.ini);

    /* Determine render order */
    determine_render_order();

    /* Reload shaders */
    for (u32 i = 0; i < shaq_state.shaders.count; i++) {
        Shader *s = &shaq_state.shaders.arr[i];
        shader_reload(s);
    }

    shaq_state.first_frame = true;
}

void shaq_new_frame(void)
{
    printf("shaq_new_frame()\n");
    if (shaq_state.should_close) {
        return;
    }

    /* compute time */
    u64 now_ns = util_get_time_nanos();
    u64 dt_ns = now_ns - shaq_state.last_frame_timestamp_ns;
    u64 t_ns = now_ns - shaq_state.start_timestamp_ns;
    shaq_state.last_frame_timestamp_ns = now_ns;
    shaq_state.last_frame_deltatime_s = (f32)((f64)dt_ns / 1000000000.0);
    shaq_state.last_frame_time_s = (f32)((f64)t_ns / 1000000000.0);

    /* DEBUG */
    struct timespec ts = {.tv_sec = 1, .tv_nsec = 166666670};
    nanosleep(&ts, &ts);
    /* END DEBUG */


    /* TODO ... */
    for (u32 i = 0; i < shaq_state.shaders.count; i++) {
        Shader *s  = &shaq_state.shaders.arr[i];
#if 1
        printf("shader[%u] = \"" HGL_SV_FMT "\"\n", i, HGL_SV_ARG(s->name));
        printf("  filepath = " HGL_SV_FMT "\n", HGL_SV_ARG(s->filepath));
#endif

        for (u32 j = 0; j < s->uniforms.count; j++) {
            Uniform *u = &s->uniforms.arr[j];
            SelValue r = sel_eval(u->exe, false);
#if 1
            printf("  uniform[%u] %d " HGL_SV_FMT " = ", j, u->type, HGL_SV_ARG(u->name));
            sel_print_value(u->type, r);
#else
            (void) r;
#endif
        } 
    }

    debug_draw_frame();

    shaq_state.first_frame = false;

    /* DEBUG */
    printf("frame arena           -- "); hgl_arena_print_usage(g_frame_arena);
    printf("longterm arena        -- "); hgl_arena_print_usage(g_longterm_arena);
    printf("longterm fs allocator -- "); hgl_fs_print_usage(g_longterm_fs_allocator);
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
    return shaq_state.last_frame_time_s;
}

f32 shaq_deltatime(void)
{
    return shaq_state.last_frame_deltatime_s;
}

IVec2 shaq_iresolution(void)
{
    return shaq_state.renderer.resolution;
}

i32 shaq_find_shader_id_by_name(StringView name)
{
    /* look up shader id */
    for (u32 i = 0; i < shaq_state.shaders.count; i++) {
        Shader *s  = &shaq_state.shaders.arr[i];
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
    for (u32 i = 0; i < shaq_state.loaded_textures.count; i++) {
        Texture *t  = &shaq_state.loaded_textures.arr[i];
        if (sv_equals(t->filepath, filepath)) {
            //printf("FOUND TEXTURE: " HGL_SV_FMT "\n", HGL_SV_ARG(filepath));
            return i;
        }
    }

    /* not found? load it.*/
    Texture t = texture_load_from_file(filepath);
    if (t.data != NULL) {
        //printf("LOADED TEXTURE: " HGL_SV_FMT "\n", HGL_SV_ARG(filepath));
        array_push(&shaq_state.loaded_textures, t);
        return shaq_state.loaded_textures.count - 1;
    }

    /* Unable to load texture */
    fprintf(stderr, "[SHAQ] Error: Unable to load texture from \"" HGL_SV_FMT "\".\n", HGL_SV_ARG(filepath));
    return -1; 
}

u32 shaq_get_shader_render_texture_by_index(u32 index)
{
    return shaq_state.shaders.arr[index].render_texture.gl_texture_id;
}

u32 shaq_get_loaded_texture_by_index(u32 index)
{
    return shaq_state.loaded_textures.arr[index].gl_texture_id;
}

/*--- Private functions -----------------------------------------------------------------*/

static i32 satisfy_dependencies_for_shader(u32 index, u32 depth)
{
    Shader *s = &shaq_state.shaders.arr[index];

    /* recursed more times than there are shaders defined - guranteed cyclic dependency */
    if (depth > shaq_state.shaders.count + 1) {
        fprintf(stderr, "[SHAQ] Error: cyclic dependency between shaders.\n");
        return -1;
    }

    /* recursively satisfy the dependencies */
    for (u32 i = 0; i < s->shader_depends.count; i++) {
        i32 err = satisfy_dependencies_for_shader(s->shader_depends.arr[i], depth + 1);
        if (err != 0) return err;
    }

    /* append shader to the end of the render order if not already present */
    for (u32 i = 0; i < shaq_state.render_order.count; i++) {
        if (shaq_state.render_order.arr[i] == index) return 0; 
    }
    array_push(&shaq_state.render_order, index);

    return 0;
}

static void determine_render_order(void)
{
    assert(shaq_state.shaders.count > 0);
    array_clear(&shaq_state.render_order);

    /* determine per-shader dependencies (on other shaders) */
    for (u32 i = 0; i < shaq_state.shaders.count; i++) {
        shader_determine_dependencies(&shaq_state.shaders.arr[i]);
    }

    /* satisfy dependencies (on other shaders) for each shader */
    for (u32 i = 0; i < shaq_state.shaders.count; i++) {
        i32 err = satisfy_dependencies_for_shader(i, 0);
        if (err != 0) {
            fprintf(stderr, "[SHAQ] Error: Could not determine a render order for shader \"" 
                    HGL_SV_FMT "\".\n", HGL_SV_ARG(shaq_state.shaders.arr[i].name));
        }
    }

    //assert(shaq_state.render_order.count == shaq_state.shaders.count);
   
    // DEBUG 
#if 0
    printf("render order: ");
    for (u32 j = 0; j < shaq_state.render_order.count; j++) {
        u32 idx = shaq_state.render_order.arr[j];
        printf(HGL_SV_FMT " ", HGL_SV_ARG(shaq_state.shaders.arr[idx].name));
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
        shader_parse_from_ini_section(&shaq_state.shaders.arr[shader_idx], s);
        shader_idx++;
    }
    shaq_state.shaders.count = shader_idx;
}

static void opengl_init()
{
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    shaq_state.renderer.resolution = ivec2_make(800, 600);
    shaq_state.renderer.window = glfwCreateWindow(shaq_state.renderer.resolution.x, 
                                                  shaq_state.renderer.resolution.y, 
                                                  "Shaq", NULL, NULL);

    if (shaq_state.renderer.window == NULL) {
        fprintf(stderr, "[RENDERER] Error: Failed to create GLFW window.\n");
        glfwTerminate();
        abort();
    }

    glfwMakeContextCurrent(shaq_state.renderer.window);
    glfwSetFramebufferSizeCallback(shaq_state.renderer.window, fb_resize_callback);
    glfwSwapInterval(1);

    i32 err = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    if (err <= 0) {
        fprintf(stderr, "[RENDERER] Error: GLAD Failed to load.\n");
        glfwTerminate();
        abort();
    }

    glViewport(0, 0, shaq_state.renderer.resolution.x, shaq_state.renderer.resolution.y);
}

static void debug_draw_frame()
{
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glfwSwapBuffers(shaq_state.renderer.window);
    glfwPollEvents();
}

static void fb_resize_callback(GLFWwindow *window, i32 w, i32 h)
{
    (void) window;
    glViewport(0, 0, w, h);
    shaq_state.renderer.resolution.x = w;
    shaq_state.renderer.resolution.y = h;
}


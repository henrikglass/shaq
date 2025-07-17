/*--- Include files ---------------------------------------------------------------------*/

#include "shaq_core.h"
#include "constants.h"
#include "shader.h"
#include "uniform.h"
#include "texture.h"
#include "util.h"

#include "sel.h"

//#define HGL_INI_ALLOC fs_alloc
//#define HGL_INI_REALLOC fs_realloc
//#define HGL_INI_free fs_realloc
#define HGL_INI_IMPLEMENTATION
#include "hgl_ini.h"

#include <time.h>

/*--- Private macros --------------------------------------------------------------------*/

/*--- Private type definitions ----------------------------------------------------------*/

/*--- Private function prototypes -------------------------------------------------------*/

void ini_parse(HglIni *ini);
void determine_render_order(void);

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

    u64 start_timestamp_ns;
    u64 last_frame_timestamp_ns;
    f32 last_frame_deltatime_s;
    f32 last_frame_time_s;
    
    IVec2 iresolution;
} shaq_state = {0};

/*--- Public functions ------------------------------------------------------------------*/

void shaq_begin(const char *ini_filepath)
{
    shaq_state.start_timestamp_ns = util_get_time_nanos();
    shaq_state.last_frame_timestamp_ns = shaq_state.start_timestamp_ns;
    shaq_state.ini_filepath = ini_filepath;
    shaq_reload();
}

b8 shaq_needs_reload(void)
{
    i64 ts;
    do {
        ts = util_get_file_modify_time(shaq_state.ini_filepath);
    } while (ts == -1);
    return shaq_state.ini_modifytime != ts;
}

b8 shaq_should_close(void)
{
    if (shaq_state.should_close) printf("should close...\n");
    return shaq_state.should_close;
}

void shaq_reload(void)
{
    printf("shaq_reload()\n");
    shaq_state.ini_modifytime = util_get_file_modify_time(shaq_state.ini_filepath);

    if (shaq_state.ini != NULL) {
        hgl_ini_close(shaq_state.ini); // TODO handle implicitly by fs alloc
    }
    shaq_state.ini = hgl_ini_open(shaq_state.ini_filepath);
    if (shaq_state.ini == NULL) {
        shaq_state.should_close = true;
    }

    ini_parse(shaq_state.ini);

    determine_render_order();
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

    /* TODO ... */
    struct timespec ts = {.tv_sec = 1, .tv_nsec = 0};
    nanosleep(&ts, &ts);
    for (u32 i = 0; i < shaq_state.shaders.count; i++) {
        Shader *s  = &shaq_state.shaders.arr[i];
        printf("shader[%u] = \"" HGL_SV_FMT "\"\n", i, HGL_SV_ARG(s->name));
        printf("  src = " HGL_SV_FMT "\n", HGL_SV_ARG(s->src));
        for (u32 j = 0; j < s->uniforms.count; j++) {
            Uniform *u = &s->uniforms.arr[j];
            printf("  uniform[%u] %d " HGL_SV_FMT " = ", j, u->type, HGL_SV_ARG(u->name));
            SelValue r = sel_run(u->exe);
            sel_print_value(u->type, r);
        } 
    }
    return; // TODO
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
    return shaq_state.iresolution;
}

i32 shaq_get_index_of_shader(HglStringView name)
{
    (void) name; // TODO
    for (u32 i = 0; i < shaq_state.shaders.count; i++) {
        Shader *s  = &shaq_state.shaders.arr[i];
        if (hgl_sv_equals(s->name, name)) {
            return i;
        }
    }

    /* no shader with name `name` found */
    printf("No shader with name \"" HGL_SV_FMT "\" found.\n", HGL_SV_ARG(name));
    return -1;    
}

/*--- Private functions -----------------------------------------------------------------*/

void ini_parse(HglIni *ini)
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

i32 satisfy_dependencies_for_shader(u32 index, u32 depth);
i32 satisfy_dependencies_for_shader(u32 index, u32 depth)
{
    Shader *s = &shaq_state.shaders.arr[index];

    /* recursed more times than there are shaders defined - guranteed cyclic dependency */
    if (depth > shaq_state.shaders.count) {
        printf("cyclic dependency.");
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

void determine_render_order(void)
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
            printf("Could not determine a render order for shader \"" HGL_SV_FMT "\".\n", 
                   HGL_SV_ARG(shaq_state.shaders.arr[i].name));
        }
    }

    //assert(shaq_state.render_order.count == shaq_state.shaders.count);
   
    // DEBUG 
    printf("render order: ");
    for (u32 j = 0; j < shaq_state.render_order.count; j++) {
        u32 idx = shaq_state.render_order.arr[j];
        printf(HGL_SV_FMT " ", HGL_SV_ARG(shaq_state.shaders.arr[idx].name));
    }
    printf("\n");
    // END DEBUG 
}


/*--- Include files ---------------------------------------------------------------------*/

#include "shaq_core.h"

#include "sel.h"

//#define HGL_INI_ALLOC fs_alloc
//#define HGL_INI_REALLOC fs_realloc
//#define HGL_INI_free fs_realloc
#define HGL_INI_IMPLEMENTATION
#include "hgl_ini.h"
#include "hgl_string.h"

#include <time.h>

/*--- Private macros --------------------------------------------------------------------*/

#define SHAQ_MAX_N_SHADERS 64
#define SHAQ_MAX_N_UNIFORMS 64

/*--- Private type definitions ----------------------------------------------------------*/

typedef struct {
    HglStringView name;
    Type type;
    ExeExpr *exe;
} Uniform;

typedef struct {
    const char *name;
    const char *src;
    Uniform uniforms[SHAQ_MAX_N_UNIFORMS];
    u32 n_uniforms;
} Shader;

/*--- Private function prototypes -------------------------------------------------------*/

static u64 get_time_nanos(void);
static time_t get_file_modify_time(const char *filepath);
void ini_parse_shader(Shader *sh, HglIniSection *s);
int ini_parse_uniform(Uniform *u, HglIniKVPair *kv);
void ini_parse(HglIni *ini);

/*--- Public variables ------------------------------------------------------------------*/

/*--- Private variables -----------------------------------------------------------------*/

static struct ShaqState {

    HglIni *ini; 
    const char *ini_filepath;
    time_t ini_modifytime;
    Shader shaders[SHAQ_MAX_N_SHADERS];
    u32 n_shaders;

    b8 should_close;

    u64 start_timestamp_ns;
    u64 last_frame_timestamp_ns;
    f32 last_frame_deltatime_s;
    f32 last_frame_time_s;
} shaq_state = {0};

/*--- Public functions ------------------------------------------------------------------*/

void shaq_begin(const char *ini_filepath)
{
    shaq_state.start_timestamp_ns = get_time_nanos();
    shaq_state.last_frame_timestamp_ns = shaq_state.start_timestamp_ns;
    shaq_state.ini_filepath = ini_filepath;
    shaq_reload();
}

b8 shaq_needs_reload(void)
{
    time_t ts;
    do {
        ts = get_file_modify_time(shaq_state.ini_filepath);
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
    shaq_state.ini_modifytime = get_file_modify_time(shaq_state.ini_filepath);

    if (shaq_state.ini != NULL) {
        hgl_ini_close(shaq_state.ini); // TODO handle implicitly by fs alloc
    }
    shaq_state.ini = hgl_ini_open(shaq_state.ini_filepath);
    if (shaq_state.ini == NULL) {
        shaq_state.should_close = true;
    }

    // TODO double buffer ?
    ini_parse(shaq_state.ini);
}

void shaq_new_frame(void)
{
    printf("shaq_new_frame()\n");
    if (shaq_state.should_close) {
        return;
    }

    /* compute time */
    u64 now_ns = get_time_nanos();
    u64 dt_ns = now_ns - shaq_state.last_frame_timestamp_ns;
    u64 t_ns = now_ns - shaq_state.start_timestamp_ns;
    shaq_state.last_frame_timestamp_ns = now_ns;
    shaq_state.last_frame_deltatime_s = (f32)((f64)dt_ns / 1000000000.0);
    shaq_state.last_frame_time_s = (f32)((f64)t_ns / 1000000000.0);

    /* TODO ... */
    struct timespec ts = {.tv_sec = 1, .tv_nsec = 0};
    nanosleep(&ts, &ts);
    for (u32 i = 0; i < shaq_state.n_shaders; i++) {
        printf("shader[%u] = %s\n", i, shaq_state.shaders[i].name);
        printf("  src = %s\n", shaq_state.shaders[i].src);
        for (u32 j = 0; j < shaq_state.shaders[i].n_uniforms; j++) {
            printf("  uniform[%u] %d " HGL_SV_FMT " = ", j,
                   shaq_state.shaders[i].uniforms[j].type, 
                   HGL_SV_ARG(shaq_state.shaders[i].uniforms[j].name));
            SelValue r = sel_run(shaq_state.shaders[i].uniforms[j].exe);
            sel_print_value(shaq_state.shaders[i].uniforms[j].type, r);
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

/*--- Private functions -----------------------------------------------------------------*/

static u64 get_time_nanos(void)
{
    struct timespec t_temp;
    clock_gettime(CLOCK_MONOTONIC, &t_temp);
    u64 ns = t_temp.tv_sec * 1000000000ll + t_temp.tv_nsec;
    return ns;
}

static time_t get_file_modify_time(const char *filepath)
{
    struct stat statbuf;
    int err = stat(filepath, &statbuf);
    if (err != 0) {
        fprintf(stderr, "error trying to get modify-time for `%s`\n", filepath);
        return (time_t)-1;
    }
    return statbuf.st_mtime;
}

void ini_parse_shader(Shader *sh, HglIniSection *s)
{
    /* parse name + source + etc. */ 
    sh->name = s->name;
    sh->n_uniforms = 0;
    sh->src = hgl_ini_get_in_section(s, "source");
    if (sh->src == NULL) {
        printf("Shader %s: missing `source` entry.\n", s->name);
    }

    /* parse uniforms */ 
    hgl_ini_reset_kv_pair_iterator(s);
    u32 uniform_idx = 0; 
    while (true) {
        HglIniKVPair *kv = hgl_ini_next_kv_pair(s);
        if(kv == NULL) {
            break;
        }
        if (ini_parse_uniform(&sh->uniforms[uniform_idx], kv) == 0) {
            uniform_idx++;
        }
    }
    sh->n_uniforms = uniform_idx;
}

int ini_parse_uniform(Uniform *u, HglIniKVPair *kv)
{
    HglStringView k = hgl_sv_trim(hgl_sv_from_cstr(kv->key));

    if (hgl_sv_starts_with_lchop(&k, "uniform")) {
        k = hgl_sv_ltrim(k);
        if      (hgl_sv_starts_with_lchop(&k, "bool"))  { u->type = TYPE_BOOL;  }
        else if (hgl_sv_starts_with_lchop(&k, "int"))   { u->type = TYPE_INT;   }
        else if (hgl_sv_starts_with_lchop(&k, "float")) { u->type = TYPE_FLOAT; }
        else if (hgl_sv_starts_with_lchop(&k, "vec2"))  { u->type = TYPE_VEC2;  }
        else if (hgl_sv_starts_with_lchop(&k, "vec3"))  { u->type = TYPE_VEC3;  }
        else if (hgl_sv_starts_with_lchop(&k, "vec4"))  { u->type = TYPE_VEC4;  }
        else if (hgl_sv_starts_with_lchop(&k, "ivec2")) { u->type = TYPE_IVEC2; }
        else if (hgl_sv_starts_with_lchop(&k, "ivec3")) { u->type = TYPE_IVEC3; }
        else if (hgl_sv_starts_with_lchop(&k, "ivec4")) { u->type = TYPE_IVEC4; }
        else if (hgl_sv_starts_with_lchop(&k, "mat2"))  { u->type = TYPE_MAT2;  }
        else if (hgl_sv_starts_with_lchop(&k, "mat3"))  { u->type = TYPE_MAT3;  }
        else if (hgl_sv_starts_with_lchop(&k, "mat4"))  { u->type = TYPE_MAT4;  }
        else if (hgl_sv_starts_with_lchop(&k, "sampler2D")) { u->type = TYPE_NIL; }// TODO 
        else {
            printf("Unknown or unsupported type in lhs expression: `%s`.\n", kv->key);
            return -1;
        }

        u->name = hgl_sv_trim(k);
        u->exe = sel_compile(kv->val);
        return 0;
    }
    
    return -1;
}

void ini_parse(HglIni *ini)
{
    hgl_ini_reset_section_iterator(ini);
    u32 shader_idx = 0; 
    while (true) {
        HglIniSection *s = hgl_ini_next_section(ini);
        if(s == NULL) {
            break;
        }
        ini_parse_shader(&shaq_state.shaders[shader_idx], s);
        shader_idx++;
    }
    shaq_state.n_shaders = shader_idx;
}


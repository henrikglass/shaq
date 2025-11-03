/*--- Include files ---------------------------------------------------------------------*/

#include "image.h"
#include "alloc.h"
#include "shaq_config.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdouble-promotion"
#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma GCC diagnostic ignored "-Wduplicated-branches"
#define STBI_MALLOC  image_alloc
#define STBI_REALLOC image_realloc
#define STBI_FREE    image_free
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#pragma GCC diagnostic pop

/*--- Private macros --------------------------------------------------------------------*/

#define N_ENTRIES      (2*SHAQ_MAX_N_LOADED_TEXTURES)
#define OCCUPIED_BIT   0x40000000
#define SEQ_N_MASK     0x2FFFFFFF
#define IS_OCCUPIED(e) ((e)->tag & OCCUPIED_BIT)
#define SEQ_N(e)       (int)((e)->tag & SEQ_N_MASK)

/*--- Private type definitions ----------------------------------------------------------*/

typedef struct
{
    Image img;
    u32 tag;
} Entry;

/*--- Private function prototypes -------------------------------------------------------*/

Image load_image(StringView filepath);
void free_image(Image *img);

/*--- Public variables ------------------------------------------------------------------*/

/*--- Private variables -----------------------------------------------------------------*/

static struct
{
    Entry arr[N_ENTRIES];
    u32 seq_counter;
} image_cache = {0};

/*--- Public functions ------------------------------------------------------------------*/

Image *image_load_from_file(StringView filepath)
{
#if 0
    for (u32 i = 0; i < N_ENTRIES; i++) {
        Entry *e = &image_cache.arr[i];
        if (IS_OCCUPIED(e)) {
            printf("[%u] -- \"" SV_FMT "\"\n", i, SV_ARG(e->img.filepath));
        } else {
            printf("[%u] -- \n", i);
        }
    }
#endif

    /* try lookup */
    u32 first_vacant_idx = N_ENTRIES;
    u32 lru_idx = N_ENTRIES;
    u32 lru = SEQ_N_MASK;
    for (u32 i = 0; i < N_ENTRIES; i++) {
        Entry *e = &image_cache.arr[i];

        /* Skip vacant slots - save the index of the first one */
        if (!IS_OCCUPIED(e)) {
            if (first_vacant_idx == N_ENTRIES) {
                first_vacant_idx = i;
            }
            continue;
        }

        /* Match? Return and update tag */
        if (sv_equals(filepath, e->img.filepath)) {
            e->tag = OCCUPIED_BIT + (SEQ_N_MASK & image_cache.seq_counter++);
            return &e->img;
        }

        /* No match - update lowest encountered sequence value (least recently used) */
        if ((e->tag & SEQ_N_MASK) < lru) {
            lru_idx = i;
            lru = (e->tag & SEQ_N_MASK);
        }
    }

    /* lookup failed. Load image data from file. */
    Entry e = {
        .img = load_image(filepath),
        .tag = OCCUPIED_BIT + (SEQ_N_MASK & image_cache.seq_counter++),
    };

    /* Unfilled slots left? Insert */
    if (first_vacant_idx != N_ENTRIES) {
        image_cache.arr[first_vacant_idx] = e;
        return &image_cache.arr[first_vacant_idx].img;
    }

    /*
     * No unfilled slots left? Evict element starting from the left. The order of the
     * elements is decided by the chosen eviction policy during the last sort.
     */
    free_image(&image_cache.arr[lru_idx].img);
    image_cache.arr[lru_idx] = e;
    return &image_cache.arr[lru_idx].img;
}

void image_free_all_cached_images()
{
    hgl_free_all(g_image_allocator);
    memset(&image_cache, 0, sizeof(image_cache));
}

/*--- Private functions -----------------------------------------------------------------*/

Image load_image(StringView filepath)
{
    Image img;
    img.filepath = sv_make_copy(filepath, image_alloc);
    stbi_set_flip_vertically_on_load(1);
    const char *filepath_cstr = sv_make_cstr_copy(filepath, tmp_alloc);
    img.data = stbi_load(filepath_cstr, &img.width, &img.height, &img.n_channels, 0);
    return img;
}

void free_image(Image *img)
{
    if (img->data != NULL) {
        stbi_image_free(img->data);
    }
}



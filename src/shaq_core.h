#ifndef SHAQ_CORE_H
#define SHAQ_CORE_H

/*--- Include files ---------------------------------------------------------------------*/

#include "hgl_int.h"
#include "hgl_float.h"

/*--- Public macros ---------------------------------------------------------------------*/

/*--- Public type definitions -----------------------------------------------------------*/

/*--- Public variables ------------------------------------------------------------------*/

/*--- Public function prototypes --------------------------------------------------------*/

void shaq_begin(const char *ini_filepath);
b8 shaq_needs_reload(void);
b8 shaq_should_close(void);
void shaq_reload(void);
void shaq_new_frame(void);
void shaq_end(void);
f32 shaq_time(void);
f32 shaq_deltatime(void);

#endif /* SHAQ_CORE_H */


/*--- Include files ---------------------------------------------------------------------*/

#include "gl_util.h"

#include "glad/glad.h"

#include <stdio.h>

/*--- Private macros --------------------------------------------------------------------*/

/*--- Private type definitions ----------------------------------------------------------*/

/*--- Private function prototypes -------------------------------------------------------*/

/*--- Public variables ------------------------------------------------------------------*/

/*--- Private variables -----------------------------------------------------------------*/

/*--- Public functions ------------------------------------------------------------------*/

i32 gl_check_errors_(const char *file, i32 line)
{
    b8 had_error = false;
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        had_error = true;
        const char *error = NULL;
        switch (errorCode)
        {
            case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        fprintf(stderr, "[OpenGL] Error: %s <%s:%d>.\n", error, file, line);
    }
    
    if (had_error) {
        return -1;
    }

    return 0;
}

/*--- Private functions -----------------------------------------------------------------*/


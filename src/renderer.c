/*--- Include files ---------------------------------------------------------------------*/

#include "renderer.h"
#include "hgl_int.h"
#include "vecmath.h"
#include "glad/glad.h"
#include "gl_util.h"

#include <GLFW/glfw3.h>

/*--- Private macros --------------------------------------------------------------------*/

/*--- Private type definitions ----------------------------------------------------------*/

/*--- Private function prototypes -------------------------------------------------------*/

static void fb_resize_callback(GLFWwindow *window, i32 w, i32 h);

/*--- Public variables ------------------------------------------------------------------*/

/*--- Private variables -----------------------------------------------------------------*/

static struct {
    GLFWwindow *window;
    IVec2 resolution;
    Shader last_pass_shader;
    u32 VBO;
    u32 VAO;
    u32 offscreen_fb;
} renderer;

/*--- Public functions ------------------------------------------------------------------*/

void renderer_init()
{
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    renderer.resolution = ivec2_make(800, 600);
    renderer.window = glfwCreateWindow(renderer.resolution.x, 
                                       renderer.resolution.y, 
                                       "Shaq", NULL, NULL);

    if (renderer.window == NULL) {
        fprintf(stderr, "[RENDERER] Error: Failed to create GLFW window.\n");
        glfwTerminate();
        abort();
    }

    glfwMakeContextCurrent(renderer.window);
    glfwSetFramebufferSizeCallback(renderer.window, fb_resize_callback);
    glfwSwapInterval(1);

    i32 err = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    if (err <= 0) {
        fprintf(stderr, "[RENDERER] Error: GLAD Failed to load.\n");
        glfwTerminate();
        abort();
    }

    glGenVertexArrays(1, &renderer.VAO);
    glBindVertexArray(renderer.VAO);
    static Vec2 fullscreen_tri_verts[3] = {
        (Vec2){.x = -1.0f, .y = -1.0f},
        (Vec2){.x =  3.0f, .y = -1.0f},
        (Vec2){.x = -1.0f, .y =  3.0f},
    }; 
    glGenBuffers(1, &renderer.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, renderer.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(fullscreen_tri_verts), fullscreen_tri_verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(Vec2), (void *)0);
    glEnableVertexAttribArray(0);
    glGenFramebuffers(1, &renderer.offscreen_fb);
    shader_make_last_pass_shader(&renderer.last_pass_shader);
    glDisable(GL_CULL_FACE);
    glViewport(0, 0, renderer.resolution.x, renderer.resolution.y);
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);

    if (gl_check_errors() != 0) {
        fprintf(stderr, "[RENDERER] Failed to setup one or more OpenGL-intrinsic things.\n");
        abort();
    }
}

void renderer_draw_shader(Shader *s)
{
#if 0
    glUseProgram(s->gl_shader_program_id);

    /* update uniforms */
    shader_prepare_for_drawing(s);

    /* prepare offscreen frame buffer */
    //glBindFramebuffer(GL_FRAMEBUFFER, renderer.offscreen_fb);
    //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 
    //                       s->render_texture.gl_texture_id, 0);
    //assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

    /* Draw */
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glfwSwapBuffers(renderer.window);
    glfwPollEvents();
#endif

#if 1
    glUseProgram(s->gl_shader_program_id);

    /* update uniforms */
    shader_prepare_for_drawing(s);

    /* prepare offscreen frame buffer */
    glBindFramebuffer(GL_FRAMEBUFFER, renderer.offscreen_fb);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 
                           s->render_texture.gl_texture_id, 0);
    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

    /* Draw */
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 3);
#endif
}

void renderer_display(Shader *s)
{
    (void) s;


    //shader_prepare_for_drawing(&renderer.last_pass_shader);
    //shader_draw(&renderer.last_pass_shader);
    glUseProgram(renderer.last_pass_shader.gl_shader_program_id); // TODO update tex uniform 
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, s->render_texture.gl_texture_id);
    glUniform1i(glGetUniformLocation(renderer.last_pass_shader.gl_shader_program_id, "tex"), GL_TEXTURE0); // TODO cache
    glUniform2iv(glGetUniformLocation(renderer.last_pass_shader.gl_shader_program_id, "iresolution"), 1, (i32 *)&renderer.resolution); 
    //IVec2 irr = ivec2_make(800, 600);
    //glUniform2iv(glGetUniformLocation(renderer.last_pass_shader.gl_shader_program_id, "iresolution"), 1, (i32 *)&irr); 
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glfwSwapBuffers(renderer.window);
    glfwPollEvents();
}

IVec2 renderer_iresolution(void)
{
    return renderer.resolution;
}

b8 renderer_should_close(void)
{
    return glfwWindowShouldClose(renderer.window);
}

/*--- Private functions -----------------------------------------------------------------*/

static void fb_resize_callback(GLFWwindow *window, i32 w, i32 h)
{
    (void) window;
    glViewport(0, 0, w, h);
    renderer.resolution.x = w;
    renderer.resolution.y = h;
}



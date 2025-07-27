/*--- Include files ---------------------------------------------------------------------*/

#include "renderer.h"

#include "hgl_int.h"
#include "vecmath.h"
#include "gl_util.h"
#include "log.h"
#include "glad/glad.h"
#include "gui.h"
#include <GLFW/glfw3.h>
#include "imguic.h"

/*--- Private macros --------------------------------------------------------------------*/

/*--- Private type definitions ----------------------------------------------------------*/

/*--- Private function prototypes -------------------------------------------------------*/

static void resize_callback(GLFWwindow *window, i32 w, i32 h);
static void key_callback(GLFWwindow *window, i32 key, i32 scancode, i32 action, i32 mods);

static void toggle_fullscreen(GLFWwindow *window);

static i32 mini(i32 x, i32 y);
static i32 maxi(i32 x, i32 y);
GLFWmonitor* get_current_monitor(GLFWwindow *window);

/*--- Public variables ------------------------------------------------------------------*/

/*--- Private variables -----------------------------------------------------------------*/

static struct {
    GLFWwindow *window;
    IVec2 resolution;
    Shader last_pass_shader;
    u32 VBO;
    u32 VAO;
    u32 offscreen_fb;

    b8 window_was_resized_this_frame;
    b8 is_fullscreen;
    b8 hide_gui;
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

    renderer.window_was_resized_this_frame = false;
    renderer.is_fullscreen = false;
    renderer.hide_gui = false;
    renderer.resolution = ivec2_make(1680, 1050);
    renderer.window = glfwCreateWindow(renderer.resolution.x, 
                                       renderer.resolution.y, 
                                       "Shaq", NULL, NULL);

    if (renderer.window == NULL) {
        log_error("Failed to create GLFW window.");
        glfwTerminate();
        exit(1);
    }

    glfwMakeContextCurrent(renderer.window);
    glfwSetFramebufferSizeCallback(renderer.window, resize_callback);
    glfwSetKeyCallback(renderer.window, key_callback);
    glfwSwapInterval(1);

    i32 err = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    if (err <= 0) {
        log_error("GLAD failed to load.");
        glfwTerminate();
        exit(1);
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
    glViewport(0, 0, renderer.resolution.x, renderer.resolution.y);
    glClearColor(0.117f, 0.117f, 0.117f, 1.0f);

    shader_make_last_pass_shader(&renderer.last_pass_shader);
    imgui_init(renderer.window);

    if (gl_check_errors() != 0) {
        log_error("Failed to setup one or more OpenGL-intrinsic things.");
        exit(1);
    }
}

void renderer_begin_frame(void)
{
    renderer.window_was_resized_this_frame = false;
}

void renderer_do_shader_pass(Shader *s)
{
    glUseProgram(s->gl_shader_program_id);

    /* update uniforms */
    shader_prepare_for_drawing(s);

    /* prepare offscreen frame buffer */
    glBindFramebuffer(GL_FRAMEBUFFER, renderer.offscreen_fb);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 
                           s->render_texture.gl_texture_id, 0);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        return; /* possibly no `source` entry in *.ini file or shader compilation failure */
    }

    /* Draw */
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void renderer_do_final_pass(Shader *s)
{
    glUseProgram(renderer.last_pass_shader.gl_shader_program_id); // TODO update tex uniform 
    glUniform1i(glGetUniformLocation(renderer.last_pass_shader.gl_shader_program_id, "tex"), 0); // TODO cache
    glUniform2iv(glGetUniformLocation(renderer.last_pass_shader.gl_shader_program_id, "iresolution"), 1, (i32 *)&renderer.resolution); 
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    glClear(GL_COLOR_BUFFER_BIT);

    if (s == NULL) {
        return;
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, s->render_texture.gl_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void renderer_end_frame()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glfwSwapBuffers(renderer.window);
    glfwPollEvents();
    gl_check_errors();
}

b8 renderer_should_close()
{
    return glfwWindowShouldClose(renderer.window);
}

b8 renderer_should_hide_gui(void)
{
    return renderer.hide_gui;
}

b8 renderer_window_was_resized()
{
    return renderer.window_was_resized_this_frame;
}

IVec2 renderer_iresolution()
{
    return renderer.resolution;
}

//GLFWwindow *renderer_get_window()
//{
//    return renderer.window;
//}

/*--- Private functions -----------------------------------------------------------------*/

static void resize_callback(GLFWwindow *window, i32 w, i32 h)
{
    (void) window;
    glViewport(0, 0, w, h);
    renderer.resolution.x = w;
    renderer.resolution.y = h;
    renderer.window_was_resized_this_frame = true;
}

static void key_callback(GLFWwindow *window, i32 key, i32 scancode, i32 action, i32 mods)
{
    (void) scancode;
    (void) mods;

    switch (key) {
        case GLFW_KEY_D: {
            if (action == GLFW_PRESS) {
                gui_toggle_darkmode();
            }
        } break;

        case GLFW_KEY_F: {
            if (action == GLFW_PRESS) {
                toggle_fullscreen(window);
            }
        } break;

        case GLFW_KEY_H: {
            if (action == GLFW_PRESS) {
                renderer.hide_gui = !renderer.hide_gui;
            }
        } break;

        case GLFW_KEY_Q: 
        case GLFW_KEY_ESCAPE: {
            if (action == GLFW_PRESS) {
                glfwSetWindowShouldClose(window, true);
            }
        } break;
    }
}

static void toggle_fullscreen(GLFWwindow *window)
{
    static IVec2 old_resolution = {0};
    static IVec2 old_position = {0};

    if (renderer.is_fullscreen) {
        printf("toggle fullscreen OFF\n");
        glfwSetWindowMonitor(window, NULL, 0, 0, 
                             old_resolution.x, old_resolution.y, 
                             GLFW_DONT_CARE);
        glfwSetWindowPos(window, old_position.x, old_position.y);
        renderer.is_fullscreen = false;
    } else {
        printf("toggle fullscreen ON\n");
        GLFWmonitor *monitor = get_current_monitor(window);
        const GLFWvidmode *mode = glfwGetVideoMode(monitor);
        old_resolution = renderer.resolution;
        glfwGetWindowPos(window, &old_position.x, &old_position.y);
        renderer.resolution.x = mode->width;
        renderer.resolution.y = mode->height;
        glfwSetWindowMonitor(window, monitor, 0, 0, 
                             renderer.resolution.x, renderer.resolution.y, 
                             GLFW_DONT_CARE);
        renderer.is_fullscreen = true;
    }

}

static i32 mini(i32 x, i32 y)
{
    return x < y ? x : y;
}

static i32 maxi(i32 x, i32 y)
{
    return x > y ? x : y;
}

GLFWmonitor* get_current_monitor(GLFWwindow *window)
{
    // https://stackoverflow.com/a/31526753/5350029
    i32 nmonitors, i;
    i32 wx, wy, ww, wh;
    i32 mx, my, mw, mh;
    i32 overlap, best_overlap;
    GLFWmonitor *best_monitor;
    GLFWmonitor **monitors;
    const GLFWvidmode *mode;

    best_overlap = 0;
    best_monitor = NULL;

    glfwGetWindowPos(window, &wx, &wy);
    glfwGetWindowSize(window, &ww, &wh);
    monitors = glfwGetMonitors(&nmonitors);

    for (i = 0; i < nmonitors; i++) {
        mode = glfwGetVideoMode(monitors[i]);
        glfwGetMonitorPos(monitors[i], &mx, &my);
        mw = mode->width;
        mh = mode->height;

        overlap = maxi(0, mini(wx + ww, mx + mw) - maxi(wx, mx)) *
                  maxi(0, mini(wy + wh, my + mh) - maxi(wy, my));

        if (best_overlap < overlap) {
            best_overlap = overlap;
            best_monitor = monitors[i];
        }
    }

    return best_monitor;
} 

/*--- Include files ---------------------------------------------------------------------*/

#include "renderer.h"

#include "shaq_core.h"
#include "shaq_config.h"
#include "user_input.h"
#include "hgl_int.h"
#include "vecmath.h"
#include "gl_util.h"
#include "log.h"
#include "gui.h"

/*--- Private macros --------------------------------------------------------------------*/

/*--- Private type definitions ----------------------------------------------------------*/

/*--- Private function prototypes -------------------------------------------------------*/

static void resize_callback(GLFWwindow *window, i32 w, i32 h);

static i32 mini(i32 x, i32 y);
static i32 maxi(i32 x, i32 y);
GLFWmonitor* get_current_monitor(GLFWwindow *window);

/*--- Public variables ------------------------------------------------------------------*/

/*--- Private variables -----------------------------------------------------------------*/

static struct {
    GLFWwindow *window;
    IVec2 window_size;
    Shader last_pass_shader;
    u32 VBO;
    u32 VAO;
    u32 offscreen_fb;

    b8 should_reload;
    b8 is_fullscreen;
    b8 shader_view_is_maximized;

    Vec2 mouse_position;
    Vec2 mouse_drag_position;
    b8 lmb_is_down;
    b8 rmb_is_down;
    b8 lmb_was_down_last_frame;
    b8 rmb_was_down_last_frame;
    u32 key_down_bitfield;
    u32 key_pressed_bitfield;
} renderer;

/*--- Public functions ------------------------------------------------------------------*/

void renderer_init()
{
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    renderer.should_reload = false;
    renderer.is_fullscreen = false;
    renderer.shader_view_is_maximized = false;
    renderer.window_size = ivec2_make(1680, 1050);
    renderer.window = glfwCreateWindow(renderer.window_size.x, 
                                       renderer.window_size.y, 
                                       "Shaq", NULL, NULL);

    if (renderer.window == NULL) {
        log_error("Failed to create GLFW window.");
        glfwTerminate();
        exit(1);
    }

    glfwMakeContextCurrent(renderer.window);
    glfwSetFramebufferSizeCallback(renderer.window, resize_callback);
    glfwSetKeyCallback(renderer.window, user_input_glfw_key_callback);
    glfwSwapInterval(SHAQ_ENABLE_VSYNC ? 1 : 0);

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
    glViewport(0, 0, renderer.window_size.x, renderer.window_size.y);
    glClearColor(0.117f, 0.617f, 0.117f, 1.0f);

    shader_make_last_pass_shader(&renderer.last_pass_shader);
    gui_init(renderer.window, glfwGetPrimaryMonitor());

    if (gl_check_errors() != 0) {
        log_error("Failed to setup one or more OpenGL-intrinsic things.");
        exit(1);
    }
}

void renderer_final()
{
    gui_final();
    glfwTerminate();
}

void renderer_reload()
{
    renderer.should_reload = false;
}

void renderer_do_shader_pass(Shader *s)
{
    if (!shader_is_ok(s)) {
        return;
    }

    glUseProgram(s->gl_shader_program_id);
    glViewport(0, 0, s->attributes.resolution.x, s->attributes.resolution.y);

    /* prepare offscreen frame buffer */
    glBindFramebuffer(GL_FRAMEBUFFER, renderer.offscreen_fb);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 
                           s->render_texture_current->gl_texture_id, 0);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        return; /* possibly no `source` entry in *.ini file or shader compilation failure */
    }

    /* Draw */
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void renderer_draw_fullscreen_shader(Shader *s)
{
    if (!shader_is_ok(s)) {
        return;
    }

    glViewport(0, 0, renderer.window_size.x, renderer.window_size.y);
    glUniform1i(glGetUniformLocation(renderer.last_pass_shader.gl_shader_program_id, "tex"), 0);
    glUniform2iv(glGetUniformLocation(renderer.last_pass_shader.gl_shader_program_id, "iresolution"), 
                 1, (i32 *)&renderer.window_size); 
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, s->render_texture_current->gl_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void renderer_begin_final_pass()
{
    glUseProgram(renderer.last_pass_shader.gl_shader_program_id);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    glClear(GL_COLOR_BUFFER_BIT);
}

void renderer_end_final_pass()
{
    glfwSwapBuffers(renderer.window);
    if (-1 == gl_check_errors()) {
        log_error("[Renderer] Internal OpenGL error.");
    }
}

void renderer_toggle_fullscreen()
{
    static IVec2 old_position = {0};
    static IVec2 old_size = {0};

    if (renderer.is_fullscreen) {
        glfwSetWindowMonitor(renderer.window, NULL, 0, 0, 
                             old_size.x, old_size.y, 
                             GLFW_DONT_CARE);
        glfwSetWindowPos(renderer.window, old_position.x, old_position.y);
        renderer.is_fullscreen = false;
    } else {
        GLFWmonitor *monitor = get_current_monitor(renderer.window);
        const GLFWvidmode *mode = glfwGetVideoMode(monitor);
        old_size = renderer.window_size;
        glfwGetWindowPos(renderer.window, &old_position.x, &old_position.y);
        renderer.window_size.x = mode->width;
        renderer.window_size.y = mode->height;
        glfwSetWindowMonitor(renderer.window, monitor, 0, 0, 
                             renderer.window_size.x, 
                             renderer.window_size.y, 
                             GLFW_DONT_CARE);
        renderer.is_fullscreen = true;
    }

    renderer.should_reload = true;
}

void renderer_toggle_maximized_shader_view()
{
    renderer.shader_view_is_maximized = !renderer.shader_view_is_maximized;
    renderer.should_reload = true;
}

GLFWwindow *renderer_get_glfw_window()
{
    return renderer.window;
}

b8 renderer_should_close()
{
    return glfwWindowShouldClose(renderer.window);
}

b8 renderer_should_reload()
{
    return renderer.should_reload;
}

b8 renderer_shader_view_is_maximized()
{
    return renderer.shader_view_is_maximized;
}

b8 renderer_is_fullscreen()
{
    return renderer.is_fullscreen;
}

IVec2 renderer_window_size()
{
    return renderer.window_size;
}

IVec2 renderer_shader_viewport_size()
{
    if (renderer.shader_view_is_maximized) {
        return renderer.window_size;
    }
    return gui_shader_window_size();
}

/*--- Private functions -----------------------------------------------------------------*/

static void resize_callback(GLFWwindow *window, i32 w, i32 h)
{
    (void) window;
    glViewport(0, 0, w, h);
    renderer.window_size.x = w;
    renderer.window_size.y = h;
    renderer.should_reload = true;
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

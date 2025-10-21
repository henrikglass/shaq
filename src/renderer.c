/*--- Include files ---------------------------------------------------------------------*/

#include "renderer.h"

#include "shaq_core.h"
#include "constants.h"
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
    glfwSetKeyCallback(renderer.window, key_callback);
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
    glClearColor(0.117f, 0.117f, 0.117f, 1.0f);

    shader_make_last_pass_shader(&renderer.last_pass_shader);
    imgui_init(renderer.window);

    if (gl_check_errors() != 0) {
        log_error("Failed to setup one or more OpenGL-intrinsic things.");
        exit(1);
    }
}

void renderer_final()
{
    imgui_final();
    glfwTerminate();
}

void renderer_reload()
{
    renderer.should_reload = false;
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

void renderer_draw_fullscreen_shader(Shader *s)
{
    if (s == NULL) {
        return;
    }

    glUniform1i(glGetUniformLocation(renderer.last_pass_shader.gl_shader_program_id, "tex"), 0);
    glUniform2iv(glGetUniformLocation(renderer.last_pass_shader.gl_shader_program_id, "iresolution"), 
                 1, (i32 *)&renderer.window_size); 
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, s->render_texture.gl_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void renderer_begin_final_pass(void)
{
    glUseProgram(renderer.last_pass_shader.gl_shader_program_id);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    glClear(GL_COLOR_BUFFER_BIT);
}

void renderer_end_final_pass(void)
{
    glfwSwapBuffers(renderer.window);
    glfwPollEvents();
    gl_check_errors();

    f64 x, y;
    glfwGetCursorPos(renderer.window, &x, &y);
    renderer.mouse_position = vec2_make(x, y);
    renderer.lmb_was_down_last_frame = renderer.lmb_is_down;
    renderer.rmb_was_down_last_frame = renderer.rmb_is_down;
    renderer.lmb_is_down = glfwGetMouseButton(renderer.window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    renderer.rmb_is_down = glfwGetMouseButton(renderer.window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
    /* 
     * TODO: Handle this in a better way. Currently, mouse_drag_position may be updated
     *       when, for instance, the shader view window is being resized
     */
    if (renderer.lmb_is_down /*&& !imgui_is_any_item_active()*/) {
        if (renderer.shader_view_is_maximized) {
            renderer.mouse_drag_position = renderer.mouse_position;
        } else {
            IVec2 swpos = gui_shader_window_position();
            IVec2 swsize = gui_shader_window_size();
            float min_x = swpos.x;
            float min_y = swpos.y;
            float max_x = swpos.x + swsize.x;
            float max_y = swpos.y + swsize.y;
            if (((f32)x >= min_x) && ((f32)x < max_x) &&
                ((f32)y >= min_y) && ((f32)y < max_y)) {
                renderer.mouse_drag_position = renderer.mouse_position;
            }
        }
    }
}

b8 renderer_should_close()
{
    return glfwWindowShouldClose(renderer.window);
}

b8 renderer_should_reload()
{
    return renderer.should_reload;
}

b8 renderer_shader_view_is_maximized(void)
{
    return renderer.shader_view_is_maximized;
}

IVec2 renderer_window_size()
{
    return renderer.window_size;
}

Vec2 renderer_mouse_position()
{
    return renderer.mouse_position;
}

Vec2 renderer_mouse_drag_position()
{
    return renderer.mouse_drag_position;
}

b8 renderer_mouse_left_button_is_down()
{
    return renderer.lmb_is_down;
}

b8 renderer_mouse_right_button_is_down()
{
    return renderer.rmb_is_down;
}

b8 renderer_mouse_left_button_was_clicked()
{
    return renderer.lmb_is_down && !renderer.lmb_was_down_last_frame;
}

b8 renderer_mouse_right_button_was_clicked()
{
    return renderer.rmb_is_down && !renderer.rmb_was_down_last_frame;
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

static void key_callback(GLFWwindow *window, i32 key, i32 scancode, i32 action, i32 mods)
{
    (void) scancode;

    if (action != GLFW_PRESS) {
        return;
    }

    if (mods == 0) {
        switch (key) {
            case GLFW_KEY_ESCAPE: {
                if (imgui_file_dialog_is_open()) {
                    imgui_close_file_dialog();
                }
            } break;
        }
    }

    if (mods == GLFW_MOD_ALT) {
        switch (key) {
            case GLFW_KEY_ENTER: {
                toggle_fullscreen(window);
                renderer.should_reload = true;
            } break;
        }
    }

    if (mods == GLFW_MOD_CONTROL) {
        switch (key) {
            case GLFW_KEY_D: {
                gui_toggle_darkmode();
            } break;

            case GLFW_KEY_F: {
                renderer.shader_view_is_maximized = !renderer.shader_view_is_maximized;
                renderer.should_reload = true;
            } break;

            case GLFW_KEY_R: {
                renderer.should_reload = true;
            } break;

            case GLFW_KEY_T: {
                shaq_reset_time();
            } break;

            case GLFW_KEY_O: {
                imgui_open_file_dialog();
            } break;

            case GLFW_KEY_Q: 
            case GLFW_KEY_W: {
                glfwSetWindowShouldClose(renderer.window, true);
            } break;
        }
    }
}

static void toggle_fullscreen(GLFWwindow *window)
{
    static IVec2 old_position = {0};
    static IVec2 old_size = {0};

    if (renderer.is_fullscreen) {
        printf("toggle fullscreen OFF\n");
        glfwSetWindowMonitor(window, NULL, 0, 0, 
                             old_size.x, old_size.y, 
                             GLFW_DONT_CARE);
        glfwSetWindowPos(window, old_position.x, old_position.y);
        renderer.is_fullscreen = false;
    } else {
        printf("toggle fullscreen ON\n");
        GLFWmonitor *monitor = get_current_monitor(window);
        const GLFWvidmode *mode = glfwGetVideoMode(monitor);
        old_size = renderer.window_size;
        glfwGetWindowPos(window, &old_position.x, &old_position.y);
        renderer.window_size.x = mode->width;
        renderer.window_size.y = mode->height;
        glfwSetWindowMonitor(window, monitor, 0, 0, 
                             renderer.window_size.x, 
                             renderer.window_size.y, 
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

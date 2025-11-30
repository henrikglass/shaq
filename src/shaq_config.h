#ifndef SHAQ_CONFIG_H
#define SHAQ_CONFIG_H

#define SHAQ_MAX_N_SHADERS                    64
#define SHAQ_MAX_N_UNIFORMS                   64
#define SHAQ_MAX_N_DYNAMIC_GUI_ITEMS          64
#define SHAQ_MAX_N_LOADED_TEXTURES            32
#define SHAQ_WIDGET_TEXT_INPUT_BUFFER_SIZE  1024
#define SHAQ_ENABLE_VSYNC                      1
#define SHAQ_FILEPATH_MAX_LEN                512
#define SHAQ_RELOAD_DURING_RESIZE              0
#define SHAQ_HUGEPAGES                         0
#define SHAQ_PROFILE                           0

#define SHAQ_DPI_CHANGE_INCREMENT              0.2f
#define SHAQ_DPI_DEFAULT                       1.0f // TODO set to default value as prescribed by the OS scaling

#define SHAQ_COLOR_DARKMODE_WINDOW_BG         RGBA(0x1E, 0x1E, 0x1E, 0xFF)
#define SHAQ_COLOR_DARKMODE_TITLE_BG          RGBA(0x25, 0x25, 0x25, 0xFF)
#define SHAQ_COLOR_LIGHTMODE_WINDOW_BG        RGBA(0xF0, 0xF0, 0xF0, 0xFF)
#define SHAQ_COLOR_LIGHTMODE_TITLE_BG         RGBA(0xF5, 0xF5, 0xF5, 0xFF)

#define SHAQ_KEY_FULLSCREEN              KEY(GLFW_MOD_ALT,     GLFW_KEY_ENTER, "Alt-Enter")
#define SHAQ_KEY_DARKMODE_TOGGLE         KEY(GLFW_MOD_CONTROL, GLFW_KEY_D, "Ctrl-D")
#define SHAQ_KEY_MAXIMIZE_SHADER_WINDOW  KEY(GLFW_MOD_CONTROL, GLFW_KEY_F, "Ctrl-F")
#define SHAQ_KEY_FORCE_RELOAD            KEY(GLFW_MOD_CONTROL, GLFW_KEY_R, "Ctrl-R")
#define SHAQ_KEY_RESET_TIME              KEY(GLFW_MOD_CONTROL, GLFW_KEY_T, "Ctrl-T")
#define SHAQ_KEY_PAUSE_TOGGLE            KEY(GLFW_MOD_CONTROL, GLFW_KEY_P, "Ctrl-P")
#define SHAQ_KEY_OPEN_FILE_DIALOG        KEY(GLFW_MOD_CONTROL, GLFW_KEY_O, "Ctrl-O")
#define SHAQ_KEY_EXIT                    KEY(GLFW_MOD_CONTROL, GLFW_KEY_W, "Ctrl-W")
#define SHAQ_KEY_DPI_LARGER              KEY(GLFW_MOD_CONTROL, 45, "Ctrl-Plus") // Maps to + on my ISO NORDIC keyboard
#define SHAQ_KEY_DPI_SMALLER             KEY(GLFW_MOD_CONTROL, 47, "Ctrl-Minus") // Maps to - on my ISO NORDIC keyboard
#define SHAQ_KEY_DPI_RESET               KEY(GLFW_MOD_CONTROL, GLFW_KEY_0, "Ctrl-0")

#endif /* SHAQ_CONFIG_H */


/*--- Include files ---------------------------------------------------------------------*/

#include "imguic.h"

extern "C" {
    #include "alloc.h"
}

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <stdio.h>

/*--- Private macros --------------------------------------------------------------------*/

/*--- Private type definitions ----------------------------------------------------------*/

/*--- Private function prototypes -------------------------------------------------------*/

/*--- Public variables ------------------------------------------------------------------*/

/*--- Private variables -----------------------------------------------------------------*/

/*--- Public functions ------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

void imgui_init(GLFWwindow *window)
{
    IMGUI_CHECKVERSION(); 
    ImGui::CreateContext();
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 450 core");

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 10.0f;
    style.ChildRounding = 10.0f;
    style.FrameRounding = 10.0f;
    style.PopupRounding = 10.0f;
    style.ScrollbarRounding = 10.0f;
    style.TabRounding = 10.0f;
    style.GrabRounding = 10.0f;
    ImGui::StyleColorsLight();
}

void imgui_final()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void imgui_set_darkmode(b8 enable)
{
    if (enable) {
        ImGui::StyleColorsDark();
    } else {
        ImGui::StyleColorsLight();
    }
}

void imgui_begin_frame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport(0, NULL /*, ImGuiDockNodeFlags_PassthruCentralNode */);
}

b8 imgui_is_any_item_active()
{
    return ImGui::IsAnyItemActive();
}

b8 imgui_begin(const char *str)
{
    return ImGui::Begin(str);
}

void imgui_begin_child(const char *label, u32 color)
{
    ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32((color >> 24) & 0xFF,
                                                     (color >> 16) & 0xFF,
                                                     (color >>  8) & 0xFF,
                                                     (color >>  0) & 0xFF));
    ImGui::BeginChild(label);
}

void imgui_begin_table(const char *label, i32 n_cols)
{
    ImGui::BeginTable(label, n_cols);
}

b8 imgui_begin_combo(const char *label, const char *preview_value)
{
    return ImGui::BeginCombo(label, preview_value);
}

b8 imgui_begin_main_menu_bar()
{
    return ImGui::BeginMainMenuBar();
}

b8 imgui_begin_menu(const char *label)
{
    return ImGui::BeginMenu(label);
}

b8 imgui_menu_item(const char *label, const char *shortcut)
{
    return ImGui::MenuItem(label, shortcut);
}

void imgui_end_menu()
{
    ImGui::EndMenu();
}

void imgui_end_main_menu_bar()
{
    ImGui::EndMainMenuBar();
}

void imgui_draw_texture(u32 gl_texture_id, int w, int h)
{
    ImGui::Image((ImTextureID)(uintptr_t)gl_texture_id, 
                 ImVec2(w, h), 
                 ImVec2(0, 1), 
                 ImVec2(1, 0));
}

void imgui_table_next_row(void)
{
    ImGui::TableNextRow();
}

void imgui_table_next_col(void)
{
    ImGui::TableNextColumn();
}

void imgui_text_unformatted(const char *str, size_t len)
{
    ImGui::TextUnformatted(str, str + len);
}

void imgui_textf(const char *fmt, ...)
{ 
    va_list args;
    va_start(args, fmt);
    ImGui::TextV(fmt, args);
    ImGui::SameLine();
    va_end(args);
} 

void imgui_text_color(const char *str, u32 color)
{
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32((color >> 24) & 0xFF,
                                                  (color >> 16) & 0xFF,
                                                  (color >>  8) & 0xFF,
                                                  (color >>  0) & 0xFF));
    ImGui::Text(str);
    ImGui::PopStyleColor();
    ImGui::SameLine();
}

void imgui_separator()
{
    ImGui::Separator();
}

void imgui_newline()
{
    ImGui::NewLine();
}

void imgui_checkbox(const char *label, b8 *b)
{
    ImGui::Checkbox(label, b);
}

void imgui_drag_int(const char *label, int *v, int min, int max)
{
    ImGui::DragInt(label, v, 1, min, max, "%d", ImGuiSliderFlags_Logarithmic);
}

void imgui_input_int(const char *label, int *v)
{
    ImGui::InputInt(label, v);
}

void imgui_input_float(const char *label, float *v)
{
    ImGui::InputFloat(label, v);
}

void imgui_input_float2(const char *label, float *v)
{
    ImGui::InputFloat2(label, v);
}

void imgui_input_float3(const char *label, float *v)
{
    ImGui::InputFloat3(label, v);
}

void imgui_input_float4(const char *label, float *v)
{
    ImGui::InputFloat4(label, v);
}

void imgui_slider_float(const char *label, float *v, float min, float max, b8 log)
{
    if (log) {
        ImGui::SliderFloat(label, v, min, max, "%.3f", ImGuiSliderFlags_Logarithmic);
    } else {
        ImGui::SliderFloat(label, v, min, max, "%.3f");
    }
}

void imgui_color_picker(const char *label, float *v)
{
    ImGui::ColorEdit4(label, v);
}

b8 imgui_selectable(const char *label, b8 is_selected)
{
    return ImGui::Selectable(label, is_selected);
}

void imgui_set_item_default_focus(void)
{
    ImGui::SetItemDefaultFocus();
}

void imgui_push_style_shader_window(void)
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(100, 80));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
}

void imgui_pop_style_shader_window(void)
{
    ImGui::PopStyleVar();
    ImGui::PopStyleVar();
    ImGui::PopStyleVar();
}

void imgui_get_current_window_dimensions(int *x, int *y, int *w, int *h)
{
    ImVec2 pos = ImGui::GetWindowPos();
    ImVec2 size = ImGui::GetContentRegionAvail();
    *x = pos.x;
    *y = pos.y;
    *w = size.x;
    *h = size.y;
}

void imgui_end_combo(void)
{
    ImGui::EndCombo();
}

void imgui_end_table()
{
    ImGui::EndTable();
}

void imgui_end_child()
{
    ImGui::EndChild();
    ImGui::PopStyleColor();
}

void imgui_end()
{
    ImGui::End();
}

void imgui_end_frame()
{
    //ImGui::ShowDemoWindow();
    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

#ifdef __cplusplus
}
#endif


/*--- Private functions -----------------------------------------------------------------*/


#include "UIModule.h"

void UIModule::Init(SDL_Window* window, SDL_GLContext gl_context) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init();
}

void UIModule::Shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}

void UIModule::ProcessEvent(SDL_Event* event) {
    ImGui_ImplSDL3_ProcessEvent(event);
}

void UIModule::NewFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

void UIModule::RenderPanels() {
    for (auto& panel : panels_) {
        ImGui::Begin(panel.name.c_str());
        panel.renderFunc();
        ImGui::End();
    }
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UIModule::AddPanel(const std::string& name, std::function<void()> renderFunc) {
    panels_.push_back({name, renderFunc});
}

#include "UIModule.h"
#include "ViewportPanel.h"

void UIModule::Init(SDL_Window* window, SDL_GLContext gl_context) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();
    SDL_GL_MakeCurrent(window, gl_context);

    ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init();

}

void UIModule::AddViewportPanel()
{
	AddPanel(std::make_unique<ViewportPanel>(800, 600));
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
        ImGui::Begin(panel.get()->GetName().c_str());
        panel.get()->Render();
        ImGui::End();
    }
    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UIModule::Update() {
    
    NewFrame();

    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus;

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);

    ImGui::Begin("DockSpaceRoot", nullptr, window_flags);

    ImGui::DockSpace(ImGui::GetID("MainDockSpace"));
    ImGui::End();
    
    RenderPanels();
}

void UIModule::AddPanel(std::unique_ptr<Panel> panel) {
    panels_.emplace_back(std::move(panel));
}

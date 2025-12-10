#include "UIEditorModule.h"
#include "ViewportPanel.h"

void UIEditorModule::Init(SDL_Window* window, SDL_GLContext gl_context) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();
    SDL_GL_MakeCurrent(window, gl_context);

    ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init();

}

void UIEditorModule::AddViewportPanel(InputModule* inputModule)
{
	AddPanel(std::make_unique<ViewportPanel>(800, 600, inputModule));
}

void UIEditorModule::Shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}

void UIEditorModule::ProcessEvent(SDL_Event* event) {
    ImGui_ImplSDL3_ProcessEvent(event);
}

void UIEditorModule::NewFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

void UIEditorModule::RenderPanels(float deltaTime) {
    for (auto& panel : panels_) {
        ImGui::Begin(panel.get()->GetName().c_str());
        panel.get()->Render(deltaTime);
        ImGui::End();
    }
    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UIEditorModule::Update(float deltaTime) {

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

    RenderPanels(deltaTime);
}

void UIEditorModule::AddPanel(std::unique_ptr<Panel> panel) {
    panels_.emplace_back(std::move(panel));
}

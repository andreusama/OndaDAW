#pragma once
#include "SDL3/SDL.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"
#include <memory>
#include "Panel.h"


class UIModule {
    public:
        void Init(SDL_Window*, SDL_GLContext);
        void Shutdown();
        void ProcessEvent(SDL_Event*);
        void NewFrame();
        void RenderPanels();
        void AddPanel(std::unique_ptr<Panel> panel);
        void AddViewportPanel();
        void Update();

    private:
        std::vector<std::unique_ptr<Panel>> panels_;
};  


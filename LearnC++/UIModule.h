#pragma once
#include "SDL3/SDL.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"
#include <vector>
#include <string>
#include <functional>

class UIModule {
    public:
        void Init(SDL_Window*, SDL_GLContext);
        void Shutdown();
        void ProcessEvent(SDL_Event*);
        void NewFrame();
        void RenderPanels();
        void AddPanel(const std::string& name, std::function<void()> renderFunc);
    private:
        struct Panel { std::string name; std::function<void()> renderFunc; };
        std::vector<Panel> panels_;
};  

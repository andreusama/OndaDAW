// Application.cpp
#include "Windows.h"
#include "Application.h"
#include "Window.h"
#include <SDL3/SDL.h>
#include "imgui.h"  
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"
#include "gl/GL.h"
#include <stdexcept>

#include <memory> // Add this include

Application::Application()
    : window_(std::make_unique<Window>("Mini DAW", 800, 600)), running_(false)
{

}

Application::~Application()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}

void Application::Run()
{
    ImGui::CreateContext();
    ImGui_ImplSDL3_InitForOpenGL(window_->GetSDLWindow(), window_->GetSDL_GLContext());
    ImGui_ImplOpenGL3_Init("#version 150");

    bool run = true;
    while (run) 
    {
        SDL_Event e;
        while (SDL_PollEvent(&e)) 
        {
            if (e.type == SDL_EVENT_QUIT) run = false;
            ImGui_ImplSDL3_ProcessEvent(&e);
        }

        ImGui_ImplSDL3_NewFrame();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Audio Engine");
        ImGui::Text("Mini DAW UI ready!");
        ImGui::End();

        ImGui::Render();
        glViewport(0, 0, 800, 600);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window_->GetSDLWindow());
    }
}
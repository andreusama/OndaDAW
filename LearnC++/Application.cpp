// Application.cpp
#include "Windows.h"
#include "Application.h"
#include "Window.h"
#include <SDL3/SDL.h>
#include <stdexcept>
#include "UIModule.h"
#include <memory>
#include <gl/GL.h>

Application::Application()
    : window_(std::make_unique<Window>("Mini DAW", 800, 600)), running_(false), ui_(std::make_unique<UIModule>())
{
}

Application::~Application()
{
    ui_->Shutdown();
}

void Application::Run()
{
    ui_->Init(window_->GetSDLWindow(), window_->GetSDL_GLContext());

    while (!window_->ShouldClose())
    {
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            window_->ProcessEvents(&e);
            ui_->ProcessEvent(&e);
        }

        ui_->NewFrame();
        ui_->RenderPanels();

        glViewport(0, 0, 800, 600);
        glClear(GL_COLOR_BUFFER_BIT);
        SDL_GL_SwapWindow(window_->GetSDLWindow());
    }
}
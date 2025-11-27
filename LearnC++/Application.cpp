// Application.cpp
#include "Windows.h"
#include "Application.h"
#include "Window.h"
#include <SDL3/SDL.h>
#include <stdexcept>
#include "UIModule.h"
#include <memory>
#include <gl/GL.h>

Application::Application(): window_(std::make_unique<Window>("Mini DAW", 800, 600)), running_(false), ui_(std::make_unique<UIModule>())
{
}

Application::~Application()
{
    ui_->Shutdown();
}

void Application::Run()
{
    ui_->Init(window_->GetSDLWindow(), window_->GetSDL_GLContext());
    ui_->AddViewportPanel();

    while (!window_->ShouldClose())
    {
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            window_->ProcessEvents(&e);
            ui_->ProcessEvent(&e);
        }
        //All glViewport does is define the area of the drawing context that you will actually draw to.
        glViewport(0, 0, 800, 600);
        //GL_COLOR_BUFFER_BIT and GL_DEPTH_BUFFER_BIT aren't functions, they're constants. 
        //You use them to tell glClear() which buffers you want it to clear - in your example, the depth buffer and the "buffers currently enabled for color writing
        glClear(GL_COLOR_BUFFER_BIT);

        ui_->Update();

        SDL_GL_SwapWindow(window_->GetSDLWindow());
    }
}
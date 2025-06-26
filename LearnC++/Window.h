// Window.h
#pragma once
#include <SDL3/SDL.h>
class Window {
public:
    Window(const char* title, int width, int height);
    ~Window();
    void PollEvents();
    bool ShouldClose() const;
    SDL_Window* GetSDLWindow() const;
    SDL_GLContext GetSDL_GLContext() const;
private:
    SDL_Window* window_;
    SDL_GLContext gl_context_;
    bool shouldClose_;
};
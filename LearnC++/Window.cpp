#include <SDL3/SDL.h>
#include "Window.h"
#include <stdexcept>
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"

Window::Window(const char* title, int width, int height)
	: shouldClose_(false) {
	SDL_Init(SDL_INIT_VIDEO);
	window_ = SDL_CreateWindow(title, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	gl_context_ = SDL_GL_CreateContext(window_);
	if (!gl_context_)
	{
		throw std::runtime_error("Failed to create SDL_GL context");
	}
	if (!window_) {
		throw std::runtime_error("Failed to create SDL window");
	}
}

Window::~Window() {
	if (gl_context_) {
		SDL_GL_DestroyContext(gl_context_);
	}
	if (window_) {
		SDL_DestroyWindow(window_);
	}
	SDL_Quit();
}

SDL_Window* Window::GetSDLWindow() const {
	return window_;
}

SDL_GLContext Window::GetSDL_GLContext() const {
	return gl_context_;
}

void Window::ProcessEvents(SDL_Event* event) {

	while (SDL_PollEvent(event)) {
		switch (event->type) {
		case SDL_EVENT_QUIT:
			shouldClose_ = true;
			break;
		case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
			shouldClose_ = true;
			break;
			// Add more cases for keyboard, mouse, resize, etc.
		}
	}
}

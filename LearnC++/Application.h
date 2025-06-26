// Application.h
#pragma once
#include "Window.h"

class Application {
public:
    Application();
    ~Application();
    void Run();
private:
    Window* window_;
    bool running_;
};
// Application.h
#pragma once
#include "Window.h"
#include <memory>

class Application {
public:
    Application();
    ~Application();
    void Run();
private:
    std::unique_ptr<Window> window_;
    bool running_;
};
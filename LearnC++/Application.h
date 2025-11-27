// Application.h
#pragma once
#include "Window.h"
#include "UIModule.h"
#include <memory>

class Application {
public:
    Application();
    ~Application();
    void Run();
private:
    std::unique_ptr<Window> window_;
    std::unique_ptr<UIModule> ui_;
    bool running_;
};
// Application.h
#pragma once
#include "Window.h"
#include "UIModule.h"
#include "InputModule.h"
#include <memory>

class Application {
public:
    Application();
    ~Application();
    void Run();

    InputModule* GetInputModule() { return inputModule_.get(); }

	float deltaTime_;
private:
    std::unique_ptr<Window> window_;
    std::unique_ptr<UIModule> ui_;
    std::unique_ptr<InputModule> inputModule_;
    bool running_;
};
// Application.h
#pragma once
#include "Window.h"
#include "UIEditorModule.h"
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
    std::unique_ptr<UIEditorModule> ui_;
    std::unique_ptr<InputModule> inputModule_;
    bool running_;
};
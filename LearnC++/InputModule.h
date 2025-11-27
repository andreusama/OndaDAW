#pragma once
#include <SDL3/SDL.h>
#include "Camera.h"
#include <memory>

class InputModule {
public:
    InputModule();
    ~InputModule();

    void ProcessKeyboardInput(Camera* camera, float deltaTime);
    void ProcessMouseInput(Camera* camera, bool isViewportFocused, bool isViewportHovered);

    bool IsKeyPressed(SDL_Scancode key) const;

private:
    const bool* keyState_;
};

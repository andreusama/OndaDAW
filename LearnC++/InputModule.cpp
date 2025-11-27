#include "InputModule.h"
#include "imgui.h"

InputModule::InputModule() : keyState_(nullptr)
{
    keyState_ = SDL_GetKeyboardState(NULL);
}

InputModule::~InputModule()
{
}

void InputModule::ProcessKeyboardInput(Camera* camera, float deltaTime)
{
    if (!camera) return;

    // Camera movement with WASD + QE
    if (keyState_[SDL_SCANCODE_W]) {
        camera->ProcessKeyboard(Camera::CameraMovement::FORWARD, deltaTime);
    }
    if (keyState_[SDL_SCANCODE_S]) {
        camera->ProcessKeyboard(Camera::CameraMovement::BACKWARD, deltaTime);
    }
    if (keyState_[SDL_SCANCODE_A]) {
        camera->ProcessKeyboard(Camera::CameraMovement::LEFT, deltaTime);
    }
    if (keyState_[SDL_SCANCODE_D]) {
        camera->ProcessKeyboard(Camera::CameraMovement::RIGHT, deltaTime);
    }
    if (keyState_[SDL_SCANCODE_Q]) {
        camera->ProcessKeyboard(Camera::CameraMovement::DOWN, deltaTime);
    }
    if (keyState_[SDL_SCANCODE_E]) {
        camera->ProcessKeyboard(Camera::CameraMovement::UP, deltaTime);
    }
}

void InputModule::ProcessMouseInput(Camera* camera, bool isViewportFocused, bool isViewportHovered)
{
    if (!camera) return;

    ImGuiIO& io = ImGui::GetIO();

    // Mouse look - only when right mouse button is held and viewport is hovered
    if (ImGui::IsMouseDown(ImGuiMouseButton_Right) && isViewportHovered) {
        ImVec2 mouseDelta = io.MouseDelta;
        if (mouseDelta.x != 0.0f || mouseDelta.y != 0.0f) {
            camera->ProcessMouseMovement(mouseDelta.x, -mouseDelta.y);
        }
    }

    // Mouse scroll for zoom - only when viewport is hovered
    if (io.MouseWheel != 0.0f && isViewportHovered) {
        camera->ProcessMouseScroll(io.MouseWheel);
    }
}

bool InputModule::IsKeyPressed(SDL_Scancode key) const
{
    return keyState_[key];
}

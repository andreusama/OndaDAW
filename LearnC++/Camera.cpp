#include "Camera.h"

// Default camera values
const float DEFAULT_SPEED = 5.0f;
const float DEFAULT_SENSITIVITY = 0.1f;
const float DEFAULT_ZOOM = 45.0f;
const float NEAR_PLANE = 0.1f;
const float FAR_PLANE = 1000.0f;

Camera::Camera(glm::vec3 position, glm::vec3 worldUp, float yaw, float pitch)
    : position_(position)
    , worldUp_(worldUp)
    , yaw_(yaw)
    , pitch_(pitch)
    , front_(glm::vec3(0.0f, 0.0f, -1.0f))
    , movementSpeed_(DEFAULT_SPEED)
    , mouseSensitivity_(DEFAULT_SENSITIVITY)
    , zoom_(DEFAULT_ZOOM)
    , nearPlane_(NEAR_PLANE)
    , farPlane_(FAR_PLANE)
{
    UpdateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() const
{
    // LookAt matrix: position, target (position + front), up
    return glm::lookAt(position_, position_ + front_, up_);
}

glm::mat4 Camera::GetProjectionMatrix(float aspectRatio) const
{
    // Perspective projection: FOV (in radians), aspect ratio, near, far
    return glm::perspective(glm::radians(zoom_), aspectRatio, nearPlane_, farPlane_);
}

glm::mat4 Camera::GetMVPMatrix(const glm::mat4& model, float aspectRatio) const
{
    glm::mat4 view = GetViewMatrix();
    glm::mat4 projection = GetProjectionMatrix(aspectRatio);

    // MVP = Projection * View * Model
    return projection * view * model;
}

void Camera::ProcessKeyboard(CameraMovement direction, float deltaTime)
{
    float velocity = movementSpeed_ * deltaTime;

    switch (direction) {
        case CameraMovement::FORWARD:
            position_ += front_ * velocity;
            break;
        case CameraMovement::BACKWARD:
            position_ -= front_ * velocity;
            break;
        case CameraMovement::LEFT:
            position_ -= right_ * velocity;
            break;
        case CameraMovement::RIGHT:
            position_ += right_ * velocity;
            break;
        case CameraMovement::UP:
            position_ += up_ * velocity;
            break;
        case CameraMovement::DOWN:
            position_ -= up_ * velocity;
            break;
    }
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch)
{
    xoffset *= mouseSensitivity_;
    yoffset *= mouseSensitivity_;

    yaw_ += xoffset;
    pitch_ += yoffset;

    // Constrain pitch to avoid screen flip
    if (constrainPitch) {
        if (pitch_ > 89.0f)
            pitch_ = 89.0f;
        if (pitch_ < -89.0f)
            pitch_ = -89.0f;
    }

    UpdateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset)
{
    zoom_ -= yoffset;

    // Constrain zoom
    if (zoom_ < 1.0f)
        zoom_ = 1.0f;
    if (zoom_ > 90.0f)
        zoom_ = 90.0f;
}

void Camera::UpdateCameraVectors()
{
    // Calculate new front vector from Euler angles
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(yaw_)) * cos(glm::radians(pitch_));
    newFront.y = sin(glm::radians(pitch_));
    newFront.z = sin(glm::radians(yaw_)) * cos(glm::radians(pitch_));

    front_ = glm::normalize(newFront);

    // Calculate right and up vectors
    right_ = glm::normalize(glm::cross(front_, worldUp_));
    up_ = glm::normalize(glm::cross(right_, front_));
}

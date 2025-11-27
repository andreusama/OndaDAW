#pragma once
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

class Camera {
public:
    // Camera movement directions
    enum class CameraMovement {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT,
        UP,
        DOWN
    };

    // Constructor with default position
    Camera(glm::vec3 position = glm::vec3(0.0f, 5.0f, 10.0f),
           glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = -90.0f,
           float pitch = -20.0f);

    // Get view and projection matrices
    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix(float aspectRatio) const;

    // Get combined MVP matrix for a given model matrix
    glm::mat4 GetMVPMatrix(const glm::mat4& model, float aspectRatio) const;

    // Camera movement
    void ProcessKeyboard(CameraMovement direction, float deltaTime);
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
    void ProcessMouseScroll(float yoffset);

    // Getters
    glm::vec3 GetPosition() const { return position_; }
    glm::vec3 GetFront() const { return front_; }
    float GetZoom() const { return zoom_; }

private:
    // Camera attributes
    glm::vec3 position_;
    glm::vec3 front_;
    glm::vec3 up_;
    glm::vec3 right_;
    glm::vec3 worldUp_;

    // Euler angles (in degrees)
    float yaw_;
    float pitch_;

    // Camera options
    float movementSpeed_;
    float mouseSensitivity_;
    float zoom_;  // Field of view

    // Projection parameters
    float nearPlane_;
    float farPlane_;

    // Update camera vectors based on Euler angles
    void UpdateCameraVectors();
};

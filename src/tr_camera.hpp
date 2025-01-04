#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <functional>

class Camera
{
public:
    static Camera *instance;

    Camera();

    glm::vec3 m_Position = glm::vec3(8595, 33784, 35878);
    glm::vec3 playerRotation = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::quat m_Rotation;

    float m_SpeedMultiplier = 1;

    float m_Elevation = 50100.0f; // ~10km over the sea level

    float m_FOV, m_NearClipPlane, m_FarClipPlane;

    glm::vec2 getPolarPosition();

    glm::mat4 getViewMatrix();
    glm::mat4 getProjectionMatrix(float screen_width, float screen_height);

    glm::vec3 getForwardVector();
    glm::vec3 getRightVector();
    glm::vec3 getUpVector();

    void HandleMovement(float deltaTime, GLFWwindow *window);
};

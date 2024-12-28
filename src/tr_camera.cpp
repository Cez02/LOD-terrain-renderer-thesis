//
// Created by cezary on 12.11.24.
//

#include "tr_camera.hpp"
#include "tr_appconfig.hpp"
#include "tr_logger.hpp"

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

#include <functional>
#include <string>


Camera *Camera::instance = nullptr;

Camera::Camera() {
    m_FarClipPlane = 100000.0f;
    m_FOV = 45.0f;
    m_NearClipPlane = 0.1;

    Camera::instance = this;
}

glm::mat4 Camera::getViewMatrix() {
    glm::vec3 directionVector = getForwardVector();

    glm::mat4 view = glm::lookAt(m_Position, m_Position + directionVector, getUpVector());

    return view;
}

glm::mat4 Camera::getProjectionMatrix(float screen_width, float screen_height){
    glm::mat4 projection =
            glm::perspective(glm::radians(m_FOV), screen_width / screen_height, m_NearClipPlane, m_FarClipPlane);
    projection[1][1] *= -1;
    return projection;
}

glm::vec3 Camera::getForwardVector() {
    return glm::normalize(m_Rotation * glm::vec4(0, 0, 1.0, 1.0));
}

glm::vec3 Camera::getRightVector() {
    return glm::normalize(m_Rotation * glm::vec4(1.0, 0, 0.0, 1.0));
}

glm::vec3 Camera::getUpVector() {
    return glm::normalize(m_Rotation * glm::vec4(0.0, 1.0, 0.0, 1.0));
}

void Camera::HandleMovement(float deltaTime, GLFWwindow *window)
{
    float playerSpeed = 1000.0f;

    log(std::to_string(deltaTime));

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        m_Position += getForwardVector() * deltaTime * playerSpeed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        m_Position -= getForwardVector()  * deltaTime * playerSpeed;

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        m_Position += getRightVector()  * deltaTime * playerSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        m_Position -= getRightVector()  * deltaTime * playerSpeed;

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        m_Position += getUpVector()  * deltaTime * playerSpeed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        m_Position -= getUpVector()  * deltaTime * playerSpeed;

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        APP_CONFIG.m_LODLevel = 1;
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        APP_CONFIG.m_LODLevel = 2;
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
        APP_CONFIG.m_LODLevel = 4;
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
        APP_CONFIG.m_LODLevel = 8;


    float rotateDelta = 0;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        rotateDelta = -deltaTime;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        rotateDelta = deltaTime;

    playerRotation.y += rotateDelta;

    auto newRot = glm::quat(playerRotation);

    newRot = glm::normalize(newRot);

    m_Rotation = glm::normalize(newRot);
}

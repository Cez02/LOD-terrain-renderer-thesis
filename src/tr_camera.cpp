//
// Created by cezary on 12.11.24.
//

#include "tr_camera.hpp"

Camera::Camera() {
    m_FarClipPlane = 10.0f;
    m_FOV = 45.0f;
    m_NearClipPlane = 0.01;
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
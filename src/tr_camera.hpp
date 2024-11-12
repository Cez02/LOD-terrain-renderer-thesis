#pragma once

#include <glm/glm.hpp>
#include <glm/ext.hpp>

class Camera {
public:
    Camera();

    glm::vec3 m_Position;
    glm::quat m_Rotation;

    float m_FOV, m_NearClipPlane, m_FarClipPlane;

    glm::mat4 getViewMatrix();
    glm::mat4 getProjectionMatrix(float screen_width, float screen_height);

    glm::vec3 getForwardVector();
    glm::vec3 getRightVector();
    glm::vec3 getUpVector();
};

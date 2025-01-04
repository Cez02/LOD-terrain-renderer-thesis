//
// Created by cezary on 12.11.24.
//

#include "tr_camera.hpp"
#include "tr_appconfig.hpp"
#include "tr_logger.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <GLFW/glfw3.h>

#include <functional>
#include <string>
#include <glm/gtx/string_cast.hpp>

static glm::vec3 orthogonal(glm::vec3 v)
{
    float x = abs(v.x);
    float y = abs(v.y);
    float z = abs(v.z);

    glm::vec3 other = x < y ? (x < z ? glm::vec3(1, 0, 0) : glm::vec3(0, 0, 1)) : (y < z ? glm::vec3(0, 1, 0) : glm::vec3(0, 0, 1));
    return cross(v, other);
}

static glm::quat get_rotation_between(glm::vec3 u, glm::vec3 v)
{
    // It is important that the inputs are of equal length when
    // calculating the half-way vector.
    u = glm::normalize(u);
    v = glm::normalize(v);

    // Unfortunately, we have to check for when u == -v, as u + v
    // in this case will be (0, 0, 0), which cannot be normalized.
    if (u == -v)
    {
        // 180 degree rotation around any orthogonal vector
        return glm::quat(0, glm::normalize(orthogonal(u)));
    }

    glm::vec3 half = glm::normalize(u + v);
    return glm::quat(dot(u, half), cross(u, half));
}

Camera *Camera::instance = nullptr;

Camera::Camera() {
    m_FarClipPlane = 100000.0f;
    m_FOV = 45.0f;
    m_NearClipPlane = 0.1;

    Camera::instance = this;
}

glm::vec2 Camera::getPolarPosition()
{
    glm::vec3 normPos = glm::normalize(m_Position);

    float x = normPos.x;
    float y = normPos.y;
    float z = normPos.z;

    float r = sqrt(x * x + y * y + z * z);
    float longitude = asin(x / sqrt(x * x + z * z)) * (z < 0 ? -1 : 1);
    float latitude = asin(y / r);

    return glm::vec2(longitude, latitude);
}

glm::mat4 Camera::getViewMatrix() {

    // we get the proper forward vector for the view matrix

    auto newRot = glm::angleAxis(playerRotation.x, glm::vec3(1, 0, 0));
    newRot =  glm::rotation(glm::vec3(0, 1, 0), glm::normalize(m_Position)) * newRot;

    glm::vec3 directionVector = glm::normalize(glm::angleAxis(playerRotation.y, glm::normalize(m_Position)) * newRot * glm::vec4(0.0, 0.0, 1.0, 1.0));
    // glm::vec3 properUp = glm::normalize(newRot * glm::vec4(0.0, 1.0, 0.0, 1.0));
    glm::vec3 properUp = glm::normalize(m_Position);

    // 100.0 to fix numeric issues
    glm::mat4 view = glm::lookAt(m_Position, m_Position + directionVector * 100.0f, properUp);

    return view;
}

glm::mat4 Camera::getProjectionMatrix(float screen_width, float screen_height){
    glm::mat4 projection =
            glm::perspective(glm::radians(m_FOV), screen_width / screen_height, m_NearClipPlane, m_FarClipPlane);
    projection[1][1] *= -1;
    return projection;
}

glm::vec3 Camera::getForwardVector() {
    // We ignore x for simplified movement
    auto newRot = glm::quat(glm::vec3(0, playerRotation.y, 0));

    newRot = glm::normalize(newRot);

    glm::quat rot = glm::normalize(newRot);

    rot = get_rotation_between(glm::vec3(0, 1, 0), glm::normalize(m_Position)) * rot;

    return glm::normalize(rot * glm::vec4(0, 0, 1.0, 1.0));
}

glm::vec3 Camera::getRightVector() {
    // We ignore x for simplified movement
    auto newRot = glm::quat(glm::vec3(0, playerRotation.y, 0));

    newRot = glm::normalize(newRot);

    glm::quat rot = glm::normalize(newRot);

    rot = get_rotation_between(glm::vec3(0, 1, 0), glm::normalize(m_Position)) * rot;

    return glm::normalize(rot * glm::vec4(1.0, 0, 0.0, 1.0));
}

glm::vec3 Camera::getUpVector() {
    // We ignore x for simplified movement
    auto newRot = glm::quat(glm::vec3(0, playerRotation.y, 0));

    newRot = glm::normalize(newRot);

    glm::quat rot = glm::normalize(newRot);

    rot = get_rotation_between(glm::vec3(0, 1, 0), glm::normalize(m_Position)) * rot;

    return glm::normalize(rot * glm::vec4(0.0, 1.0, 0.0, 1.0));
}


void Camera::HandleMovement(float deltaTime, GLFWwindow *window)
{
    float playerSpeed = 10.0f;

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        m_SpeedMultiplier = 100.0;
    }
    else
        m_SpeedMultiplier = 1.0;

    playerSpeed *= m_SpeedMultiplier;

    float angleToEarthRatio = 360.0f / 40075000.0f;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        m_Position = glm::rotate(glm::identity<glm::quat>(), deltaTime * angleToEarthRatio * playerSpeed, getRightVector()) * glm::vec4(m_Position, 1.0);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        m_Position = glm::rotate(glm::identity<glm::quat>(), -deltaTime * angleToEarthRatio * playerSpeed, getRightVector()) * glm::vec4(m_Position, 1.0);

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        m_Position = glm::rotate(glm::identity<glm::quat>(), -deltaTime * angleToEarthRatio * playerSpeed, getForwardVector()) * glm::vec4(m_Position, 1.0);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        m_Position = glm::rotate(glm::identity<glm::quat>(), deltaTime * angleToEarthRatio * playerSpeed, getForwardVector()) * glm::vec4(m_Position, 1.0);

    // if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    //     m_Position += getUpVector()  * deltaTime * playerSpeed;
    // if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    //     m_Position -= getUpVector()  * deltaTime * playerSpeed;


    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        m_Elevation += deltaTime * playerSpeed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        m_Elevation -= deltaTime * playerSpeed;


    m_Position = glm::normalize(m_Position) * m_Elevation;

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        APP_CONFIG.m_LODLevel = 1;
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        APP_CONFIG.m_LODLevel = 2;
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
        APP_CONFIG.m_LODLevel = 4;
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
        APP_CONFIG.m_LODLevel = 8;


    float rotateDelta = 0, rotateDeltaX = 0;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        rotateDelta = -deltaTime * 1.0f;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        rotateDelta = deltaTime * 1.0f;

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        rotateDeltaX = deltaTime;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        rotateDeltaX = -deltaTime;

    playerRotation.y += rotateDelta;
    playerRotation.x += rotateDeltaX;
    //
    // if (playerRotation.x * 360.0 / 2*glm::two_pi<float>() < 60.0f)
    //     playerRotation.x = 60.0f * glm::two_pi<float>() / 360.0f;
    // else if(playerRotation.x * 360.0 / 2*glm::two_pi<float>() < -60.0f)
    //     playerRotation.x = -60.0f * glm::two_pi<float>() / 360.0f;

    playerRotation.x = glm::clamp(playerRotation.x, -1.2f, 1.2f);

    // log(std::to_string(playerRotation.x));

    auto newRot = glm::quat(glm::vec3(playerRotation.x, 0, 0));

    newRot = glm::normalize(newRot);

    m_Rotation = glm::angleAxis(playerRotation.y, glm::vec3(0, 1, 0)) * glm::angleAxis(playerRotation.x, glm::vec3(1, 0, 0));
    m_Rotation = glm::rotation(glm::vec3(0, 1, 0), glm::normalize(m_Position)) * m_Rotation;

    log(glm::to_string(glm::eulerAngles(get_rotation_between(glm::vec3(0, 1, 0), glm::normalize(m_Position)))));
}


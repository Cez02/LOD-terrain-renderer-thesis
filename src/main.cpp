#include <bits/stdc++.h>
#include <src/tr_window.hpp>
#include <src/tr_renderer.hpp>

#include <src/tr_logger.hpp>

#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/ext.hpp>

using namespace std;

int main(){

    log("Starting program");
    log("Launching window");

    Window mainWindow(800, 600, "Terrain renderer");

    log("Window initializaiton finished. Initializing vulkan...");

    Renderer mainRenderer("shaders/simple_shader.vert.spv", "shaders/simple_shader.frag.spv");

    mainRenderer.initVulkan(&mainWindow);
    log("Vulkan initialized. Initializaing scene data...");

    mainRenderer.getCamera() = Camera();
    mainRenderer.getCamera().m_Position = glm::vec3(0, 0, 0);
    mainRenderer.getCamera().m_Rotation = glm::quat(glm::vec3(0, 0, 0));

    log("Setup complete. Starting application loop.");

    glm::vec3 playerPosition = glm::vec3(0.0f);
    glm::vec3 playerRotation = glm::vec3(0.0f);

    float time = glfwGetTime();
    float deltaTime = 0;
    float playerSpeed = 1.0;

    while(!glfwWindowShouldClose(mainWindow.getGLFWWindow())){
        glfwPollEvents();

        /* Handle logic */

        Camera cam = mainRenderer.getCamera();

        if (glfwGetKey(mainWindow.getGLFWWindow(), GLFW_KEY_W) == GLFW_PRESS)
            playerPosition += cam.getForwardVector() * deltaTime;
        if (glfwGetKey(mainWindow.getGLFWWindow(), GLFW_KEY_S) == GLFW_PRESS)
            playerPosition -= cam.getForwardVector()  * deltaTime;

        if (glfwGetKey(mainWindow.getGLFWWindow(), GLFW_KEY_A) == GLFW_PRESS)
            playerPosition += cam.getRightVector()  * deltaTime;
        if (glfwGetKey(mainWindow.getGLFWWindow(), GLFW_KEY_D) == GLFW_PRESS)
            playerPosition -= cam.getRightVector()  * deltaTime;

        if (glfwGetKey(mainWindow.getGLFWWindow(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            playerPosition += cam.getUpVector()  * deltaTime;
        if (glfwGetKey(mainWindow.getGLFWWindow(), GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
            playerPosition -= cam.getUpVector()  * deltaTime;

        float rotateDelta = 0;
        if (glfwGetKey(mainWindow.getGLFWWindow(), GLFW_KEY_E) == GLFW_PRESS)
            rotateDelta = -deltaTime;
        if (glfwGetKey(mainWindow.getGLFWWindow(), GLFW_KEY_Q) == GLFW_PRESS)
            rotateDelta = deltaTime;

        playerRotation.y += rotateDelta;

//        if(playerRotation.y > 2*glm::pi<float>())
//            playerRotation.y -= 2*glm::pi<float>();
//        else if(playerRotation.y < 0)
//            playerRotation.y += 2*glm::pi<float>();
//
//        auto rot = glm::angleAxis(rotateDelta, cam.getUpVector());

//        auto newRot = rot * cam.m_Rotation;
        auto newRot = glm::quat(playerRotation);
//        if(glm::dot(newRot, cam.m_Rotation) < 0)
//            newRot *= -1;

        float dotProd = glm::dot(newRot, cam.m_Rotation);

        newRot = glm::normalize(newRot);

        mainRenderer.getCamera().m_Position = playerPosition;
        mainRenderer.getCamera().m_Rotation = glm::normalize(newRot);

        std::cout << std::endl;
        std::cout << "Player position: " << glm::to_string(playerPosition) << std::endl;
        std::cout << "Player rotation: " << glm::to_string(playerRotation) << std::endl;
        std::cout << "Player rotation: " << glm::to_string(glm::eulerAngles(mainRenderer.getCamera().m_Rotation)) << std::endl;
        std::cout << "Dot prod: " << dotProd << std::endl;

        std::cout << std::endl;

        /* Draw */

        mainRenderer.drawFrame();

        deltaTime = abs(glfwGetTime() - time);
        time = glfwGetTime();

    }

    mainRenderer.cleanup();
}
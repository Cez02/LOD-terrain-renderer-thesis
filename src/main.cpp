#include <bits/stdc++.h>
#include <src/tr_window.hpp>
#include <src/tr_renderer.hpp>
#include <src/tr_shadercompiler.hpp>
#include <src/tr_appconfig.hpp>

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

    Renderer mainRenderer;

    // Vulkan setup
    mainRenderer.initVulkan(&mainWindow);

    APP_CONFIG.print();

    // Scene setup
    log("Vulkan initialized. Initializaing scene data...");
    SceneData scene;
    scene.getHeightmaps().emplace_back("./heightmaps/N44E013.hgt");
    scene.getHeightmaps().emplace_back("./heightmaps/N44E014.hgt");
    scene.getHeightmaps().emplace_back("./heightmaps/N44E015.hgt");
    mainRenderer.initVKSceneElements(scene);

    scene.getCamera() = Camera();
    scene.getCamera().m_Position = glm::vec3(8359, 33461, 36210);
    scene.getCamera().m_Rotation = glm::quat(glm::vec3(0, 0, 0));


    // App logic
    log("Setup complete. Starting application loop.");
    glm::vec3 playerPosition = glm::vec3(8359, 33461, 36210);
    glm::vec3 playerRotation = glm::vec3(0.0f);

    float time = glfwGetTime();
    float deltaTime = 0;
    float playerSpeed = 1000.0;

    float deltaTimeSum = 0;
    float averageDeltaTime = 0;
    int frameCount = 0;

    while(!glfwWindowShouldClose(mainWindow.getGLFWWindow())){
        glfwPollEvents();

        /* Handle logic */

        log("Receiving input");

        Camera cam = mainRenderer.getCamera();

        if (glfwGetKey(mainWindow.getGLFWWindow(), GLFW_KEY_W) == GLFW_PRESS)
            playerPosition += cam.getForwardVector() * deltaTime * playerSpeed;
        if (glfwGetKey(mainWindow.getGLFWWindow(), GLFW_KEY_S) == GLFW_PRESS)
            playerPosition -= cam.getForwardVector()  * deltaTime * playerSpeed;

        if (glfwGetKey(mainWindow.getGLFWWindow(), GLFW_KEY_A) == GLFW_PRESS)
            playerPosition += cam.getRightVector()  * deltaTime * playerSpeed;
        if (glfwGetKey(mainWindow.getGLFWWindow(), GLFW_KEY_D) == GLFW_PRESS)
            playerPosition -= cam.getRightVector()  * deltaTime * playerSpeed;

        if (glfwGetKey(mainWindow.getGLFWWindow(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            playerPosition += cam.getUpVector()  * deltaTime * playerSpeed;
        if (glfwGetKey(mainWindow.getGLFWWindow(), GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
            playerPosition -= cam.getUpVector()  * deltaTime * playerSpeed;

        if (glfwGetKey(mainWindow.getGLFWWindow(), GLFW_KEY_1) == GLFW_PRESS)
            APP_CONFIG.m_LODLevel = 1;
        if (glfwGetKey(mainWindow.getGLFWWindow(), GLFW_KEY_2) == GLFW_PRESS)
            APP_CONFIG.m_LODLevel = 2;
        if (glfwGetKey(mainWindow.getGLFWWindow(), GLFW_KEY_3) == GLFW_PRESS)
            APP_CONFIG.m_LODLevel = 4;
        if (glfwGetKey(mainWindow.getGLFWWindow(), GLFW_KEY_4) == GLFW_PRESS)
            APP_CONFIG.m_LODLevel = 8;


//        log("Camera rotations...");

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
//        std::cout << "Player position: " << glm::to_string(playerPosition) << std::endl;
//        std::cout << "Player rotation: " << glm::to_string(playerRotation) << std::endl;
//        std::cout << "Player rotation: " << glm::to_string(glm::eulerAngles(mainRenderer.getCamera().m_Rotation)) << std::endl;
//        std::cout << "Dot prod: " << dotProd << std::endl;

        std::cout << std::endl;

        /* Draw */

        log("Drawing frame");
        mainRenderer.drawFrame(scene);


//        log("Calculating delta times");
        deltaTime = abs(glfwGetTime() - time);
        time = glfwGetTime();

        deltaTimeSum += deltaTime;
        frameCount++;
        if (frameCount > 10)
        {
            averageDeltaTime = deltaTimeSum / static_cast<float>(frameCount);
            deltaTimeSum = deltaTime;
            frameCount = 1;
        }

        std::string titleBase = "Terrain renderer (";
        titleBase += std::to_string(averageDeltaTime * 1000.0f);

        std::stringstream stream;
        stream << "Terrain renderer (" << std::fixed << std::setprecision(2) << (averageDeltaTime * 1000.0f) << " ms, " << (1.0f / averageDeltaTime) << " FPS)";
        std::string s = stream.str();

        glfwSetWindowTitle(mainWindow.getGLFWWindow(), s.c_str());
    }

    mainRenderer.cleanup(scene);
}
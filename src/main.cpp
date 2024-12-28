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


    // load all files in heightmaps directory
    for (auto dir : std::filesystem::directory_iterator("./heightmaps"))
    {
        if (!dir.is_directory())
            scene.getHeightmaps().emplace_back(dir.path());
    }


    mainRenderer.initVKSceneElements(scene);

    scene.getCamera() = Camera();
    scene.getCamera().m_Position = glm::vec3(8359, 33461, 36210);
    scene.getCamera().m_Rotation = glm::quat(glm::vec3(0, 0, 0));


    GUIHandler guiHandler;

    mainRenderer.m_GUIHandler = &guiHandler;



    // App logic
    log("Setup complete. Starting application loop.");

    float time = glfwGetTime();
    float deltaTime = 0;

    float deltaTimeSum = 0;
    float averageDeltaTime = 0;
    int frameCount = 0;

    while(!glfwWindowShouldClose(mainWindow.getGLFWWindow()))
        {
        glfwPollEvents();

        /* Handle logic */

        log("Receiving input");

        mainRenderer.m_Camera.HandleMovement(deltaTime, mainWindow.getGLFWWindow());


        // GUI Logic

        guiHandler.m_GUIData.m_DeltaTime = averageDeltaTime;


        /* Draw */

        log("Drawing frame");
        mainRenderer.drawFrame(scene);
        mainRenderer.Present();
        log("Done drawing.");


        RenderStatistics stats = mainRenderer.getRenderStatistics();
        guiHandler.m_GUIData.m_RenderedTriangleCount = stats.PrimitiveCount;



        std::cout << std::endl;

        deltaTime = abs(glfwGetTime() - time);
        time = glfwGetTime();

        deltaTimeSum += deltaTime;
        frameCount++;
        if (deltaTimeSum > 0.3f && frameCount > 3)
        {
            averageDeltaTime = deltaTimeSum / static_cast<float>(frameCount);
            deltaTimeSum = deltaTime;
            frameCount = 1;
        }
    }

    mainRenderer.waitAllFences();

    mainRenderer.cleanup(scene);
}
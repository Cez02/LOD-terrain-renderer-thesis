#include <bits/stdc++.h>
#include <src/tr_window.hpp>
#include <src/tr_renderer.hpp>
#include <src/tr_shadercompiler.hpp>
#include <src/tr_appconfig.hpp>

#include <src/tr_logger.hpp>

#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/ext.hpp>

#include "tr_texture.h"

using namespace std;

glm::uvec2 parseGeoCoordinates(std::string input)
{
    glm::vec2 coords;

    coords.x = ((input[3] == 'W' || input[3] == 'w') ? -1 : 1) * ((input[4] - '0')*100 + (input[5] - '0')*10 + (input[6] - '0'));
    coords.y = ((input[0] == 'S' || input[0] == 's') ? -1 : 1) * ((input[1] - '0')*10 + (input[2] - '0'));

    coords.y += 0;
    coords.y /= 180;
    coords.y *= glm::pi<float>();

    coords.x += 0;
    coords.x /= 180;
    coords.x *= glm::pi<float>();

    return coords;
}

/*
 * Dodać wysokość nad poziomem morza
 *
 *
 */

struct StartupOptions
{

};

void parseOpts(int argc, char **argv, StartupOptions &opts)
{
    for (int i = 0; i<argc; i++)
    {
        std::string opt = std::string(argv[i]);

        if (opt.compare("--starting-position") == 0)
        {

        }
    }
}

void setup(Renderer *mainRenderer, Window *mainWindow, GUIHandler *guiHandler, Texture *compassTexture, SceneData *scene)
{
    // Vulkan setup
    mainRenderer->initVulkan(mainWindow);

    APP_CONFIG.print();

    // Scene setup
    log("Vulkan initialized. Initializaing scene data...");


    // load all files in heightmaps directory
    for (auto dir : std::filesystem::directory_iterator("./heightmaps"))
    {
        if (!dir.is_directory())
            scene->getHeightmaps().emplace_back(dir.path());
    }

    log("Initializing vk scene elements.");

    mainRenderer->initVKSceneElements(*scene);

    log("Finished setting up Vulkan.");

    // scene.getCamera() = Camera();
    // scene.getCamera().m_Position = glm::vec3(8359, 33461, 36210);
    // scene.getCamera().m_Rotation = glm::quat(glm::vec3(0, 0, 0));

    mainRenderer->m_GUIHandler = guiHandler;

    Camera::instance = &mainRenderer->m_Camera;

    // load assets

    log("Loading compass image.");
    bool ret = LoadTextureFromFile(
            "assets/Compass.png",
            compassTexture,
            mainRenderer->m_Device,
            mainRenderer->m_PhysicalDevice,
            mainRenderer->m_CommandPool,
            mainRenderer->m_GraphicsQueue
    );

    if (!ret)
    {
        log("Failed to load the compass image asset.");
        exit(1);
    }

    guiHandler->m_GUIData.m_CompassTexture = compassTexture;


    // App logic
    log("Setup complete. Starting application loop.");

    mainRenderer->m_RendererReady = true;
}

int main(int argc, char **argv){

    log("Starting program");
    log("Launching window");

    StartupOptions opts;

    parseOpts(argc, argv, opts);

    Window mainWindow(1000, 850, "Terrain renderer (loading...)");

    log("Window initializaiton finished. Initializing vulkan...");

    Renderer mainRenderer;
    GUIHandler guiHandler;
    Texture my_texture;
    SceneData scene;

    std::thread worker_thread(setup, &mainRenderer, &mainWindow, &guiHandler, &my_texture, &scene);


    float time = glfwGetTime();
    float deltaTime = 0;

    float deltaTimeSum = 0;
    float averageDeltaTime = 0;
    int frameCount = 0;

    float dotCounter = glfwGetTime();
    int dotCount = 0;

    while(!glfwWindowShouldClose(mainWindow.getGLFWWindow()))
        {
        glfwPollEvents();

        /* Handle logic */

        // log("Receiving input");

        if (!mainRenderer.m_RendererReady)
        {
            if (glm::abs(dotCounter - glfwGetTime()) > 1.0f)
            {
                dotCounter = glfwGetTime();
                dotCount++;

                if (dotCount > 3)
                    dotCount = 0;

                std::string windowName = "Terrain renderer (loading";
                for (int i = 0; i < dotCount; i++)
                    windowName += ".";
                for (int i = dotCount; i < 3; i++)
                    windowName += " ";

                windowName += ")";

                glfwSetWindowTitle(mainWindow.getGLFWWindow(), windowName.c_str());
            }
            continue;
        }
        if (worker_thread.joinable())
        {
            worker_thread.join();
            glfwSetWindowTitle(mainWindow.getGLFWWindow(), "Terrain renderer");
        }

        mainRenderer.m_Camera.HandleMovement(deltaTime, mainWindow.getGLFWWindow());


        // GUI Logic

        guiHandler.m_GUIData.m_DeltaTime = averageDeltaTime;
        guiHandler.m_GUIData.m_PlayerPolarPosition = mainRenderer.m_Camera.getPolarPosition();

            // calculate the compass rotation
        guiHandler.m_GUIData.m_CompassRotation =
            glm::atan(mainRenderer.m_Camera.m_Position.z, mainRenderer.m_Camera.m_Position.x) +
            mainRenderer.m_Camera.playerRotation.y +
            glm::half_pi<float>();


        /* Draw */

        log("Drawing frame");
        mainRenderer.drawFrame(scene);
        mainRenderer.Present();
        log("Done drawing.");


        RenderStatistics stats = mainRenderer.getRenderStatistics();
        guiHandler.m_GUIData.m_RenderedTriangleCount = stats.PrimitiveCount;
        guiHandler.m_GUIData.m_MeshShaderInvocations = stats.MeshShaderInvocations;
        guiHandler.m_GUIData.m_TaskShaderInvocations = stats.TaskShaderInvocations;



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

    RemoveTexture(&my_texture, mainRenderer.m_Device);
    mainRenderer.cleanup(scene);
}
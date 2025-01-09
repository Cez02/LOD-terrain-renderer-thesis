#pragma once

#include "src/tr_window.hpp"
#include "src/tr_camera.hpp"
#include "src/tr_scenedata.hpp"

#include <string>
#include <vector>
#include <optional>
#include <functional>

#include "tr_guihandler.h"

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct UploadContext {
    VkFence _uploadFence;
    VkCommandPool _commandPool;
    VkCommandBuffer _commandBuffer;
};

struct RenderStatistics
{
    uint64_t PrimitiveCount;
    uint64_t TaskShaderInvocations;
    uint64_t MeshShaderInvocations;
};


class Renderer {
public:
    const int MAX_FRAMES_IN_FLIGHT = 2;

    static std::vector<char> readFile(const std::string &filepath);

    VkInstance m_Instance;
    VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
    VkDevice m_Device;

    VkDebugUtilsMessengerEXT m_DebugMessenger;

    VkQueue m_GraphicsQueue;
    VkQueue m_PresentationQueue;

    VkSurfaceKHR m_Surface;

    VkSwapchainKHR m_SwapChain;
    std::vector<VkImage> m_SwapChainImages;
    std::vector<VkImageView> m_SwapChainImageViews;
    VkFormat m_SwapChainImageFormat;
    VkExtent2D m_SwapChainExtent;

    std::vector<VkFramebuffer> m_SwapChainFramebuffers;

    VkRenderPass m_RenderPass;

    VkDescriptorSetLayout m_DescriptorSetLayout;
    VkDescriptorPool m_DescriptorPool;

    VkImage m_DepthImage;
    VkDeviceMemory m_DepthImageMemory;
    VkImageView m_DepthImageView;

    VkPipelineLayout m_PipelineLayout;
    VkPipeline m_GraphicsPipeline;

    VkQueryPool m_QueryPool;

    VkCommandPool m_CommandPool;
    std::vector<VkCommandBuffer> m_CommandBuffers;

    std::vector<VkSemaphore> m_ImageAvailableSemaphores;
    std::vector<VkSemaphore> m_RenderFinishedSemaphores;
    std::vector<VkFence> m_InFlightFences;

    UploadContext m_UploadContext;

    std::vector<VkBuffer> m_RenderStatisticsDataBuffer;
    std::vector<VkDeviceMemory> m_RenderStaticsDataMemory;
    std::vector<void*> m_RenderingStatisticsMappedData;

    RenderStatistics m_RenderStatistics;

    uint32_t m_CurrentFrame = 0;

    bool m_RendererReady = false;

    Camera m_Camera;

    Window *m_AppWindow;

    GUIHandler *m_GUIHandler;

    const std::vector<const char*> VALIDATION_LAYERS = {
            "VK_LAYER_KHRONOS_validation"
    };

    const std::vector<const char*> DEVICE_EXTENSIONS = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_EXT_MESH_SHADER_EXTENSION_NAME,
            VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME,
            VK_KHR_16BIT_STORAGE_EXTENSION_NAME,
            VK_EXT_TRANSFORM_FEEDBACK_EXTENSION_NAME,
            VK_EXT_PRIMITIVES_GENERATED_QUERY_EXTENSION_NAME
    };

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

    std::vector<const char*> getRequiredExtensions();
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData);
    void setupDebugMessenger();

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);

    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat findDepthFormat();

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    VkShaderModule createShaderModule(const std::vector<char>& code);

    VkCommandBufferBeginInfo command_buffer_begin_info(VkCommandBufferUsageFlags flags /*= 0*/);
    VkSubmitInfo submit_info(VkCommandBuffer* cmd);
    void immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function);

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, SceneData &scene);

    bool checkValidationLayerSupport();

    void createInstance();

    void pickPhysicalDevice();

    void createLogicalDevice();

    void createSurface();

    void createSwapChain();

    void createImageViews();

    void createRenderPass();

    void createDescriptorSetLayout();

    void createDescriptorPool();

    void createGraphicsPipeline();

    void createQueryPool();

    void createFramebuffers();

    void createCommandPool();

    void createDepthResources();

    void createCommandBuffers();

    void createSyncObjects();

    void createRenderStatisticsBuffer();

    void initImgui();

    Renderer();

    RenderStatistics getRenderStatistics();

    void initVulkan(Window *appWindow);

    void initVKSceneElements(SceneData &scene);

    void drawFrame(SceneData &scene);
    void Present();

    void waitAllFences();

    void cleanup(SceneData loadedScene);

    Camera& getCamera();
};
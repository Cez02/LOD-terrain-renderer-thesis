#include "tr_renderer.hpp"
#include <fstream>
#include <exception>

#include "src/tr_logger.hpp"
#include "src/tr_appconfig.hpp"
#include "src/tr_shadercompiler.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <vulkan/vk_enum_string_helper.h>

#include <stdexcept>
#include <cstring>
#include <string>
#include <set>
#include <cstdint>
#include <limits>
#include <algorithm>
#include <fstream>
#include <array>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <glm/gtx/string_cast.hpp>

#include "vk_ext_mesh_shader.h"

static void check_vk_result(VkResult err)
{
    if (err == 0)
        return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0)
        abort();
}

SwapChainSupportDetails Renderer::querySwapChainSupport(VkPhysicalDevice device)
{
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_Surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_Surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_Surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR Renderer::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR Renderer::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_IMMEDIATE_KHR;
}

VkExtent2D Renderer::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(m_AppWindow->getGLFWWindow(), &width, &height);

        VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

bool Renderer::checkValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : VALIDATION_LAYERS) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL Renderer::debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {

    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}


std::vector<const char*> Renderer::getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

void Renderer::setupDebugMessenger() {
    if(!enableValidationLayers)
        return;

    // to do
}

void Renderer::createInstance()
{
    if (enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    // App info

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    // Create info

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
        createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

//    createInfo.enabledLayerCount = 0;


    VkResult result = vkCreateInstance(&createInfo, nullptr, &m_Instance);

    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance! Error code: " + std::to_string(result));
    }
}

// implementation
VkCommandBufferBeginInfo Renderer::command_buffer_begin_info(VkCommandBufferUsageFlags flags /*= 0*/)
{
    VkCommandBufferBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.pNext = nullptr;

    info.pInheritanceInfo = nullptr;
    info.flags = flags;
    return info;
}

VkSubmitInfo Renderer::submit_info(VkCommandBuffer* cmd)
{
    VkSubmitInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    info.pNext = nullptr;

    info.waitSemaphoreCount = 0;
    info.pWaitSemaphores = nullptr;
    info.pWaitDstStageMask = nullptr;
    info.commandBufferCount = 1;
    info.pCommandBuffers = cmd;
    info.signalSemaphoreCount = 0;
    info.pSignalSemaphores = nullptr;

    return info;
}

void Renderer::immediate_submit(std::function<void(VkCommandBuffer cmd)>&& function)
{

	VkCommandBuffer cmd = m_UploadContext._commandBuffer;

	//begin the command buffer recording. We will use this command buffer exactly once before resetting, so we tell vulkan that
	VkCommandBufferBeginInfo cmdBeginInfo = command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

	check_vk_result(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

	//execute the function
	function(cmd);

	check_vk_result(vkEndCommandBuffer(cmd));

	VkSubmitInfo submit = submit_info(&cmd);


	//submit command buffer to the queue and execute it.
	// _uploadFence will now block until the graphic commands finish execution
	check_vk_result(vkQueueSubmit(m_GraphicsQueue, 1, &submit, m_UploadContext._uploadFence));

	vkWaitForFences(m_Device, 1, &m_UploadContext._uploadFence, true, 9999999999);
	vkResetFences(m_Device, 1, &m_UploadContext._uploadFence);

	// reset the command buffers inside the command pool
	vkResetCommandPool(m_Device, m_UploadContext._commandPool, 0);
}

QueueFamilyIndices Renderer::findQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_Surface, &presentSupport);

        if (presentSupport) {
            indices.presentFamily = i;
        }

        if(indices.isComplete()){
            break;
        }

        i++;
    }

    return indices;
}

bool Renderer::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(DEVICE_EXTENSIONS.begin(), DEVICE_EXTENSIONS.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

bool Renderer::isDeviceSuitable(VkPhysicalDevice device) {
    VkPhysicalDeviceProperties2 deviceProperties { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
    VkPhysicalDeviceMeshShaderPropertiesEXT meshShaderProperties { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT };
    VkPhysicalDeviceFeatures deviceFeatures;

    deviceProperties.pNext = &meshShaderProperties;

    vkGetPhysicalDeviceProperties2(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    // mesh shader props
    // APP_CONFIG.m_MeshShaderConfig.m_MaxPreferredMeshWorkGroupInvocations = meshShaderProperties.maxPreferredMeshWorkGroupInvocations;
    // APP_CONFIG.m_MeshShaderConfig.m_MaxPreferredTaskWorkGroupInvocations = meshShaderProperties.maxPreferredTaskWorkGroupInvocations;
    APP_CONFIG.m_MeshShaderConfig.m_MaxPreferredMeshWorkGroupInvocations = 32;
    APP_CONFIG.m_MeshShaderConfig.m_MaxPreferredTaskWorkGroupInvocations = 32;

    APP_CONFIG.m_MeshShaderConfig.m_MaxMeshOutputPrimitives = meshShaderProperties.maxMeshOutputPrimitives;
    APP_CONFIG.m_MeshShaderConfig.m_MaxMeshOutputVertices = meshShaderProperties.maxMeshOutputVertices;
    // APP_CONFIG.m_MeshShaderConfig.m_MaxMeshOutputPrimitives = 64;
    // APP_CONFIG.m_MeshShaderConfig.m_MaxMeshOutputVertices = 64;

    APP_CONFIG.m_MeshShaderConfig.m_MaxTaskWorkgroupCount = meshShaderProperties.maxTaskWorkGroupCount[0];

    APP_CONFIG.m_MeshletInfo.m_MeshletLength = 9; // Fixed length

    APP_CONFIG.m_MeshShaderConfig.m_MaxTaskWorkGroupInvocations = meshShaderProperties.maxTaskWorkGroupInvocations;

    memcpy(APP_CONFIG.m_MeshShaderConfig.maxMeshWorkGroupSize, meshShaderProperties.maxMeshWorkGroupSize, sizeof(uint32_t) * 3);
    memcpy(APP_CONFIG.m_MeshShaderConfig.maxTaskWorkGroupSize, meshShaderProperties.maxTaskWorkGroupSize, sizeof(uint32_t) * 3);

    APP_CONFIG.m_MeshShaderConfig.m_MaxTaskWorkgroupSizeTotal = meshShaderProperties.maxTaskWorkGroupTotalCount;

    log("Device: " + std::string(deviceProperties.properties.deviceName));
    log("Max preffered mesh workgroup invocations: " + std::to_string(meshShaderProperties.maxPreferredMeshWorkGroupInvocations));
    log("Max preffered task workgroup invocations: " + std::to_string(meshShaderProperties.maxPreferredTaskWorkGroupInvocations));
    log("Max vertices out: " + std::to_string(meshShaderProperties.maxMeshOutputVertices));
    log("Max primitives out: " + std::to_string(meshShaderProperties.maxMeshOutputPrimitives));
    log("Max payload size: " + std::to_string(meshShaderProperties.maxTaskPayloadSize));

    log("Supported API version: " + std::to_string(VK_API_VERSION_MAJOR(deviceProperties.properties.apiVersion)) + "." + std::to_string(VK_API_VERSION_MINOR(deviceProperties.properties.apiVersion)));

    // log("Max task workgroup size: " + std::to_string(APP_CONFIG.m_MeshShaderConfig.m_MaxTaskWorkgroupSizeTotal));
    // log("Max task workgroup size: " + std::to_string(APP_CONFIG.m_MeshShaderConfig.m_MaxTaskWorkgroupSizeTotal));

    // the rest

    QueueFamilyIndices indices = findQueueFamilies(device);

    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }



    return deviceProperties.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
           && indices.isComplete()
           && swapChainAdequate
           && extensionsSupported;
}

void Renderer::pickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(m_Instance, &deviceCount, devices.data());

    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            log("Picked this device!");
            m_PhysicalDevice = device;
            break;
        }
    }

    if (m_PhysicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }

}

void Renderer::createLogicalDevice()
{
    QueueFamilyIndices indices = findQueueFamilies(m_PhysicalDevice);

    // Set up queue family infos

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    // Device creations
    VkPhysicalDeviceFeatures2 deviceFeatures{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };


    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());;

    createInfo.pNext = &deviceFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(DEVICE_EXTENSIONS.size());
    createInfo.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data();

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
        createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
    } else
    {
        createInfo.enabledLayerCount = 0;
    }


    // enable latest vulkan features


    VkPhysicalDeviceVulkan11Features features11 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES };

    deviceFeatures.pNext = &features11;

    VkPhysicalDeviceVulkan12Features features12 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };

    features11.pNext = &features12;

    VkPhysicalDeviceVulkan13Features features13 = { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };

    features12.pNext = &features13;

    // Turn on mesh shader features
    VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeatures { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT };

    features13.pNext = &meshShaderFeatures;

    // Turn on primitives generated queries
    VkPhysicalDevicePrimitivesGeneratedQueryFeaturesEXT primitivesGeneratedQueryFeaturesEXT { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIMITIVES_GENERATED_QUERY_FEATURES_EXT };

    meshShaderFeatures.pNext = &primitivesGeneratedQueryFeaturesEXT;
    primitivesGeneratedQueryFeaturesEXT.pNext = VK_NULL_HANDLE;

    /*
    VkPhysicalDevice16BitStorageFeatures storageFeatures { VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_16BIT_STORAGE_FEATURES };

    storageFeatures.storageBuffer16BitAccess = true;

    meshShaderFeatures.pNext = &storageFeatures;
    storageFeatures.pNext = VK_NULL_HANDLE;
    */

    vkGetPhysicalDeviceFeatures2(m_PhysicalDevice, &deviceFeatures);


    // set options

    if(meshShaderFeatures.meshShader == VK_FALSE || meshShaderFeatures.taskShader == VK_FALSE){
        throw std::runtime_error("Mesh shaders are not supported on this physical device!");
    }


    log("\tCreating device");

    if (vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(m_Device, indices.graphicsFamily.value(), 0, &m_GraphicsQueue);
    vkGetDeviceQueue(m_Device, indices.presentFamily.value(), 0, &m_PresentationQueue);

    log("\tLoading vk_ext_mesh_shader");

    if(!load_VK_EXT_mesh_shader(m_Device, vkGetDeviceProcAddr))
        throw std::runtime_error("Failed to load ext_mesh_shader");

}

void Renderer::createSurface()
{
    if (glfwCreateWindowSurface(m_Instance, m_AppWindow->getGLFWWindow(), nullptr, &m_Surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
}

void Renderer::createSwapChain()
{
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(m_PhysicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_Surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(m_PhysicalDevice);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(m_Device, &createInfo, nullptr, &m_SwapChain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &imageCount, nullptr);
    m_SwapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_Device, m_SwapChain, &imageCount, m_SwapChainImages.data());

    m_SwapChainImageFormat = surfaceFormat.format;
    m_SwapChainExtent = extent;
}

void Renderer::createImageViews()
{
    m_SwapChainImageViews.resize(m_SwapChainImages.size());

    for (size_t i = 0; i < m_SwapChainImages.size(); i++) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = m_SwapChainImages[i];

        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = m_SwapChainImageFormat;

        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(m_Device, &createInfo, nullptr, &m_SwapChainImageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image views!");
        }
    }

}

uint32_t Renderer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

VkImageView Renderer::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;


    VkImageView imageView;
    if (vkCreateImageView(m_Device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }

    return imageView;
}

void Renderer::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(m_Device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_Device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(m_Device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(m_Device, image, imageMemory, 0);
}


void Renderer::createRenderPass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = m_SwapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = findDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(m_Device, &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }

}

std::vector<char> readFile(const std::string &filepath)
{

    std::ifstream inputStream{filepath, std::ios::ate | std::ios::binary};

    if(!inputStream.is_open()){
        throw std::invalid_argument("Could not open file at path " + filepath);
    }

    size_t fileSize = static_cast<size_t>(inputStream.tellg());

    std::vector<char> buffer(fileSize);

    inputStream.seekg(0);
    inputStream.read(buffer.data(), fileSize);

    inputStream.close();

    return buffer;
}

VkShaderModule Renderer::createShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(m_Device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;

}

void Renderer::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex, SceneData &scene)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }



    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_RenderPass;
    renderPassInfo.framebuffer = m_SwapChainFramebuffers[imageIndex];

    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_SwapChainExtent;

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.439, 0.788, 1.0, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};    renderPassInfo.clearValueCount = 1;

    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();


//    log("Begin render pass...");

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);


//    log("Setting up viewport stuff");

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_SwapChainExtent.width);
    viewport.height = static_cast<float>(m_SwapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_SwapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    // push constants
    glm::mat4 constants =
            m_Camera.getProjectionMatrix(m_AppWindow->getDimensions().x, m_AppWindow->getDimensions().y) *
            m_Camera.getViewMatrix();


//    log("Pushing transform matrix as constant");

    //upload the matrix to the GPU via push constants
    vkCmdPushConstants(commandBuffer, m_PipelineLayout, VK_SHADER_STAGE_MESH_BIT_EXT, 0, sizeof(glm::mat4), &constants);

//    log("Binding and drawing heightmaps...");
    // draw meshes
    log(glm::to_string(getCamera().m_Position));
    for(auto & heightmap : scene.getHeightmaps()) {
        heightmap.Bind(commandBuffer, m_PipelineLayout, m_CurrentFrame);
        heightmap.Draw(commandBuffer, m_PipelineLayout, getCamera().m_Position);
    }

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    m_GUIHandler->DrawGUI();



    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);


    vkCmdEndRenderPass(commandBuffer);

    // vkCmdEndQuery(m_CommandBuffers[m_CurrentFrame], m_QueryPool, 0);

//    log("Ending command buffer");
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void Renderer::createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding heightmapDataLayoutBinding{};
    heightmapDataLayoutBinding.binding = 0;
    heightmapDataLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    heightmapDataLayoutBinding.descriptorCount = 1;
    heightmapDataLayoutBinding.stageFlags = VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_FRAGMENT_BIT;
    heightmapDataLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding meshletDescriptionLayoutBinding{};
    meshletDescriptionLayoutBinding.binding = 1;
    meshletDescriptionLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    meshletDescriptionLayoutBinding.descriptorCount = 1;
    meshletDescriptionLayoutBinding.stageFlags = VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_FRAGMENT_BIT;
    meshletDescriptionLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding renderingStatisticsLayoutBinding{};
    renderingStatisticsLayoutBinding.binding = 2;
    renderingStatisticsLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    renderingStatisticsLayoutBinding.descriptorCount = 1;
    renderingStatisticsLayoutBinding.stageFlags = VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_FRAGMENT_BIT;
    renderingStatisticsLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding bindings[3] = {
            heightmapDataLayoutBinding,
            meshletDescriptionLayoutBinding,
            renderingStatisticsLayoutBinding
    };

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 3;
    layoutInfo.pBindings = bindings;

    if (vkCreateDescriptorSetLayout(m_Device, &layoutInfo, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void Renderer::createDescriptorPool() {
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSize.descriptorCount = 100 * (MAX_FRAMES_IN_FLIGHT) ;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = static_cast<uint32_t>(100 * MAX_FRAMES_IN_FLIGHT) ;

    if (vkCreateDescriptorPool(m_Device, &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void Renderer::createGraphicsPipeline()
{
    shaders::CompileShaders();

    auto taskShaderCode = readFile("shaders/simple_shader.task.spv");
    auto meshShaderCode = readFile("shaders/simple_shader.mesh.spv");
    auto fragShaderCode = readFile("shaders/simple_shader.frag.spv");

    VkShaderModule taskShaderModule = createShaderModule(taskShaderCode);
    VkShaderModule meshShaderModule = createShaderModule(meshShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    // stage creation

    VkPipelineShaderStageCreateInfo taskShaderStageInfo { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
    taskShaderStageInfo.stage = VK_SHADER_STAGE_TASK_BIT_EXT;
    taskShaderStageInfo.module = taskShaderModule;
    taskShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo meshShaderStageInfo { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
    meshShaderStageInfo.stage = VK_SHADER_STAGE_MESH_BIT_EXT;
    meshShaderStageInfo.module = meshShaderModule;
    meshShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";


    VkPipelineShaderStageCreateInfo shaderStages[] = {taskShaderStageInfo, meshShaderStageInfo, fragShaderStageInfo };

    // dynamic states

    std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    // vertex input stuff

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {}; // Optional
    depthStencil.back = {}; // Optional


    // ...

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) m_SwapChainExtent.width;
    viewport.height = (float) m_SwapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;


    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_SwapChainExtent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;


    // Rasterizer

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;

#ifdef DRAW_WIREFRAME
    rasterizer.polygonMode = VK_POLYGON_MODE_LINE;
#else
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
#endif
    rasterizer.lineWidth = 1.0f;

    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; // for now
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional


    // multisampling - Won't be used for now

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional


    // color blending

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional


    // layout

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    pipelineLayoutInfo.setLayoutCount = 1; // Optional
    pipelineLayoutInfo.pSetLayouts = &m_DescriptorSetLayout; // Optional

    VkPushConstantRange push_constant_transform_matrix;
    push_constant_transform_matrix.offset = 0;
    push_constant_transform_matrix.size = sizeof(glm::mat4);
    push_constant_transform_matrix.stageFlags = VK_SHADER_STAGE_MESH_BIT_EXT;

    VkPushConstantRange push_constant_heightmap_info;
    push_constant_heightmap_info.offset = sizeof(glm::mat4);
    push_constant_heightmap_info.size = sizeof(HeightmapPushConstantData);
    push_constant_heightmap_info.stageFlags = VK_SHADER_STAGE_TASK_BIT_EXT;

    VkPushConstantRange pushConstants[3] = {
            push_constant_heightmap_info,
            push_constant_transform_matrix
    };

    pipelineLayoutInfo.pushConstantRangeCount = 2; // Optional
    pipelineLayoutInfo.pPushConstantRanges = pushConstants; // Optional

    if (vkCreatePipelineLayout(m_Device, &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    }



    // graphics pipeline

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 3;
    pipelineInfo.pStages = shaderStages;

    // vertex input is not part of the mesh shader pipeline
    pipelineInfo.pVertexInputState = nullptr;
    pipelineInfo.pInputAssemblyState = nullptr;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil; // Optional
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;

    pipelineInfo.layout = m_PipelineLayout;

    pipelineInfo.renderPass = m_RenderPass;
    pipelineInfo.subpass = 0;

    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    if (vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_GraphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    // cleanup


    vkDestroyShaderModule(m_Device, taskShaderModule, nullptr);
    vkDestroyShaderModule(m_Device, meshShaderModule, nullptr);
    vkDestroyShaderModule(m_Device, fragShaderModule, nullptr);
}

void Renderer::createQueryPool()
{
    VkQueryPoolCreateInfo queryPoolInfo{};
    queryPoolInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    queryPoolInfo.queryType = VK_QUERY_TYPE_PRIMITIVES_GENERATED_EXT;
    queryPoolInfo.queryCount = 1; // Number of queries
    // queryPoolInfo.pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT;

    vkCreateQueryPool(m_Device, &queryPoolInfo, nullptr, &m_QueryPool);
}

void Renderer::createFramebuffers()
{
    m_SwapChainFramebuffers.resize(m_SwapChainImageViews.size());

    for (size_t i = 0; i < m_SwapChainImageViews.size(); i++) {
        std::array<VkImageView, 2> attachments = {
                m_SwapChainImageViews[i],
                m_DepthImageView
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_RenderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = m_SwapChainExtent.width;
        framebufferInfo.height = m_SwapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_Device, &framebufferInfo, nullptr, &m_SwapChainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void Renderer::createCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(m_PhysicalDevice);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(m_Device, &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
}

VkFormat Renderer::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, &props);

        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("failed to find supported format!");
}

VkFormat Renderer::findDepthFormat() {
    return findSupportedFormat(
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

static bool hasStencilComponent(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void Renderer::createDepthResources()
{
    VkFormat depthFormat = findDepthFormat();

    createImage(m_SwapChainExtent.width, m_SwapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_DepthImage, m_DepthImageMemory);
    m_DepthImageView = createImageView(m_DepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void Renderer::createCommandBuffers()
{
    m_CommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_CommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    if (vkAllocateCommandBuffers(m_Device, &allocInfo, m_CommandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void Renderer::createSyncObjects()
{
    m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(m_Device, &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(m_Device, &fenceInfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS )  {
            throw std::runtime_error("failed to create semaphores!");
        }
    }

    fenceInfo.flags = 0;

    if (vkCreateFence(m_Device, &fenceInfo, nullptr, &(m_UploadContext._uploadFence)) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create semaphores!");
    }
}

void Renderer::createRenderStatisticsBuffer()
{
    m_RenderingStatisticsMappedData.resize(MAX_FRAMES_IN_FLIGHT);
    m_RenderStaticsDataMemory.resize(MAX_FRAMES_IN_FLIGHT);
    m_RenderStatisticsDataBuffer.resize(MAX_FRAMES_IN_FLIGHT);

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkDeviceSize size = sizeof(RenderStatistics);
        VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

        VkBufferCreateInfo bufferInfo { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
        bufferInfo.size = size;
        bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if(vkCreateBuffer(m_Device, &bufferInfo, nullptr, &(m_RenderStatisticsDataBuffer[i])) != VK_SUCCESS){
            throw std::runtime_error("Failed to create render statistics buffer!!!");
        }

        VkMemoryRequirements memReqs;
        vkGetBufferMemoryRequirements(m_Device, m_RenderStatisticsDataBuffer[i], &memReqs);

        VkMemoryAllocateInfo allocInfo { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
        allocInfo.allocationSize = memReqs.size;
        allocInfo.memoryTypeIndex = findMemoryType(memReqs.memoryTypeBits, properties);

        if (vkAllocateMemory(m_Device, &allocInfo, nullptr, &(m_RenderStaticsDataMemory[i])) != VK_SUCCESS){
            throw std::runtime_error("Failed to allocate memory for the buffer!!!");
        }

        vkBindBufferMemory(m_Device, m_RenderStatisticsDataBuffer[i], m_RenderStaticsDataMemory[i], 0);

        vkMapMemory(m_Device, m_RenderStaticsDataMemory[i], 0, sizeof(RenderStatistics), 0, &(m_RenderingStatisticsMappedData[i]));
    }
}

void Renderer::initImgui()
{
    ImGui::CreateContext();
    ImGui::GetIO().ConfigFlags = ImGuiConfigFlags_NavEnableKeyboard;

    ImGui_ImplGlfw_InitForVulkan(m_AppWindow->getGLFWWindow(), true);

    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.Instance = m_Instance;
    initInfo.DescriptorPool = m_DescriptorPool;
    initInfo.RenderPass = m_RenderPass;
    initInfo.Device = m_Device;
    initInfo.PhysicalDevice = m_PhysicalDevice;
    initInfo.ImageCount = MAX_FRAMES_IN_FLIGHT;
    initInfo.Queue = m_GraphicsQueue;
    initInfo.MinImageCount = MAX_FRAMES_IN_FLIGHT;
    initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    ImGui_ImplVulkan_Init(&initInfo);

    ImGui_ImplVulkan_CreateFontsTexture();
}

void Renderer::initVulkan(Window *appWindow)
{
    m_AppWindow = appWindow;

    log("Creating instance");
    createInstance();

    log("Setup debug messenger");
    setupDebugMessenger();

    log("Create surface");
    createSurface();

    log("Picking physical device");
    pickPhysicalDevice();

    log("Creating logical device");
    createLogicalDevice();

    log("Creating swapchain");
    createSwapChain();

    log("Creating image view");
    createImageViews();

    log("Creating render pass");
    createRenderPass();

    log("Creating descriptor set layout");
    createDescriptorSetLayout();

    log("Creating descriptor pool");
    createDescriptorPool();

    log("Creating graphics pipeline");
    createGraphicsPipeline();

    log("Creating query pool");
    createQueryPool();

    log("Creating command pool");
    createCommandPool();

    log("Creating depth resources");
    createDepthResources();

    log("Creating framebuffers");
    createFramebuffers();

    log("Creating command buffers");
    createCommandBuffers();

    log("Creating sync objects");
    createSyncObjects();

    log("Creating render statistics data buffer");
    createRenderStatisticsBuffer();

    log("Setup of test meshlet");
    // setting up test meshlet





    log("Setting up memory transfer stuff");

    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(m_PhysicalDevice);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(m_Device, &poolInfo, nullptr, &(m_UploadContext._commandPool)) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }


    log("Initializing ImGui");
    initImgui();
}

uint32_t imageIndex;

void Renderer::drawFrame(SceneData &scene)
{
    vkWaitForFences(m_Device, 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);


    vkResetFences(m_Device, 1, &m_InFlightFences[m_CurrentFrame]);

    m_RenderStatistics = *(static_cast<RenderStatistics*>(m_RenderingStatisticsMappedData[m_CurrentFrame]));
    memset(m_RenderingStatisticsMappedData[m_CurrentFrame], 0, sizeof(RenderStatistics));

    vkAcquireNextImageKHR(m_Device, m_SwapChain, UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);


    recordCommandBuffer(m_CommandBuffers[m_CurrentFrame], imageIndex, scene);


    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {m_ImageAvailableSemaphores[m_CurrentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_CommandBuffers[m_CurrentFrame];


    VkSemaphore signalSemaphores[] = {m_RenderFinishedSemaphores[m_CurrentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(m_GraphicsQueue, 1, &submitInfo, m_InFlightFences[m_CurrentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }


}

void Renderer::Present()
{

    VkSemaphore waitSemaphores[] = {m_ImageAvailableSemaphores[m_CurrentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    VkSemaphore signalSemaphores[] = {m_RenderFinishedSemaphores[m_CurrentFrame]};

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {m_SwapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(m_PresentationQueue, &presentInfo);

    m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::waitAllFences()
{
    for (int i = 0; i<MAX_FRAMES_IN_FLIGHT; i++)
    {
	    vkWaitForFences(m_Device, 1, &(m_InFlightFences[i]), true, 9999999999);
    }
}

void Renderer::cleanup(SceneData loadedScene)
{
    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplVulkan_Shutdown();
    ImGui::DestroyContext();

    vkDeviceWaitIdle(m_Device);

    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(m_Device, m_ImageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(m_Device, m_RenderFinishedSemaphores[i], nullptr);
        vkDestroyFence(m_Device, m_InFlightFences[i], nullptr);

        vkDestroyBuffer(m_Device, m_RenderStatisticsDataBuffer[i], nullptr);
        vkFreeMemory(m_Device, m_RenderStaticsDataMemory[i], nullptr);

    }

    vkDestroyFence(m_Device, m_UploadContext._uploadFence, nullptr);

    vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
    vkDestroyCommandPool(m_Device, (m_UploadContext._commandPool), nullptr);

    for (auto framebuffer : m_SwapChainFramebuffers) {
        vkDestroyFramebuffer(m_Device, framebuffer, nullptr);
    }

    vkDestroyDescriptorPool(m_Device, m_DescriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(m_Device, m_DescriptorSetLayout, nullptr);

    // cleanup scene

    for(auto &heightmap : loadedScene.getHeightmaps()){
        heightmap.Deinit();
    }


    vkDestroyQueryPool(m_Device, m_QueryPool, VK_NULL_HANDLE);

    vkDestroyPipeline(m_Device, m_GraphicsPipeline, nullptr);
    vkDestroyPipelineLayout(m_Device, m_PipelineLayout, nullptr);
    vkDestroyRenderPass(m_Device, m_RenderPass, nullptr);

    vkDestroyImageView(m_Device, m_DepthImageView, nullptr);
    vkDestroyImage(m_Device, m_DepthImage, nullptr);
    vkFreeMemory(m_Device, m_DepthImageMemory, nullptr);

    for (auto imageView : m_SwapChainImageViews) {
        vkDestroyImageView(m_Device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(m_Device, m_SwapChain, nullptr);

    vkDestroyDevice(m_Device, nullptr);
    vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
    vkDestroyInstance(m_Instance, nullptr);
}

std::vector<char> Renderer::readFile(const std::string &filepath)
{

    std::ifstream inputStream{filepath, std::ios::ate | std::ios::binary};

    if(!inputStream.is_open()){
        throw std::invalid_argument("Could not open file at path " + filepath);
    }

    size_t fileSize = static_cast<size_t>(inputStream.tellg());

    std::vector<char> buffer(fileSize);

    inputStream.seekg(0);
    inputStream.read(buffer.data(), fileSize);

    inputStream.close();

    return buffer;
}

Renderer::Renderer()
{

}

RenderStatistics Renderer::getRenderStatistics()
{
    return m_RenderStatistics;
}

Camera &Renderer::getCamera() {
    return m_Camera;
}

void Renderer::initVKSceneElements(SceneData &scene) {
    int heightmapIndex = 0;
    std::vector<VkDescriptorSetLayout> layouts(1, m_DescriptorSetLayout);

    for(auto &heightmap : scene.getHeightmaps()){
        heightmap.Init(m_Device, m_PhysicalDevice);
        heightmapIndex++;

        heightmap.m_DescriptorSets.m_DescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

        // setup descriptor sets for each heightmap
        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            // allocate set for heightmap data
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = m_DescriptorPool;
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = layouts.data();

            log("Initializing heightmap buffer descriptor set");

            VkResult returnCode = VK_SUCCESS;
            if ((returnCode = vkAllocateDescriptorSets(m_Device, &allocInfo, &heightmap.m_DescriptorSets.m_DescriptorSets[i])) != VK_SUCCESS) {
                throw std::runtime_error("failed to allocate descriptor sets! " + std::string(string_VkResult(returnCode)));
            }

            // bind heightmap buffer to descriptor set
            VkDescriptorBufferInfo heightmapBufferInfo{};
            heightmapBufferInfo.buffer = heightmap.m_HeightmapDataBuffer;
            heightmapBufferInfo.offset = 0;
            heightmapBufferInfo.range = heightmap.m_DataOrganizedForMeshlets.size() * sizeof(heightmap.m_DataOrganizedForMeshlets[0]);

            VkWriteDescriptorSet descriptorHeightmapWrite{};
            descriptorHeightmapWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorHeightmapWrite.dstSet = heightmap.m_DescriptorSets.m_DescriptorSets[i];
            descriptorHeightmapWrite.dstBinding = 0;
            descriptorHeightmapWrite.dstArrayElement = 0;

            descriptorHeightmapWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            descriptorHeightmapWrite.descriptorCount = 1;

            descriptorHeightmapWrite.pBufferInfo = &heightmapBufferInfo;
            descriptorHeightmapWrite.pImageInfo = nullptr; // Optional
            descriptorHeightmapWrite.pTexelBufferView = nullptr; // Optional


            // bind meshlet descriptions buffer to descriptor set
            VkDescriptorBufferInfo meshletDescriptionBufferInfo{};
            meshletDescriptionBufferInfo.buffer = heightmap.m_MeshletDescriptionBuffer;
            meshletDescriptionBufferInfo.offset = 0;
            meshletDescriptionBufferInfo.range = heightmap.m_Meshlets.size() *
#ifdef ENCODED_MESHLETS
            sizeof(EncodedMeshletDescription);
#else
            sizeof(MeshletDescription);
#endif

            VkWriteDescriptorSet descriptorMeshletWrite{};
            descriptorMeshletWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorMeshletWrite.dstSet = heightmap.m_DescriptorSets.m_DescriptorSets[i];
            descriptorMeshletWrite.dstBinding = 1;
            descriptorMeshletWrite.dstArrayElement = 0;

            descriptorMeshletWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            descriptorMeshletWrite.descriptorCount = 1;

            descriptorMeshletWrite.pBufferInfo = &meshletDescriptionBufferInfo;
            descriptorMeshletWrite.pImageInfo = nullptr; // Optional
            descriptorMeshletWrite.pTexelBufferView = nullptr; // Optional




            // bind rendering statistics
            VkDescriptorBufferInfo renderingStatisticsBufferInfo{};
            renderingStatisticsBufferInfo.buffer = m_RenderStatisticsDataBuffer[i];
            renderingStatisticsBufferInfo.offset = 0;
            renderingStatisticsBufferInfo.range = sizeof(RenderStatistics);

            VkWriteDescriptorSet descriptorRenderingStatisticsWrite{};
            descriptorRenderingStatisticsWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorRenderingStatisticsWrite.dstSet = heightmap.m_DescriptorSets.m_DescriptorSets[i];
            descriptorRenderingStatisticsWrite.dstBinding = 2;
            descriptorRenderingStatisticsWrite.dstArrayElement = 0;

            descriptorRenderingStatisticsWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            descriptorRenderingStatisticsWrite.descriptorCount = 1;

            descriptorRenderingStatisticsWrite.pBufferInfo = &renderingStatisticsBufferInfo;
            descriptorRenderingStatisticsWrite.pImageInfo = nullptr; // Optional
            descriptorRenderingStatisticsWrite.pTexelBufferView = nullptr; // Optional


            VkWriteDescriptorSet writes[3] = {
                    descriptorHeightmapWrite,
                    descriptorMeshletWrite,
                    descriptorRenderingStatisticsWrite
            };

            vkUpdateDescriptorSets(m_Device, 3, writes, 0, nullptr);
        }
    }
}


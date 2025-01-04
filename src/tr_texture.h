#pragma once

#include <vulkan/vulkan.h>
#define IMGUI_DEFINE_MATH_OPERATORS

#include <imgui.h>
#include <imgui_impl_vulkan.h>


class Texture {
public:
    VkDescriptorSet DS;         // Descriptor set: this is what you'll pass to Image()
    int             Width;
    int             Height;
    int             Channels;

    // Need to keep track of these to properly cleanup
    VkImageView     ImageView;
    VkImage         Image;
    VkDeviceMemory  ImageMemory;
    VkSampler       Sampler;
    VkBuffer        UploadBuffer;
    VkDeviceMemory  UploadBufferMemory;
};

uint32_t findMemoryType(uint32_t type_filter, VkPhysicalDevice physicalDevice, VkMemoryPropertyFlags properties);

bool LoadTextureFromFile(const char* filename, Texture* tex_data, VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool vulkanWindow, VkQueue queue);

void RemoveTexture(Texture* tex_data, VkDevice device);


#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

#include "tr_descriptorbundle.hpp"

class Heightmap {
private:

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice physicalDevice);

public:
    float *m_Data;
    uint m_Resolution;

    VkDevice m_Device;
    VkBuffer m_MeshletBuffer;
    VkDeviceMemory m_MeshletBufferMemory;

    DescriptorSetBundle m_DescriptorSets;

    size_t getHeightmapDataSize();

    void Init(int resolution, VkDevice device, VkPhysicalDevice physicalDevice);
    void Deinit();

    VkDescriptorSet &getDescriptorSet(uint index);

    void Bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, int frame);
    void Draw(VkCommandBuffer commandBuffer);
};
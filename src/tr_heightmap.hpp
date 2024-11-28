#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

#include "tr_descriptorbundle.hpp"

struct HeightmapMeshletDescription {
    glm::uint m_Start, m_Length;
};

struct HeightmapPushConstantData {
    uint HeightmapLength;
    uint BaseMeshletOffset;
    float Longitude;
    float Latitude;
};

class Heightmap {
private:

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice physicalDevice);

public:
    float *m_Data;
    size_t m_HeightmapLength;
    size_t m_DataSize;
    std::vector<HeightmapMeshletDescription> m_Meshlets;

    float m_Longitude, m_Latitude;

    VkDevice m_Device;

    VkBuffer m_HeightmapDataBuffer;
    VkDeviceMemory m_HeightmapDataBufferMemory;

    VkBuffer m_MeshletDescriptionBuffer;
    VkDeviceMemory m_MeshletDescriptionBufferMemory;

    DescriptorSetBundle m_DescriptorSets;



    void Init(std::string dataPath, VkDevice device, VkPhysicalDevice physicalDevice);
    void Deinit();

    void Bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, int frame);
    void Draw(VkCommandBuffer commandBuffer);
};
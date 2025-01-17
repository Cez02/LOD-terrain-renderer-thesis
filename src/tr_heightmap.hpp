#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>

#include "tr_descriptorbundle.hpp"
#include "shaders/common.h"

struct HeightmapPushConstantData {
    glm::vec3 ObservatorPosition;
    int padding;
    glm::vec3 ObserverLookingDirection;
    uint16_t MeshletCount;
    uint16_t HeightmapLength;
    uint16_t BaseMeshletOffset;
    float Longitude;
    float Latitude;
    uint8_t LODLevel;
};

class Heightmap {
private:

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice physicalDevice);

public:
    int16_t *m_Data;
    std::vector<int16_t> m_DataOrganizedForMeshlets;
    size_t m_HeightmapLength;
    size_t m_DataSize;
#ifdef ENCODED_MESHLETS
    std::vector<EncodedMeshletDescription> m_Meshlets;
#else
    std::vector<MeshletDescription> m_Meshlets;
#endif


    std::string m_DataPath;
    float m_Longitude, m_Latitude;

    VkDevice m_Device;

    VkBuffer m_HeightmapDataBuffer;
    VkDeviceMemory m_HeightmapDataBufferMemory;

    VkBuffer m_MeshletDescriptionBuffer;
    VkDeviceMemory m_MeshletDescriptionBufferMemory;

    DescriptorSetBundle m_DescriptorSets;

    bool CheckIfShouldDraw();

    Heightmap(std::string dataPath);

    void Init(VkDevice device, VkPhysicalDevice physicalDevice);
    void Deinit();

    void Bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, int frame);
    void Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, glm::vec3 observatorPosition);
};
#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "src/vk_ext_mesh_shader.h"

#ifndef MESHLET_MAX_INDEXED_PRIMITIVES_NUM
#define MESHLET_MAX_INDEXED_PRIMITIVES_NUM 128
#endif

#ifndef MESHLET_MAX_VERTICES_NUM
#define MESHLET_MAX_VERTICES_NUM 64
#endif

struct Meshlet {
    uint numPrimitives;
    uint numVertices;
    alignas(16) glm::uvec4 indexedPrimitives[MESHLET_MAX_INDEXED_PRIMITIVES_NUM];
    alignas(16) glm::vec4 vertices[MESHLET_MAX_VERTICES_NUM];
};

class Mesh {
public:
    std::vector<glm::vec3> Vertices;
    std::vector<glm::ivec3> IndexedPrimitives;
};

class Mesh_vk {

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice physicalDevice);

public:
    Meshlet m_Meshlets[1];

    VkBuffer m_MeshletBuffer;
    VkDeviceMemory m_MeshletBufferMemory;

    void Init(VkDevice device, VkPhysicalDevice physicalDevice);
    void Deinit(VkDevice device);

    void Bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkDescriptorSet &descriptorSet);
    void Draw(VkCommandBuffer commandBuffer);
};
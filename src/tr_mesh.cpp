#include "tr_mesh.hpp"
#include <glm/glm.hpp>

#include <stdexcept>
#include <cstring>

void Mesh_vk::Init(VkDevice device, VkPhysicalDevice physicalDevice) {
    memset(&(m_Meshlets[0]), 0, sizeof(m_Meshlets[0]));

    m_Meshlets[0].vertices[0] = glm::vec4(-0.5, 0, 0, 0);
    m_Meshlets[0].vertices[1] = glm::vec4(0, 1, 0, 0);
    m_Meshlets[0].vertices[2] = glm::vec4(0.5, 0, 0, 0);

    m_Meshlets[0].numVertices = 3;
    m_Meshlets[0].numPrimitives = 1;
    m_Meshlets[0].indexedPrimitives[0] = glm::uvec4(0, 2, 1, 0);

    // test data - to fill
    VkDeviceSize size = sizeof(m_Meshlets[0]);
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    // Buffer creation and allocation

    VkBufferCreateInfo bufferInfo { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufferInfo.size = size;
    bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if(vkCreateBuffer(device, &bufferInfo, nullptr, &m_MeshletBuffer) != VK_SUCCESS){
        throw std::runtime_error("Failed to create test buffer!!!");
    }

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(device, m_MeshletBuffer, &memReqs);

    VkMemoryAllocateInfo allocInfo { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = findMemoryType(memReqs.memoryTypeBits, properties, physicalDevice);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &m_MeshletBufferMemory) != VK_SUCCESS){
        throw std::runtime_error("Failed to allocate memory for the buffer!!!");
    }

    vkBindBufferMemory(device, m_MeshletBuffer, m_MeshletBufferMemory, 0);


    // We have the buffer now :)) Let's map it
    void *data;
    vkMapMemory(device, m_MeshletBufferMemory, 0, size, 0, &data);
    memcpy(data, &m_Meshlets[0], sizeof(m_Meshlets[0]));
    vkUnmapMemory(device, m_MeshletBufferMemory);
}

uint32_t Mesh_vk::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice physicalDevice) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void Mesh_vk::Bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkDescriptorSet &descriptorSet) {
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);
}

void Mesh_vk::Draw(VkCommandBuffer commandBuffer) {
    vkCmdDrawMeshTasksEXT(commandBuffer, 1, 1, 1);
}

void Mesh_vk::Deinit(VkDevice device) {
    vkFreeMemory(device, m_MeshletBufferMemory, nullptr);

    vkDestroyBuffer(device, m_MeshletBuffer, nullptr);
}
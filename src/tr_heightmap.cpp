//
// Created by cezary on 13.11.24.
//

#include "tr_heightmap.hpp"

void Heightmap::Init(int resolution, VkDevice device, VkPhysicalDevice physicalDevice) {
    m_Data = new float[resolution * resolution];

    for(int y = 0; y<resolution; y++){
        for(int x = 0; x<resolution; x++){
            float xF = (float)x - 3.5f;
            float yF = (float)y - 3.5f;
            float dis = glm::sqrt(xF*xF + yF*yF);

            m_Data[y * resolution + x] = -dis / 16.0f;
        }
    }

    m_Resolution = resolution;

    size_t dataSize = sizeof(float) * resolution * resolution;

    // test data - to fill
    VkDeviceSize size = dataSize;
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    // Buffer creation and allocation

    m_Device = device;

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
    memcpy(data, m_Data, dataSize);
    vkUnmapMemory(device, m_MeshletBufferMemory);

    // descriptor sets stuff


}

uint32_t Heightmap::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice physicalDevice) {
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

void Heightmap::Bind(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, int frame) {
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &(m_DescriptorSets.m_DescriptorSets[frame]), 0, nullptr);
}

void Heightmap::Draw(VkCommandBuffer commandBuffer) {
    vkCmdDrawMeshTasksEXT(commandBuffer, 1, 1, 1);
}

void Heightmap::Deinit() {
    delete [] m_Data;

    vkDestroyBuffer(m_Device, m_MeshletBuffer, nullptr);
    vkFreeMemory(m_Device, m_MeshletBufferMemory, nullptr);
}

VkDescriptorSet &Heightmap::getDescriptorSet(uint index) {
    return m_DescriptorSets.m_DescriptorSets[index];
}

size_t Heightmap::getHeightmapDataSize() {
    return m_Resolution * m_Resolution * sizeof(float);
}

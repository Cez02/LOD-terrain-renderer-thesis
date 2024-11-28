//
// Created by cezary on 13.11.24.
//

#include "tr_heightmap.hpp"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <fstream>
#include <filesystem>

#include "tr_logger.hpp"

static
void LoadTerrain(std::string path, float **dst, size_t *size, float *latitude, float *longitude) {
    std::ifstream inputfile(path, std::ios::binary);

    std::string hgtName = std::filesystem::path(path).stem();

    *latitude = ((hgtName[0] == 'S' || hgtName[0] == 's') ? -1 : 1) * ((hgtName[1] - '0')*10 + (hgtName[2] - '0'));
    *longitude = ((hgtName[3] == 'W' || hgtName[3] == 'w') ? -1 : 1) * ((hgtName[4] - '0')*100 + (hgtName[5] - '0')*10 + (hgtName[6] - '0'));

    *latitude += 0;
    *latitude /= 180;
    *latitude *= glm::pi<float>();

    *longitude += 0;
    *longitude /= 180;
    *longitude *= glm::pi<float>();

    std::vector<char> bytes(
            (std::istreambuf_iterator<char>(inputfile)),
            (std::istreambuf_iterator<char>()));

    *dst = new float[bytes.size() / 2];
    *size = bytes.size();

    int i = 0;
    while(i < bytes.size()) {
        uint8_t x = bytes[i],
                y = bytes[i + 1];

        float val = ((int16_t) ((((uint16_t) x << 8)) | (((uint16_t) y))));

        if (val < -10)
            val = -10;

        (*dst)[i / 2] = val;

        i += 2;
    }

    inputfile.close();
}

void Heightmap::Init(std::string dataPath, VkDevice device, VkPhysicalDevice physicalDevice) {

    // load data

    LoadTerrain(dataPath, &m_Data, &m_DataSize, &m_Latitude, &m_Longitude);

    std::cout << "Loaded terrain of buffer size: " << m_DataSize << std::endl;

    // build meshlets

    uint32_t  lengthOfHeightmap = m_DataSize / sizeof(uint16_t);
    lengthOfHeightmap = sqrt(lengthOfHeightmap);
    m_HeightmapLength = lengthOfHeightmap;

    int maxMeshletSize = 32;

    // this is basic we only generate meshlets describing singular points that generate from themselves 2 triangles.

//    int meshletMaxRange = (lengthOfHeightmap - 1)*(lengthOfHeightmap - 1);

    int sizePerLine = 0;

    for(int y = 0; y<lengthOfHeightmap - 1; y++) {
        int dataPointer = 0;
        sizePerLine = 0;

        for (int x = 0; x < lengthOfHeightmap - 1;) {

            // building one meshlet
            HeightmapMeshletDescription description;
            memset(&description, 0, sizeof(HeightmapMeshletDescription));


            description.m_Start = y * lengthOfHeightmap + x;

            if (x + maxMeshletSize > lengthOfHeightmap - 1) {
                description.m_Length = lengthOfHeightmap - 1 - x;
            } else {
                description.m_Length = maxMeshletSize;
            }
//
            if(y == 0)
                std::cout << "New meshlet " << sizePerLine << ", start: " << description.m_Start << ", length: " << description.m_Length << std::endl;

            x += description.m_Length;

            sizePerLine ++;
            m_Meshlets.push_back(description);
            m_Meshlets.emplace_back(); // padding
        }

//        log("Size per line: " + std::to_string(sizePerLine));
    }



    // Heightmap data buffer creation and allocation

    m_Device = device;
    VkDeviceSize size = m_DataSize;
    VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    VkBufferCreateInfo bufferInfo { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufferInfo.size = size;
    bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if(vkCreateBuffer(device, &bufferInfo, nullptr, &m_HeightmapDataBuffer) != VK_SUCCESS){
        throw std::runtime_error("Failed to create test buffer!!!");
    }

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(device, m_HeightmapDataBuffer, &memReqs);

    VkMemoryAllocateInfo allocInfo { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    allocInfo.allocationSize = memReqs.size;
    allocInfo.memoryTypeIndex = findMemoryType(memReqs.memoryTypeBits, properties, physicalDevice);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &m_HeightmapDataBufferMemory) != VK_SUCCESS){
        throw std::runtime_error("Failed to allocate memory for the buffer!!!");
    }

    vkBindBufferMemory(device, m_HeightmapDataBuffer, m_HeightmapDataBufferMemory, 0);

    void *data;
    vkMapMemory(device, m_HeightmapDataBufferMemory, 0, size, 0, &data);
    memcpy(data, m_Data, m_DataSize);
    vkUnmapMemory(device, m_HeightmapDataBufferMemory);



    // Meshlet description buffer creation and allocation

    m_Device = device;
    size = m_Meshlets.size() * sizeof(HeightmapMeshletDescription);

    VkBufferCreateInfo meshletDescriptionBufferInfo { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    meshletDescriptionBufferInfo.size = size;
    meshletDescriptionBufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    meshletDescriptionBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if(vkCreateBuffer(device, &meshletDescriptionBufferInfo, nullptr, &m_MeshletDescriptionBuffer) != VK_SUCCESS){
        throw std::runtime_error("Failed to create test buffer!!!");
    }

    VkMemoryRequirements meshletDescriptionMemReqs;
    vkGetBufferMemoryRequirements(device, m_MeshletDescriptionBuffer, &meshletDescriptionMemReqs);

    VkMemoryAllocateInfo meshletDescriptionAllocInfo { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    meshletDescriptionAllocInfo.allocationSize = meshletDescriptionMemReqs.size;
    meshletDescriptionAllocInfo.memoryTypeIndex = findMemoryType(meshletDescriptionMemReqs.memoryTypeBits, properties, physicalDevice);

    if (vkAllocateMemory(device, &meshletDescriptionAllocInfo, nullptr, &m_MeshletDescriptionBufferMemory) != VK_SUCCESS){
        throw std::runtime_error("Failed to allocate memory for the buffer!!!");
    }

    vkBindBufferMemory(device, m_MeshletDescriptionBuffer, m_MeshletDescriptionBufferMemory, 0);

    data = nullptr;
    vkMapMemory(device, m_MeshletDescriptionBufferMemory, 0, size, 0, &data);
    memcpy(data, m_Meshlets.data(), m_Meshlets.size() * sizeof(HeightmapMeshletDescription));
    vkUnmapMemory(device, m_MeshletDescriptionBufferMemory);
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

    int maxTasksEmitted = 128;

//    log("\tBind descriptor sets");

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &(m_DescriptorSets.m_DescriptorSets[frame]), 0, nullptr);

//    log("\tBuffer buffer info constants");

    HeightmapPushConstantData data;
    data.HeightmapLength = m_HeightmapLength;
    data.Latitude = m_Latitude;
    data.Longitude = m_Longitude;
    data.BaseMeshletOffset = 0;

    for(int i = 0; i<m_Meshlets.size();) {
        data.BaseMeshletOffset = i;

        vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_TASK_BIT_EXT, sizeof(glm::mat4), sizeof(HeightmapPushConstantData), &data);

//        log("\tDraw call");

        vkCmdDrawMeshTasksEXT(commandBuffer, maxTasksEmitted, 1, 1);

        i += maxTasksEmitted;
    }
}

void Heightmap::Draw(VkCommandBuffer commandBuffer) {

    int factor = m_Meshlets.size() / 5;
//    for(int i = 0; i<5; i++) {
//        vkCmdDrawMeshTasksEXT(commandBuffer, factor, 1, 1);
//    }
}

void Heightmap::Deinit() {
    delete [] m_Data;

    vkDestroyBuffer(m_Device, m_HeightmapDataBuffer, nullptr);
    vkFreeMemory(m_Device, m_HeightmapDataBufferMemory, nullptr);

    vkDestroyBuffer(m_Device, m_MeshletDescriptionBuffer, nullptr);
    vkFreeMemory(m_Device, m_MeshletDescriptionBufferMemory, nullptr);
}
//
// Created by cezary on 13.11.24.
//

#include "tr_heightmap.hpp"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <fstream>
#include <filesystem>
#include <glm/gtx/string_cast.hpp>

#include "tr_logger.hpp"
#include "tr_appconfig.hpp"

static
void LoadTerrain(std::string path, int16_t **dst, size_t *size, float *latitude, float *longitude) {
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

    *dst = new int16_t[bytes.size() / 2];
    *size = bytes.size();

    int i = 0;
    while(i < bytes.size()) {
        uint8_t x = bytes[i],
                y = bytes[i + 1];

        int16_t val = ((int16_t) ((((uint16_t) x << 8)) | (((uint16_t) y))));

        if (val < -10)
            val = -10;

        (*dst)[i / 2] = val;

        i += 2;
    }

    inputfile.close();
}

Heightmap::Heightmap(std::string dataPath)
    : m_DataPath(dataPath) { }

void Heightmap::Init(VkDevice device, VkPhysicalDevice physicalDevice) {

    // load data

    LoadTerrain(m_DataPath, &m_Data, &m_DataSize, &m_Latitude, &m_Longitude);


    std::cout << "Loaded terrain of buffer size: " << m_DataSize << std::endl;

    // build meshlets

    uint32_t  lengthOfHeightmap = m_DataSize / sizeof(uint16_t);
    lengthOfHeightmap = sqrt(lengthOfHeightmap);
    m_HeightmapLength = lengthOfHeightmap;




    // meshlet building
    int sizePerLine = 0;
    uint organizedDataOffset = 0;


    int maxMeshletSize = APP_CONFIG.m_MeshletInfo.m_MeshletLength;

    for(int y = 0; y<lengthOfHeightmap - 1; y += maxMeshletSize - 1) {
        int dataPointer = 0;
        sizePerLine = 0;

        for (int x = 0; x < lengthOfHeightmap - 1;) {

            // building one meshlet
            MeshletDescription description;
            memset(&description, 0, sizeof(description));

            description.Offset = uvec2(x, y);

            if (x + maxMeshletSize > lengthOfHeightmap) {
                description.Dimensions.x = lengthOfHeightmap - x;
            } else {
                description.Dimensions.x = maxMeshletSize;
            }

            if (y + maxMeshletSize > lengthOfHeightmap) {
                description.Dimensions.y = lengthOfHeightmap - y;
            } else {
                description.Dimensions.y = maxMeshletSize;
            }

            // now we build a linear chunk in the organized array

            description.HeightmapDataOffset = organizedDataOffset;

            for(int dy = 0; dy < description.Dimensions.y; dy++){
                for(int dx = 0; dx < description.Dimensions.x; dx++){
                    m_DataOrganizedForMeshlets.push_back(m_Data[x + dx + (dy + y) * m_HeightmapLength]);
                }
            }

            // log("Built meshlet starting at " + glm::to_string(description.Offset) + " with dimensions of " + glm::to_string(description.Dimensions));

            x += description.Dimensions.x - 1;

            organizedDataOffset += (description.Dimensions.x) * (description.Dimensions.y);

            sizePerLine ++;

#ifdef ENCODED_MESHLETS
            m_Meshlets.push_back(encodeMeshlet(description, APP_CONFIG.m_MeshletInfo.m_MeshletLength));
#else
            m_Meshlets.push_back(description);
#endif
        }

//        log("Size per line: " + std::to_string(sizePerLine));
    }



    // Heightmap data buffer creation and allocation

    m_Device = device;
    VkDeviceSize size = m_DataOrganizedForMeshlets.size() * sizeof(int16_t);
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
    memcpy(data, m_DataOrganizedForMeshlets.data(), m_DataOrganizedForMeshlets.size() * sizeof(int16_t));
    vkUnmapMemory(device, m_HeightmapDataBufferMemory);



    // Meshlet description buffer creation and allocation

    m_Device = device;
#ifdef ENCODED_MESHLETS
    size = m_Meshlets.size() * sizeof(EncodedMeshletDescription);
#else
    size = m_Meshlets.size() * sizeof(MeshletDescription);
#endif

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
    memcpy(data, m_Meshlets.data(), m_Meshlets.size() *
#ifdef ENCODED_MESHLETS
            sizeof(EncodedMeshletDescription));
#else
            sizeof(MeshletDescription));
#endif



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
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &(m_DescriptorSets.m_DescriptorSets[frame]), 0, nullptr);
}

void Heightmap::Draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, glm::vec3 observatorPosition) {

    HeightmapPushConstantData data;
    data.MeshletCount = m_Meshlets.size();
    data.HeightmapLength = m_HeightmapLength;
    data.Latitude = m_Latitude;
    data.Longitude = m_Longitude;
    data.BaseMeshletOffset = 0;
    data.LODLevel = APP_CONFIG.m_LODLevel;
    data.ObservatorPosition = observatorPosition;
    uint maxTasksEmitted = APP_CONFIG.m_MeshShaderConfig.m_MaxPreferredTaskWorkGroupInvocations;

    int k =0;

    // vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_TASK_BIT_EXT, sizeof(glm::mat4), sizeof(HeightmapPushConstantData), &data);
    // vkCmdDrawMeshTasksEXT(commandBuffer, m_Meshlets.size() / (maxTasksEmitted), 1, 1);

    for(int i = 0; i<m_Meshlets.size();) {
        data.BaseMeshletOffset = i;

        vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_TASK_BIT_EXT, sizeof(glm::mat4), sizeof(HeightmapPushConstantData), &data);

        uint meshletsDrawn = maxTasksEmitted * APP_CONFIG.m_MeshShaderConfig.m_MaxPreferredMeshWorkGroupInvocations;

        vkCmdDrawMeshTasksEXT(commandBuffer, maxTasksEmitted, 1, 1);

        i += meshletsDrawn ;
        k++;

        // if (k ==2)
        //     break;
    }
}

void Heightmap::Deinit() {
    delete [] m_Data;

    vkDestroyBuffer(m_Device, m_HeightmapDataBuffer, nullptr);
    vkFreeMemory(m_Device, m_HeightmapDataBufferMemory, nullptr);

    vkDestroyBuffer(m_Device, m_MeshletDescriptionBuffer, nullptr);
    vkFreeMemory(m_Device, m_MeshletDescriptionBufferMemory, nullptr);
}
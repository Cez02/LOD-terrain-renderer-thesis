#pragma once

#include <vulkan/vulkan.hpp>

#define DRAW_WIREFRAME 1



class AppConfig{
public:
    struct MSConfig {
        uint32_t m_MaxMeshOutputVertices;
        uint32_t m_MaxMeshOutputPrimitives;
        uint32_t m_MaxPreferredTaskWorkGroupInvocations;
        uint32_t m_MaxPreferredMeshWorkGroupInvocations;
        VkBool32 m_PrefersLocalInvocationVertexOutput;
        VkBool32 m_PrefersLocalInvocationPrimitiveOutput;
        VkBool32 m_PrefersCompactVertexOutput;
        VkBool32 m_PrefersCompactPrimitiveOutput;
        VkBool32 m_MaxTaskWorkGroupInvocations;

        uint32_t  m_MaxTaskWorkgroupCount;

        uint32_t           maxTaskWorkGroupSize[3];
        uint32_t           maxMeshWorkGroupSize[3];

        uint32_t m_MaxTaskWorkgroupSizeTotal;

        uint32_t m_MeshletsPerTaskInvocation = 2;

    } m_MeshShaderConfig;

    struct MeshletInfo {
        uint32_t m_MeshletLength;
    } m_MeshletInfo;

    uint32_t m_LODLevel = 1;

    void print();
};

extern AppConfig APP_CONFIG;
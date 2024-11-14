#pragma once

#include <vulkan/vulkan.hpp>

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
    } m_MeshShaderConfig;
};

extern AppConfig APP_CONFIG;
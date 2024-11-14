#pragma once

#include <vector>
#include <vulkan/vulkan.hpp>

class DescriptorSetBundle {
public:
    std::vector<VkDescriptorSet> m_DescriptorSets;
};

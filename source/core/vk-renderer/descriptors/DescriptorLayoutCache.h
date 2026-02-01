#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <unordered_map>

namespace Core::Renderer
{
class DescriptorLayoutCache
{
public:
    void init(VkDevice device);

    void cleanup();

    VkDescriptorSetLayout createDescriptorLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings);

    VkDescriptorSetLayout createDescriptorLayout(std::initializer_list<VkDescriptorSetLayoutBinding> bindings);

private:
    VkDevice mDevice = VK_NULL_HANDLE;

    struct DescriptorLayoutInfo
    {
        std::vector<VkDescriptorSetLayoutBinding> bindings;

        bool operator==(const DescriptorLayoutInfo& other) const;
    };

    struct DescriptorLayoutHash
    {
        size_t operator()(const DescriptorLayoutInfo& key) const;
    };

    std::unordered_map<DescriptorLayoutInfo, VkDescriptorSetLayout, DescriptorLayoutHash> _layoutCache;
};
} // namespace Core::Renderer
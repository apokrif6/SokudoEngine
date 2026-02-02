#pragma once

#include <string>
#include <vulkan/vulkan.h>
#include <vector>
#include <unordered_map>

namespace Core::Renderer
{
enum class DescriptorLayoutType
{
    GlobalScene,
    PrimitiveData,
    PrimitiveTextures,
    MaterialData
};

class DescriptorLayoutCache
{
public:
    void init(VkDevice device);

    void cleanup();

    VkDescriptorSetLayout getLayout(DescriptorLayoutType type);

private:
    VkDevice mDevice = VK_NULL_HANDLE;

    VkDescriptorSetLayout createDescriptorLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings,
                                                 const std::string& debugName);

    struct DescriptorLayoutInfo
    {
        std::vector<VkDescriptorSetLayoutBinding> bindings;

        bool operator==(const DescriptorLayoutInfo& other) const;
    };

    struct DescriptorLayoutHash
    {
        size_t operator()(const DescriptorLayoutInfo& key) const;
    };

    std::unordered_map<DescriptorLayoutInfo, VkDescriptorSetLayout, DescriptorLayoutHash> mLayoutBindingCache;

    std::unordered_map<DescriptorLayoutType, VkDescriptorSetLayout> mLayoutTypeCache;
};
} // namespace Core::Renderer
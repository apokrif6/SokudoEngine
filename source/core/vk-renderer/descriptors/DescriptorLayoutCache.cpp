#include "DescriptorLayoutCache.h"

#include <algorithm>
#include <ranges>

namespace Core::Renderer
{
void DescriptorLayoutCache::init(VkDevice device)
{
    mDevice = device;
}

void DescriptorLayoutCache::cleanup()
{
    for (const auto &layout: _layoutCache | std::views::values)
    {
        vkDestroyDescriptorSetLayout(mDevice, layout, nullptr);
    }
    _layoutCache.clear();
}

VkDescriptorSetLayout
DescriptorLayoutCache::createDescriptorLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings)
{
    DescriptorLayoutInfo layoutInfo;
    layoutInfo.bindings = bindings;

    std::ranges::sort(layoutInfo.bindings,
                      [](const VkDescriptorSetLayoutBinding& a, const VkDescriptorSetLayoutBinding& b)
                      { return a.binding < b.binding; });

    if (const auto it = _layoutCache.find(layoutInfo); it != _layoutCache.end())
    {
        return it->second;
    }

    VkDescriptorSetLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.pBindings = layoutInfo.bindings.data();
    createInfo.bindingCount = static_cast<uint32_t>(layoutInfo.bindings.size());

    VkDescriptorSetLayout layout;
    vkCreateDescriptorSetLayout(mDevice, &createInfo, nullptr, &layout);
    // TODO
    // add ensure when ensure wrappers are ready

    _layoutCache[layoutInfo] = layout;

    return layout;
}

bool DescriptorLayoutCache::DescriptorLayoutInfo::operator==(const DescriptorLayoutInfo& other) const
{
    if (other.bindings.size() != bindings.size())
    {
        return false;
    }
    for (size_t i = 0; i < bindings.size(); i++)
    {
        if (other.bindings[i].binding != bindings[i].binding)
        {
            return false;
        }
        if (other.bindings[i].descriptorType != bindings[i].descriptorType)
        {
            return false;
        }
        if (other.bindings[i].descriptorCount != bindings[i].descriptorCount)
        {
            return false;
        }
        if (other.bindings[i].stageFlags != bindings[i].stageFlags)
        {
            return false;
        }
    }
    return true;
}

size_t DescriptorLayoutCache::DescriptorLayoutHash::operator()(const DescriptorLayoutInfo& key) const
{
    size_t result = 0;
    for (const auto& binding : key.bindings)
    {
        size_t bindingHash = binding.binding | binding.descriptorType << 8 | binding.stageFlags << 16;
        result ^= std::hash<size_t>()(bindingHash) + 0x9e3779b9 + (result << 6) + (result >> 2);
    }
    return result;
}
} // namespace Core::Renderer
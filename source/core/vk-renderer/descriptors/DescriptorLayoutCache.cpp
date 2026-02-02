#include "DescriptorLayoutCache.h"

#include <algorithm>
#include <ranges>

#include "core/vk-renderer/debug/DebugUtils.h"

namespace Core::Renderer
{
void DescriptorLayoutCache::init(VkDevice device) { mDevice = device; }

void DescriptorLayoutCache::cleanup()
{
    for (const auto& layout : mLayoutBindingCache | std::views::values)
    {
        vkDestroyDescriptorSetLayout(mDevice, layout, nullptr);
    }
    mLayoutBindingCache.clear();
}

VkDescriptorSetLayout DescriptorLayoutCache::getLayout(DescriptorLayoutType type)
{
    if (const auto it = mLayoutTypeCache.find(type); it != mLayoutTypeCache.end())
    {
        return it->second;
    }

    VkDescriptorSetLayout layout = VK_NULL_HANDLE;

    switch (type)
    {
    case DescriptorLayoutType::GlobalScene:
        layout = createDescriptorLayout(
            {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT},
             {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT},
             {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT},
             {3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT}},
            "DescriptorSetLayout_GlobalScene");
        break;
    case DescriptorLayoutType::PrimitiveData:
        layout = createDescriptorLayout(
            {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT}},
            "DescriptorSetLayout_PrimitiveData");
        break;
    case DescriptorLayoutType::PrimitiveTextures:
    {
        std::vector<VkDescriptorSetLayoutBinding> textureBindings(5);
        for (size_t i = 0; i < textureBindings.size(); ++i)
        {
            textureBindings[i].binding = static_cast<uint32_t>(i);
            textureBindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            textureBindings[i].descriptorCount = 1;
            textureBindings[i].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        }
        layout = createDescriptorLayout(textureBindings, "DescriptorSetLayout_PrimitiveTextures");
        break;
    }
    case DescriptorLayoutType::MaterialData:
        layout = createDescriptorLayout(
            {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT}},
            "DescriptorSetLayout_MaterialData");
        break;
    }

    mLayoutTypeCache[type] = layout;

    return layout;
}

VkDescriptorSetLayout
DescriptorLayoutCache::createDescriptorLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings,
                                              const std::string& debugName)
{
    DescriptorLayoutInfo layoutInfo;
    layoutInfo.bindings = bindings;

    std::ranges::sort(layoutInfo.bindings, [](const VkDescriptorSetLayoutBinding& a,
                                              const VkDescriptorSetLayoutBinding& b) { return a.binding < b.binding; });

    if (const auto it = mLayoutBindingCache.find(layoutInfo); it != mLayoutBindingCache.end())
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

    Debug::setObjectName(mDevice, reinterpret_cast<uint64_t>(layout), VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, debugName);

    mLayoutBindingCache[layoutInfo] = layout;

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
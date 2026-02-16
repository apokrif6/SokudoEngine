#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace Core::Renderer
{
class DescriptorAllocator
{
public:
    void init(VkDevice device);

    void cleanup();

    bool allocate(VkDescriptorSetLayout layout, VkDescriptorSet& set);

private:
    VkDevice mDevice = VK_NULL_HANDLE;
    VkDescriptorPool mCurrentPool = VK_NULL_HANDLE;

    std::vector<VkDescriptorPool> mUsedPools;
    std::vector<VkDescriptorPool> mFreePools;

    VkDescriptorPool grabPool();
};

} // namespace Core::Renderer
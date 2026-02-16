#include "DescriptorAllocator.h"
#include <vector>

void Core::Renderer::DescriptorAllocator::init(VkDevice device) { mDevice = device; }

void Core::Renderer::DescriptorAllocator::cleanup()
{
    for (auto freePool : mFreePools)
    {
        vkDestroyDescriptorPool(mDevice, freePool, nullptr);
    }
    for (auto usedPool : mUsedPools)
    {
        vkDestroyDescriptorPool(mDevice, usedPool, nullptr);
    }

    mUsedPools.clear();
    mFreePools.clear();
    mCurrentPool = VK_NULL_HANDLE;
}

bool Core::Renderer::DescriptorAllocator::allocate(VkDescriptorSetLayout layout, VkDescriptorSet& set)
{
    if (mCurrentPool == VK_NULL_HANDLE)
    {
        mCurrentPool = grabPool();
        mUsedPools.push_back(mCurrentPool);
    }

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pSetLayouts = &layout;
    allocInfo.descriptorPool = mCurrentPool;
    allocInfo.descriptorSetCount = 1;

    VkResult result = vkAllocateDescriptorSets(mDevice, &allocInfo, &set);

    if (result == VK_SUCCESS)
    {
        return true;
    }

    if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL)
    {
        mCurrentPool = grabPool();
        mUsedPools.push_back(mCurrentPool);
        allocInfo.descriptorPool = mCurrentPool;

        result = vkAllocateDescriptorSets(mDevice, &allocInfo, &set);
        return result == VK_SUCCESS;
    }

    return false;
}

VkDescriptorPool Core::Renderer::DescriptorAllocator::grabPool()
{
    if (!mFreePools.empty())
    {
        VkDescriptorPool pool = mFreePools.back();
        mFreePools.pop_back();
        return pool;
    }

    std::vector<VkDescriptorPoolSize> poolSizes = {{VK_DESCRIPTOR_TYPE_SAMPLER, 500},
                                                   {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4000},
                                                   {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 4000},
                                                   {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
                                                   {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
                                                   {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
                                                   {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2000},
                                                   {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2000},
                                                   {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
                                                   {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
                                                   {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 500}};

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = 0;
    poolInfo.maxSets = 1000;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();

    VkDescriptorPool newPool;
    vkCreateDescriptorPool(mDevice, &poolInfo, nullptr, &newPool);
    // TODO
    // add assert
    return newPool;
}

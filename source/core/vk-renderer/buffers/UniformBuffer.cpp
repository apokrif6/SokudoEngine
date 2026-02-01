#include "UniformBuffer.h"
#include "core/tools/Logger.h"

#include "VkBootstrap.h"
#include "core/vk-renderer/debug/DebugUtils.h"

bool Core::Renderer::UniformBuffer::init(VkRenderData& renderData, VkUniformBufferData& UBOData, size_t bufferSize,
                                         const std::string& name, VkDescriptorSetLayout customLayout,
                                         std::vector<VkDescriptorPoolSize> extraPoolSizes)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    VmaAllocationCreateInfo vmaAllocCreateInfo{};
    vmaAllocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    VmaAllocationInfo vmaAllocInfo{};

    if (vmaCreateBuffer(renderData.rdAllocator, &bufferInfo, &vmaAllocCreateInfo, &UBOData.rdUniformBuffer,
                        &UBOData.rdUniformBufferAlloc, &vmaAllocInfo) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: could not allocate uniform buffer via VMA\n", __FUNCTION__);
        return false;
    }

    UBOData.rdName = "Uniform Buffer " + name;
    vmaSetAllocationName(renderData.rdAllocator, UBOData.rdUniformBufferAlloc, UBOData.rdName.c_str());

    Debug::setObjectName(renderData.rdVkbDevice.device, (uint64_t)UBOData.rdUniformBuffer, VK_OBJECT_TYPE_BUFFER,
                         UBOData.rdName);

    UBOData.ownsLayout = true;

    if (customLayout != VK_NULL_HANDLE)
    {
        // create complex layout e.g. (Binding 0: UBO, Binding 1: UBO, Binding 3: UBO)
        // e.g. used to render primitive. GlobalScene plus IBL textures as one descriptor set
        UBOData.rdUBODescriptorLayout = customLayout;
        UBOData.ownsLayout = false;
    }
    else
    {
        // create standard layout with (Binding 0: UBO)
        VkDescriptorSetLayoutBinding uboBind{};
        uboBind.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboBind.binding = 0;
        uboBind.descriptorCount = 1;
        uboBind.pImmutableSamplers = nullptr;
        uboBind.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo uboCreateInfo{};
        uboCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        uboCreateInfo.bindingCount = 1;
        uboCreateInfo.pBindings = &uboBind;

        if (vkCreateDescriptorSetLayout(renderData.rdVkbDevice.device, &uboCreateInfo, nullptr,
                                        &UBOData.rdUBODescriptorLayout) != VK_SUCCESS)
        {
            Logger::log(1, "%s error: could not create UBO descriptor set layout\n", __FUNCTION__);
            return false;
        }
    }

    std::vector<VkDescriptorPoolSize> poolSizes = {{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}};

    if (!extraPoolSizes.empty())
    {
        poolSizes.insert(poolSizes.end(), extraPoolSizes.begin(), extraPoolSizes.end());
    }

    VkDescriptorPoolCreateInfo descriptorPool{};
    descriptorPool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPool.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    descriptorPool.pPoolSizes = poolSizes.data();
    descriptorPool.maxSets = 1;

    if (vkCreateDescriptorPool(renderData.rdVkbDevice.device, &descriptorPool, nullptr, &UBOData.rdUBODescriptorPool) !=
        VK_SUCCESS)
    {
        Logger::log(1, "%s error: could not create UBO descriptor pool\n", __FUNCTION__);
        return false;
    }

    VkDescriptorSetAllocateInfo descriptorAllocateInfo{};
    descriptorAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorAllocateInfo.descriptorPool = UBOData.rdUBODescriptorPool;
    descriptorAllocateInfo.descriptorSetCount = 1;
    descriptorAllocateInfo.pSetLayouts = &UBOData.rdUBODescriptorLayout;

    if (vkAllocateDescriptorSets(renderData.rdVkbDevice.device, &descriptorAllocateInfo, &UBOData.rdUBODescriptorSet) !=
        VK_SUCCESS)
    {
        Logger::log(1, "%s error: could not allocate UBO descriptor set\n", __FUNCTION__);
        return false;
    }

    VkDescriptorBufferInfo uboInfo{};
    uboInfo.buffer = UBOData.rdUniformBuffer;
    uboInfo.offset = 0;
    uboInfo.range = bufferSize;

    VkWriteDescriptorSet writeDescriptorSet{};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeDescriptorSet.dstSet = UBOData.rdUBODescriptorSet;
    writeDescriptorSet.dstBinding = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.pBufferInfo = &uboInfo;

    vkUpdateDescriptorSets(renderData.rdVkbDevice.device, 1, &writeDescriptorSet, 0, nullptr);

    UBOData.rdUniformBufferSize = bufferSize;

    Logger::log(1, "%s: created uniform buffer %s of size %i\n", __FUNCTION__, name.c_str(), uboInfo.range);

    return true;
}

void Core::Renderer::UniformBuffer::cleanup(VkRenderData& renderData, VkUniformBufferData& UBOData)
{
    vkDestroyDescriptorPool(renderData.rdVkbDevice.device, UBOData.rdUBODescriptorPool, nullptr);
    if (UBOData.ownsLayout)
    {
        vkDestroyDescriptorSetLayout(renderData.rdVkbDevice.device, UBOData.rdUBODescriptorLayout, nullptr);
    }
    vmaDestroyBuffer(renderData.rdAllocator, UBOData.rdUniformBuffer, UBOData.rdUniformBufferAlloc);
}

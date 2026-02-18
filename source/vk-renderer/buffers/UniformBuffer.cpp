#include "UniformBuffer.h"
#include "tools/Logger.h"

#include "VkBootstrap.h"
#include "vk-renderer/debug/DebugUtils.h"

bool Core::Renderer::UniformBuffer::init(VkRenderData& renderData, VkUniformBufferData& UBOData, size_t bufferSize,
                                         const std::string& name, DescriptorLayoutType layoutType)
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

    UBOData.rdUBODescriptorLayout = renderData.rdDescriptorLayoutCache->getLayout(layoutType);

    UBOData.rdName = "Uniform Buffer " + name;
    vmaSetAllocationName(renderData.rdAllocator, UBOData.rdUniformBufferAlloc, UBOData.rdName.c_str());

    Debug::setObjectName(renderData.rdVkbDevice.device, reinterpret_cast<uint64_t>(UBOData.rdUniformBuffer),
                         VK_OBJECT_TYPE_BUFFER, UBOData.rdName);

    if (!renderData.rdDescriptorAllocator->allocate(UBOData.rdUBODescriptorLayout, UBOData.rdUBODescriptorSet))
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
    vmaDestroyBuffer(renderData.rdAllocator, UBOData.rdUniformBuffer, UBOData.rdUniformBufferAlloc);
}

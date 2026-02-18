#include "ShaderStorageBuffer.h"
#include "tools/Logger.h"

#include "VkBootstrap.h"
#include "core/vk-renderer/debug/DebugUtils.h"

bool Core::Renderer::ShaderStorageBuffer::init(VkRenderData& renderData, VkShaderStorageBufferData& SSBOData,
                                               size_t bufferSize, const std::string& name)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    VmaAllocationCreateInfo vmaAllocCreateInfo{};
    vmaAllocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

    VmaAllocationInfo vmaAllocInfo{};

    if (vmaCreateBuffer(renderData.rdAllocator, &bufferInfo, &vmaAllocCreateInfo, &SSBOData.rdShaderStorageBuffer,
                        &SSBOData.rdShaderStorageBufferAlloc, &vmaAllocInfo) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: could not allocate shader storage buffer via VMA\n", __FUNCTION__);
        return false;
    }

    SSBOData.rdName = "Shader Storage Buffer " + name;
    vmaSetAllocationName(renderData.rdAllocator, SSBOData.rdShaderStorageBufferAlloc, SSBOData.rdName.c_str());

    Debug::setObjectName(renderData.rdVkbDevice.device, reinterpret_cast<uint64_t>(SSBOData.rdShaderStorageBuffer), VK_OBJECT_TYPE_BUFFER,
                         SSBOData.rdName);

    SSBOData.rdSSBODescriptorLayout = renderData.rdDescriptorLayoutCache->getLayout(DescriptorLayoutType::SingleSSBO);

    if (!renderData.rdDescriptorAllocator->allocate(SSBOData.rdSSBODescriptorLayout, SSBOData.rdSSBODescriptorSet))
    {
        Logger::log(1, "%s error: could not allocate SSBO descriptor set\n", __FUNCTION__);
        return false;
    }

    VkDescriptorBufferInfo ssboInfo{};
    ssboInfo.buffer = SSBOData.rdShaderStorageBuffer;
    ssboInfo.offset = 0;
    ssboInfo.range = bufferSize;

    VkWriteDescriptorSet writeDescriptorSet{};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    writeDescriptorSet.dstSet = SSBOData.rdSSBODescriptorSet;
    writeDescriptorSet.dstBinding = 0;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.pBufferInfo = &ssboInfo;

    vkUpdateDescriptorSets(renderData.rdVkbDevice.device, 1, &writeDescriptorSet, 0, nullptr);

    SSBOData.rdShaderStorageBufferSize = bufferSize;

    Logger::log(1, "%s: created shader storage buffer of size %i\n", __FUNCTION__, ssboInfo.range);

    return true;
}

void Core::Renderer::ShaderStorageBuffer::uploadData(VkRenderData& renderData, VkShaderStorageBufferData& SSBOData,
                                                     std::vector<glm::mat4> matrices)
{
    if (matrices.empty())
    {
        return;
    }

    void* data;
    vmaMapMemory(renderData.rdAllocator, SSBOData.rdShaderStorageBufferAlloc, &data);
    std::memcpy(data, matrices.data(), SSBOData.rdShaderStorageBufferSize);
    vmaUnmapMemory(renderData.rdAllocator, SSBOData.rdShaderStorageBufferAlloc);
}

void Core::Renderer::ShaderStorageBuffer::cleanup(VkRenderData& renderData, VkShaderStorageBufferData& SSBOData)
{
    vkDestroyDescriptorSetLayout(renderData.rdVkbDevice.device, SSBOData.rdSSBODescriptorLayout, nullptr);
    vmaDestroyBuffer(renderData.rdAllocator, SSBOData.rdShaderStorageBuffer, SSBOData.rdShaderStorageBufferAlloc);
}

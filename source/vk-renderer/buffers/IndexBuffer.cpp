#include <cstring>

#include "IndexBuffer.h"
#include "vk-renderer/buffers/CommandBuffer.h"
#include "tools/Logger.h"
#include "vk-renderer/debug/DebugUtils.h"

bool Core::Renderer::IndexBuffer::init(VkRenderData& renderData, VkIndexBufferData& indexBufferData,
                                       unsigned int bufferSize, const std::string& name)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo bufferAllocCreateInfo{};
    bufferAllocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    VmaAllocationInfo bufferAllocInfo{};

    if (vmaCreateBuffer(renderData.rdAllocator, &bufferInfo, &bufferAllocCreateInfo, &indexBufferData.rdIndexBuffer,
                        &indexBufferData.rdIndexBufferAlloc, &bufferAllocInfo) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: could not allocate index buffer via VMA\n", __FUNCTION__);
        return false;
    }

    indexBufferData.rdName = "Index Buffer " + name;
    vmaSetAllocationName(renderData.rdAllocator, indexBufferData.rdIndexBufferAlloc, indexBufferData.rdName.c_str());

    Debug::setObjectName(renderData.rdVkbDevice.device, reinterpret_cast<uint64_t>(indexBufferData.rdIndexBuffer), VK_OBJECT_TYPE_BUFFER,
                         indexBufferData.rdName);

    VkBufferCreateInfo stagingBufferInfo{};
    stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    stagingBufferInfo.size = bufferSize;
    stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VmaAllocationCreateInfo stagingAllocCreateInfo{};
    stagingAllocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

    VmaAllocationInfo stagingAllocInfo{};

    if (vmaCreateBuffer(renderData.rdAllocator, &stagingBufferInfo, &stagingAllocCreateInfo,
                        &indexBufferData.rdStagingBuffer, &indexBufferData.rdStagingBufferAlloc,
                        &stagingAllocInfo) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: could not allocate index staging buffer via VMA\n", __FUNCTION__);
        return false;
    }

    indexBufferData.rdName = "Index Staging Buffer " + name;
    vmaSetAllocationName(renderData.rdAllocator, indexBufferData.rdStagingBufferAlloc, indexBufferData.rdName.c_str());

    indexBufferData.rdIndexBufferSize = bufferSize;
    return true;
}

bool Core::Renderer::IndexBuffer::uploadData(VkRenderData& renderData, VkIndexBufferData& indexBufferData,
                                             const std::vector<uint32_t>& indexData)
{
    const unsigned int bufferSize = indexData.size() * sizeof(uint32_t);
    /* buffer too small, resize */
    if (indexBufferData.rdIndexBufferSize < bufferSize)
    {
        cleanup(renderData, indexBufferData);

        if (!init(renderData, indexBufferData, bufferSize, indexBufferData.rdName))
        {
            return false;
        }
        Logger::log(1, "%s: index buffer resize to %i bytes\n", __FUNCTION__, bufferSize);
        indexBufferData.rdIndexBufferSize = bufferSize;
    }

    /* copy data to staging buffer*/
    void* data;
    vmaMapMemory(renderData.rdAllocator, indexBufferData.rdStagingBufferAlloc, &data);
    std::memcpy(data, indexData.data(), bufferSize);
    vmaUnmapMemory(renderData.rdAllocator, indexBufferData.rdStagingBufferAlloc);

    VkBufferMemoryBarrier vertexBufferBarrier{};
    vertexBufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    vertexBufferBarrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
    vertexBufferBarrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    vertexBufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vertexBufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vertexBufferBarrier.buffer = indexBufferData.rdStagingBuffer;
    vertexBufferBarrier.offset = 0;
    vertexBufferBarrier.size = indexBufferData.rdIndexBufferSize;

    VkBufferCopy stagingBufferCopy{};
    stagingBufferCopy.srcOffset = 0;
    stagingBufferCopy.dstOffset = 0;
    stagingBufferCopy.size = indexBufferData.rdIndexBufferSize;

    vkCmdCopyBuffer(renderData.rdCommandBuffer, indexBufferData.rdStagingBuffer, indexBufferData.rdIndexBuffer, 1,
                    &stagingBufferCopy);
    vkCmdPipelineBarrier(renderData.rdCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                         0, 0, nullptr, 1, &vertexBufferBarrier, 0, nullptr);

    return true;
}

void Core::Renderer::IndexBuffer::cleanup(VkRenderData& renderData, VkIndexBufferData& indexBufferData)
{
    vmaDestroyBuffer(renderData.rdAllocator, indexBufferData.rdStagingBuffer, indexBufferData.rdStagingBufferAlloc);
    vmaDestroyBuffer(renderData.rdAllocator, indexBufferData.rdIndexBuffer, indexBufferData.rdIndexBufferAlloc);
}
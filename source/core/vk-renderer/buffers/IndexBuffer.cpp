#include <cstring>

#include "IndexBuffer.h"
#include "core/vk-renderer/buffers/CommandBuffer.h"
#include "core/tools/Logger.h"

bool Core::Renderer::IndexBuffer::init(Core::Renderer::VkRenderData& renderData, VkIndexBufferData& indexBufferData,
                                       unsigned int bufferSize)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo bufferAllocInfo{};
    bufferAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    if (vmaCreateBuffer(renderData.rdAllocator, &bufferInfo, &bufferAllocInfo, &indexBufferData.rdIndexBuffer,
                        &indexBufferData.rdIndexBufferAlloc, nullptr) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: could not allocate index buffer via VMA\n", __FUNCTION__);
        return false;
    }

    VkBufferCreateInfo stagingBufferInfo{};
    stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    stagingBufferInfo.size = bufferSize;
    stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VmaAllocationCreateInfo stagingAllocInfo{};
    stagingAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

    if (vmaCreateBuffer(renderData.rdAllocator, &stagingBufferInfo, &stagingAllocInfo, &indexBufferData.rdStagingBuffer,
                        &indexBufferData.rdStagingBufferAlloc, nullptr) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: could not allocate index staging buffer via VMA\n", __FUNCTION__);
        return false;
    }

    indexBufferData.rdIndexBufferSize = bufferSize;
    return true;
}

bool Core::Renderer::IndexBuffer::uploadData(Core::Renderer::VkRenderData& renderData,
                                             Core::Renderer::VkIndexBufferData& indexBufferData,
                                             const std::vector<uint32_t>& indexData)
{
    const unsigned int bufferSize = indexData.size() * sizeof(uint32_t);
    /* buffer too small, resize */
    if (indexBufferData.rdIndexBufferSize < bufferSize)
    {
        cleanup(renderData, indexBufferData);

        if (!init(renderData, indexBufferData, bufferSize))
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

void Core::Renderer::IndexBuffer::cleanup(Core::Renderer::VkRenderData& renderData, VkIndexBufferData& indexBufferData)
{
    vmaDestroyBuffer(renderData.rdAllocator, indexBufferData.rdStagingBuffer, indexBufferData.rdStagingBufferAlloc);
    vmaDestroyBuffer(renderData.rdAllocator, indexBufferData.rdIndexBuffer, indexBufferData.rdIndexBufferAlloc);
}
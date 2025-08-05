#include <cstring>
#include "VertexBuffer.h"
#include "core/tools/Logger.h"

bool Core::Renderer::VertexBuffer::init(Core::Renderer::VkRenderData& renderData, VkVertexBufferData& vertexBufferData,
                                        unsigned int bufferSize)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo bufferAllocInfo{};
    bufferAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    if (vmaCreateBuffer(renderData.rdAllocator, &bufferInfo, &bufferAllocInfo, &vertexBufferData.rdVertexBuffer,
                        &vertexBufferData.rdVertexBufferAlloc, nullptr) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: could not allocate vertex buffer via VMA\n", __FUNCTION__);
        return false;
    }

    VkBufferCreateInfo stagingBufferInfo{};
    stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    stagingBufferInfo.size = bufferSize;
    stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VmaAllocationCreateInfo stagingAllocInfo{};
    stagingAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

    if (vmaCreateBuffer(renderData.rdAllocator, &stagingBufferInfo, &stagingAllocInfo,
                        &vertexBufferData.rdStagingBuffer, &vertexBufferData.rdStagingBufferAlloc,
                        nullptr) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: could not allocate vertex staging buffer via VMA\n", __FUNCTION__);
        return false;
    }

    vertexBufferData.rdVertexBufferSize = bufferSize;

    return true;
}

bool Core::Renderer::VertexBuffer::uploadData(Core::Renderer::VkRenderData& renderData,
                                              VkVertexBufferData& vertexBufferData, Core::Renderer::VkMesh vertexData)
{
    unsigned int vertexDataSize = vertexData.vertices.size() * sizeof(VkVertex);

    // buffer too small, should resize
    if (vertexBufferData.rdVertexBufferSize < vertexDataSize)
    {
        cleanup(renderData, vertexBufferData);

        if (!init(renderData, vertexBufferData, vertexDataSize))
        {
            Logger::log(1, "%s error: could not create vertex buffer of size %i bytes\n", __FUNCTION__, vertexDataSize);
            return false;
        }

        Logger::log(1, "%s: vertex buffer resize to %i bytes\n", __FUNCTION__, vertexDataSize);
        vertexBufferData.rdVertexBufferSize = vertexDataSize;
    }

    void* data;
    vmaMapMemory(renderData.rdAllocator, vertexBufferData.rdStagingBufferAlloc, &data);
    std::memcpy(data, vertexData.vertices.data(), vertexDataSize);
    vmaUnmapMemory(renderData.rdAllocator, vertexBufferData.rdStagingBufferAlloc);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = renderData.rdCommandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer tempCmdBuffer;
    vkAllocateCommandBuffers(renderData.rdVkbDevice, &allocInfo, &tempCmdBuffer);

    VkCommandBufferBeginInfo cmdBeginInfo{};
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(tempCmdBuffer, &cmdBeginInfo) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: failed to begin command buffer\n", __FUNCTION__);
        return false;
    }

    VkBufferCopy stagingBufferCopy{};
    stagingBufferCopy.srcOffset = 0;
    stagingBufferCopy.dstOffset = 0;
    stagingBufferCopy.size = vertexDataSize;

    vkCmdCopyBuffer(tempCmdBuffer, vertexBufferData.rdStagingBuffer, vertexBufferData.rdVertexBuffer, 1,
                    &stagingBufferCopy);

    VkBufferMemoryBarrier vertexBufferBarrier{};
    vertexBufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    vertexBufferBarrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
    vertexBufferBarrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    vertexBufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vertexBufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vertexBufferBarrier.buffer = vertexBufferData.rdStagingBuffer;
    vertexBufferBarrier.offset = 0;
    vertexBufferBarrier.size = vertexDataSize;

    vkCmdPipelineBarrier(tempCmdBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                         0, 0, nullptr, 1, &vertexBufferBarrier, 0, nullptr);

    if (vkEndCommandBuffer(tempCmdBuffer) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: failed to end command buffer\n", __FUNCTION__);
        return false;
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &tempCmdBuffer;

    VkFence fence;
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vkCreateFence(renderData.rdVkbDevice, &fenceInfo, nullptr, &fence);

    vkQueueSubmit(renderData.rdGraphicsQueue, 1, &submitInfo, fence);
    vkWaitForFences(renderData.rdVkbDevice, 1, &fence, VK_TRUE, UINT64_MAX);

    vkDestroyFence(renderData.rdVkbDevice, fence, nullptr);
    vkFreeCommandBuffers(renderData.rdVkbDevice, renderData.rdCommandPool, 1, &tempCmdBuffer);

    return true;
}

bool Core::Renderer::VertexBuffer::uploadData(Core::Renderer::VkRenderData& renderData,
                                              VkVertexBufferData& vertexBufferData, std::vector<glm::vec3> vertexData)
{
    unsigned int vertexDataSize = vertexData.size() * sizeof(glm::vec3);

    /* buffer too small, resize */
    if (vertexBufferData.rdVertexBufferSize < vertexDataSize)
    {
        cleanup(renderData, vertexBufferData);

        if (!init(renderData, vertexBufferData, vertexDataSize))
        {
            Logger::log(1, "%s error: could not create vertex buffer of size %i bytes\n", __FUNCTION__, vertexDataSize);
            return false;
        }
        Logger::log(1, "%s: vertex buffer resize to %i bytes\n", __FUNCTION__, vertexDataSize);
        vertexBufferData.rdVertexBufferSize = vertexDataSize;
    }

    /* copy data to staging buffer*/
    void* data;
    vmaMapMemory(renderData.rdAllocator, vertexBufferData.rdStagingBufferAlloc, &data);
    std::memcpy(data, vertexData.data(), vertexDataSize);
    vmaUnmapMemory(renderData.rdAllocator, vertexBufferData.rdStagingBufferAlloc);

    VkBufferMemoryBarrier vertexBufferBarrier{};
    vertexBufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    vertexBufferBarrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
    vertexBufferBarrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    vertexBufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vertexBufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vertexBufferBarrier.buffer = vertexBufferData.rdStagingBuffer;
    vertexBufferBarrier.offset = 0;
    vertexBufferBarrier.size = vertexBufferData.rdVertexBufferSize;

    VkBufferCopy stagingBufferCopy{};
    stagingBufferCopy.srcOffset = 0;
    stagingBufferCopy.dstOffset = 0;
    stagingBufferCopy.size = vertexDataSize;

    vkCmdCopyBuffer(renderData.rdCommandBuffer, vertexBufferData.rdStagingBuffer, vertexBufferData.rdVertexBuffer, 1,
                    &stagingBufferCopy);
    vkCmdPipelineBarrier(renderData.rdCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                         0, 0, nullptr, 1, &vertexBufferBarrier, 0, nullptr);

    return true;
}

bool Core::Renderer::VertexBuffer::uploadData(Core::Renderer::VkRenderData& renderData,
                                              Core::Renderer::VkVertexBufferData& vertexBufferData,
                                              std::vector<Core::Renderer::NewVertex> vertexData)
{
    unsigned int vertexDataSize = vertexData.size() * sizeof(NewVertex);

    /* buffer too small, resize */
    if (vertexBufferData.rdVertexBufferSize < vertexDataSize)
    {
        cleanup(renderData, vertexBufferData);

        if (!init(renderData, vertexBufferData, vertexDataSize))
        {
            Logger::log(1, "%s error: could not create vertex buffer of size %i bytes\n", __FUNCTION__, vertexDataSize);
            return false;
        }
        Logger::log(1, "%s: vertex buffer resize to %i bytes\n", __FUNCTION__, vertexDataSize);
        vertexBufferData.rdVertexBufferSize = vertexDataSize;
    }

    /* copy data to staging buffer*/
    void* data;
    vmaMapMemory(renderData.rdAllocator, vertexBufferData.rdStagingBufferAlloc, &data);
    std::memcpy(data, vertexData.data(), vertexDataSize);
    vmaUnmapMemory(renderData.rdAllocator, vertexBufferData.rdStagingBufferAlloc);

    VkBufferMemoryBarrier vertexBufferBarrier{};
    vertexBufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    vertexBufferBarrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
    vertexBufferBarrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    vertexBufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vertexBufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vertexBufferBarrier.buffer = vertexBufferData.rdStagingBuffer;
    vertexBufferBarrier.offset = 0;
    vertexBufferBarrier.size = vertexBufferData.rdVertexBufferSize;

    VkBufferCopy stagingBufferCopy{};
    stagingBufferCopy.srcOffset = 0;
    stagingBufferCopy.dstOffset = 0;
    stagingBufferCopy.size = vertexDataSize;

    vkCmdCopyBuffer(renderData.rdCommandBuffer, vertexBufferData.rdStagingBuffer, vertexBufferData.rdVertexBuffer, 1,
                    &stagingBufferCopy);
    vkCmdPipelineBarrier(renderData.rdCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                         0, 0, nullptr, 1, &vertexBufferBarrier, 0, nullptr);

    return true;
}

bool Core::Renderer::VertexBuffer::uploadData(Core::Renderer::VkRenderData& renderData,
                                              Core::Renderer::VkVertexBufferData& vertexBufferData,
                                              const std::vector<Core::Renderer::Debug::LineVertex>& vertexData)
{
    unsigned int vertexDataSize = vertexData.size() * sizeof(Core::Renderer::Debug::LineVertex);

    /* buffer too small, resize */
    if (vertexBufferData.rdVertexBufferSize < vertexDataSize)
    {
        cleanup(renderData, vertexBufferData);

        if (!init(renderData, vertexBufferData, vertexDataSize))
        {
            Logger::log(1, "%s error: could not create vertex buffer of size %i bytes\n", __FUNCTION__, vertexDataSize);
            return false;
        }
        Logger::log(1, "%s: vertex buffer resize to %i bytes\n", __FUNCTION__, vertexDataSize);
        vertexBufferData.rdVertexBufferSize = vertexDataSize;
    }

    /* copy data to staging buffer*/
    void* data;
    vmaMapMemory(renderData.rdAllocator, vertexBufferData.rdStagingBufferAlloc, &data);
    std::memcpy(data, vertexData.data(), vertexDataSize);
    vmaUnmapMemory(renderData.rdAllocator, vertexBufferData.rdStagingBufferAlloc);

    VkBufferMemoryBarrier vertexBufferBarrier{};
    vertexBufferBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    vertexBufferBarrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
    vertexBufferBarrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    vertexBufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vertexBufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vertexBufferBarrier.buffer = vertexBufferData.rdStagingBuffer;
    vertexBufferBarrier.offset = 0;
    vertexBufferBarrier.size = vertexBufferData.rdVertexBufferSize;

    VkBufferCopy stagingBufferCopy{};
    stagingBufferCopy.srcOffset = 0;
    stagingBufferCopy.dstOffset = 0;
    stagingBufferCopy.size = vertexDataSize;

    vkCmdCopyBuffer(renderData.rdCommandBuffer, vertexBufferData.rdStagingBuffer, vertexBufferData.rdVertexBuffer, 1,
                    &stagingBufferCopy);
    vkCmdPipelineBarrier(renderData.rdCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                         0, 0, nullptr, 1, &vertexBufferBarrier, 0, nullptr);

    return true;
}

void Core::Renderer::VertexBuffer::cleanup(Core::Renderer::VkRenderData& renderData,
                                           VkVertexBufferData& vertexBufferData)
{
    vmaDestroyBuffer(renderData.rdAllocator, vertexBufferData.rdStagingBuffer, vertexBufferData.rdStagingBufferAlloc);
    vmaDestroyBuffer(renderData.rdAllocator, vertexBufferData.rdVertexBuffer, vertexBufferData.rdVertexBufferAlloc);
}

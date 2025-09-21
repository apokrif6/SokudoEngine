#include <cstring>
#include "VertexBuffer.h"
#include "core/vk-renderer/debug/DebugUtils.h"

bool Core::Renderer::VertexBuffer::init(Core::Renderer::VkRenderData &renderData, VkVertexBufferData &vertexBufferData,
                                        unsigned int bufferSize, const std::string& name)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo bufferAllocCreateInfo{};
    bufferAllocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    VmaAllocationInfo bufferAllocInfo{};

    if (vmaCreateBuffer(renderData.rdAllocator, &bufferInfo, &bufferAllocCreateInfo, &vertexBufferData.rdVertexBuffer,
                        &vertexBufferData.rdVertexBufferAlloc, &bufferAllocInfo) != VK_SUCCESS) {
        Logger::log(1, "%s error: could not allocate vertex buffer via VMA\n", __FUNCTION__);
        return false;
    }

    vertexBufferData.rdName = "Vertex Buffer " + name;
    vmaSetAllocationName(renderData.rdAllocator,
                         vertexBufferData.rdVertexBufferAlloc,
                         vertexBufferData.rdName.c_str());

    Core::Renderer::Debug::setObjectName(renderData.rdVkbDevice.device,
                  (uint64_t)vertexBufferData.rdVertexBuffer,
                  VK_OBJECT_TYPE_BUFFER,
                  vertexBufferData.rdName);

    VkBufferCreateInfo stagingBufferInfo{};
    stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    stagingBufferInfo.size = bufferSize;
    stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VmaAllocationCreateInfo stagingAllocCreateInfo{};
    stagingAllocCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

    VmaAllocationInfo stagingAllocInfo{};

    if (vmaCreateBuffer(renderData.rdAllocator, &stagingBufferInfo, &stagingAllocCreateInfo,
                        &vertexBufferData.rdStagingBuffer, &vertexBufferData.rdStagingBufferAlloc,
                        &stagingAllocInfo) != VK_SUCCESS) {
        Logger::log(1, "%s error: could not allocate vertex staging buffer via VMA\n", __FUNCTION__);
        return false;
    }

    vertexBufferData.rdName = "Vertex Staging Buffer " + name;
    vmaSetAllocationName(renderData.rdAllocator,
                         vertexBufferData.rdStagingBufferAlloc,
                         vertexBufferData.rdName.c_str());

    vertexBufferData.rdVertexBufferSize = bufferSize;

    return true;
}

bool Core::Renderer::VertexBuffer::uploadData(Core::Renderer::VkRenderData &renderData,
                                              VkVertexBufferData &vertexBufferData, Core::Renderer::VkMesh vertexData)
{
    unsigned int vertexDataSize = vertexData.vertices.size() * sizeof(VkVertex);

    // buffer too small, should resize
    if (vertexBufferData.rdVertexBufferSize < vertexDataSize) {
        cleanup(renderData, vertexBufferData);

        if (!init(renderData, vertexBufferData, vertexDataSize, vertexBufferData.rdName)) {
            Logger::log(1, "%s error: could not create vertex buffer of size %i bytes\n", __FUNCTION__, vertexDataSize);
            return false;
        }

        Logger::log(1, "%s: vertex buffer resize to %i bytes\n", __FUNCTION__, vertexDataSize);
        vertexBufferData.rdVertexBufferSize = vertexDataSize;
    }

    void *data;
    vmaMapMemory(renderData.rdAllocator, vertexBufferData.rdStagingBufferAlloc, &data);
    std::memcpy(data, vertexData.vertices.data(), vertexDataSize);
    vmaUnmapMemory(renderData.rdAllocator, vertexBufferData.rdStagingBufferAlloc);

    VkBufferCopy stagingBufferCopy{};
    stagingBufferCopy.srcOffset = 0;
    stagingBufferCopy.dstOffset = 0;
    stagingBufferCopy.size = vertexDataSize;

    vkCmdCopyBuffer(renderData.rdCommandBuffer, vertexBufferData.rdStagingBuffer, vertexBufferData.rdVertexBuffer, 1,
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

    vkCmdPipelineBarrier(renderData.rdCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                         0, 0, nullptr, 1, &vertexBufferBarrier, 0, nullptr);

    return true;
}

void Core::Renderer::VertexBuffer::cleanup(Core::Renderer::VkRenderData &renderData,
                                           VkVertexBufferData &vertexBufferData)
{
    vmaDestroyBuffer(renderData.rdAllocator, vertexBufferData.rdStagingBuffer, vertexBufferData.rdStagingBufferAlloc);
    vmaDestroyBuffer(renderData.rdAllocator, vertexBufferData.rdVertexBuffer, vertexBufferData.rdVertexBufferAlloc);
}

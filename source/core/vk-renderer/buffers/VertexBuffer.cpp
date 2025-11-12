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

void Core::Renderer::VertexBuffer::cleanup(Core::Renderer::VkRenderData &renderData,
                                           VkVertexBufferData &vertexBufferData)
{
    vmaDestroyBuffer(renderData.rdAllocator, vertexBufferData.rdStagingBuffer, vertexBufferData.rdStagingBufferAlloc);
    vmaDestroyBuffer(renderData.rdAllocator, vertexBufferData.rdVertexBuffer, vertexBufferData.rdVertexBufferAlloc);
}

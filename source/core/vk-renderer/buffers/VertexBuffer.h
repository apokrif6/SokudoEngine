#pragma once

#include "core/vk-renderer/VkRenderData.h"
#include "core/vk-renderer/debug/DebugRenderer.h"
#include "core/tools/Logger.h"

namespace Core::Renderer
{
class VertexBuffer
{
public:
    static bool init(Core::Renderer::VkRenderData& renderData, VkVertexBufferData& VkVertexBufferData,
                     unsigned int bufferSize, const std::string& name);

    template <typename T>
    static bool uploadData(Core::Renderer::VkRenderData& renderData, VkVertexBufferData& vertexBufferData,
                           const std::vector<T>& vertexData)
    {
        unsigned int vertexDataSize = vertexData.size() * sizeof(T);

        if (vertexBufferData.rdVertexBufferSize < vertexDataSize)
        {
            cleanup(renderData, vertexBufferData);

            if (!init(renderData, vertexBufferData, vertexDataSize, vertexBufferData.rdName))
            {
                Logger::log(1, "%s error: could not create vertex buffer of size %i bytes\n", __FUNCTION__,
                            vertexDataSize);
                return false;
            }
            Logger::log(1, "%s: vertex buffer resize to %i bytes\n", __FUNCTION__, vertexDataSize);
            vertexBufferData.rdVertexBufferSize = vertexDataSize;
        }

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

        vkCmdCopyBuffer(renderData.rdCommandBuffer, vertexBufferData.rdStagingBuffer, vertexBufferData.rdVertexBuffer,
                        1, &stagingBufferCopy);
        vkCmdPipelineBarrier(renderData.rdCommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 0, nullptr, 1, &vertexBufferBarrier, 0, nullptr);

        return true;
    }

    static void cleanup(Core::Renderer::VkRenderData& renderData, VkVertexBufferData& vertexBufferData);
};
} // namespace Core::Renderer
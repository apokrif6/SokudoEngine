#pragma once

#include "engine/Engine.h"
#include "core/vk-renderer/VkRenderData.h"

namespace Core::Renderer
{
class UniformBuffer
{
public:
    static bool init(VkRenderData& renderData, VkUniformBufferData& UBOData, size_t bufferSize, const std::string& name,
                     DescriptorLayoutType layoutType);

    template <typename T> static void uploadData(VkRenderData& renderData, VkUniformBufferData& UBOData, const T& data)
    {
        static_assert(std::is_standard_layout_v<T>, "Data type must be standard layout to be uploaded to GPU");

        void* mappedData;
        vmaMapMemory(renderData.rdAllocator, UBOData.rdUniformBufferAlloc, &mappedData);
        std::memcpy(mappedData, &data, sizeof(T));
        vmaUnmapMemory(renderData.rdAllocator, UBOData.rdUniformBufferAlloc);
    }

    static void cleanup(VkRenderData& renderData, VkUniformBufferData& UBOData);
};
} // namespace Core::Renderer
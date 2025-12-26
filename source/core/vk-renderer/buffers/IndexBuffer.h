#pragma once

#include <vulkan/vulkan.h>
#include "core/vk-renderer/VkRenderData.h"

namespace Core::Renderer
{
class IndexBuffer
{
public:
    static bool init(VkRenderData& renderData, VkIndexBufferData& indexBufferData,
                     unsigned int bufferSize, const std::string& name);

    static bool uploadData(VkRenderData& renderData, VkIndexBufferData& indexBufferData,
                           const std::vector<uint32_t>& indexData);

    static void cleanup(VkRenderData& renderData, VkIndexBufferData& IndexBufferData);
};
} // namespace Core::Renderer
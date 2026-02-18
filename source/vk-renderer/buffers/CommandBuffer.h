#pragma once

#include "vk-renderer/VkRenderData.h"

namespace Core::Renderer
{
class CommandBuffer
{
public:
    static bool init(const VkRenderData& renderData, VkCommandBuffer& commandBuffer);

    static void cleanup(const VkRenderData& renderData, VkCommandBuffer& commandBuffer);
};
} // namespace Core::Renderer
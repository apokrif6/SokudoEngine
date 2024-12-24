#pragma once

#include "VkRenderData.h"

namespace Core::Renderer
{
class CommandBuffer
{
  public:
    static bool init(const Core::Renderer::VkRenderData& renderData, VkCommandBuffer& commandBuffer);

    static void cleanup(const Core::Renderer::VkRenderData& renderData, VkCommandBuffer& commandBuffer);
};
}
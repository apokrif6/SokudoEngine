#pragma once

#include "VkRenderData.h"

class CommandBuffer
{
  public:
    static bool init(VkRenderData& renderData, VkCommandBuffer& commandBuffer);

    static void cleanup(VkRenderData& renderData, VkCommandBuffer& commandBuffer);
};

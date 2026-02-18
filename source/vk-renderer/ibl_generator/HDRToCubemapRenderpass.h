#pragma once

#include "vk-renderer/VkRenderData.h"

namespace Core::Renderer
{
class HDRToCubemapRenderpass
{
public:
    static bool init(VkRenderData& renderData, VkRenderPass& outRenderPass);

    static void cleanup(VkRenderData& renderData, const VkRenderPass& renderPass);
};
} // namespace Core::Renderer

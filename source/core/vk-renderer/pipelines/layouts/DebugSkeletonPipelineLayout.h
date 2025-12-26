#pragma once

#include "core/vk-renderer/VkRenderData.h"

namespace Core::Renderer
{
class DebugSkeletonPipelineLayout
{
public:
    static bool init(VkRenderData& renderData, VkPipelineLayout& pipelineLayout);

    static void cleanup(VkRenderData& renderData, VkPipelineLayout& pipelineLayout);
};
} // namespace Core::Renderer
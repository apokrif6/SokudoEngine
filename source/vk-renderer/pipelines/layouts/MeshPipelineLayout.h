#pragma once

#include "vk-renderer/VkRenderData.h"

namespace Core::Renderer
{
class MeshPipelineLayout
{
public:
    static bool init(VkRenderData& renderData, VkPipelineLayout& pipelineLayout);

    static void cleanup(VkRenderData& renderData, VkPipelineLayout& pipelineLayout);
};
} // namespace Core::Renderer

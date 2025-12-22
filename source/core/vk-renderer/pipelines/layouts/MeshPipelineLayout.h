#pragma once

#include "core/vk-renderer/VkRenderData.h"

namespace Core::Renderer
{
class MeshPipelineLayout
{
public:
    static bool init(Core::Renderer::VkRenderData& renderData, VkPipelineLayout& pipelineLayout);

    static void cleanup(Core::Renderer::VkRenderData& renderData, VkPipelineLayout& pipelineLayout);
};
} // namespace Core::Renderer

#pragma

#include "core/vk-renderer/VkRenderData.h"

namespace Core::Renderer
{
class PipelineLayout
{
public:
    static bool init(Core::Renderer::VkRenderData& renderData, VkTextureData& textureData,
                     VkPipelineLayout& pipelineLayout, const PipelineLayoutConfig& pipelineLayoutConfig = {});

    static void cleanup(Core::Renderer::VkRenderData& renderData, VkPipelineLayout& pipelineLayout);
};
} // namespace Core::Renderer
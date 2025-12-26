#pragma

#include "core/vk-renderer/VkRenderData.h"

namespace Core::Renderer
{
class PipelineLayout
{
public:
    static bool init(VkRenderData& renderData, VkTextureData& textureData,
                     VkPipelineLayout& pipelineLayout, const PipelineLayoutConfig& pipelineLayoutConfig = {});

    static void cleanup(VkRenderData& renderData, VkPipelineLayout& pipelineLayout);
};
} // namespace Core::Renderer
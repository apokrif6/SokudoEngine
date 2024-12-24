#pragma

#include "VkRenderData.h"

namespace Core::Renderer
{
class PipelineLayout
{
  public:
    static bool init(Core::Renderer::VkRenderData& renderData, VkTextureData& textureData, VkPipelineLayout& pipelineLayout);

    static void cleanup(Core::Renderer::VkRenderData& renderData, VkPipelineLayout& pipelineLayout);
};
}
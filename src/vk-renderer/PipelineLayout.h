#pragma

#include "VkRenderData.h"

class PipelineLayout
{
  public:
    static bool init(VkRenderData& renderData, VkTextureData &textureData, VkPipelineLayout& pipelineLayout);

    static void cleanup(VkRenderData& renderData, VkPipelineLayout& pipelineLayout);
};

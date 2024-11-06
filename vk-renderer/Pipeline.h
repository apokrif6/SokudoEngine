#pragma once

#include "VkRenderData.h"

class Pipeline
{
  public:
    static bool init(VkRenderData& renderData, VkPipelineLayout& pipelineLayout, VkPipeline& pipeline,
                     VkPrimitiveTopology topology, const std::string& vertexShaderFilename,
                     const std::string& fragmentShaderFilename);

    static void cleanup(VkRenderData& renderData, VkPipeline& pipeline);
};
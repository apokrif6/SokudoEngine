#pragma once

#include <string>
#include <vulkan/vulkan.h>

#include "VkRenderData.h"

namespace Core::Renderer
{
class GltfPipeline
{
  public:
    static bool init(Core::Renderer::VkRenderData renderData, VkPipelineLayout& pipelineLayout, VkPipeline& pipeline,
                     VkPrimitiveTopology topology, const std::string& vertexShaderFilename,
                     const std::string& fragmentShaderFilename);

    static void cleanup(Core::Renderer::VkRenderData renderData, VkPipeline& pipeline);
};
}
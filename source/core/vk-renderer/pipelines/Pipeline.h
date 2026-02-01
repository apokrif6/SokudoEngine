#pragma once

#include "core/vk-renderer/VkRenderData.h"

namespace Core::Renderer
{
class Pipeline
{
public:
    static bool init(VkRenderData& renderData, VkPipelineLayout& pipelineLayout, VkPipeline& pipeline,
                     VkPrimitiveTopology topology, const std::string& vertexShaderFilename,
                     const std::string& fragmentShaderFilename, const PipelineConfig& config = {},
                     VkRenderPass renderPass = VK_NULL_HANDLE);

    static void cleanup(VkRenderData& renderData, VkPipeline& pipeline);
};
} // namespace Core::Renderer
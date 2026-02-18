#include "PipelineLayout.h"
#include "tools/Logger.h"

#include "VkBootstrap.h"

bool Core::Renderer::PipelineLayout::init(VkRenderData& renderData, VkPipelineLayout& pipelineLayout,
                                          const PipelineLayoutConfig& pipelineLayoutConfig)
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(pipelineLayoutConfig.setLayouts.size());
    pipelineLayoutInfo.pSetLayouts = pipelineLayoutConfig.setLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pipelineLayoutConfig.pushConstantRanges.size());
    pipelineLayoutInfo.pPushConstantRanges = pipelineLayoutConfig.pushConstantRanges.data();

    if (vkCreatePipelineLayout(renderData.rdVkbDevice.device, &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
        VK_SUCCESS)
    {
        Logger::log(1, "%s error: could not create pipeline layout\n", __FUNCTION__);
        return false;
    }

    return true;
}

void Core::Renderer::PipelineLayout::cleanup(VkRenderData& renderData, VkPipelineLayout& pipelineLayout)
{
    vkDestroyPipelineLayout(renderData.rdVkbDevice.device, pipelineLayout, nullptr);
}

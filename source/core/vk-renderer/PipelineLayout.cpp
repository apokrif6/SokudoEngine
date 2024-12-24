#include "PipelineLayout.h"
#include "core/tools/Logger.h"

#include <VkBootstrap.h>

bool Core::Renderer::PipelineLayout::init(Core::Renderer::VkRenderData renderData, VkTextureData& textureData, VkPipelineLayout& pipelineLayout)
{
    VkDescriptorSetLayout layouts[] = {textureData.texTextureDescriptorLayout, renderData.rdUBODescriptorLayout};

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 2;
    pipelineLayoutInfo.pSetLayouts = layouts;
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    if (vkCreatePipelineLayout(renderData.rdVkbDevice.device, &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
        VK_SUCCESS)
    {
        Logger::log(1, "%s error: could not create pipeline layout\n", __FUNCTION__);
        return false;
    }
    return true;
}

void Core::Renderer::PipelineLayout::cleanup(Core::Renderer::VkRenderData renderData, VkPipelineLayout& pipelineLayout)
{
    vkDestroyPipelineLayout(renderData.rdVkbDevice.device, pipelineLayout, nullptr);
}

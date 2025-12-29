#include "DebugSkeletonPipelineLayout.h"
#include "core/tools/Logger.h"

bool Core::Renderer::DebugSkeletonPipelineLayout::init(VkRenderData& renderData, VkPipelineLayout& pipelineLayout)
{
    VkDescriptorSetLayout layouts[] = {renderData.rdGlobalSceneDescriptorLayout};

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = std::size(layouts);
    pipelineLayoutInfo.pSetLayouts = layouts;
    pipelineLayoutInfo.pushConstantRangeCount = 0;

    if (vkCreatePipelineLayout(renderData.rdVkbDevice.device, &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
        VK_SUCCESS)
    {
        Logger::log(1, "%s error: could not create debug skeleton pipeline layout\n", __FUNCTION__);
        return false;
    }

    return true;
}

void Core::Renderer::DebugSkeletonPipelineLayout::cleanup(VkRenderData& renderData, VkPipelineLayout& pipelineLayout)
{
    vkDestroyPipelineLayout(renderData.rdVkbDevice.device, pipelineLayout, nullptr);
}
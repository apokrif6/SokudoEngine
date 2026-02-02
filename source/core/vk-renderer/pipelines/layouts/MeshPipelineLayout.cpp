#include "MeshPipelineLayout.h"
#include "core/tools/Logger.h"

bool Core::Renderer::MeshPipelineLayout::init(VkRenderData& renderData, VkPipelineLayout& pipelineLayout)
{
    const auto& descriptorLayoutCache = renderData.rdDescriptorLayoutCache;

    const VkDescriptorSetLayout layouts[] = {descriptorLayoutCache->getLayout(DescriptorLayoutType::GlobalScene),
                                             descriptorLayoutCache->getLayout(DescriptorLayoutType::PrimitiveData),
                                             descriptorLayoutCache->getLayout(DescriptorLayoutType::PrimitiveTextures),
                                             descriptorLayoutCache->getLayout(DescriptorLayoutType::MaterialData)};

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(PrimitiveFlagsPushConstants);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = std::size(layouts);
    pipelineLayoutInfo.pSetLayouts = layouts;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(renderData.rdVkbDevice.device, &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
        VK_SUCCESS)
    {
        Logger::log(1, "%s error: failed to create pipeline layout\n", __FUNCTION__);
        return false;
    }

    return true;
}

void Core::Renderer::MeshPipelineLayout::cleanup(VkRenderData& renderData, VkPipelineLayout& pipelineLayout)
{
    vkDestroyPipelineLayout(renderData.rdVkbDevice.device, pipelineLayout, nullptr);
}
#include "MeshPipelineLayout.h"
#include "core/tools/Logger.h"

bool Core::Renderer::MeshPipelineLayout::init(VkRenderData& renderData, VkPipelineLayout& pipelineLayout)
{
    auto& descriptorLayoutCache = renderData.rdDescriptorLayoutCache;

    renderData.rdGlobalSceneDescriptorLayout = descriptorLayoutCache->createDescriptorLayout(
        {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT},
         {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT},
         {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT},
         {3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT}});

    renderData.rdPrimitiveDataDescriptorLayout = descriptorLayoutCache->createDescriptorLayout(
        {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT}});

    std::vector<VkDescriptorSetLayoutBinding> textureBindings(5);
    for (size_t i = 0; i < textureBindings.size(); ++i)
    {
        textureBindings[i].binding = static_cast<uint32_t>(i);
        textureBindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        textureBindings[i].descriptorCount = 1;
        textureBindings[i].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    }
    renderData.rdPrimitiveTextureDescriptorLayout = descriptorLayoutCache->createDescriptorLayout(textureBindings);

    renderData.rdPrimitiveMaterialDescriptorLayout = descriptorLayoutCache->createDescriptorLayout(
        {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT}});

    const VkDescriptorSetLayout layouts[] = {
        renderData.rdGlobalSceneDescriptorLayout, renderData.rdPrimitiveDataDescriptorLayout,
        renderData.rdPrimitiveTextureDescriptorLayout, renderData.rdPrimitiveMaterialDescriptorLayout};

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
#include "MeshPipelineLayout.h"
#include "core/tools/Logger.h"

bool Core::Renderer::MeshPipelineLayout::init(VkRenderData& renderData, VkPipelineLayout& pipelineLayout)
{
    std::vector<VkDescriptorSetLayoutBinding> sceneBindings = {
        {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT},
        {1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT},
        {2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT},
        {3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT}};

    VkDescriptorSetLayoutCreateInfo sceneLayoutInfo{};
    sceneLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    sceneLayoutInfo.bindingCount = static_cast<uint32_t>(sceneBindings.size());
    sceneLayoutInfo.pBindings = sceneBindings.data();

    if (vkCreateDescriptorSetLayout(renderData.rdVkbDevice.device, &sceneLayoutInfo, nullptr,
                                    &renderData.rdGlobalSceneDescriptorLayout) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: failed to create scene descriptor layout\n", __FUNCTION__);
        return false;
    }

    VkDescriptorSetLayoutBinding primitiveDataBinding{};
    primitiveDataBinding.binding = 0;
    primitiveDataBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    primitiveDataBinding.descriptorCount = 1;
    primitiveDataBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo primitiveDataLayoutInfo{};
    primitiveDataLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    primitiveDataLayoutInfo.bindingCount = 1;
    primitiveDataLayoutInfo.pBindings = &primitiveDataBinding;

    if (vkCreateDescriptorSetLayout(renderData.rdVkbDevice.device, &primitiveDataLayoutInfo, nullptr,
                                    &renderData.rdPrimitiveDataDescriptorLayout) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: failed to create primitive data descriptor layout\n", __FUNCTION__);
        return false;
    }

    std::vector<VkDescriptorSetLayoutBinding> textureBindings(5);
    for (size_t i = 0; i < textureBindings.size(); ++i)
    {
        textureBindings[i].binding = i;
        textureBindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        textureBindings[i].descriptorCount = 1;
        textureBindings[i].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    }

    VkDescriptorSetLayoutCreateInfo textureLayoutInfo{};
    textureLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    textureLayoutInfo.bindingCount = static_cast<uint32_t>(textureBindings.size());
    textureLayoutInfo.pBindings = textureBindings.data();

    if (vkCreateDescriptorSetLayout(renderData.rdVkbDevice.device, &textureLayoutInfo, nullptr,
                                    &renderData.rdPrimitiveTextureDescriptorLayout) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: failed to create texture descriptor layout\n", __FUNCTION__);
        return false;
    }

    VkDescriptorSetLayoutBinding materialBinding{};
    materialBinding.binding = 0;
    materialBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    materialBinding.descriptorCount = 1;
    materialBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo materialLayoutInfo{};
    materialLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    materialLayoutInfo.bindingCount = 1;
    materialLayoutInfo.pBindings = &materialBinding;

    if (vkCreateDescriptorSetLayout(renderData.rdVkbDevice.device, &materialLayoutInfo, nullptr,
                                    &renderData.rdPrimitiveMaterialDescriptorLayout) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: failed to create material descriptor layout\n", __FUNCTION__);
        return false;
    }

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
    vkDestroyDescriptorSetLayout(renderData.rdVkbDevice.device, renderData.rdGlobalSceneDescriptorLayout, nullptr);
    vkDestroyDescriptorSetLayout(renderData.rdVkbDevice.device, renderData.rdPrimitiveDataDescriptorLayout, nullptr);
    vkDestroyDescriptorSetLayout(renderData.rdVkbDevice.device, renderData.rdPrimitiveTextureDescriptorLayout, nullptr);
    vkDestroyDescriptorSetLayout(renderData.rdVkbDevice.device, renderData.rdPrimitiveMaterialDescriptorLayout,
                                 nullptr);
    vkDestroyPipelineLayout(renderData.rdVkbDevice.device, pipelineLayout, nullptr);
}
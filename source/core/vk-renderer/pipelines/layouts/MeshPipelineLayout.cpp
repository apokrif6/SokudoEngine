#include "MeshPipelineLayout.h"
#include "core/tools/Logger.h"

bool Core::Renderer::MeshPipelineLayout::init(Core::Renderer::VkRenderData& renderData,
                                              VkPipelineLayout& pipelineLayout)
{
    VkDescriptorSetLayoutBinding textureBinding{};
    textureBinding.binding = 0;
    textureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    textureBinding.descriptorCount = 1;
    textureBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo textureLayoutInfo{};
    textureLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    textureLayoutInfo.bindingCount = 1;
    textureLayoutInfo.pBindings = &textureBinding;

    if (vkCreateDescriptorSetLayout(renderData.rdVkbDevice.device, &textureLayoutInfo, nullptr,
                                    &renderData.rdMeshTextureDescriptorLayout) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: failed to create texture descriptor layout\n", __FUNCTION__);
        return false;
    }

    VkDescriptorSetLayoutBinding viewBinding{};
    viewBinding.binding = 0;
    viewBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    viewBinding.descriptorCount = 1;
    viewBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo viewLayoutInfo{};
    viewLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    viewLayoutInfo.bindingCount = 1;
    viewLayoutInfo.pBindings = &viewBinding;

    if (vkCreateDescriptorSetLayout(renderData.rdVkbDevice.device, &viewLayoutInfo, nullptr,
                                    &renderData.rdMeshViewMatrixDescriptorLayout) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: failed to create view matrix descriptor layout\n", __FUNCTION__);
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
                                    &renderData.rdMeshMaterialDescriptorLayout) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: failed to create material descriptor layout\n", __FUNCTION__);
        return false;
    }

    VkDescriptorSetLayout layouts[] = {renderData.rdMeshTextureDescriptorLayout,
                                       renderData.rdMeshViewMatrixDescriptorLayout,
                                       renderData.rdMeshMaterialDescriptorLayout};

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 3;
    pipelineLayoutInfo.pSetLayouts = layouts;

    if (vkCreatePipelineLayout(renderData.rdVkbDevice.device, &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
        VK_SUCCESS)
    {
        Logger::log(1, "%s error: failed to create pipeline layout\n", __FUNCTION__);
        return false;
    }

    return true;
}

void Core::Renderer::MeshPipelineLayout::cleanup(Core::Renderer::VkRenderData& renderData,
                                                 VkPipelineLayout& pipelineLayout)
{
    vkDestroyDescriptorSetLayout(renderData.rdVkbDevice.device, renderData.rdMeshTextureDescriptorLayout, nullptr);
    vkDestroyDescriptorSetLayout(renderData.rdVkbDevice.device, renderData.rdMeshViewMatrixDescriptorLayout, nullptr);
    vkDestroyDescriptorSetLayout(renderData.rdVkbDevice.device, renderData.rdMeshMaterialDescriptorLayout, nullptr);
    vkDestroyPipelineLayout(renderData.rdVkbDevice.device, pipelineLayout, nullptr);
}
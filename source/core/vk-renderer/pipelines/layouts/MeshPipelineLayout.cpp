#include "MeshPipelineLayout.h"
#include "core/tools/Logger.h"

bool Core::Renderer::MeshPipelineLayout::init(VkRenderData& renderData,
                                              VkPipelineLayout& pipelineLayout)
{
    std::array<VkDescriptorSetLayoutBinding, 5> textureBindings{};

    for (uint32_t i = 0; i < textureBindings.size(); i++)
    {
        textureBindings[i].binding = i;
        textureBindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        textureBindings[i].descriptorCount = 1;
        textureBindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    }

    VkDescriptorSetLayoutCreateInfo textureLayoutInfo{};
    textureLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    textureLayoutInfo.bindingCount = static_cast<uint32_t>(textureBindings.size());
    textureLayoutInfo.pBindings = textureBindings.data();

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

    VkDescriptorSetLayoutBinding bonesBinding{};
    bonesBinding.binding = 0;
    bonesBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bonesBinding.descriptorCount = 1;
    bonesBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo bonesLayoutInfo{};
    bonesLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    bonesLayoutInfo.bindingCount = 1;
    bonesLayoutInfo.pBindings = &bonesBinding;

    if (vkCreateDescriptorSetLayout(renderData.rdVkbDevice.device, &bonesLayoutInfo, nullptr,
                                    &renderData.rdMeshBonesTransformDescriptorLayout) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: failed to create bones descriptor layout\n", __FUNCTION__);
        return false;
    }

    VkDescriptorSetLayoutBinding modelBinding{};
    modelBinding.binding = 0;
    modelBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    modelBinding.descriptorCount = 1;
    modelBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo modelLayoutInfo{};
    modelLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    modelLayoutInfo.bindingCount = 1;
    modelLayoutInfo.pBindings = &modelBinding;

    if (vkCreateDescriptorSetLayout(renderData.rdVkbDevice.device, &modelLayoutInfo, nullptr,
                                    &renderData.rdMeshModelDescriptorLayout) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: failed to create model descriptor layout\n", __FUNCTION__);
        return false;
    }

    VkDescriptorSetLayoutBinding cameraBinding{};
    cameraBinding.binding = 0;
    cameraBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    cameraBinding.descriptorCount = 1;
    cameraBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo cameraLayoutInfo{};
    cameraLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    cameraLayoutInfo.bindingCount = 1;
    cameraLayoutInfo.pBindings = &cameraBinding;

    if (vkCreateDescriptorSetLayout(renderData.rdVkbDevice.device, &cameraLayoutInfo, nullptr,
                                    &renderData.rdMeshCameraDescriptorLayout) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: failed to create camera descriptor layout\n", __FUNCTION__);
        return false;
    }

    VkDescriptorSetLayoutBinding lightsBinding{};
    lightsBinding.binding = 0;
    lightsBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    lightsBinding.descriptorCount = 1;
    lightsBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo lightsLayoutInfo{};
    lightsLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    lightsLayoutInfo.bindingCount = 1;
    lightsLayoutInfo.pBindings = &lightsBinding;

    if (vkCreateDescriptorSetLayout(renderData.rdVkbDevice.device, &lightsLayoutInfo, nullptr,
                                    &renderData.rdMeshLightsDescriptorLayout) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: failed to create lights descriptor layout\n", __FUNCTION__);
        return false;
    }
    
    VkDescriptorSetLayoutBinding environmentMapBinding{};
    environmentMapBinding.binding = 0;
    environmentMapBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    environmentMapBinding.descriptorCount = 1;
    environmentMapBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo environmentMapLayoutInfo{};
    environmentMapLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    environmentMapLayoutInfo.bindingCount = 1;
    environmentMapLayoutInfo.pBindings = &environmentMapBinding;

    if (vkCreateDescriptorSetLayout(renderData.rdVkbDevice.device, &environmentMapLayoutInfo, nullptr,
                                    &renderData.rdMeshEnvironmentMapDescriptorLayout) != VK_SUCCESS)
    {
        Logger::log(1, "%s error: failed to create environment map descriptor layout\n", __FUNCTION__);
        return false;
    }

    VkDescriptorSetLayout layouts[] = {
        renderData.rdMeshTextureDescriptorLayout,  renderData.rdMeshViewMatrixDescriptorLayout,
        renderData.rdMeshMaterialDescriptorLayout, renderData.rdMeshBonesTransformDescriptorLayout,
        renderData.rdMeshModelDescriptorLayout,    renderData.rdMeshCameraDescriptorLayout,
        renderData.rdMeshLightsDescriptorLayout,   renderData.rdMeshEnvironmentMapDescriptorLayout};

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
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

void Core::Renderer::MeshPipelineLayout::cleanup(VkRenderData& renderData,
                                                 VkPipelineLayout& pipelineLayout)
{
    vkDestroyDescriptorSetLayout(renderData.rdVkbDevice.device, renderData.rdMeshTextureDescriptorLayout, nullptr);
    vkDestroyDescriptorSetLayout(renderData.rdVkbDevice.device, renderData.rdMeshViewMatrixDescriptorLayout, nullptr);
    vkDestroyDescriptorSetLayout(renderData.rdVkbDevice.device, renderData.rdMeshMaterialDescriptorLayout, nullptr);
    vkDestroyDescriptorSetLayout(renderData.rdVkbDevice.device, renderData.rdMeshBonesTransformDescriptorLayout,
                                 nullptr);
    vkDestroyDescriptorSetLayout(renderData.rdVkbDevice.device, renderData.rdMeshModelDescriptorLayout, nullptr);
    vkDestroyDescriptorSetLayout(renderData.rdVkbDevice.device, renderData.rdMeshCameraDescriptorLayout, nullptr);
    vkDestroyDescriptorSetLayout(renderData.rdVkbDevice.device, renderData.rdMeshLightsDescriptorLayout, nullptr);
    vkDestroyDescriptorSetLayout(renderData.rdVkbDevice.device, renderData.rdMeshEnvironmentMapDescriptorLayout, nullptr);
    vkDestroyPipelineLayout(renderData.rdVkbDevice.device, pipelineLayout, nullptr);
}
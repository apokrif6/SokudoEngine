#include "Primitive.h"
#include "core/vk-renderer/buffers/UniformBuffer.h"
#include "Texture.h"
#include "core/scene/objects/Mesh.h"

Core::Renderer::Primitive::Primitive(const std::vector<NewVertex>& vertexBufferData,
                                     const std::vector<uint32_t>& indexBufferData,
                                     const std::unordered_map<aiTextureType, VkTextureData>& textures,
                                     const MaterialInfo& materialInfo,
                                     const Animations::BonesInfo& bonesInfo,
                                     VkRenderData& renderData, VkDescriptorSet materialDescriptorSet)
    : mVertexBufferData(vertexBufferData), mIndexBufferData(indexBufferData), mTextures(textures),
      mMaterialInfo(materialInfo), mBonesInfo(bonesInfo), mMaterialDescriptorSet(materialDescriptorSet)
{
    auto foundAlbedoTexture = mTextures.find(aiTextureType_DIFFUSE);
    if (foundAlbedoTexture != mTextures.end())
    {
        mAlbedoTexture = foundAlbedoTexture->second;
    }
    else
    {
        auto foundBaseColorTexture = mTextures.find(aiTextureType_BASE_COLOR);
        if (foundBaseColorTexture != mTextures.end())
        {
            mAlbedoTexture = foundBaseColorTexture->second;
        }
    }

    createVertexBuffer(renderData);
    createIndexBuffer(renderData);
    createMaterialBuffer(renderData);
    createBonesTransformBuffer(renderData);
    createModelMatrixBuffer(renderData);
    createCameraBuffer(renderData);
    createLightsBuffer(renderData);

    primitiveFlagsPushConstants.hasSkinning = mBonesInfo.bones.empty() ? 0 : 1;
}

void Core::Renderer::Primitive::createVertexBuffer(VkRenderData& renderData)
{
    VertexBuffer::init(renderData, primitiveRenderData.rdModelVertexBufferData,
                                       mVertexBufferData.size() * sizeof(NewVertex), "Primitive");
}

void Core::Renderer::Primitive::createIndexBuffer(VkRenderData& renderData)
{
    IndexBuffer::init(renderData, primitiveRenderData.rdModelIndexBufferData,
                                      static_cast<int64_t>(mIndexBufferData.size()), "Primitive");
}

void Core::Renderer::Primitive::createMaterialBuffer(VkRenderData& renderData)
{
    UniformBuffer::init(renderData, mMaterialUBO, sizeof(MaterialInfo), "Material");

    UniformBuffer::uploadData(renderData, mMaterialUBO, mMaterialInfo);
}

void Core::Renderer::Primitive::createBonesTransformBuffer(VkRenderData& renderData)
{
    UniformBuffer::init(renderData, mBonesTransformUBO,
                                        mBonesInfo.finalTransforms.size() * sizeof(glm::mat4), "BonesTransform");
}

void Core::Renderer::Primitive::createModelMatrixBuffer(VkRenderData& renderData)
{
    UniformBuffer::init(renderData, mModelUBO, sizeof(glm::mat4), "ModelMatrix");
}

void Core::Renderer::Primitive::createCameraBuffer(VkRenderData& renderData)
{
    UniformBuffer::init(renderData, mCameraUBO, sizeof(CameraInfo), "Camera");
}

void Core::Renderer::Primitive::createLightsBuffer(VkRenderData& renderData)
{
    UniformBuffer::init(renderData, mLightsUBO, sizeof(LightsInfo), "Lights");

    // dummy lights
    lightsData.positions[0] = glm::vec4(0.f, 10.f, 10.f, 1.f);
    lightsData.colors[0] = glm::vec4(1.f, 1.f, 1.f, 1.f);
    lightsData.count = glm::ivec4(1, 0, 0, 0);

    for (int i = 1; i < MAX_LIGHTS; ++i)
    {
        lightsData.positions[i] = glm::vec4(0.f);
        lightsData.colors[i] = glm::vec4(0.f);
    }

    UniformBuffer::uploadData(renderData, mLightsUBO, lightsData);
}

void Core::Renderer::Primitive::uploadVertexBuffer(VkRenderData& renderData)
{
    VertexBuffer::uploadData(renderData, primitiveRenderData.rdModelVertexBufferData,
                                             mVertexBufferData);
}

void Core::Renderer::Primitive::uploadIndexBuffer(VkRenderData& renderData)
{
    IndexBuffer::uploadData(renderData, primitiveRenderData.rdModelIndexBufferData, mIndexBufferData);
}

void Core::Renderer::Primitive::uploadUniformBuffer(VkRenderData& renderData,
                                                    const glm::mat4& modelMatrix)
{
    UniformBuffer::uploadData(renderData, mBonesTransformUBO, mBonesInfo.finalTransforms);

    UniformBuffer::uploadData(renderData, mModelUBO, modelMatrix);

    CameraInfo cameraInfo{};
    cameraInfo.position = glm::vec4(renderData.rdCameraWorldPosition, 1.f);
    UniformBuffer::uploadData(renderData, mCameraUBO, cameraInfo);
}

void Core::Renderer::Primitive::draw(const VkRenderData& renderData)
{
    vkCmdBindPipeline(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderData.rdMeshPipeline);

    vkCmdBindDescriptorSets(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            renderData.rdMeshPipelineLayout, 0, 1, &mMaterialDescriptorSet, 0, nullptr);

    vkCmdBindDescriptorSets(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            renderData.rdMeshPipelineLayout, 1, 1,
                            &renderData.rdPerspectiveViewMatrixUBO.rdUBODescriptorSet, 0, nullptr);

    vkCmdBindDescriptorSets(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            renderData.rdMeshPipelineLayout, 2, 1, &mMaterialUBO.rdUBODescriptorSet, 0, nullptr);

    const VkDescriptorSet& bonesSet = mBonesInfo.bones.empty() ? renderData.rdDummyBonesUBO.rdUBODescriptorSet
                                                               : mBonesTransformUBO.rdUBODescriptorSet;

    vkCmdBindDescriptorSets(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            renderData.rdMeshPipelineLayout, 3, 1, &bonesSet, 0, nullptr);

    vkCmdBindDescriptorSets(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            renderData.rdMeshPipelineLayout, 4, 1, &mModelUBO.rdUBODescriptorSet, 0, nullptr);

    vkCmdBindDescriptorSets(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            renderData.rdMeshPipelineLayout, 5, 1, &mCameraUBO.rdUBODescriptorSet, 0, nullptr);

    vkCmdBindDescriptorSets(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            renderData.rdMeshPipelineLayout, 6, 1, &mLightsUBO.rdUBODescriptorSet, 0, nullptr);

    vkCmdBindDescriptorSets(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                        renderData.rdMeshPipelineLayout, 7, 1, &renderData.rdIrradianceMap.descriptorSet, 0, nullptr);

    vkCmdBindDescriptorSets(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                        renderData.rdMeshPipelineLayout, 8, 1, &renderData.rdPrefilterMap.descriptorSet, 0, nullptr);

    vkCmdBindDescriptorSets(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                        renderData.rdMeshPipelineLayout, 9, 1, &renderData.rdBRDFLUT.descriptorSet, 0, nullptr);

    vkCmdPushConstants(renderData.rdCommandBuffer, renderData.rdMeshPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                       sizeof(PrimitiveFlagsPushConstants), &primitiveFlagsPushConstants);

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(renderData.rdCommandBuffer, 0, 1,
                           &primitiveRenderData.rdModelVertexBufferData.rdVertexBuffer, &offset);

    vkCmdBindIndexBuffer(renderData.rdCommandBuffer, primitiveRenderData.rdModelIndexBufferData.rdIndexBuffer, 0,
                         VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(renderData.rdCommandBuffer, static_cast<uint32_t>(mIndexBufferData.size()), 1, 0, 0, 0);
}

void Core::Renderer::Primitive::cleanup(VkRenderData& renderData)
{
    VertexBuffer::cleanup(renderData, primitiveRenderData.rdModelVertexBufferData);

    IndexBuffer::cleanup(renderData, primitiveRenderData.rdModelIndexBufferData);

    for (auto texture : mTextures)
    {
        Texture::cleanup(renderData, texture.second);
    }

    UniformBuffer::cleanup(renderData, mModelUBO);
    UniformBuffer::cleanup(renderData, mBonesTransformUBO);
    UniformBuffer::cleanup(renderData, mMaterialUBO);
    UniformBuffer::cleanup(renderData, mCameraUBO);
    UniformBuffer::cleanup(renderData, mLightsUBO);
}

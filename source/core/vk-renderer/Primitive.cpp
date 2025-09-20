#include "Primitive.h"
#include "core/vk-renderer/buffers/UniformBuffer.h"
#include "Texture.h"
#include "Mesh.h"

Core::Renderer::Primitive::Primitive(const std::vector<Core::Renderer::NewVertex>& vertexBufferData,
                                     const std::vector<uint32_t>& indexBufferData,
                                     const Core::Renderer::VkTextureData& textureData,
                                     const Core::Renderer::MaterialInfo& materialInfo,
                                     const Core::Animations::BonesInfo& bonesInfo,
                                     Core::Renderer::VkRenderData& renderData)
    : mVertexBufferData(vertexBufferData), mIndexBufferData(indexBufferData), mTextureData(textureData),
      mMaterialInfo(materialInfo), mBonesInfo(bonesInfo)
{
    createVertexBuffer(renderData);
    createIndexBuffer(renderData);
    createMaterialBuffer(renderData);
    createBonesTransformBuffer(renderData);
    createModelMatrixBuffer(renderData);
}

void Core::Renderer::Primitive::createVertexBuffer(Core::Renderer::VkRenderData& renderData)
{
    Core::Renderer::VertexBuffer::init(renderData, primitiveRenderData.rdModelVertexBufferData,
                                       mVertexBufferData.size() * sizeof(Core::Renderer::NewVertex), "Primitive");
}

void Core::Renderer::Primitive::createIndexBuffer(Core::Renderer::VkRenderData& renderData)
{
    Core::Renderer::IndexBuffer::init(renderData, primitiveRenderData.rdModelIndexBufferData,
                                      static_cast<int64_t>(mIndexBufferData.size()), "Primitive");
}

void Core::Renderer::Primitive::createMaterialBuffer(Core::Renderer::VkRenderData& renderData)
{
    Core::Renderer::UniformBuffer::init(renderData, mMaterialUBO, sizeof(Core::Renderer::MaterialInfo), "Material");

    Core::Renderer::UniformBuffer::uploadData(renderData, mMaterialUBO, mMaterialInfo);
}

void Core::Renderer::Primitive::createBonesTransformBuffer(Core::Renderer::VkRenderData& renderData)
{
    Core::Renderer::UniformBuffer::init(renderData, mBonesTransformUBO,
                                        mBonesInfo.finalTransforms.size() * sizeof(glm::mat4), "BonesTransform");
}

void Core::Renderer::Primitive::createModelMatrixBuffer(Core::Renderer::VkRenderData& renderData)
{
    Core::Renderer::UniformBuffer::init(renderData, mModelUBO, sizeof(glm::mat4), "ModelMatrix");
}

void Core::Renderer::Primitive::uploadVertexBuffer(Core::Renderer::VkRenderData& renderData)
{
    Core::Renderer::VertexBuffer::uploadData(renderData, primitiveRenderData.rdModelVertexBufferData,
                                             mVertexBufferData);
}

void Core::Renderer::Primitive::uploadIndexBuffer(Core::Renderer::VkRenderData& renderData)
{
    Core::Renderer::IndexBuffer::uploadData(renderData, primitiveRenderData.rdModelIndexBufferData, mIndexBufferData);
}

void Core::Renderer::Primitive::uploadUniformBuffer(Core::Renderer::VkRenderData& renderData, const glm::mat4& modelMatrix)
{
    Core::Renderer::UniformBuffer::uploadData(renderData, mBonesTransformUBO, mBonesInfo.finalTransforms);

    Core::Renderer::UniformBuffer::uploadData(renderData, mModelUBO, modelMatrix);
}

void Core::Renderer::Primitive::draw(const Core::Renderer::VkRenderData& renderData)
{
    vkCmdBindPipeline(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderData.rdMeshPipeline);

    if (mMaterialInfo.useTexture)
    {
        vkCmdBindDescriptorSets(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                renderData.rdMeshPipelineLayout, 0, 1, &mTextureData.texTextureDescriptorSet, 0,
                                nullptr);
    }
    else
    {
        // if material don't use texture, create dummy descriptor set with default texture
        vkCmdBindDescriptorSets(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                renderData.rdMeshPipelineLayout, 0, 1,
                                &renderData.rdPlaceholderTexture.texTextureDescriptorSet, 0, nullptr);
    }

    vkCmdBindDescriptorSets(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            renderData.rdMeshPipelineLayout, 1, 1,
                            &renderData.rdPerspectiveViewMatrixUBO.rdUBODescriptorSet, 0, nullptr);

    vkCmdBindDescriptorSets(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            renderData.rdMeshPipelineLayout, 2, 1, &mMaterialUBO.rdUBODescriptorSet, 0, nullptr);

    vkCmdBindDescriptorSets(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            renderData.rdMeshPipelineLayout, 3, 1, &mBonesTransformUBO.rdUBODescriptorSet, 0, nullptr);

    vkCmdBindDescriptorSets(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            renderData.rdMeshPipelineLayout, 4, 1, &mModelUBO.rdUBODescriptorSet, 0, nullptr);

    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(renderData.rdCommandBuffer, 0, 1,
                           &primitiveRenderData.rdModelVertexBufferData.rdVertexBuffer, &offset);

    vkCmdBindIndexBuffer(renderData.rdCommandBuffer, primitiveRenderData.rdModelIndexBufferData.rdIndexBuffer, 0,
                         VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(renderData.rdCommandBuffer, static_cast<int64_t>(mIndexBufferData.size()), 1, 0, 0, 0);
}

void Core::Renderer::Primitive::cleanup(Core::Renderer::VkRenderData& renderData)
{
    Core::Renderer::VertexBuffer::cleanup(renderData, primitiveRenderData.rdModelVertexBufferData);

    Core::Renderer::IndexBuffer::cleanup(renderData, primitiveRenderData.rdModelIndexBufferData);

    Core::Renderer::Texture::cleanup(renderData, mTextureData);

    Core::Renderer::UniformBuffer::cleanup(renderData, mModelUBO);
    Core::Renderer::UniformBuffer::cleanup(renderData, mBonesTransformUBO);
    Core::Renderer::UniformBuffer::cleanup(renderData, mMaterialUBO);
}

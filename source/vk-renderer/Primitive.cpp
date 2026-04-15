#include "Primitive.h"
#include "vk-renderer/buffers/UniformBuffer.h"
#include "buffers/IndexBuffer.h"
#include "buffers/VertexBuffer.h"

Core::Renderer::Primitive::Primitive(
    const std::vector<Vertex>& vertexBufferData, const std::vector<uint32_t>& indexBufferData,
    const std::unordered_map<aiTextureType, std::shared_ptr<Assets::TextureAsset>>& textures,
    const MaterialInfo& materialInfo, VkDescriptorSet materialDescriptorSet, const Animations::BonesInfo& bonesInfo,
    VkRenderData& renderData)
    : mVertexBufferData(vertexBufferData), mIndexBufferData(indexBufferData), mTextures(textures),
      mMaterialInfo(materialInfo), mMaterialDescriptorSet(materialDescriptorSet), mBonesInfo(bonesInfo)
{
    createVertexBuffer(renderData);
    createIndexBuffer(renderData);
    createMaterialBuffer(renderData);
    createPrimitiveDataBuffer(renderData);

    primitiveFlagsPushConstants.hasSkinning = mBonesInfo.bones.empty() ? 0 : 1;
}

void Core::Renderer::Primitive::createVertexBuffer(VkRenderData& renderData)
{
    VertexBuffer::init(renderData, primitiveRenderData.rdModelVertexBufferData,
                       mVertexBufferData.size() * sizeof(Vertex), "Primitive");
}

void Core::Renderer::Primitive::createIndexBuffer(VkRenderData& renderData)
{
    IndexBuffer::init(renderData, primitiveRenderData.rdModelIndexBufferData,
                      static_cast<int64_t>(mIndexBufferData.size()), "Primitive");
}

void Core::Renderer::Primitive::createMaterialBuffer(VkRenderData& renderData)
{
    UniformBuffer::init(renderData, mMaterialUBO, sizeof(MaterialInfo), "Material", DescriptorLayoutType::MaterialData);

    UniformBuffer::uploadData(renderData, mMaterialUBO, mMaterialInfo);
}

void Core::Renderer::Primitive::createPrimitiveDataBuffer(VkRenderData& renderData)
{
    UniformBuffer::init(renderData, mPrimitiveDataUBO, sizeof(PrimitiveData), "Primitive Data",
                        DescriptorLayoutType::PrimitiveData);
}

void Core::Renderer::Primitive::uploadVertexBuffer(VkRenderData& renderData)
{
    VertexBuffer::uploadData(renderData, primitiveRenderData.rdModelVertexBufferData, mVertexBufferData);
}

void Core::Renderer::Primitive::uploadIndexBuffer(VkRenderData& renderData)
{
    IndexBuffer::uploadData(renderData, primitiveRenderData.rdModelIndexBufferData, mIndexBufferData);
}

void Core::Renderer::Primitive::uploadUniformBuffer(VkRenderData& renderData, const glm::mat4& modelMatrix)
{
    PrimitiveData data{};
    data.model = modelMatrix;

    if (!mBonesInfo.finalTransforms.empty())
    {
        std::ranges::copy(mBonesInfo.finalTransforms, std::begin(data.bones));
    }

    UniformBuffer::uploadData(renderData, mPrimitiveDataUBO, data);
}

void Core::Renderer::Primitive::draw(const VkRenderData& renderData, PrimitiveRenderType renderType)
{
    VkPipeline pipeline;
    VkPipelineLayout layout;
    if (renderType == Sprite)
    {
        pipeline = renderData.rdSpritePipeline;
        layout = renderData.rdSpritePipelineLayout;
    }
    else
    {
        pipeline = renderData.rdMeshPipeline;
        layout = renderData.rdMeshPipelineLayout;
    }
    vkCmdBindPipeline(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    vkCmdBindDescriptorSets(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1,
                            &renderData.rdGlobalSceneUBO.rdUBODescriptorSet, 0, nullptr);

    vkCmdBindDescriptorSets(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 1, 1,
                            &mPrimitiveDataUBO.rdUBODescriptorSet, 0, nullptr);

    if (renderType == Sprite)
    {
        vkCmdBindDescriptorSets(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 2, 1,
                                &mMaterialDescriptorSet, 0, nullptr);
    }
    else
    {
        vkCmdBindDescriptorSets(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 2, 1,
                                &mMaterialDescriptorSet, 0, nullptr);

        vkCmdBindDescriptorSets(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 3, 1,
                                &mMaterialUBO.rdUBODescriptorSet, 0, nullptr);

        vkCmdPushConstants(renderData.rdCommandBuffer, layout,
                           VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                           sizeof(PrimitiveFlagsPushConstants), &primitiveFlagsPushConstants);
    }
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

    UniformBuffer::cleanup(renderData, mMaterialUBO);
    UniformBuffer::cleanup(renderData, mPrimitiveDataUBO);
}

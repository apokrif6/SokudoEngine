#include "Primitive.h"
#include "core/vk-renderer/buffers/UniformBuffer.h"
#include "Texture.h"

Core::Renderer::Primitive::Primitive(const std::vector<Core::Renderer::NewVertex>& vertexBufferData,
                                     const std::vector<uint32_t>& indexBufferData,
                                     const Core::Renderer::VkTextureData& textureData,
                                     const Core::Renderer::MaterialInfo& materialInfo,
                                     Core::Renderer::VkRenderData& renderData)
    : mVertexBufferData(vertexBufferData), mIndexBufferData(indexBufferData), mTextureData(textureData),
      mMaterialInfo(materialInfo)
{
    createVertexBuffer(renderData);
    createIndexBuffer(renderData);
    createMaterialBuffer(renderData);
}

void Core::Renderer::Primitive::createVertexBuffer(Core::Renderer::VkRenderData& renderData)
{
    Core::Renderer::VertexBuffer::init(renderData, primitiveRenderData.rdModelVertexBufferData,
                                       mVertexBufferData.size() * sizeof(Core::Renderer::NewVertex));
}

void Core::Renderer::Primitive::createIndexBuffer(Core::Renderer::VkRenderData& renderData)
{
    Core::Renderer::IndexBuffer::init(renderData, primitiveRenderData.rdModelIndexBufferData,
                                      static_cast<int64_t>(mIndexBufferData.size()));
}

void Core::Renderer::Primitive::createMaterialBuffer(Core::Renderer::VkRenderData& renderData)
{
    Core::Renderer::UniformBuffer::init(renderData, mMaterialUBO, sizeof(Core::Renderer::MaterialInfo));

    Core::Renderer::UniformBuffer::uploadData(renderData, mMaterialUBO, mMaterialInfo);
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

void Core::Renderer::Primitive::draw(const Core::Renderer::VkRenderData& renderData)
{
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
                                &renderData.rdModelTexture.texTextureDescriptorSet, 0, nullptr);
    }

    vkCmdBindDescriptorSets(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            renderData.rdMeshPipelineLayout, 1, 1,
                            &renderData.rdPerspectiveViewMatrixUBO.rdUBODescriptorSet, 0, nullptr);

    vkCmdBindDescriptorSets(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            renderData.rdMeshPipelineLayout, 2, 1, &mMaterialUBO.rdUBODescriptorSet, 0, nullptr);

    VkDeviceSize offsets[1] = {0};
    vkCmdBindVertexBuffers(renderData.rdCommandBuffer, 0, 1,
                           &primitiveRenderData.rdModelVertexBufferData.rdVertexBuffer, offsets);

    vkCmdBindIndexBuffer(renderData.rdCommandBuffer, primitiveRenderData.rdModelIndexBufferData.rdIndexBuffer, 0,
                         VK_INDEX_TYPE_UINT32);

    vkCmdBindPipeline(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderData.rdMeshPipeline);

    vkCmdDrawIndexed(renderData.rdCommandBuffer, static_cast<int64_t>(mIndexBufferData.size()), 1, 0, 0, 0);
}

void Core::Renderer::Primitive::cleanup(Core::Renderer::VkRenderData& renderData)
{

    Core::Renderer::VertexBuffer::cleanup(renderData, primitiveRenderData.rdModelVertexBufferData);

    Core::Renderer::IndexBuffer::cleanup(renderData, primitiveRenderData.rdModelIndexBufferData);

    Core::Renderer::Texture::cleanup(renderData, mTextureData);
}

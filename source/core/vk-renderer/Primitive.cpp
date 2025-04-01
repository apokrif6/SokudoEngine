#include "Primitive.h"

Core::Renderer::Primitive::Primitive(const std::string& primitiveName,
                                     const std::vector<Core::Renderer::NewVertex>& vertexBufferData,
                                     const std::vector<uint32_t>& indexBufferData,
                                     const Core::Renderer::VkTextureArrayData& textureData,
                                     Core::Renderer::VkRenderData& renderData,
                                     Core::Renderer::VkPrimitiveRenderData& primitiveRenderData)
    : mPrimitiveName(primitiveName), mVertexBufferData(vertexBufferData), mIndexBufferData(indexBufferData),
      mTextureData(textureData)
{
    createVertexBuffers(renderData, primitiveRenderData);
    createIndexBuffer(renderData, primitiveRenderData);
}

void Core::Renderer::Primitive::createVertexBuffers(Core::Renderer::VkRenderData& renderData,
                                                    Core::Renderer::VkPrimitiveRenderData& primitiveRenderData)
{
    Core::Renderer::VertexBuffer::init(renderData, primitiveRenderData.rdModelVertexBufferData,
                                       mVertexBufferData.size() * sizeof(Core::Renderer::NewVertex));
}

void Core::Renderer::Primitive::createIndexBuffer(Core::Renderer::VkRenderData& renderData,
                                                  Core::Renderer::VkPrimitiveRenderData& primitiveRenderData)
{
    Core::Renderer::IndexBuffer::init(renderData, primitiveRenderData.rdModelIndexBufferData,
                                      static_cast<int64_t>(mIndexBufferData.size()));
}

void Core::Renderer::Primitive::assignTextureDescriptors(Core::Renderer::VkRenderData& renderData,
                                                         Core::Renderer::VkPrimitiveRenderData& primitiveRenderData)
{
}

void Core::Renderer::Primitive::uploadVertexBuffers(Core::Renderer::VkRenderData& renderData,
                                                    Core::Renderer::VkPrimitiveRenderData& primitiveRenderData)
{
    Core::Renderer::VertexBuffer::uploadData(renderData, primitiveRenderData.rdModelVertexBufferData,
                                             mVertexBufferData);
}

void Core::Renderer::Primitive::uploadIndexBuffer(Core::Renderer::VkRenderData& renderData,
                                                  Core::Renderer::VkPrimitiveRenderData& primitiveRenderData)
{
    Core::Renderer::IndexBuffer::uploadData(renderData, primitiveRenderData.rdModelIndexBufferData, mIndexBufferData);
}

void Core::Renderer::Primitive::draw(const Core::Renderer::VkRenderData& renderData,
                                     const Core::Renderer::VkPrimitiveRenderData& primitiveRenderData)
{
    /*   for (const Core::Renderer::VkTextureData& textureData : mTextureData)
       {
           vkCmdBindDescriptorSets(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                   renderData.rdPipelineLayout, 0, 1, &textureData.texTextureDescriptorSet, 0, nullptr);
       }
       */
    vkCmdBindDescriptorSets(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            renderData.rdMeshPipelineLayout, 0, 1, &mTextureData.descriptorSet, 0, nullptr);

    vkCmdBindDescriptorSets(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            renderData.rdMeshPipelineLayout, 1, 1,
                            &renderData.rdPerspectiveViewMatrixUBO.rdUBODescriptorSet, 0, nullptr);

    VkDeviceSize offsets[1] = {0};
    vkCmdBindVertexBuffers(renderData.rdCommandBuffer, 0, 1,
                           &primitiveRenderData.rdModelVertexBufferData.rdVertexBuffer, offsets);

    vkCmdBindIndexBuffer(renderData.rdCommandBuffer, primitiveRenderData.rdModelIndexBufferData.rdIndexBuffer, 0,
                         VK_INDEX_TYPE_UINT32);

    vkCmdBindPipeline(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderData.rdMeshPipeline);

    vkCmdDrawIndexed(renderData.rdCommandBuffer, static_cast<int64_t>(mIndexBufferData.size()), 1, 0, 0, 0);
}

void Core::Renderer::Primitive::cleanup(Core::Renderer::VkRenderData& renderData,
                                        Core::Renderer::VkPrimitiveRenderData& primitiveRenderData)
{

    Core::Renderer::VertexBuffer::cleanup(renderData, primitiveRenderData.rdModelVertexBufferData);

    Core::Renderer::IndexBuffer::cleanup(renderData, primitiveRenderData.rdModelIndexBufferData);

    // Core::Renderer::Texture::cleanup(renderData, gltfRenderData.rdGltfModelTexture);
}

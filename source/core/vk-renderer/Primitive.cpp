#include "Primitive.h"

Core::Renderer::Primitive::Primitive(const std::string& primitiveName,
                                     const std::vector<Core::Renderer::NewVertex>& vertexBufferData,
                                     const std::vector<uint32_t>& indexBufferData, int64_t indexCount,
                                     Core::Renderer::VkRenderData& renderData,
                                     Core::Renderer::VkPrimitiveRenderData& primitiveRenderData)
    : mPrimitiveName(primitiveName), mVertexBufferData(vertexBufferData), mIndexBufferData(indexBufferData),
      mIndexCount(indexCount)
{
    createVertexBuffers(renderData, primitiveRenderData);
    createIndexBuffer(renderData, primitiveRenderData);

    renderData.rdPrimitiveTriangleCount = indexBufferData.size();
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
    Core::Renderer::IndexBuffer::init(renderData, primitiveRenderData.rdModelIndexBufferData, mIndexCount);
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
    /*vkCmdBindDescriptorSets(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            renderData.rdGltfPipelineLayout, 0, 1,
                            &primitiveRenderData.rdModelTexture.texTextureDescriptorSet, 0, nullptr);
*/
    VkDeviceSize offsets[1] = {0};
    vkCmdBindVertexBuffers(renderData.rdCommandBuffer, 0, 1,
                           &primitiveRenderData.rdModelVertexBufferData.rdVertexBuffer, offsets);

    vkCmdBindIndexBuffer(renderData.rdCommandBuffer, primitiveRenderData.rdModelIndexBufferData.rdIndexBuffer, 0,
                         VK_INDEX_TYPE_UINT32);

    vkCmdBindPipeline(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, renderData.rdMeshPipeline);

    vkCmdDrawIndexed(renderData.rdCommandBuffer, static_cast<uint32_t>(mIndexCount), 1, 0, 0, 0);
}
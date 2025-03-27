#include "Primitive.h"

Core::Renderer::Primitive::Primitive(const std::string& primitiveName, int64_t vertexSize,
                                     const std::vector<Core::Renderer::NewVertex>& vertexBufferData,
                                     int64_t vertexCount, const std::vector<uint32_t>& indexBufferData,
                                     int64_t indexCount, Core::Renderer::VkRenderData& renderData,
                                     Core::Renderer::VkPrimitiveRenderData& primitiveRenderData)
    : mPrimitiveName(primitiveName), mVertexSize(vertexSize), mVertexBufferData(vertexBufferData),
      mVertexCount(vertexCount), mIndexBufferData(indexBufferData), mIndexCount(indexCount)
{
    createVertexBuffers(renderData, primitiveRenderData);
    createIndexBuffer(renderData, primitiveRenderData);

    renderData.rdPrimitiveTriangleCount = indexBufferData.size();
}

void Core::Renderer::Primitive::createVertexBuffers(Core::Renderer::VkRenderData& renderData,
                                                    Core::Renderer::VkPrimitiveRenderData& primitiveRenderData)
{
    /*glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec4 color;
    glm::vec2 uv;*/
    /*    primitiveRenderData.rdModelVertexBufferData.resize(5);

        for (int i = 0; i < 5; ++i)
        {
            Core::Renderer::VertexBuffer::init(renderData, primitiveRenderData.rdModelVertexBufferData[i],
       mVertexBufferData.size() * sizeof(Core::Renderer::NewVertex)));
        }*/

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
    /*    for (int i = 0; i < 5; ++i)
        {
            Core::Renderer::VertexBuffer::uploadData(renderData, primitiveRenderData.rdModelVertexBufferData[i],
                                                     mVertexBufferData);
        }*/

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

    vkCmdDraw(renderData.rdCommandBuffer, static_cast<uint32_t>(mIndexCount), 1, 0, 0);

    // vkCmdDrawIndexed(renderData.rdCommandBuffer, static_cast<uint32_t>(mIndexCount), 1, 0, 0, 0);
}
#include "Mesh.h"

void Core::Renderer::Mesh::addPrimitive(const std::vector<Core::Renderer::NewVertex>& vertexBufferData,
                                        const std::vector<uint32_t>& indexBufferData,
                                        const Core::Renderer::VkTextureData& textureData,
                                        Core::Renderer::VkRenderData& renderData,
                                        const Core::Renderer::MaterialInfo& MaterialInfo)
{
    mPrimitives.emplace_back(vertexBufferData, indexBufferData, textureData, MaterialInfo, renderData);
}

void Core::Renderer::Mesh::draw(const Core::Renderer::VkRenderData& renderData)
{
    for (auto& primitive : mPrimitives)
    {
        primitive.draw(renderData);
    }
}

void Core::Renderer::Mesh::cleanup(Core::Renderer::VkRenderData& renderData)
{
    for (auto& primitive : mPrimitives)
    {
        primitive.cleanup(renderData);
    }
}
void Core::Renderer::Mesh::uploadVertexBuffers(Core::Renderer::VkRenderData& renderData)
{
    for (auto& primitive : mPrimitives)
    {
        primitive.uploadVertexBuffer(renderData);
    }
}
void Core::Renderer::Mesh::uploadIndexBuffers(Core::Renderer::VkRenderData& renderData)
{
    for (auto& primitive : mPrimitives)
    {
        primitive.uploadIndexBuffer(renderData);
    }
}

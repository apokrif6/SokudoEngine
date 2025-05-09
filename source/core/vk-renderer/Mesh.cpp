#include "Mesh.h"
#include "core/vk-renderer/buffers/UniformBuffer.h"

void Core::Renderer::Mesh::addPrimitive(const std::vector<Core::Renderer::NewVertex>& vertexBufferData,
                                        const std::vector<uint32_t>& indexBufferData, const VkTextureData& textureData,
                                        VkRenderData& renderData, const MaterialInfo& materialInfo,
                                        const Animations::BonesInfo& bonesInfo)
{
    mPrimitives.emplace_back(vertexBufferData, indexBufferData, textureData, materialInfo, bonesInfo, renderData);
}

void Core::Renderer::Mesh::updateData(Core::Renderer::VkRenderData& renderData)
{
    for (auto& primitive : mPrimitives)
    {
        primitive.uploadVertexBuffer(renderData);
        primitive.uploadIndexBuffer(renderData);
        primitive.uploadUniformBuffer(renderData);
    }
}

void Core::Renderer::Mesh::draw(Core::Renderer::VkRenderData& renderData)
{
    mAnimator->update(this);

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
void Core::Renderer::Mesh::uploadUniformBuffers(Core::Renderer::VkRenderData& renderData)
{
    for (auto& primitive : mPrimitives)
    {
        primitive.uploadUniformBuffer(renderData);
    }
}

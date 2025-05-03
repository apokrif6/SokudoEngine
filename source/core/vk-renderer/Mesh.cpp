#include "Mesh.h"
#include "core/vk-renderer/buffers/UniformBuffer.h"

void Core::Renderer::Mesh::addPrimitive(const std::vector<Core::Renderer::NewVertex>& vertexBufferData,
                                        const std::vector<uint32_t>& indexBufferData, const VkTextureData& textureData,
                                        VkRenderData& renderData, const MaterialInfo& materialInfo,
                                        const Animations::BonesInfo& bonesInfo)
{
    mPrimitives.emplace_back(vertexBufferData, indexBufferData, textureData, materialInfo, bonesInfo, renderData);

    // TODO
    // I don't like it. should be refactored
    if (mBonesTransformUBO.rdUniformBuffer == VK_NULL_HANDLE)
    {
        createBonesTransformBuffer(renderData);
    }
}

void Core::Renderer::Mesh::draw(const Core::Renderer::VkRenderData& renderData)
{
    mAnimator->update(this);

    vkCmdBindDescriptorSets(renderData.rdCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            renderData.rdMeshPipelineLayout, 3, 1, &mBonesTransformUBO.rdUBODescriptorSet, 0, nullptr);

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

void Core::Renderer::Mesh::createBonesTransformBuffer(Core::Renderer::VkRenderData& renderData)
{
    Core::Renderer::UniformBuffer::init(renderData, mBonesTransformUBO, sizeof(std::vector<glm::mat4>));

    // TODO
    // should be moved to uploadUniformBuffers to play animations instead of vertex skinning pose
    Core::Renderer::UniformBuffer::uploadData(renderData, mBonesTransformUBO, mBonesTransform);
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

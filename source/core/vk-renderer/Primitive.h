#pragma once

#include "string"
#include "core/vk-renderer/buffers/VertexBuffer.h"
#include "core/vk-renderer/buffers/IndexBuffer.h"
#include "core/utils/ShapeUtils.h"

namespace Core::Renderer
{
class Primitive
{
  public:
    Primitive(const std::vector<Core::Renderer::NewVertex>& vertexBufferData,
              const std::vector<uint32_t>& indexBufferData, const Core::Renderer::VkTextureData& textureData,
              const Core::Renderer::MaterialInfo& materialInfo, Core::Renderer::VkRenderData& renderData);

    void uploadVertexBuffer(Core::Renderer::VkRenderData& renderData);

    void uploadIndexBuffer(Core::Renderer::VkRenderData& renderData);

    void draw(const Core::Renderer::VkRenderData& renderData);

    void cleanup(Core::Renderer::VkRenderData& renderData);

  private:
    void createVertexBuffer(Core::Renderer::VkRenderData& renderData);

    void createIndexBuffer(Core::Renderer::VkRenderData& renderData);

    void createMaterialBuffer(Core::Renderer::VkRenderData& renderData);

    Core::Renderer::VkPrimitiveRenderData primitiveRenderData;

    std::vector<Core::Renderer::NewVertex> mVertexBufferData;
    std::vector<uint32_t> mIndexBufferData;
    Core::Renderer::VkTextureData mTextureData;

    Core::Renderer::VkUniformBufferData mMaterialUBO{};
    Core::Renderer::MaterialInfo mMaterialInfo{};
};
} // namespace Core::Renderer
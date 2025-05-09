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
    Primitive(const std::vector<Renderer::NewVertex>& vertexBufferData, const std::vector<uint32_t>& indexBufferData,
              const Renderer::VkTextureData& textureData, const Renderer::MaterialInfo& materialInfo,
              const Animations::BonesInfo& bonesInfo, Core::Renderer::VkRenderData& renderData);

    void uploadVertexBuffer(Renderer::VkRenderData& renderData);

    void uploadIndexBuffer(Renderer::VkRenderData& renderData);

    void uploadUniformBuffer(Renderer::VkRenderData& renderData);

    void draw(const Renderer::VkRenderData& renderData);

    void cleanup(Renderer::VkRenderData& renderData);

    Animations::BonesInfo& getBonesInfo() { return mBonesInfo; }

  private:
    void createVertexBuffer(Renderer::VkRenderData& renderData);

    void createIndexBuffer(Renderer::VkRenderData& renderData);

    void createMaterialBuffer(Renderer::VkRenderData& renderData);

    void createBonesTransformBuffer(Renderer::VkRenderData& renderData);

    Renderer::VkPrimitiveRenderData primitiveRenderData;

    std::vector<Renderer::NewVertex> mVertexBufferData;
    std::vector<uint32_t> mIndexBufferData;
    Renderer::VkTextureData mTextureData;

    Renderer::VkUniformBufferData mMaterialUBO{};
    Renderer::MaterialInfo mMaterialInfo{};

    Renderer::VkUniformBufferData mBonesTransformUBO{};
    Animations::BonesInfo mBonesInfo{};
};
} // namespace Core::Renderer
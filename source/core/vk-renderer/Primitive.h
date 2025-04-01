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
    Primitive(const std::string& primitiveName, const std::vector<Core::Renderer::NewVertex>& vertexBufferData,
              const std::vector<uint32_t>& indexBufferData, const Core::Renderer::VkTextureArrayData& textureData,
              Core::Renderer::VkRenderData& renderData, Core::Renderer::VkPrimitiveRenderData& primitiveRenderData);

    void uploadVertexBuffers(Core::Renderer::VkRenderData& renderData,
                             Core::Renderer::VkPrimitiveRenderData& primitiveRenderData);

    void uploadIndexBuffer(Core::Renderer::VkRenderData& renderData,
                           Core::Renderer::VkPrimitiveRenderData& primitiveRenderData);

    void draw(const Core::Renderer::VkRenderData& renderData,
              const Core::Renderer::VkPrimitiveRenderData& primitiveRenderData);

    void cleanup(Core::Renderer::VkRenderData& renderData, Core::Renderer::VkPrimitiveRenderData& primitiveRenderData);

  private:
    void createVertexBuffers(Core::Renderer::VkRenderData& renderData,
                             Core::Renderer::VkPrimitiveRenderData& primitiveRenderData);

    void createIndexBuffer(Core::Renderer::VkRenderData& renderData,
                           Core::Renderer::VkPrimitiveRenderData& primitiveRenderData);

    void assignTextureDescriptors(Core::Renderer::VkRenderData& renderData,
                                  Core::Renderer::VkPrimitiveRenderData& primitiveRenderData);

    std::string mPrimitiveName;
    int64_t mIndexCount = -1;

    std::vector<Core::Renderer::NewVertex> mVertexBufferData;
    std::vector<uint32_t> mIndexBufferData;
    Core::Renderer::VkTextureArrayData mTextureData;
};
} // namespace Core::Renderer
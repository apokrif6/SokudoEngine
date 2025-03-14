#pragma once

#include "string"
#include "core/vk-renderer/buffers/VertexBuffer.h"
#include "core/vk-renderer/buffers/IndexBuffer.h"

namespace Core::Renderer
{
class Primitive
{
  public:
    Primitive(const std::string& primitiveName, int64_t vertexSize,
              const std::vector<Core::Renderer::NewVertex>& vertexBufferData, int64_t vertexCount,
              const std::vector<uint32_t>& indexBufferData, int64_t indexCount,
              Core::Renderer::VkRenderData& renderData, Core::Renderer::VkPrimitiveRenderData& primitiveRenderData);

    void uploadVertexBuffers(Core::Renderer::VkRenderData& renderData,
                             Core::Renderer::VkPrimitiveRenderData& primitiveRenderData);

    void uploadIndexBuffer(Core::Renderer::VkRenderData& renderData,
                           Core::Renderer::VkPrimitiveRenderData& primitiveRenderData);

    void draw(const Core::Renderer::VkRenderData& renderData,
              const Core::Renderer::VkPrimitiveRenderData& primitiveRenderData);

  private:
    void createVertexBuffers(Core::Renderer::VkRenderData& renderData,
                             Core::Renderer::VkPrimitiveRenderData& primitiveRenderData);

    void createIndexBuffer(Core::Renderer::VkRenderData& renderData,
                           Core::Renderer::VkPrimitiveRenderData& primitiveRenderData);

    std::string mPrimitiveName;
    int64_t mVertexSize = -1;
    int64_t mVertexCount = -1;
    int64_t mIndexCount = -1;

    std::vector<Core::Renderer::NewVertex> mVertexBufferData;
    std::vector<uint32_t> mIndexBufferData;
};
} // namespace Core::Renderer
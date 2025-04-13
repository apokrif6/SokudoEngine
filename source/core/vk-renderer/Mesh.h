#pragma once

#include <vector>
#include "Primitive.h"

namespace Core::Renderer
{
class Mesh
{
  public:
    explicit Mesh(const std::string& name) : mName(name) {}

    void addPrimitive(const std::vector<Core::Renderer::NewVertex>& vertexBufferData,
                      const std::vector<uint32_t>& indexBufferData, const Core::Renderer::VkTextureData& textureData,
                      Core::Renderer::VkRenderData& renderData, const Core::Renderer::MaterialInfo& MaterialInfo);

    void uploadVertexBuffers(Core::Renderer::VkRenderData& renderData);

    void uploadIndexBuffers(Core::Renderer::VkRenderData& renderData);

    void draw(const Core::Renderer::VkRenderData& renderData);

    void cleanup(Core::Renderer::VkRenderData& renderData);

  private:
    std::string mName;
    std::vector<Core::Renderer::Primitive> mPrimitives;
};
} // namespace Core::Renderer

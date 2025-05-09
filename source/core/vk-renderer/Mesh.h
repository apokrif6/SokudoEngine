#pragma once

#include <utility>
#include <vector>
#include <memory>
#include "Primitive.h"
#include "core/animations/Animator.h"

// read comment in Animator.h
namespace Core::Animations
{
class Animator;
}

namespace Core::Renderer
{
class Mesh
{
  public:
    explicit Mesh(std::string name) : mName(std::move(name)) { mAnimator = std::make_unique<Animations::Animator>(); }

    void addPrimitive(const std::vector<Core::Renderer::NewVertex>& vertexBufferData,
                      const std::vector<uint32_t>& indexBufferData, const VkTextureData& textureData,
                      VkRenderData& renderData, const MaterialInfo& materialInfo,
                      const Animations::BonesInfo& bonesInfo);

    void updateData(Core::Renderer::VkRenderData& renderData);

    void draw(Core::Renderer::VkRenderData& renderData);

    void cleanup(Core::Renderer::VkRenderData& renderData);

    [[nodiscard]] std::vector<Core::Renderer::Primitive>& getPrimitives() { return mPrimitives; }

    [[nodiscard]] std::string getMeshName() const { return mName; }

  private:
    // TODO
    // should this be private?
    // or there is any optimization to upload only specific buffers instead of all of them in updateData
    // I don't know (｡•́︿•̀｡)
    void uploadVertexBuffers(Core::Renderer::VkRenderData& renderData);

    void uploadIndexBuffers(Core::Renderer::VkRenderData& renderData);

    void uploadUniformBuffers(Core::Renderer::VkRenderData& renderData);

    std::string mName;
    std::vector<Core::Renderer::Primitive> mPrimitives;

    std::unique_ptr<Animations::Animator> mAnimator;
};
} // namespace Core::Renderer

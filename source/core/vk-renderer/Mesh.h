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

    void uploadVertexBuffers(Core::Renderer::VkRenderData& renderData);

    void uploadIndexBuffers(Core::Renderer::VkRenderData& renderData);

    void draw(const Core::Renderer::VkRenderData& renderData);

    void cleanup(Core::Renderer::VkRenderData& renderData);

    [[nodiscard]] std::vector<Core::Renderer::Primitive> getPrimitives() const { return mPrimitives; }

  private:
    void createBonesTransformBuffer(Core::Renderer::VkRenderData& renderData);

    std::string mName;
    std::vector<Core::Renderer::Primitive> mPrimitives;

    std::unique_ptr<Animations::Animator> mAnimator;

    Renderer::VkUniformBufferData mBonesTransformUBO{};
    std::vector<glm::mat4> mBonesTransform;
};
} // namespace Core::Renderer

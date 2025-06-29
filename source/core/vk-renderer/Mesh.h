#pragma once

#include <utility>
#include <vector>
#include <memory>
#include "Primitive.h"
#include "core/vk-renderer/debug/Skeleton.h"

namespace Core::Renderer
{
class Mesh
{
  public:
    explicit Mesh(std::string name, Animations::Skeleton& skeleton) : mName(std::move(name)), mSkeleton(std::move(skeleton)) {}

    void addPrimitive(const std::vector<Core::Renderer::NewVertex>& vertexBufferData,
                      const std::vector<uint32_t>& indexBufferData, const VkTextureData& textureData,
                      VkRenderData& renderData, const MaterialInfo& materialInfo,
                      const Animations::BonesInfo& bonesInfo);

    void updateData(Core::Renderer::VkRenderData& renderData);

    void draw(Core::Renderer::VkRenderData& renderData);

    void cleanup(Core::Renderer::VkRenderData& renderData);

    [[nodiscard]] std::vector<Core::Renderer::Primitive>& getPrimitives() { return mPrimitives; }

    [[nodiscard]] std::string getMeshName() const { return mName; }

    [[nodiscard]] Animations::Skeleton& getSkeleton() { return mSkeleton; }

    void setupAnimations(const std::vector<Core::Animations::AnimationClip>& animationClips)
    {
        mAnimations = animationClips;
    }

    [[nodiscard]] const std::vector<Core::Animations::AnimationClip>& getAnimations() const { return mAnimations; }

    [[nodiscard]] bool hasAnimations() const { return !mAnimations.empty(); }

    void initDebugSkeleton(Core::Renderer::VkRenderData& renderData)
    {
        mSkeleton.initDebug(renderData);
    }

  private:
    std::string mName;
    std::vector<Core::Renderer::Primitive> mPrimitives;
    Core::Animations::Skeleton mSkeleton;
    std::vector<Core::Animations::AnimationClip> mAnimations;
};
} // namespace Core::Renderer

#pragma once

#include "core/scene/objects/SceneObject.h"
#include <utility>
#include <vector>
#include <memory>
#include "core/vk-renderer/Primitive.h"
#include "core/vk-renderer/debug/Skeleton.h"

namespace Core::Renderer
{
// right now every mesh is a skeletal mesh
// in future animation-related code should be moved to a separate SkeletalMesh class
class Mesh : public Core::Scene::SceneObject
{
  public:
    explicit Mesh(std::string name, Animations::Skeleton skeleton);

    void addPrimitive(const std::vector<Core::Renderer::NewVertex>& vertexBufferData,
                      const std::vector<uint32_t>& indexBufferData,
                      const std::unordered_map<aiTextureType, Renderer::VkTextureData>& textures,
                      VkRenderData& renderData, const MaterialInfo& materialInfo,
                      const Animations::BonesInfo& bonesInfo,
                      VkDescriptorSet materialDescriptorSet);

    [[nodiscard]] Scene::ObjectType getType() const override { return Scene::ObjectType::Mesh; }

    void update(Core::Renderer::VkRenderData& renderData) override;

    void draw(Core::Renderer::VkRenderData& renderData) override;

    void cleanup(Core::Renderer::VkRenderData& renderData) override;

    [[nodiscard]] std::vector<Core::Renderer::Primitive>& getPrimitives() { return mPrimitives; }

    [[nodiscard]] Animations::Skeleton& getSkeleton() { return mSkeleton; }

    void setupAnimations(const std::vector<Core::Animations::AnimationClip>& animationClips)
    {
        mAnimations = animationClips;
    }

    [[nodiscard]] const std::vector<Core::Animations::AnimationClip>& getAnimations() const { return mAnimations; }

    [[nodiscard]] bool hasAnimations() const { return !mAnimations.empty(); }

    [[nodiscard]] bool shouldPlayAnimation() const { return mShouldPlayAnimation; }

    void setShouldPlayAnimation(bool shouldPlay)
    {
        mShouldPlayAnimation = shouldPlay;
    }

    [[nodiscard]] bool shouldDrawDebugSkeleton() const { return mShouldDrawDebugSkeleton; }

    void setShouldDrawDebugSkeleton(bool shouldDraw)
    {
        mShouldDrawDebugSkeleton = shouldDraw;
    }

    void setCurrentAnimationIndex(uint32_t index)
    {
        if (index < mAnimations.size())
        {
            mCurrentAnimationIndex = index;
        }
    }

    [[nodiscard]] uint16_t getCurrentAnimationIndex() const { return mCurrentAnimationIndex; }

    [[nodiscard]] Core::Animations::AnimationClip& getCurrentAnimation()
    {
        return mAnimations[mCurrentAnimationIndex];
    }

    void initDebugSkeleton(Core::Renderer::VkRenderData& renderData)
    {
        mSkeleton.initDebug(renderData);
    }

    void setMeshFilePath(std::string path) { mMeshFilePath = std::move(path); }

    void setAnimationFiles(std::vector<std::string> files) { mAnimationFiles = std::move(files); }

    YAML::Node serialize() const override;

    void deserialize(const YAML::Node& node) override;

  private:
    std::vector<Core::Renderer::Primitive> mPrimitives;
    Core::Animations::Skeleton mSkeleton;
    std::vector<Core::Animations::AnimationClip> mAnimations;
    bool mShouldPlayAnimation = false;
    bool mShouldDrawDebugSkeleton = false;
    uint16_t mCurrentAnimationIndex = 0;

    // metadata for serialization
    // probably should be moved to other place (I don't know where exactly)
    std::string mMeshFilePath;
    std::vector<std::string> mAnimationFiles;
};
} // namespace Core::Renderer

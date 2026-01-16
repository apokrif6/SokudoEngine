#pragma once

#include "core/scene/objects/SceneObject.h"
#include <utility>
#include <vector>
#include "core/vk-renderer/Primitive.h"

// should be moved to Component folder, and renamed to MeshComponent
namespace Core::Component
{
// right now every mesh is a skeletal mesh
// in future animation-related code should be moved to a separate SkeletalMesh class
class MeshComponent : public Component
{
public:
    MeshComponent() = default;

    explicit MeshComponent(Animations::Skeleton skeleton);

    ~MeshComponent() override;

    [[nodiscard]] std::string_view getTypeName() const override { return "MeshComponent"; }

    void onAddedToScene() override;

    void onRemovedFromScene() override;

    void addPrimitive(const std::vector<Renderer::NewVertex>& vertexBufferData,
                      const std::vector<uint32_t>& indexBufferData,
                      const std::unordered_map<aiTextureType, Renderer::VkTextureData>& textures,
                      Renderer::VkRenderData& renderData, const Renderer::MaterialInfo& materialInfo,
                      const Animations::BonesInfo& bonesInfo, VkDescriptorSet materialDescriptorSet);

    void update(Renderer::VkRenderData& renderData) override;

    void draw(Renderer::VkRenderData& renderData) override;

    void cleanup(Renderer::VkRenderData& renderData) override;

    [[nodiscard]] std::vector<Renderer::Primitive>& getPrimitives() { return mPrimitives; }

    [[nodiscard]] Animations::Skeleton& getSkeleton() { return mSkeleton; }

    void setupAnimations(const std::vector<Animations::AnimationClip>& animationClips)
    {
        mAnimations = animationClips;
    }

    [[nodiscard]] const std::vector<Animations::AnimationClip>& getAnimations() const { return mAnimations; }

    [[nodiscard]] bool hasAnimations() const { return !mAnimations.empty(); }

    [[nodiscard]] bool shouldPlayAnimation() const { return mShouldPlayAnimation; }

    void setShouldPlayAnimation(bool shouldPlay) { mShouldPlayAnimation = shouldPlay; }

    [[nodiscard]] bool shouldDrawDebugSkeleton() const { return mShouldDrawDebugSkeleton; }

    void setShouldDrawDebugSkeleton(bool shouldDraw) { mShouldDrawDebugSkeleton = shouldDraw; }

    void setCurrentAnimationIndex(uint32_t index)
    {
        if (index < mAnimations.size())
        {
            mCurrentAnimationIndex = index;
        }
    }

    [[nodiscard]] uint16_t getCurrentAnimationIndex() const { return mCurrentAnimationIndex; }

    [[nodiscard]] Animations::AnimationClip& getCurrentAnimation() { return mAnimations[mCurrentAnimationIndex]; }

    void initDebugSkeleton(Renderer::VkRenderData& renderData) { mSkeleton.initDebug(renderData); }

    void setMeshFilePath(std::string path) { mMeshFilePath = std::move(path); }

    std::string_view getMeshFilePath() const { return mMeshFilePath; }

    void setAnimationFiles(std::vector<std::string> files) { mAnimationFiles = std::move(files); }

    YAML::Node serialize() const override;

    void deserialize(const YAML::Node& node) override;

private:
    std::vector<Renderer::Primitive> mPrimitives;
    Animations::Skeleton mSkeleton;
    std::vector<Animations::AnimationClip> mAnimations;
    bool mShouldPlayAnimation = false;
    bool mShouldDrawDebugSkeleton = false;
    uint16_t mCurrentAnimationIndex = 0;

    // metadata for serialization
    // probably should be moved to other place (I don't know where exactly)
    std::string mMeshFilePath;
    std::vector<std::string> mAnimationFiles;
};
} // namespace Core::Renderer

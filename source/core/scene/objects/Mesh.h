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
class Mesh : public Scene::SceneObject
{
public:
    explicit Mesh(std::string name, Animations::Skeleton skeleton);

    ~Mesh() override;

    virtual void onAddedToScene() override;

    virtual void onRemovedFromScene() override;

    void addPrimitive(const std::vector<NewVertex>& vertexBufferData,
                      const std::vector<uint32_t>& indexBufferData,
                      const std::unordered_map<aiTextureType, VkTextureData>& textures,
                      VkRenderData& renderData, const MaterialInfo& materialInfo,
                      const Animations::BonesInfo& bonesInfo, VkDescriptorSet materialDescriptorSet);

    [[nodiscard]] Scene::ObjectType getType() const override { return Scene::ObjectType::Mesh; }

    void update(VkRenderData& renderData) override;

    void draw(VkRenderData& renderData) override;

    void cleanup(VkRenderData& renderData) override;

    [[nodiscard]] std::vector<Primitive>& getPrimitives() { return mPrimitives; }

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

    void initDebugSkeleton(VkRenderData& renderData) { mSkeleton.initDebug(renderData); }

    void setMeshFilePath(std::string path) { mMeshFilePath = std::move(path); }

    void setAnimationFiles(std::vector<std::string> files) { mAnimationFiles = std::move(files); }

    YAML::Node serialize() const override;

    void deserialize(const YAML::Node& node) override;

private:
    std::vector<Primitive> mPrimitives;
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

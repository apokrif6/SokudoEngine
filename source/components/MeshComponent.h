#pragma once

#include "IKTargetComponent.h"
#include "animations/Skeleton.h"
#include "animations/ik/IKSolverCCD.h"
#include "scene/objects/SceneObject.h"
#include <utility>
#include <vector>
#include "vk-renderer/Primitive.h"

// should be moved to Component folder, and renamed to MeshComponent
namespace Core::Component
{
// right now every mesh is a skeletal mesh
// in future animation-related code should be moved to a separate SkeletalMesh class
class MeshComponent : public Component
{
public:
    MeshComponent() = default;

    explicit MeshComponent(const Resources::SkeletonData* skeletonData);

    ~MeshComponent() override;

    [[nodiscard]] std::string_view getTypeName() const override { return "MeshComponent"; }

    void onAdded() override;

    void onRemoved() override;

    void addPrimitive(const std::vector<Renderer::Vertex>& vertexBufferData,
                      const std::vector<uint32_t>& indexBufferData,
                      const std::unordered_map<aiTextureType, std::shared_ptr<Assets::TextureAsset>>& textures,
                      Renderer::VkRenderData& renderData, const Renderer::MaterialInfo& materialInfo,
                      VkDescriptorSet materialDescriptorSet, const Animations::BonesInfo& bonesInfo);

    void update(Renderer::VkRenderData& renderData) override;

    void draw(Renderer::VkRenderData& renderData) override;

    void cleanup(Renderer::VkRenderData& renderData) override;

    [[nodiscard]] std::vector<Renderer::Primitive>& getPrimitives() { return mPrimitives; }

    [[nodiscard]] Animations::Skeleton& getSkeleton() { return mSkeleton; }

    [[nodiscard]] const std::vector<Animations::AnimationClip>& getAnimations() const { return mAnimations; }

    [[nodiscard]] bool hasAnimations() const { return !mAnimations.empty(); }

    [[nodiscard]] bool shouldPlayAnimation() const { return mShouldPlayAnimation; }

    void setShouldPlayAnimation(bool shouldPlay) { mShouldPlayAnimation = shouldPlay; }

    [[nodiscard]] bool shouldBlendAnimations() const { return mShouldBlendAnimations; }

    void setShouldBlendAnimations(bool shouldBlend) { mShouldBlendAnimations = shouldBlend; }

    [[nodiscard]] float getBlendFactor() const { return mBlendFactor; }

    void setBlendFactor(float blendFactor) { mBlendFactor = blendFactor; }

    [[nodiscard]] bool shouldDrawDebugSkeleton() const { return mShouldDrawDebugSkeleton; }

    void setShouldDrawDebugSkeleton(bool shouldDraw) { mShouldDrawDebugSkeleton = shouldDraw; }

    [[nodiscard]] float getCurrentAnimationTime() const { return mCurrentAnimationTime; }

    void setAnimationTime(float time) { mCurrentAnimationTime = time; }

    void setSourceMesh(const std::string_view& path, uint32_t primitiveIndex);

    [[nodiscard]] uint16_t getCurrentAnimationIndex() const { return mCurrentAnimationIndex; }

    void setCurrentAnimationIndex(uint32_t index)
    {
        if (index < mAnimations.size())
        {
            mCurrentAnimationIndex = index;
        }
    }

    [[nodiscard]] uint16_t getTargetAnimationIndex() const { return mTargetAnimationIndex; }

    void setTargetAnimationIndex(uint32_t index)
    {
        if (index < mAnimations.size())
        {
            mTargetAnimationIndex = index;
        }
    }

    [[nodiscard]] Animations::AnimationBlendingMode getBlendingMode() const { return mBlendingMode; }

    void setBlendingMode(const Animations::AnimationBlendingMode mode) { mBlendingMode = mode; }

    void addMask(const Animations::AnimationMask& mask) { mMasks.push_back(mask); }

    [[nodiscard]] size_t getMasksCount() const { return mMasks.size(); }

    [[nodiscard]] const std::string& getMaskName(const int index) const
    {
        if (index >= 0 && index < mMasks.size())
        {
            return mMasks[index].name;
        }
        throw std::out_of_range("Invalid mask index");
    }

    [[nodiscard]] int getCurrentMaskIndex() const { return mCurrentMaskIndex; }

    void setMaskIndex(const int index)
    {
        if (index >= -1 && index < mMasks.size())
        {
            mCurrentMaskIndex = index;
        }
    }

    [[nodiscard]] Animations::AnimationMask& getMask(const int index) { return mMasks[index]; }

    [[nodiscard]] float getWeightForBone(const std::string& boneName, float globalBlendFactor);

    [[nodiscard]] const std::vector<std::unique_ptr<Animations::IKSolverCCD>>& getIKSolvers() const
    {
        return mIKSolvers;
    }

    [[nodiscard]] IKTargetComponent* getIKTarget() const { return mIKTarget; }

    // TODO
    // just for tests. should be replaced with proper component selector
    void TEST_setIKTargetAndCreateTestSolver();

    [[nodiscard]] Animations::AnimationClip& getCurrentAnimation() { return mAnimations[mCurrentAnimationIndex]; }

    [[nodiscard]] std::string_view getMeshFilePath() const { return mMeshFilePath; }

    [[nodiscard]] int32_t getPrimitiveIndex() const { return mPrimitiveIndex; }

    void loadAnimationFromFile(const std::string_view& filePath);

    void setAnimationFiles(std::vector<std::string> files) { mAnimationFiles = std::move(files); }

    [[nodiscard]] YAML::Node serialize() const override;

    void deserialize(const YAML::Node& node) override;

    [[nodiscard]] int getBoneIndex(const std::string& boneName);

private:
    std::vector<Renderer::Primitive> mPrimitives;
    Animations::Skeleton mSkeleton;
#pragma region Animation
    // TODO
    // I guess it should be moved to global animation manager, and mesh should store only shared pointers
    std::vector<Animations::AnimationClip> mAnimations;
    bool mShouldPlayAnimation = false;
    bool mShouldBlendAnimations = false;
    bool mShouldDrawDebugSkeleton = false;
    uint16_t mCurrentAnimationIndex = 0;
    uint16_t mTargetAnimationIndex = 0;
    float mCurrentAnimationTime = 0.f;

    Animations::AnimationBlendingMode mBlendingMode = Animations::AnimationBlendingMode::Crossfade;
    float mBlendFactor = 0.f;
    std::vector<Animations::AnimationMask> mMasks;
    int mCurrentMaskIndex = -1;

    std::vector<std::unique_ptr<Animations::IKSolverCCD>> mIKSolvers;
    IKTargetComponent* mIKTarget = nullptr;
#pragma endregion
    // metadata for serialization
    // probably should be moved to other place (I don't know where exactly)
    std::string mMeshFilePath;
    std::string mMeshNodeName;
    int32_t mPrimitiveIndex = -1;
    std::vector<std::string> mAnimationFiles;
};
} // namespace Core::Component

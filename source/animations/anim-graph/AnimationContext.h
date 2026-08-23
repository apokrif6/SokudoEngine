#pragma once

namespace Core::Resources
{
struct SkeletonData;
}

namespace Core::Component
{
class MeshComponent;
}

namespace Core::Assets
{
class AnimationAsset;
}

namespace Core::Animations
{
class AnimGraph;
class AnimInstance;
struct AnimationClip;

struct AnimationContext
{
    float deltaTime = 0.f;

    const AnimGraph* graph = nullptr;
    AnimInstance* instance = nullptr;
    const Resources::SkeletonData* skeletonData = nullptr;

    // TODO
    // I don't know do I like it here. probably not
    Component::MeshComponent* meshComponent = nullptr;

    std::vector<std::shared_ptr<Assets::AnimationAsset>> animationAssets{};
};
} // namespace Core::Animations
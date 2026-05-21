#pragma once

namespace Core::Resources
{
struct SkeletonData;
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
    // implement something like AnimationsDatabase and store only reference to it in context
    const std::vector<AnimationClip>* animations = nullptr;
};
} // namespace Core::Animations
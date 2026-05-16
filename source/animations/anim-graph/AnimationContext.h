#pragma once

namespace Core::Resources
{
struct SkeletonData;
}
namespace Core::Animations
{
class AnimInstance;
struct AnimationContext
{
    float deltaTime = 0.f;
    float currentTime = 0.f;

    AnimInstance* instance = nullptr;
    const Resources::SkeletonData* skeletonData = nullptr;
};
} // namespace Core::Animations
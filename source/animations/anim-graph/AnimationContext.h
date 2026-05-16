#pragma once

namespace Core::Resources
{
struct SkeletonData;
}
namespace Core::Animations
{
struct AnimationContext
{
    float deltaTime = 0.f;
    float currentTime = 0.f;

    const Resources::SkeletonData* skeletonData = nullptr;
};
} // namespace Core::Animations
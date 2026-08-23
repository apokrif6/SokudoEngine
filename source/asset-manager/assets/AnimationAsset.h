#pragma once

#include "Asset.h"
#include "animations/AnimationsData.h"

namespace Core::Assets
{
class AnimationAsset : public Asset
{
public:
    explicit AnimationAsset(const std::string& path);

    ~AnimationAsset() override = default;

    [[nodiscard]] const Animations::AnimationClip& getAnimation() const { return mAnimation; }

private:
    Animations::AnimationClip mAnimation;
};

} // namespace Core::Assets

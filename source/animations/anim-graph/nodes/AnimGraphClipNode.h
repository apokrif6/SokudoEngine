
#pragma once

#include "AnimGraphNode.h"

namespace Core::Animations
{
struct AnimationClip;

class AnimGraphClipNode final : public AnimGraphNode
{
public:
    explicit AnimGraphClipNode(const AnimationClip* clip) : mClip(clip) {}

    void setTime(const float time) { mTime = time; }

    Pose evaluate(AnimationContext& context) override;

private:
    const AnimationClip* mClip = nullptr;
    float mTime = 0.f;
};

} // namespace Core::Animations

#pragma once

#include "AnimGraphNode.h"

namespace Core::Animations
{
struct AnimationClip;

class AnimGraphClipNode final : public AnimGraphNode
{
public:
    explicit AnimGraphClipNode(const AnimationClip* clip) : mClip(clip) {}

    Pose evaluate(AnimationContext& context) override;

private:
    const AnimationClip* mClip = nullptr;
};

} // namespace Core::Animations
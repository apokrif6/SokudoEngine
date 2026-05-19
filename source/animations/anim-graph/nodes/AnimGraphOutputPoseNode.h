#pragma once

#include "AnimGraphNode.h"
#include <memory>

namespace Core::Animations
{
class AnimGraphOutputPoseNode final : public AnimGraphNode
{
public:
    explicit AnimGraphOutputPoseNode(std::shared_ptr<AnimGraphNode> input) : mInput(std::move(input)) {}

    Pose evaluate(AnimationContext& context) const override;

private:
    std::shared_ptr<AnimGraphNode> mInput;
};
} // namespace Core::Animations
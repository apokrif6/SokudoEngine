#pragma once

#include "AnimGraphNode.h"
#include "animations/ik/IIKSolver.h"

namespace Core::Animations
{
class AnimGraphIKNode final : public AnimGraphNode
{
public:
    explicit AnimGraphIKNode(std::shared_ptr<AnimGraphNode> input) : mInput(std::move(input)) {}

    void addSolver(std::unique_ptr<IIKSolver> solver) { mSolvers.push_back(std::move(solver)); }

    Pose evaluate(AnimationContext& context) override;

private:
    std::shared_ptr<AnimGraphNode> mInput;

    std::vector<std::unique_ptr<IIKSolver>> mSolvers;
};

} // namespace Core::Animations
#pragma once

#include "AnimGraphNode.h"
#include "animations/ik/IIKSolver.h"

namespace Core::Animations
{
class AnimGraphIKNode final : public AnimGraphNode
{
public:
    explicit AnimGraphIKNode();

    [[nodiscard]] PinID getInputPin() const { return mInputPin; }

    [[nodiscard]] PinID getOutputPin() const { return mOutputPin; }

    void addSolver(std::unique_ptr<IIKSolver> solver) { mSolvers.push_back(std::move(solver)); }

    void removeSolver(const size_t index)
    {
        if (index < mSolvers.size())
        {
            mSolvers.erase(mSolvers.begin() + index);
        }
    }

    std::vector<std::unique_ptr<IIKSolver>>& getSolvers() { return mSolvers; }

    Pose evaluate(AnimationContext& context) const override;

private:
    PinID mInputPin{};

    PinID mOutputPin{};

    std::vector<std::unique_ptr<IIKSolver>> mSolvers;
};

} // namespace Core::Animations
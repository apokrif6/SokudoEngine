#pragma once

#include <glm/vec3.hpp>

namespace Core::Animations
{
struct BonesInfo;
struct BoneNode;
class IIKSolver
{
public:
    virtual ~IIKSolver() = default;

    virtual void solve(BonesInfo& bonesInfo, const BoneNode& rootNode) = 0;

    virtual void setTarget(const glm::vec3& target) = 0;
};
} // namespace Core::Animations

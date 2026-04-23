#pragma once

#include "glm/glm.hpp"
#include "glm/detail/type_quat.hpp"
#include <vector>
#include <map>
#include <string>
#include <memory>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

constexpr size_t maxNumberOfBonesPerVertex = 4;

namespace Core::Animations
{
struct VertexBoneData
{
    unsigned int boneIDs[maxNumberOfBonesPerVertex] = {};
    float weights[maxNumberOfBonesPerVertex] = {0.f};

    VertexBoneData() = default;
};

struct Bone
{
    glm::mat4 offset{};
    glm::mat4 animatedGlobalTransform{};
    glm::mat4 finalTransform{};

    Bone() = default;
    explicit Bone(const glm::mat4& inOffset)
        : offset(inOffset), animatedGlobalTransform(glm::mat4(0.0)), finalTransform(glm::mat4(0.0))
    {
    }
};

struct BonesInfo
{
    std::vector<VertexBoneData> vertexToBone;
    std::map<std::string, int> boneNameToIndexMap;
    std::vector<Bone> bones;
    std::vector<glm::mat4> finalTransforms;
};

struct BoneTransform
{
    glm::vec3 position{0.f};
    glm::quat rotation{1.f, 0.f, 0.f, 0.f};
    glm::vec3 scale{1.f};

    BoneTransform() = default;
    explicit BoneTransform(const glm::vec3& inPosition, const glm::quat& inRotation, const glm::vec3& inScale)
        : position(inPosition), rotation(inRotation), scale(inScale)
    {
    }

    explicit BoneTransform(const glm::mat4& matrix)
    {
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(matrix, scale, rotation, position, skew, perspective);
    }

    [[nodiscard]] glm::mat4 toMatrix() const
    {
        return glm::translate(glm::mat4(1.0f), position) * glm::mat4_cast(rotation) *
               glm::scale(glm::mat4(1.0f), scale);
    }
};

struct KeyframeVec3
{
    float time;
    glm::vec3 value;
};

struct KeyframeQuat
{
    float time;
    glm::quat value;
};

struct Node
{
    std::string name;
    glm::mat4 transform;
    std::vector<std::shared_ptr<Node>> children;
};

struct AnimationChannel
{
    std::string boneName;
    std::vector<KeyframeVec3> positions;
    std::vector<KeyframeQuat> rotations;
    std::vector<KeyframeVec3> scalings;
};

struct AnimationClip
{
    std::string name;
    float duration;
    float ticksPerSecond;
    std::vector<AnimationChannel> channels;
};

struct BoneNode
{
    std::string name;
    glm::mat4 localTransform;
    std::vector<BoneNode> children;
};

struct AnimationMask
{
    std::string name;
    std::map<std::string, float> boneWeights;

    void setWeightRecursively(const BoneNode& node, float weight, bool includeChildren = true)
    {
        boneWeights[node.name] = weight;

        if (!includeChildren)
        {
            return;
        }

        for (const BoneNode& child : node.children)
        {
            setWeightRecursively(child, weight, true);
        }
    }
};

enum class AnimationBlendingMode : uint8_t
{
    Crossfade,
    Masked
};
} // namespace Core::Animations
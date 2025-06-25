#pragma once

#include "core/tools/Logger.h"
#include "glm/glm.hpp"
#include "glm/detail/type_quat.hpp"
#include <vector>
#include <map>
#include <string>
#include <memory>
#include <unordered_map>

#define MAX_NUM_BONES_PER_VERTEX 4
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a) / sizeof(a[0]))

namespace Core::Animations
{
struct VertexBoneData
{
    unsigned int boneIDs[MAX_NUM_BONES_PER_VERTEX] = {0};
    float weights[MAX_NUM_BONES_PER_VERTEX] = {0.f};

    VertexBoneData() = default;

    void AddBoneData(unsigned int boneID, float weight)
    {
        for (unsigned int i = 0; i < ARRAY_SIZE_IN_ELEMENTS(boneIDs); i++)
        {
            if (weights[i] == 0.f)
            {
                boneIDs[i] = boneID;
                weights[i] = weight;
                Logger::log(1, "%s: bone %d weight %f index %i\n", __FUNCTION__, boneID, weight, i);
                return;
            }
        }
    }
};

struct Bone
{
    glm::mat4 offset{};
    glm::mat4 finalTransform{};

    Bone() = default;
    explicit Bone(const glm::mat4& inOffset) : offset(inOffset), finalTransform(glm::mat4(0.0)) {}
};

struct BonesInfo
{
    std::vector<VertexBoneData> vertexToBone;
    std::map<std::string, int> boneNameToIndexMap;
    std::vector<Bone> bones;
    std::vector<glm::mat4> finalTransforms;
};

struct KeyframeVec3 {
    float time;
    glm::vec3 value;
};

struct KeyframeQuat {
    float time;
    glm::quat value;
};

struct Node {
    std::string name;
    glm::mat4 transform;
    std::vector<std::shared_ptr<Node>> children;
};

struct AnimationChannel {
    std::string boneName;
    std::vector<KeyframeVec3> positions;
    std::vector<KeyframeQuat> rotations;
    std::vector<KeyframeVec3> scalings;
};

struct AnimationClip {
    std::string name;
    float duration;
    float ticksPerSecond;
    std::vector<AnimationChannel> channels;
};


struct BoneNode {
    std::string name;
    glm::mat4 localTransform;
    std::vector<BoneNode> children;
};

struct Skeleton {
    BoneNode rootNode;
};
} // namespace Core::Animations
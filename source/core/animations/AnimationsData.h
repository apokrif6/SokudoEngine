#pragma once

#include "core/tools/Logger.h"
#include "glm/glm.hpp"
#include "glm/detail/type_quat.hpp"
#include "core/vk-renderer/debug/Skeleton.h"
#include "core/serialization/Serializable.h"
#include <vector>
#include <map>
#include <string>
#include <memory>
#include <unordered_map>

#define MAX_NUM_BONES_PER_VERTEX 4

namespace Core::Animations
{
struct VertexBoneData
{
    unsigned int boneIDs[MAX_NUM_BONES_PER_VERTEX] = {0};
    float weights[MAX_NUM_BONES_PER_VERTEX] = {0.f};

    VertexBoneData() = default;
};

struct Bone
{
    glm::mat4 offset{};
    glm::mat4 animatedGlobalTransform{};
    glm::mat4 finalTransform{};

    Bone() = default;
    explicit Bone(const glm::mat4& inOffset) : offset(inOffset), animatedGlobalTransform(glm::mat4(0.0)), finalTransform(glm::mat4(0.0)) {}
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


struct BoneNode : public Serialization::ISerializable
{
    std::string name;
    glm::mat4 localTransform;
    std::vector<BoneNode> children;

    YAML::Node serialize() const override
    {
        YAML::Node n;
        n["name"] = name;

        YAML::Node matrix;
        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                matrix.push_back(localTransform[i][j]);
            }
        }

        n["localTransform"] = matrix;

        for (const auto& child : children)
        {
            n["children"].push_back(child.serialize());
        }

        return n;
    }

    void deserialize(const YAML::Node& node) override
    {
        name = node["name"].as<std::string>();

        const auto& matrixNode = node["localTransform"];
        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                localTransform[i][j] = matrixNode[i * 4 + j].as<float>();
            }
        }

        children.clear();
        for (const auto& childNode : node["children"])
        {
            BoneNode child;
            child.deserialize(childNode);
            children.push_back(std::move(child));
        }
    }
};
}
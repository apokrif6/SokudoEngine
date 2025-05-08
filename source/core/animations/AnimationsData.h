#pragma once

#include "core/tools/Logger.h"
#include "glm/glm.hpp"
#include <vector>
#include <map>
#include <string>

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
} // namespace Core::Animations
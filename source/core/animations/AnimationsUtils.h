#pragma once

#include "assimp/matrix4x4.h"
#include "AnimationsData.h"
#include "assimp/quaternion.h"
#include "assimp/scene.h"
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Core::Animations
{
class AnimationsUtils
{
public:
    static inline glm::mat4 convertMatrixToGlm(aiMatrix4x4 from)
    {
        glm::mat4 to;
        // a,b,c,d in assimp is the row
        // 1,2,3,4 is the column
        to[0][0] = from.a1;
        to[1][0] = from.a2;
        to[2][0] = from.a3;
        to[3][0] = from.a4;
        to[0][1] = from.b1;
        to[1][1] = from.b2;
        to[2][1] = from.b3;
        to[3][1] = from.b4;
        to[0][2] = from.c1;
        to[1][2] = from.c2;
        to[2][2] = from.c3;
        to[3][2] = from.c4;
        to[0][3] = from.d1;
        to[1][3] = from.d2;
        to[2][3] = from.d3;
        to[3][3] = from.d4;
        return to;
    }

    static inline glm::vec3 convertVectorToGlm(aiVector3D vec)
    {
        glm::vec3 v;
        v.x = vec.x;
        v.y = vec.y;
        v.z = vec.z;
        return v;
    }

    static inline glm::quat convertQuaternionToGlm(const aiQuaternion& pOrientation)
    {
        return {pOrientation.w, pOrientation.x, pOrientation.y, pOrientation.z};
    }

    static Core::Animations::AnimationClip loadAnimationFromFile(const std::string& filePath);

    static Core::Animations::BoneNode buildBoneHierarchy(const aiNode* node);
};
} // namespace Core::Animations

#include "AnimatorSingleton.h"
#include "AnimationsUtils.h"

void Core::Animations::AnimatorSingleton::update(const Renderer::VkRenderData& renderData, Renderer::Mesh* mesh)
{
    updateBonesTransform(mesh);
    mAnimationTime += renderData.rdTickDiff;
}

void Core::Animations::AnimatorSingleton::updateBonesTransform(Renderer::Mesh* mesh)
{
    for (Renderer::Primitive& primitive : mesh->getPrimitives())
    {
        Animations::BonesInfo& bonesInfo = primitive.getBonesInfo();
        size_t bonesInfoSize = bonesInfo.bones.size();

        bonesInfo.finalTransforms.resize(bonesInfoSize, glm::mat4(1.0));

        const aiScene* scene = importers[mesh->getMeshName()].get()->GetScene();
        if (scene->HasAnimations())
        {
            mGlobalInverseTransform =
                glm::inverse(Core::Animations::AnimationsUtils::convertMatrixToGlm(scene->mRootNode->mTransformation));

            const aiAnimation* animation = scene->mAnimations[0];
            float currentTime = mAnimationTime;
            float duration = animation->mDuration;
            float ticksPerSecond = animation->mTicksPerSecond != 0 ? animation->mTicksPerSecond : 30.f;
            float timeInTicks = fmod(currentTime * ticksPerSecond, duration);

            readNodeHierarchy(mesh, scene->mRootNode, glm::mat4(1.0f), animation, timeInTicks);
        }

        for (int i = 0; i < bonesInfoSize; i++)
        {
            bonesInfo.finalTransforms[i] = bonesInfo.bones[i].finalTransform;
        }
    }
}
void Core::Animations::AnimatorSingleton::readNodeHierarchy(Core::Renderer::Mesh* mesh, const aiNode* node,
                                                   const glm::mat4& parentTransform, const aiAnimation* animation,
                                                   float animationTime)
{
    const std::string nodeName = node->mName.C_Str();
    glm::mat4 nodeTransform = Core::Animations::AnimationsUtils::convertMatrixToGlm(node->mTransformation);

    if (const aiNodeAnim* nodeAnim = findAnimationNode(animation, nodeName))
    {
        glm::vec3 position = interpolatePosition(nodeAnim, animationTime);
        glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);

        glm::quat rotation = interpolateRotation(nodeAnim, animationTime);
        glm::mat4 rotationMatrix = glm::mat4_cast(rotation);

        glm::vec3 scale = interpolateScaling(nodeAnim, animationTime);
        glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);

        nodeTransform = translationMatrix * rotationMatrix * scaleMatrix;
    }
    else
    {
        nodeTransform = glm::mat4(1.f);
    }

    glm::mat4 globalTransform = parentTransform * nodeTransform;

    for (Renderer::Primitive& primitive : mesh->getPrimitives())
    {
        Animations::BonesInfo& bonesInfo = primitive.getBonesInfo();
        if (bonesInfo.boneNameToIndexMap.contains(nodeName))
        {
            int boneIndex = bonesInfo.boneNameToIndexMap[nodeName];
            bonesInfo.bones[boneIndex].finalTransform =
                mGlobalInverseTransform * globalTransform * bonesInfo.bones[boneIndex].offset;
        }
    }

    for (int i = 0; i < node->mNumChildren; i++)
    {
        readNodeHierarchy(mesh, node->mChildren[i], globalTransform, animation, animationTime);
    }
}

const aiNodeAnim* Core::Animations::AnimatorSingleton::findAnimationNode(const aiAnimation* animation,
                                                                const std::string& nodeName)
{
    for (int i = 0; i < animation->mNumChannels; i++)
    {
        const aiNodeAnim* node = animation->mChannels[i];

        if (std::string(node->mNodeName.data) == nodeName)
        {
            return node;
        }
    }
    return nullptr;
}

glm::vec3 Core::Animations::AnimatorSingleton::interpolatePosition(const aiNodeAnim* nodeAnim, float animationTime)
{
    if (nodeAnim->mNumPositionKeys == 1)
    {
        return Core::Animations::AnimationsUtils::convertVectorToGlm(nodeAnim->mPositionKeys[0].mValue);
    }

    for (unsigned int i = 0; i < nodeAnim->mNumPositionKeys - 1; i++)
    {
        if (animationTime < nodeAnim->mPositionKeys[i + 1].mTime)
        {
            float t = (animationTime - nodeAnim->mPositionKeys[i].mTime) /
                      (nodeAnim->mPositionKeys[i + 1].mTime - nodeAnim->mPositionKeys[i].mTime);

            glm::vec3 start = Core::Animations::AnimationsUtils::convertVectorToGlm(nodeAnim->mPositionKeys[i].mValue);
            glm::vec3 end =
                Core::Animations::AnimationsUtils::convertVectorToGlm(nodeAnim->mPositionKeys[i + 1].mValue);

            return glm::mix(start, end, t);
        }
    }
    return glm::vec3(0.0f);
}

glm::quat Core::Animations::AnimatorSingleton::interpolateRotation(const aiNodeAnim* nodeAnim, float animationTime)
{
    if (nodeAnim->mNumRotationKeys == 1)
    {
        return Core::Animations::AnimationsUtils::convertQuaternionToGlm(nodeAnim->mRotationKeys[0].mValue);
    }

    for (unsigned int i = 0; i < nodeAnim->mNumRotationKeys - 1; i++)
    {
        if (animationTime < nodeAnim->mRotationKeys[i + 1].mTime)
        {
            float t = (animationTime - nodeAnim->mRotationKeys[i].mTime) /
                      (nodeAnim->mRotationKeys[i + 1].mTime - nodeAnim->mRotationKeys[i].mTime);

            glm::quat start =
                Core::Animations::AnimationsUtils::convertQuaternionToGlm(nodeAnim->mRotationKeys[i].mValue);
            glm::quat end =
                Core::Animations::AnimationsUtils::convertQuaternionToGlm(nodeAnim->mRotationKeys[i + 1].mValue);

            return glm::slerp(start, end, t);
        }
    }
    return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
}

glm::vec3 Core::Animations::AnimatorSingleton::interpolateScaling(const aiNodeAnim* nodeAnim, float animationTime)
{
    if (nodeAnim->mNumScalingKeys == 1)
    {
        return Core::Animations::AnimationsUtils::convertVectorToGlm(nodeAnim->mScalingKeys[0].mValue);
    }

    for (unsigned int i = 0; i < nodeAnim->mNumScalingKeys - 1; i++)
    {
        if (animationTime < nodeAnim->mScalingKeys[i + 1].mTime)
        {
            float t = (animationTime - nodeAnim->mScalingKeys[i].mTime) /
                      (nodeAnim->mScalingKeys[i + 1].mTime - nodeAnim->mScalingKeys[i].mTime);

            glm::vec3 start = Core::Animations::AnimationsUtils::convertVectorToGlm(nodeAnim->mScalingKeys[i].mValue);
            glm::vec3 end = Core::Animations::AnimationsUtils::convertVectorToGlm(nodeAnim->mScalingKeys[i + 1].mValue);

            return glm::mix(start, end, t);
        }
    }
    return glm::vec3(1.0f);
}

#include "AnimationsUtils.h"
#include "AnimationsData.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "Animator.h"
#include "glm/gtc/type_ptr.hpp"
#include "core/engine/Engine.h"

Core::Animations::AnimationClip Core::Animations::AnimationsUtils::loadAnimationFromFile(const std::string& filePath)
{
    Assimp::Importer importer{};
    const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_GlobalScale);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->HasAnimations())
    {
        Logger::log(1, "Failed to load animation: %s\n", filePath.c_str());
        return {};
    }

    aiAnimation* aiAnim = scene->mAnimations[0];
    if (!aiAnim)
    {
        Logger::log(1, "No animation found in file: %s\n", filePath.c_str());
        return {};
    }

    Core::Animations::AnimationClip clip;
    clip.name = aiAnim->mName.C_Str();
    clip.duration = static_cast<float>(aiAnim->mDuration);
    clip.ticksPerSecond = aiAnim->mTicksPerSecond != 0.0 ? static_cast<float>(aiAnim->mTicksPerSecond) : 25.0f;

    for (unsigned int i = 0; i < aiAnim->mNumChannels; i++)
    {
        aiNodeAnim* channel = aiAnim->mChannels[i];
        Core::Animations::AnimationChannel animChannel;
        animChannel.boneName = channel->mNodeName.C_Str();

        for (unsigned int j = 0; j < channel->mNumPositionKeys; j++)
        {
            const auto& key = channel->mPositionKeys[j];
            auto convertedPosition = Core::Animations::AnimationsUtils::convertVectorToGlm(key.mValue);
            animChannel.positions.push_back({
                static_cast<float>(key.mTime),
                convertedPosition
            });
        }

        for (unsigned int j = 0; j < channel->mNumRotationKeys; j++)
        {
            const auto& key = channel->mRotationKeys[j];
            auto convertedRotation = Core::Animations::AnimationsUtils::convertQuaternionToGlm(key.mValue);
            animChannel.rotations.push_back({
                static_cast<float>(key.mTime),
                convertedRotation
            });
        }

        for (unsigned int j = 0; j < channel->mNumScalingKeys; j++)
        {
            const auto& key = channel->mScalingKeys[j];
            animChannel.scalings.push_back({
                static_cast<float>(key.mTime),
                glm::vec3(key.mValue.x, key.mValue.y, key.mValue.z)
            });
        }

        clip.channels.push_back(animChannel);
    }

    Core::Engine::getInstance().getSystem<Animator>()->loadedAnimations.push_back(filePath);

    return clip;
}

Core::Animations::BoneNode Core::Animations::AnimationsUtils::buildBoneHierarchy(const aiNode* node)
{
    Core::Animations::BoneNode boneNode;
    boneNode.name = node->mName.C_Str();
    boneNode.localTransform = glm::transpose(glm::make_mat4(&node->mTransformation.a1));

    for (unsigned int i = 0; i < node->mNumChildren; ++i)
    {
        boneNode.children.push_back(buildBoneHierarchy(node->mChildren[i]));
    }

    return boneNode;
}

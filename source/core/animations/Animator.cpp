#include <sstream>
#include "Animator.h"
#include "AnimationsUtils.h"
#include "AnimatorSingleton.h"
#include "assimp/Importer.hpp"

void Core::Animations::Animator::update(Renderer::Mesh* mesh) { updateBonesTransform(mesh); }

void Core::Animations::Animator::updateBonesTransform(Renderer::Mesh* mesh)
{
    for (Renderer::Primitive& primitive : mesh->getPrimitives())
    {
        Animations::BonesInfo& bonesInfo = primitive.getBonesInfo();
        size_t bonesInfoSize = bonesInfo.bones.size();

        bonesInfo.finalTransforms.resize(bonesInfoSize);

        readNodeHierarchy(
            mesh,
            Animations::AnimatorSingleton::getInstance().importers[mesh->getMeshName()].get()->GetScene()->mRootNode,
            glm::mat4(1.0f));

        for (int i = 0; i < bonesInfoSize; i++)
        {
            bonesInfo.finalTransforms[i] = bonesInfo.bones[i].finalTransform;
        }
    }
}
void Core::Animations::Animator::readNodeHierarchy(Core::Renderer::Mesh* mesh, const aiNode* node,
                                                   const glm::mat4& parentTransform)
{
    const std::string nodeName = node->mName.C_Str();
    glm::mat4 nodeTransform = Core::Animations::AnimationsUtils::convertMatrixToGlm(node->mTransformation);
    glm::mat4 globalTransform = parentTransform * nodeTransform;

    for (Renderer::Primitive& primitive : mesh->getPrimitives())
    {
        Animations::BonesInfo& bonesInfo = primitive.getBonesInfo();
        if (bonesInfo.boneNameToIndexMap.contains(nodeName))
        {
            int boneIndex = bonesInfo.boneNameToIndexMap[nodeName];
            bonesInfo.bones[boneIndex].finalTransform = globalTransform * bonesInfo.bones[boneIndex].offset;
        }
    }

    for (int i = 0; i < node->mNumChildren; i++)
    {
        readNodeHierarchy(mesh, node->mChildren[i], globalTransform);
    }
}

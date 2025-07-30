#include <functional>
#include "Mesh.h"
#include "core/vk-renderer/buffers/UniformBuffer.h"
#include "core/animations/AnimatorSingleton.h"

void buildDebugSkeletonLines(const Core::Animations::Skeleton& skeleton,
                             const Core::Animations::BonesInfo& bonesInfo,
                             std::vector<Core::Renderer::Debug::DebugBone>& debugBones,
                             const Core::Animations::BoneNode& node,
                             const glm::mat4& parentTransform)
{
    glm::mat4 currentTransform = parentTransform;
    if (bonesInfo.boneNameToIndexMap.contains(node.name))
    {
        int boneIndex = bonesInfo.boneNameToIndexMap.at(node.name);
        currentTransform = bonesInfo.bones[boneIndex].animatedGlobalTransform;
    }
    else
    {
        currentTransform = parentTransform * node.localTransform;
    }

    glm::vec3 parentPos = glm::vec3(parentTransform * glm::vec4(0, 0, 0, 1));
    glm::vec3 currentPos = glm::vec3(currentTransform * glm::vec4(0, 0, 0, 1));

    if (parentTransform != glm::mat4(1.f))
    {
        debugBones.push_back({parentPos, currentPos});
    }

    for (const auto& child : node.children)
    {
        buildDebugSkeletonLines(skeleton, bonesInfo, debugBones, child, currentTransform);
    }
}


void Core::Renderer::Mesh::addPrimitive(const std::vector<Core::Renderer::NewVertex>& vertexBufferData,
                                        const std::vector<uint32_t>& indexBufferData, const VkTextureData& textureData,
                                        VkRenderData& renderData, const MaterialInfo& materialInfo,
                                        const Animations::BonesInfo& bonesInfo)
{
    mPrimitives.emplace_back(vertexBufferData, indexBufferData, textureData, materialInfo, bonesInfo, renderData);
}

void Core::Renderer::Mesh::update(Core::Renderer::VkRenderData& renderData)
{
    for (auto& primitive : mPrimitives)
    {
        primitive.uploadVertexBuffer(renderData);
        primitive.uploadIndexBuffer(renderData);
        primitive.uploadUniformBuffer(renderData, getTransform().getMatrix());

        if (renderData.shouldDrawDebugSkeleton)
        {
            std::vector<Debug::DebugBone> debugBones;
            buildDebugSkeletonLines(mSkeleton, primitive.getBonesInfo(), debugBones,
                                    mSkeleton.getRootNode(), glm::mat4(1.0f));
            mSkeleton.updateDebug(renderData, debugBones);
        }
    }
}

void Core::Renderer::Mesh::draw(Core::Renderer::VkRenderData& renderData)
{
    if (renderData.shouldPlayAnimation)
    {
        Animations::AnimatorSingleton::getInstance().update(renderData, this);
    }

    for (auto& primitive : mPrimitives)
    {
        primitive.draw(renderData);

        if (renderData.shouldDrawDebugSkeleton)
        {
            mSkeleton.drawDebug(renderData);
        }
    }
}

void Core::Renderer::Mesh::cleanup(Core::Renderer::VkRenderData& renderData)
{
    for (auto& primitive : mPrimitives)
    {
        primitive.cleanup(renderData);
    }
}

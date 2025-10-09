#include <functional>
#include "Mesh.h"
#include "core/vk-renderer/buffers/UniformBuffer.h"
#include "core/engine/Engine.h"
#include "core/animations/AnimationsUtils.h"

void buildDebugSkeletonLines(const Core::Animations::Skeleton& skeleton, const Core::Animations::BonesInfo& bonesInfo,
                             std::vector<Core::Renderer::Debug::DebugBone>& debugBones,
                             const Core::Animations::BoneNode& node, const glm::mat4& parentTransform,
                             const glm::mat4& meshTransform = glm::mat4(1.f))
{
    glm::mat4 currentTransform;
    if (bonesInfo.boneNameToIndexMap.contains(node.name))
    {
        int boneIndex = bonesInfo.boneNameToIndexMap.at(node.name);
        currentTransform = meshTransform * bonesInfo.bones[boneIndex].animatedGlobalTransform;
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
        buildDebugSkeletonLines(skeleton, bonesInfo, debugBones, child, currentTransform, meshTransform);
    }
}

Core::Renderer::Mesh::Mesh(std::string name, Core::Animations::Skeleton skeleton)
    : Core::Scene::SceneObject(std::move(name)), mSkeleton(std::move(skeleton))
{
    Core::Engine::getInstance().getSystem<Animations::Animator>()->addMesh(this);
}

void Core::Renderer::Mesh::addPrimitive(const std::vector<Core::Renderer::NewVertex>& vertexBufferData,
                                        const std::vector<uint32_t>& indexBufferData,
                                        const std::unordered_map<aiTextureType, Renderer::VkTextureData>& textures,
                                        VkRenderData& renderData, const MaterialInfo& materialInfo,
                                        const Animations::BonesInfo& bonesInfo)
{
    mPrimitives.emplace_back(vertexBufferData, indexBufferData, textures, materialInfo, bonesInfo, renderData);
}

void Core::Renderer::Mesh::update(Core::Renderer::VkRenderData& renderData)
{
    for (auto& primitive : mPrimitives)
    {
        primitive.uploadVertexBuffer(renderData);
        primitive.uploadIndexBuffer(renderData);
        primitive.uploadUniformBuffer(renderData, getTransform().getMatrix());

        if (shouldDrawDebugSkeleton())
        {
            std::vector<Debug::DebugBone> debugBones;
            buildDebugSkeletonLines(mSkeleton, primitive.getBonesInfo(), debugBones, mSkeleton.getRootNode(),
                                    mTransform.getMatrix(), mTransform.getMatrix());
            mSkeleton.updateDebug(renderData, debugBones);
        }
    }
}

void Core::Renderer::Mesh::draw(Core::Renderer::VkRenderData& renderData)
{
    for (auto& primitive : mPrimitives)
    {
        primitive.draw(renderData);

        if (shouldDrawDebugSkeleton())
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

    mSkeleton.cleanup(renderData);
}

YAML::Node Core::Renderer::Mesh::serialize() const
{
    YAML::Node node = SceneObject::serialize();
    node["meshFile"] = mMeshFilePath;
    for (auto& animPath : mAnimationFiles)
    {
        node["animations"].push_back(animPath);
    }
    node["shouldPlayAnimation"] = mShouldPlayAnimation;
    node["currentAnimationIndex"] = mCurrentAnimationIndex;
    return node;
}

void Core::Renderer::Mesh::deserialize(const YAML::Node& node)
{
    SceneObject::deserialize(node);
    mMeshFilePath = node["meshFile"].as<std::string>();
    mAnimationFiles.clear();
    if (node["animations"])
    {
        for (auto& animNode : node["animations"])
        {
            mAnimationFiles.push_back(animNode.as<std::string>());
        }
    }
    mShouldPlayAnimation = node["shouldPlayAnimation"].as<bool>();
    mCurrentAnimationIndex = node["currentAnimationIndex"].as<uint16_t>();

    // I don't like this here, it should be done somewhere else (maybe time for a resource manager hehe?)
    auto& renderData = Core::Engine::getInstance().getRenderData();
    Core::Utils::MeshData meshData = Core::Utils::loadMeshFromFile(mMeshFilePath, renderData);

    std::vector<Core::Animations::AnimationClip> animations;
    for (auto& animPath : mAnimationFiles)
    {
        animations.push_back(Core::Animations::AnimationsUtils::loadAnimationFromFile(animPath));
    }
    meshData.animations = animations;
    mSkeleton = meshData.skeleton;
    setupAnimations(meshData.animations);
    initDebugSkeleton(renderData);

    for (auto& primitive : meshData.primitives)
    {
        addPrimitive(primitive.vertices, primitive.indices, primitive.textures,
                     renderData, primitive.material, primitive.bones);
    }
}

#pragma once

#include "animations/AnimationsData.h"
#include "vk-renderer/VkRenderData.h"
#include <assimp/material.h>

namespace Core::Resources
{
struct PrimitiveData
{
    std::vector<Renderer::Vertex> vertices;
    std::vector<uint32_t> indices;
    std::unordered_map<aiTextureType, std::shared_ptr<Assets::TextureAsset>> textures;
    Renderer::MaterialInfo material;
    // TODO
    // should it be part of MaterialInfo?
    VkDescriptorSet materialDescriptorSet{};
    Animations::BonesInfo bones;
};

struct MeshNode
{
    std::string name;
    glm::mat4 localTransform;
    std::vector<PrimitiveData> primitives;
    std::vector<MeshNode> children;
};

struct SkeletonData
{
    Animations::BoneNode rootNode;
    std::unordered_map<std::string, int> boneNameToIndexMap;
    std::vector<int> boneParents;
};

struct MeshData
{
    MeshNode rootNode;
    SkeletonData skeletonData;
    std::vector<Animations::AnimationClip> animations;
};
} // namespace Core::Resources
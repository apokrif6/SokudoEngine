#pragma once

#include <string>
#include <glm/fwd.hpp>
#include "vk-renderer/VkRenderData.h"
#include "unordered_map"
#include "animations/AnimationsData.h"
#include "animations/Skeleton.h"
#include "assimp/material.h"

namespace Core::Utils
{
struct PrimitiveData
{
    std::vector<Renderer::Vertex> vertices;
    std::vector<uint32_t> indices;
    std::unordered_map<aiTextureType, Renderer::VkTextureData> textures;
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

struct MeshData
{
    MeshNode rootNode;
    Animations::Skeleton skeleton;
    std::vector<Animations::AnimationClip> animations;
};

MeshData loadMeshFromFile(const std::string& fileName, Renderer::VkRenderData& renderData);
} // namespace Core::Utils

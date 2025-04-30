#pragma once

#include <string>
#include <glm/fwd.hpp>
#include "assimp/scene.h"
#include "core/vk-renderer/VkRenderData.h"
#include "unordered_map"
#include "core/animations/AnimationsData.h"

namespace Core::Utils
{
struct PrimitiveData
{
    std::vector<Core::Renderer::NewVertex> vertices;
    std::vector<uint32_t> indices;
    std::unordered_map<aiTextureType, Core::Renderer::VkTextureData> textures;
    Core::Renderer::MaterialInfo material;
    Core::Animations::BoneInfo bones;
};

struct MeshData
{
    std::vector<PrimitiveData> primitives;
};

MeshData loadMeshFromFile(const std::string& fileName, Core::Renderer::VkRenderData& renderData);
} // namespace Core::Utils

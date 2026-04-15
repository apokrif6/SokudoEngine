#pragma once

#include <string>
#include <glm/fwd.hpp>
#include "vk-renderer/VkRenderData.h"
#include "animations/AnimationsData.h"
#include "assimp/material.h"
#include "resources/Mesh.h"

namespace Core::Utils
{
Resources::MeshData loadMeshFromFile(const std::string& fileName, Renderer::VkRenderData& renderData);

void collectPrimitivesRecursive(const Resources::MeshNode& node, const glm::mat4& parentTransform,
                                std::vector<Resources::PrimitiveData>& outAllPrimitives);

void createSpritePrimitiveData(const std::string& spritePath, Renderer::VkRenderData& renderData,
                               Resources::PrimitiveData& outPrimitiveData);
} // namespace Core::Utils

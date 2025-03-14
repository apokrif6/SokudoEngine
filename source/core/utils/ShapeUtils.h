#pragma once

#include <string>
#include <glm/fwd.hpp>
#include "assimp/scene.h"
#include "core/vk-renderer/VkRenderData.h"

namespace Core::Utils
{
struct VertexData
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec3 bitangent;
    glm::vec2 uv;
    glm::vec4 color;
};

struct ShapeData
{
    std::vector<VertexData> vertices;
    std::vector<uint32_t> indices;
};

ShapeData loadShapeFromFile(const std::string& filename);

std::vector<Core::Renderer::NewVertex> getVerticesFromShapeData(const ShapeData& shapeData);

std::vector<uint32_t> getIndicesFromShapeData(const ShapeData& shapeData);
}

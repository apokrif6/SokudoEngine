#include "Model.h"
#include "Logger.h"

void Model::init()
{
    VertexData.vertices.resize(6);

    VertexData.vertices[0].position = glm::vec3(-0.5f, -0.5f, 0.5f);
    VertexData.vertices[1].position = glm::vec3(0.5f, 0.5f, 0.5f);
    VertexData.vertices[2].position = glm::vec3(-0.5f, 0.5f, 0.5f);
    VertexData.vertices[3].position = glm::vec3(-0.5f, -0.5f, 0.5f);
    VertexData.vertices[4].position = glm::vec3(0.5f, -0.5f, 0.5f);
    VertexData.vertices[5].position = glm::vec3(0.5f, 0.5f, 0.5f);

    VertexData.vertices[0].uv = glm::vec2(0.0, 0.0);
    VertexData.vertices[1].uv = glm::vec2(1.0, 1.0);
    VertexData.vertices[2].uv = glm::vec2(0.0, 1.0);
    VertexData.vertices[3].uv = glm::vec2(0.0, 0.0);
    VertexData.vertices[4].uv = glm::vec2(1.0, 0.0);
    VertexData.vertices[5].uv = glm::vec2(1.0, 1.0);

    Logger::log(1, "%s: loaded %d vertices\n", __FUNCTION__, VertexData.vertices.size());
}

VkMesh Model::getVertexData() { return VertexData; }

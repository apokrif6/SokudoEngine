#include "GridModel.h"
#include "core/tools/Logger.h"

Core::Renderer::VkMesh Core::Model::GridModel::getVertexData()
{
    if (mVertexData.vertices.empty())
    {
        init();
    }

    return mVertexData;
}

void Core::Model::GridModel::init()
{
    mVertexData.vertices.resize(36);

    mVertexData.vertices[0].position = glm::vec3(1.0f, 1.0f, 0.0f);
    mVertexData.vertices[1].position = glm::vec3(-1.0f, -1.0f, 0.0f);
    mVertexData.vertices[2].position = glm::vec3(-1.0f, 1.0f, 0.0f);
    mVertexData.vertices[3].position = glm::vec3(-1.0f, -1.0f, 0.0f);
    mVertexData.vertices[4].position = glm::vec3(1.0f, 1.0f, 0.0f);
    mVertexData.vertices[6].position = glm::vec3(1.0f, -1.0f, 0.0f);

    Logger::log(1, "%s: GridModel - loaded %d vertices\n", __FUNCTION__, mVertexData.vertices.size());
}
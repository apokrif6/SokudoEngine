#include "ArrowModel.h"
#include "core/tools/Logger.h"

Core::Renderer::VkMesh Core::Model::ArrowModel::getVertexData()
{
    if (mVertexData.vertices.empty())
    {
        init();
    }

    return mVertexData;
}

void Core::Model::ArrowModel::init()
{
    mVertexData.vertices.resize(6);

    mVertexData.vertices[0].position = glm::vec3(0.f, 0.f, 0.f);
    mVertexData.vertices[1].position = glm::vec3(1.f, 0.f, 0.f);
    mVertexData.vertices[2].position = glm::vec3(1.f, 0.f, 0.f);
    mVertexData.vertices[3].position = glm::vec3(0.8f, 0, 0.075f);
    mVertexData.vertices[4].position = glm::vec3(1.f, 0.f, 0.f);
    mVertexData.vertices[5].position = glm::vec3(0.8f, 0.f, -0.075f);

    mVertexData.vertices[0].color = glm::vec3(0.8f, 0.f, 0.f);
    mVertexData.vertices[1].color = glm::vec3(0.8f, 0.f, 0.f);
    mVertexData.vertices[2].color = glm::vec3(0.8f, 0.f, 0.f);
    mVertexData.vertices[3].color = glm::vec3(0.8f, 0.f, 0.f);
    mVertexData.vertices[4].color = glm::vec3(0.8f, 0.f, 0.f);
    mVertexData.vertices[5].color = glm::vec3(0.8f, 0.f, 0.f);

    Logger::log(1, "%s: ArrowModel - loaded %d vertices\n", __FUNCTION__, mVertexData.vertices.size());
}
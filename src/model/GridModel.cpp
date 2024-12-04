#include "GridModel.h"
#include "tools/Logger.h"

VkMesh GridModel::getVertexData()
{
    if (mVertexData.vertices.empty())
    {
        init();
    }

    return mVertexData;
}

void GridModel::init()
{
    const int gridSize = 10;
    const float spacing = 1.0f;

    const int numLines = gridSize + 1;
    const int gridVertexCount = numLines * 4;

    mVertexData.vertices.resize(gridVertexCount);

    int index = 0;
    for (int i = -gridSize / 2; i <= gridSize / 2; ++i)
    {
        // Horizontal lines
        mVertexData.vertices[index].position = glm::vec3(i * spacing, 0.0f, -gridSize / 2 * spacing);
        mVertexData.vertices[index].color = glm::vec3(0.5f, 0.5f, 0.5f);
        ++index;
        mVertexData.vertices[index].position = glm::vec3(i * spacing, 0.0f, gridSize / 2 * spacing);
        mVertexData.vertices[index].color = glm::vec3(0.5f, 0.5f, 0.5f);
        ++index;

        // Vertical lines
        mVertexData.vertices[index].position = glm::vec3(-gridSize / 2 * spacing, 0.0f, i * spacing);
        mVertexData.vertices[index].color = glm::vec3(0.5f, 0.5f, 0.5f);
        ++index;
        mVertexData.vertices[index].position = glm::vec3(gridSize / 2 * spacing, 0.0f, i * spacing);
        mVertexData.vertices[index].color = glm::vec3(0.5f, 0.5f, 0.5f);
        ++index;
    }

    Logger::log(1, "%s: GridModel - loaded %d vertices\n", __FUNCTION__, mVertexData.vertices.size());
}
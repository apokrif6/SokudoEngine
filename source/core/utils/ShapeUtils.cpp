#include "ShapeUtils.h"
#include "core/tools/Logger.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

void processMesh(Core::Utils::ShapeData& shapeData, aiMesh* mesh, aiNode* node)
{
    shapeData.vertices.resize(mesh->mNumVertices);

    for (size_t i = 0; i < mesh->mNumVertices; ++i)
    {
        if (mesh->HasPositions())
        {
            aiVector3D& position = mesh->mVertices[i];
            position *= node->mTransformation;

            shapeData.vertices[i].position = {position.x, position.y, position.z};
        }

        if (mesh->HasNormals())
        {
            aiVector3D& normal = mesh->mNormals[i];
            normal *= node->mTransformation;

            shapeData.vertices[i].normal = {normal.x, normal.y, normal.z};
        }

        if (mesh->HasTextureCoords(0))
        {
            aiVector3D& texCoords = mesh->mTextureCoords[0][i];

            shapeData.vertices[i].uv = {texCoords.x, texCoords.y};
        }

        if (mesh->HasVertexColors(0))
        {
            aiColor4D& vertColor = mesh->mColors[0][i];

            shapeData.vertices[i].color = {vertColor.r, vertColor.g, vertColor.b, vertColor.a};
        }
    }

    for (size_t i = 0; i < mesh->mNumFaces; ++i)
    {
        aiFace& face = mesh->mFaces[i];
        for (size_t j = 0; j < face.mNumIndices; ++j)
        {
            shapeData.indices.push_back(face.mIndices[j]);
        }
    }
}

void processNode(Core::Utils::ShapeData& shapeData, aiNode* node, const aiScene* scene)
{
    for (size_t i = 0; i < node->mNumMeshes; ++i)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

        processMesh(shapeData, mesh, node);
    }

    for (size_t i = 0; i < node->mNumChildren; ++i)
    {
        processNode(shapeData, node->mChildren[i], scene);
    }
}

Core::Utils::ShapeData Core::Utils::loadShapeFromFile(const std::string& fileName)
{
    Assimp::Importer importer;
    importer.ReadFile(fileName, aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices);

    const aiScene* scene = importer.GetScene();

    if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
    {
        Logger::log(1, "%s error: Failed to load mesh %s \n", __FUNCTION__, fileName.c_str());
        return {};
    }

    Core::Utils::ShapeData shape;
    processNode(shape, scene->mRootNode, scene);
    return shape;
}

std::vector<Core::Renderer::NewVertex> Core::Utils::getVerticesFromShapeData(const Core::Utils::ShapeData& shapeData)
{
    std::vector<Core::Renderer::NewVertex> vertices;
    vertices.reserve(shapeData.vertices.size());
    for (const Core::Utils::VertexData& vertex : shapeData.vertices)
    {
        vertices.push_back({.position = vertex.position,
                            .normal = vertex.normal,
                            .tangent = vertex.tangent,
                            .color = vertex.color,
                            .uv = vertex.uv});
    }

    return vertices;
}

std::vector<uint32_t> Core::Utils::getIndicesFromShapeData(const Core::Utils::ShapeData& shapeData)
{
    std::vector<uint32_t> indices;
    indices.reserve(shapeData.indices.size());
    for (uint32_t index : shapeData.indices)
    {
        indices.push_back(index);
    }

    return indices;
}

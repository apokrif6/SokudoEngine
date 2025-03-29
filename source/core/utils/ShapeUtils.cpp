#include "ShapeUtils.h"
#include "core/tools/Logger.h"
#include "core/vk-renderer/Texture.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/gtc/type_ptr.hpp>

void processMaterialTextures(Core::Utils::ShapeData& shapeData, const aiMaterial* material, const aiScene* scene,
                             Core::Renderer::VkRenderData& renderData)
{
    unsigned int texIndex = 0;

    aiString texPath;

    for (texIndex = 0; texIndex < material->GetTextureCount(aiTextureType_DIFFUSE); ++texIndex)
    {
        material->GetTexture(aiTextureType_DIFFUSE, texIndex, &texPath);

        std::string textureFilePath = texPath.C_Str();
        Core::Renderer::VkTextureData textureData;

        std::future<bool> textureLoadFuture =
            Core::Renderer::Texture::loadTexture(renderData, textureData, textureFilePath);

        if (textureLoadFuture.get())
        {
            shapeData.textures.push_back(textureData);
        }
    }

    for (texIndex = 0; texIndex < material->GetTextureCount(aiTextureType_NORMALS); ++texIndex)
    {
        material->GetTexture(aiTextureType_NORMALS, texIndex, &texPath);

        std::string textureFilePath = texPath.C_Str();
        Core::Renderer::VkTextureData textureData;

        std::future<bool> textureLoadFuture =
            Core::Renderer::Texture::loadTexture(renderData, textureData, textureFilePath);

        if (textureLoadFuture.get())
        {
            shapeData.textures.push_back(textureData);
        }
    }
}

void processMesh(Core::Utils::ShapeData& shapeData, const aiMesh* mesh, const aiScene* scene,
                 const aiMaterial* material, const glm::mat4& transform, Core::Renderer::VkRenderData& renderData)
{
    size_t indexOffset = shapeData.vertices.size();

    shapeData.vertices.resize(indexOffset + mesh->mNumVertices);

    for (size_t i = 0; i < mesh->mNumVertices; ++i)
    {
        if (mesh->HasPositions())
        {
            aiVector3D& position = mesh->mVertices[i];
            glm::vec4 transformedPos = transform * glm::vec4(position.x, position.y, position.z, 1.0f);

            shapeData.vertices[indexOffset + i].position = {transformedPos.x, transformedPos.y, transformedPos.z};
        }

        if (mesh->HasNormals())
        {
            aiVector3D& normal = mesh->mNormals[i];
            glm::vec3 transformedNormal = glm::mat3(transform) * glm::vec3(normal.x, normal.y, normal.z);

            shapeData.vertices[indexOffset + i].normal = glm::normalize(transformedNormal);
        }

        if (mesh->HasTangentsAndBitangents())
        {
            aiVector3D& tangent = mesh->mTangents[i];

            shapeData.vertices[indexOffset + i].tangent = {tangent.x, tangent.y, tangent.z};
        }

        if (mesh->HasVertexColors(0))
        {
            aiColor4D& vertColor = mesh->mColors[0][i];

            shapeData.vertices[indexOffset + i].color = {vertColor.r, vertColor.g, vertColor.b, vertColor.a};
        }

        if (mesh->HasTextureCoords(0))
        {
            aiVector3D& texCoords = mesh->mTextureCoords[0][i];

            shapeData.vertices[indexOffset + i].uv = {texCoords.x, texCoords.y};
        }
    }

    for (size_t i = 0; i < mesh->mNumFaces; ++i)
    {
        aiFace& face = mesh->mFaces[i];
        for (size_t j = 0; j < face.mNumIndices; ++j)
        {
            shapeData.indices.push_back(face.mIndices[j] + indexOffset);
        }
    }

    processMaterialTextures(shapeData, material, scene, renderData);
}

void processNode(Core::Utils::ShapeData& shapeData, aiNode* node, const aiScene* scene,
                 const glm::mat4& parentTransform, Core::Renderer::VkRenderData& renderData)
{
    glm::mat4 nodeTransform = glm::transpose(glm::make_mat4(&node->mTransformation.a1));
    glm::mat4 globalTransform = parentTransform * nodeTransform;

    for (size_t i = 0; i < node->mNumMeshes; ++i)
    {
        const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        const aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        processMesh(shapeData, mesh, scene, material, globalTransform, renderData);
    }

    for (size_t i = 0; i < node->mNumChildren; ++i)
    {
        processNode(shapeData, node->mChildren[i], scene, globalTransform, renderData);
    }
}

Core::Utils::ShapeData Core::Utils::loadShapeFromFile(const std::string& fileName,
                                                      Core::Renderer::VkRenderData& renderData)
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
    processNode(shape, scene->mRootNode, scene, glm::mat4(1.0f), renderData);
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

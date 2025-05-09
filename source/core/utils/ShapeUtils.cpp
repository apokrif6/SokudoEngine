#include "ShapeUtils.h"
#include "core/vk-renderer/Texture.h"
#include "core/animations/AnimationsUtils.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/gtc/type_ptr.hpp>

// TODO
// find better place for it
int boneCounter = 0;

int getBoneID(Core::Utils::PrimitiveData& primitiveData, const aiBone* bone)
{
    int boneID;
    const std::string boneName = bone->mName.C_Str();

    if (!primitiveData.bones.boneNameToIndexMap.contains(boneName))
    {
        Core::Animations::Bone newBone;
        primitiveData.bones.bones.emplace_back(newBone);
        boneID = boneCounter++;
    }
    else
    {
        boneID = primitiveData.bones.boneNameToIndexMap[boneName];
    }

    primitiveData.bones.boneNameToIndexMap[boneName] = boneID;
    primitiveData.bones.bones[boneID] =
        Core::Animations::Bone{Core::Animations::AnimationsUtils::convertMatrixToGlm(bone->mOffsetMatrix)};

    return boneID;
}

void setVertexBoneData(Core::Renderer::NewVertex& vertex, int id, float weight)
{
    for (int i = 0; i < MAX_NUM_BONES_PER_VERTEX; i++)
    {
        if (vertex.weights[i] == 0.f)
        {
            vertex.boneID[i] = id;
            vertex.weights[i] = weight;
            break;
        }
    }
}

void processSingleBone(Core::Utils::PrimitiveData& primitiveData, const aiBone* bone)
{
    Logger::log(1, "Bone '%s': num vertices affected by this bone: %d\n", bone->mName.C_Str(), bone->mNumWeights);

    int boneID = getBoneID(primitiveData, bone);
    Logger::log(1, "bone id %d\n", boneID);

    aiVertexWeight* weights = bone->mWeights;
    for (int boneWeight = 0; boneWeight < bone->mNumWeights; ++boneWeight)
    {
        unsigned int vertexID = weights[boneWeight].mVertexId;
        float weight = weights[boneWeight].mWeight;
        setVertexBoneData(primitiveData.vertices[vertexID], boneID, weight);
    }
}

void processBones(Core::Utils::PrimitiveData& primitiveData, const aiMesh* mesh)
{
    for (unsigned int i = 0; i < mesh->mNumBones; i++)
    {
        processSingleBone(primitiveData, mesh->mBones[i]);
    }

    primitiveData.bones.finalTransforms.resize(primitiveData.bones.bones.size(), glm::mat4(1.0));
}

void processMesh(Core::Utils::MeshData& meshData, const aiMesh* mesh, const aiScene* scene, const aiMaterial* material,
                 const glm::mat4& transform, Core::Renderer::VkRenderData& renderData,
                 std::vector<Core::Renderer::VkTextureData> loadedTextures)
{
    boneCounter = 0;
    Core::Utils::PrimitiveData primitiveData;
    Core::Renderer::MaterialInfo materialInfo = {};

    if (scene->HasMaterials())
    {
        size_t textureCount = material->GetTextureCount(aiTextureType_DIFFUSE);
        if (textureCount > 0)
        {
            materialInfo.useTexture = 1;

            aiString path;
            if (material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
            {
                bool foundTexture = false;
                for (auto& loadedTexture : loadedTextures)
                {
                    if (loadedTexture.texName == path.C_Str())
                    {
                        primitiveData.textures[aiTextureType_DIFFUSE] = loadedTexture;
                        foundTexture = true;
                        break;
                    }
                }

                if (!foundTexture)
                {
                    std::string texturePath = path.C_Str();
                    Core::Renderer::VkTextureData textureData;
                    std::future<bool> textureLoadFuture =
                        Core::Renderer::Texture::loadTexture(renderData, textureData, texturePath);

                    if (textureLoadFuture.get())
                    {
                        primitiveData.textures.emplace(aiTextureType_DIFFUSE, textureData);
                        loadedTextures.emplace_back(textureData);
                    }
                    else
                    {
                        Logger::log(1, "Failed to load texture: %s\n", texturePath.c_str());
                    }
                }
            }
        }
        else
        {
            aiColor4D diffuseColor;
            if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor))
            {
                materialInfo.baseColorFactor =
                    glm::vec4(diffuseColor.r, diffuseColor.g, diffuseColor.b, diffuseColor.a);
            }
        }
    }

    primitiveData.material = materialInfo;

    for (size_t i = 0; i < mesh->mNumVertices; ++i)
    {
        Core::Renderer::NewVertex vertex{};
        if (mesh->HasPositions())
        {
            aiVector3D& position = mesh->mVertices[i];
            glm::vec4 transformedPos = transform * glm::vec4(position.x, position.y, position.z, 1.0f);
            vertex.position = {transformedPos.x, transformedPos.y, transformedPos.z};
        }

        if (mesh->HasNormals())
        {
            aiVector3D& normal = mesh->mNormals[i];
            glm::vec3 transformedNormal = glm::mat3(transform) * glm::vec3(normal.x, normal.y, normal.z);
            vertex.normal = glm::normalize(transformedNormal);
        }

        if (mesh->HasTangentsAndBitangents())
        {
            aiVector3D& tangent = mesh->mTangents[i];
            vertex.tangent = {tangent.x, tangent.y, tangent.z};
        }

        if (mesh->HasVertexColors(0))
        {
            aiColor4D& vertColor = mesh->mColors[0][i];
            vertex.color = {vertColor.r, vertColor.g, vertColor.b, vertColor.a};
        }

        if (mesh->HasTextureCoords(0))
        {
            aiVector3D& texCoords = mesh->mTextureCoords[0][i];
            vertex.uv = {texCoords.x, 1.f - texCoords.y};
        }

        primitiveData.vertices.emplace_back(vertex);
    }

    for (size_t i = 0; i < mesh->mNumFaces; ++i)
    {
        aiFace& face = mesh->mFaces[i];
        for (size_t j = 0; j < face.mNumIndices; ++j)
        {
            primitiveData.indices.push_back(face.mIndices[j]);
        }
    }

    if (mesh->HasBones())
    {
        processBones(primitiveData, mesh);
    }

    meshData.primitives.emplace_back(primitiveData);
}

void processNode(Core::Utils::MeshData& meshData, aiNode* node, const aiScene* scene, const glm::mat4& parentTransform,
                 Core::Renderer::VkRenderData& renderData, std::vector<Core::Renderer::VkTextureData> loadedTextures)
{
    glm::mat4 nodeTransform = glm::transpose(glm::make_mat4(&node->mTransformation.a1));
    glm::mat4 globalTransform = parentTransform * nodeTransform;

    for (size_t i = 0; i < node->mNumMeshes; ++i)
    {
        const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        const aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        processMesh(meshData, mesh, scene, material, globalTransform, renderData, loadedTextures);
    }

    for (size_t i = 0; i < node->mNumChildren; ++i)
    {
        processNode(meshData, node->mChildren[i], scene, globalTransform, renderData, loadedTextures);
    }
}

Core::Utils::MeshData Core::Utils::loadMeshFromFile(const std::string& fileName,
                                                    Core::Renderer::VkRenderData& renderData)
{
    std::shared_ptr<Assimp::Importer> importer = std::make_shared<Assimp::Importer>();
    importer->ReadFile(fileName, aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices |
                                     aiProcess_TransformUVCoords | aiProcess_GlobalScale);

    const aiScene* scene = importer->GetScene();

    if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
    {
        Logger::log(1, "%s error: Failed to load mesh %s \n", __FUNCTION__, fileName.c_str());
        return {};
    }

    Core::Utils::MeshData mesh;
    std::vector<Core::Renderer::VkTextureData> loadedTextures;
    mesh.importer = importer;
    processNode(mesh, scene->mRootNode, scene, glm::mat4(1.0f), renderData, loadedTextures);
    return mesh;
}

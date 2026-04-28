#include "ModelLoader.h"
#include "AssetManager.h"
#include "vk-renderer/Texture.h"
#include "animations/AnimationsUtils.h"
#include "assets/TextureAsset.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/gtc/type_ptr.hpp>
#include "utils/FileUtils.h"
#include "core/Assertion.h"

Core::Resources::MeshData Core::Assets::ModelLoader::loadMeshFromFile(const std::string& fileName,
                                                                      Renderer::VkRenderData& renderData)
{
    Assimp::Importer importer{};
    importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
    importer.ReadFile(fileName, aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices |
                                    aiProcess_TransformUVCoords | aiProcess_GlobalScale | aiProcess_CalcTangentSpace);

    const aiScene* scene = importer.GetScene();

    if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
    {
        Logger::log(1, "%s error: Failed to load mesh %s \n", __FUNCTION__, fileName.c_str());
        return {};
    }

    Resources::MeshData mesh;

    const std::string baseDir = Utils::FileUtils::getParentDirectory(fileName);

    std::unordered_map<std::string, int> globalBoneIndexMap;
    buildGlobalBoneIndexMap(scene, globalBoneIndexMap);

    processNodeHierarchy(mesh.rootNode, scene->mRootNode, scene, renderData, baseDir, globalBoneIndexMap);

    mesh.skeletonData.rootNode = Animations::AnimationsUtils::buildBoneHierarchy(scene->mRootNode);

    return mesh;
}

void Core::Assets::ModelLoader::collectPrimitivesRecursive(const Resources::MeshNode& node,
                                                           const glm::mat4& parentTransform,
                                                           std::vector<Resources::PrimitiveData>& outAllPrimitives)
{
    const glm::mat4 globalTransform = parentTransform * node.localTransform;

    for (auto& primitive : node.primitives)
    {
        Resources::PrimitiveData transformedPrimitive = primitive;
        for (auto& vertex : transformedPrimitive.vertices)
        {
            vertex.position = glm::vec3(globalTransform * glm::vec4(vertex.position, 1.0f));
            vertex.normal = glm::normalize(glm::mat3(globalTransform) * vertex.normal);
        }
        outAllPrimitives.push_back(std::move(transformedPrimitive));
    }

    for (const auto& child : node.children)
    {
        collectPrimitivesRecursive(child, globalTransform, outAllPrimitives);
    }
}
void Core::Assets::ModelLoader::buildGlobalBoneIndexMap(const aiScene* scene,
                                                        std::unordered_map<std::string, int>& outGlobalBoneIndexMap)
{
    int nextID = 0;
    for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
    {
        const aiMesh* mesh = scene->mMeshes[i];
        for (unsigned int b = 0; b < mesh->mNumBones; ++b)
        {
            if (std::string name = mesh->mBones[b]->mName.C_Str(); !outGlobalBoneIndexMap.contains(name))
            {
                outGlobalBoneIndexMap[name] = nextID++;
            }
        }
    }
}

void Core::Assets::ModelLoader::processNodeHierarchy(Resources::MeshNode& outNode, aiNode* node, const aiScene* scene,
                                                     Renderer::VkRenderData& renderData, const std::string& baseDir,
                                                     const std::unordered_map<std::string, int>& globalBoneIndexMap)
{
    outNode.name = node->mName.C_Str();

    outNode.localTransform = glm::transpose(glm::make_mat4(&node->mTransformation.a1));

    for (size_t i = 0; i < node->mNumMeshes; ++i)
    {
        const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        const aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        processMesh(outNode.primitives, mesh, material, renderData, baseDir, globalBoneIndexMap);
    }

    for (size_t i = 0; i < node->mNumChildren; ++i)
    {
        Resources::MeshNode childNode;
        processNodeHierarchy(childNode, node->mChildren[i], scene, renderData, baseDir, globalBoneIndexMap);
        outNode.children.push_back(std::move(childNode));
    }
}

void Core::Assets::ModelLoader::processMesh(std::vector<Resources::PrimitiveData>& outPrimitives, const aiMesh* mesh,
                                            const aiMaterial* material, Renderer::VkRenderData& renderData,
                                            const std::string& baseDir,
                                            const std::unordered_map<std::string, int>& globalBoneIndexMap)
{
    Resources::PrimitiveData primitiveData;
    Renderer::MaterialInfo materialInfo = {};

    aiColor4D baseColor(1.f, 1.f, 1.f, 1.f);
    if (AI_SUCCESS == material->Get(AI_MATKEY_BASE_COLOR, baseColor))
    {
        materialInfo.baseColorFactor = glm::vec4(baseColor.r, baseColor.g, baseColor.b, baseColor.a);
    }
    else if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_DIFFUSE, baseColor))
    {
        materialInfo.baseColorFactor = glm::vec4(baseColor.r, baseColor.g, baseColor.b, baseColor.a);
    }

    material->Get(AI_MATKEY_METALLIC_FACTOR, materialInfo.metallicFactor);
    material->Get(AI_MATKEY_ROUGHNESS_FACTOR, materialInfo.roughnessFactor);

    if (material->GetTextureCount(aiTextureType_BASE_COLOR) > 0 || material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
    {
        aiTextureType textureType =
            material->GetTextureCount(aiTextureType_BASE_COLOR) > 0 ? aiTextureType_BASE_COLOR : aiTextureType_DIFFUSE;

        aiString path;
        if (material->GetTexture(textureType, 0, &path) == AI_SUCCESS)
        {
            auto convertMode = [](const int mode)
            {
                return mode == aiTextureMapMode_Clamp ? VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
                                                      : VK_SAMPLER_ADDRESS_MODE_REPEAT;
            };

            VkSamplerAddressMode addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            VkSamplerAddressMode addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;

            int mapModeU = aiTextureMapMode_Wrap;
            int mapModeV = aiTextureMapMode_Wrap;
            if (material->Get(AI_MATKEY_MAPPINGMODE_U(textureType, 0), mapModeU) == AI_SUCCESS)
            {
                addressModeU = convertMode(mapModeU);
            }
            if (material->Get(AI_MATKEY_MAPPINGMODE_V(textureType, 0), mapModeV) == AI_SUCCESS)
            {
                addressModeV = convertMode(mapModeV);
            }

            std::string textureFileName = baseDir + path.C_Str();
            auto textureAsset = AssetManager::getInstance().getOrCreate<TextureAsset>(textureFileName, renderData,
                                                                                      VK_FORMAT_R8G8B8A8_SRGB);

            if (textureAsset)
            {
                primitiveData.textures[aiTextureType_DIFFUSE] = textureAsset;
                materialInfo.useAlbedoMap = 1;
            }
        }
    }

    if (material->GetTextureCount(aiTextureType_NORMALS) > 0)
    {
        aiString path;
        if (material->GetTexture(aiTextureType_NORMALS, 0, &path) == AI_SUCCESS)
        {
            std::string textureFileName = baseDir + path.C_Str();
            auto textureAsset = AssetManager::getInstance().getOrCreate<TextureAsset>(textureFileName, renderData,
                                                                                      VK_FORMAT_R8G8B8A8_UNORM);

            if (textureAsset)
            {
                primitiveData.textures[aiTextureType_NORMALS] = textureAsset;
                materialInfo.useNormalMap = 1;
            }
        }
    }

    if (material->GetTextureCount(aiTextureType_METALNESS) > 0 ||
        material->GetTextureCount(aiTextureType_DIFFUSE_ROUGHNESS) > 0)
    {
        aiTextureType textureType = material->GetTextureCount(aiTextureType_METALNESS) > 0
                                        ? aiTextureType_METALNESS
                                        : aiTextureType_DIFFUSE_ROUGHNESS;

        aiString path;
        if (material->GetTexture(textureType, 0, &path) == AI_SUCCESS)
        {
            std::string textureFileName = baseDir + path.C_Str();
            auto textureAsset = AssetManager::getInstance().getOrCreate<TextureAsset>(textureFileName, renderData,
                                                                                      VK_FORMAT_R8G8B8A8_UNORM);

            if (textureAsset)
            {
                primitiveData.textures[aiTextureType_METALNESS] = textureAsset;
                materialInfo.useMetallicRoughnessMap = 1;
            }
        }
    }

    if (material->GetTextureCount(aiTextureType_AMBIENT_OCCLUSION) > 0 ||
        material->GetTextureCount(aiTextureType_LIGHTMAP) > 0)
    {
        aiTextureType textureType = material->GetTextureCount(aiTextureType_AMBIENT_OCCLUSION) > 0
                                        ? aiTextureType_AMBIENT_OCCLUSION
                                        : aiTextureType_LIGHTMAP;

        aiString path;
        if (material->GetTexture(textureType, 0, &path) == AI_SUCCESS)
        {
            std::string textureFileName = baseDir + path.C_Str();
            auto textureAsset = AssetManager::getInstance().getOrCreate<TextureAsset>(textureFileName, renderData,
                                                                                      VK_FORMAT_R8G8B8A8_UNORM);

            if (textureAsset)
            {
                primitiveData.textures[aiTextureType_AMBIENT_OCCLUSION] = textureAsset;
                materialInfo.useAOMap = 1;
            }
        }
    }

    if (material->GetTextureCount(aiTextureType_EMISSIVE) > 0)
    {
        aiString path;
        if (material->GetTexture(aiTextureType_EMISSIVE, 0, &path) == AI_SUCCESS)
        {
            std::string textureFileName = baseDir + path.C_Str();
            auto textureAsset = AssetManager::getInstance().getOrCreate<TextureAsset>(textureFileName, renderData,
                                                                                      VK_FORMAT_R8G8B8A8_SRGB);

            if (textureAsset)
            {
                primitiveData.textures[aiTextureType_EMISSIVE] = textureAsset;
                materialInfo.useEmissiveMap = 1;
            }
        }
    }

    aiColor3D emissiveColor(0.f, 0.f, 0.f);
    if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveColor))
    {
        materialInfo.emissiveFactor = glm::vec4(emissiveColor.r, emissiveColor.g, emissiveColor.b, 1.f);
    }

    primitiveData.material = materialInfo;

    VkDescriptorSetLayout layout =
        renderData.rdDescriptorLayoutCache->getLayout(Renderer::DescriptorLayoutType::PBRTextures);

    if (!renderData.rdDescriptorAllocator->allocate(layout, primitiveData.materialDescriptorSet))
    {
        Logger::log(1, "Failed to allocate material descriptor set via Allocator!\n");
    }
    else
    {
        auto makeInfo = [](const std::shared_ptr<TextureAsset>& textureAsset)
        {
            const auto& textureData = textureAsset->getTextureData();
            VkDescriptorImageInfo info{};
            info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            info.imageView = textureData.imageView;
            info.sampler = textureData.sampler;
            return info;
        };

        VkDescriptorImageInfo imageInfos[5]{};

        imageInfos[0] = primitiveData.textures.contains(aiTextureType_DIFFUSE)
                            ? makeInfo(primitiveData.textures[aiTextureType_DIFFUSE])
                            : makeInfo(renderData.rdPlaceholderTexture);
        imageInfos[1] = primitiveData.textures.contains(aiTextureType_NORMALS)
                            ? makeInfo(primitiveData.textures[aiTextureType_NORMALS])
                            : makeInfo(renderData.rdPlaceholderTexture);
        imageInfos[2] = primitiveData.textures.contains(aiTextureType_METALNESS)
                            ? makeInfo(primitiveData.textures[aiTextureType_METALNESS])
                            : makeInfo(renderData.rdPlaceholderTexture);
        imageInfos[3] = primitiveData.textures.contains(aiTextureType_AMBIENT_OCCLUSION)
                            ? makeInfo(primitiveData.textures[aiTextureType_AMBIENT_OCCLUSION])
                            : makeInfo(renderData.rdPlaceholderTexture);
        imageInfos[4] = primitiveData.textures.contains(aiTextureType_EMISSIVE)
                            ? makeInfo(primitiveData.textures[aiTextureType_EMISSIVE])
                            : makeInfo(renderData.rdPlaceholderTexture);

        std::array<VkWriteDescriptorSet, 5> writes{};
        for (uint32_t i = 0; i < 5; ++i)
        {
            writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writes[i].dstSet = primitiveData.materialDescriptorSet;
            writes[i].dstBinding = i;
            writes[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            writes[i].descriptorCount = 1;
            writes[i].pImageInfo = &imageInfos[i];
        }

        vkUpdateDescriptorSets(renderData.rdVkbDevice.device, static_cast<uint32_t>(writes.size()), writes.data(), 0,
                               nullptr);
    }

    for (size_t i = 0; i < mesh->mNumVertices; ++i)
    {
        Renderer::Vertex vertex{};
        if (mesh->HasPositions())
        {
            aiVector3D& position = mesh->mVertices[i];
            vertex.position = {position.x, position.y, position.z};
        }

        if (mesh->HasNormals())
        {
            aiVector3D& normal = mesh->mNormals[i];
            vertex.normal = {normal.x, normal.y, normal.z};
        }

        if (mesh->HasTangentsAndBitangents())
        {
            glm::vec3 n = {mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z};
            glm::vec3 t = {mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z};
            glm::vec3 b = {mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z};

            float w = glm::dot(glm::cross(n, t), b) < 0.0f ? -1.0f : 1.0f;
            vertex.tangent = glm::vec4(t, w);
        }

        if (mesh->HasVertexColors(0))
        {
            aiColor4D& vertColor = mesh->mColors[0][i];
            vertex.color = {vertColor.r, vertColor.g, vertColor.b, vertColor.a};
        }

        if (mesh->HasTextureCoords(0))
        {
            aiVector3D& texCoords = mesh->mTextureCoords[0][i];
            vertex.uv = {texCoords.x, texCoords.y};
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
        processBones(primitiveData, mesh, globalBoneIndexMap);
    }

    outPrimitives.emplace_back(std::move(primitiveData));
}

void Core::Assets::ModelLoader::processBones(Resources::PrimitiveData& primitiveData, const aiMesh* mesh,
                                             const std::unordered_map<std::string, int>& globalBoneIndexMap)
{
    primitiveData.bones.bones.resize(globalBoneIndexMap.size());
    primitiveData.bones.finalTransforms.resize(globalBoneIndexMap.size(), glm::mat4(1.0f));

    for (unsigned int i = 0; i < mesh->mNumBones; i++)
    {
        processSingleBone(primitiveData, mesh->mBones[i], globalBoneIndexMap);
    }

    primitiveData.bones.finalTransforms.resize(primitiveData.bones.bones.size(), glm::mat4(1.0));
}

void Core::Assets::ModelLoader::processSingleBone(Resources::PrimitiveData& primitiveData, const aiBone* bone,
                                                  const std::unordered_map<std::string, int>& globalBoneIndexMap)
{
    const std::string boneName = bone->mName.C_Str();
    const int boneID = globalBoneIndexMap.at(boneName);

    Logger::log(1, "Bone '%s': num vertices affected by this bone: %d\n", boneName.c_str(), bone->mNumWeights);

    primitiveData.bones.boneNameToIndexMap[boneName] = boneID;
    primitiveData.bones.bones[boneID] =
        Animations::Bone{Animations::AnimationsUtils::convertMatrixToGlm(bone->mOffsetMatrix)};

    for (unsigned int boneWeight = 0; boneWeight < bone->mNumWeights; ++boneWeight)
    {
        const unsigned int vertexID = bone->mWeights[boneWeight].mVertexId;
        const float weight = bone->mWeights[boneWeight].mWeight;
        setVertexBoneData(primitiveData.vertices[vertexID], boneID, weight);
    }
}

void Core::Assets::ModelLoader::setVertexBoneData(Renderer::Vertex& vertex, int id, float weight)
{
    for (size_t i = 0; i < maxNumberOfBonesPerVertex; i++)
    {
        if (vertex.weights[i] == 0.f)
        {
            vertex.boneID[i] = id;
            vertex.weights[i] = weight;
            break;
        }
    }
}

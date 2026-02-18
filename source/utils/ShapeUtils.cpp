#include "ShapeUtils.h"
#include "vk-renderer/Texture.h"
#include "animations/AnimationsUtils.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/gtc/type_ptr.hpp>
#include <filesystem>

int getBoneID(Core::Utils::PrimitiveData& primitiveData, const aiBone* bone)
{
    int boneID;
    const std::string boneName = bone->mName.C_Str();

    if (!primitiveData.bones.boneNameToIndexMap.contains(boneName))
    {
        boneID = static_cast<int>(primitiveData.bones.bones.size());
        Core::Animations::Bone newBone;
        primitiveData.bones.bones.emplace_back(newBone);
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

void setVertexBoneData(Core::Renderer::Vertex& vertex, int id, float weight)
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

void processMesh(std::vector<Core::Utils::PrimitiveData>& outPrimitives, const aiMesh* mesh, const aiScene* scene,
                 const aiMaterial* material, Core::Renderer::VkRenderData& renderData, const std::string& baseDir)
{
    Core::Utils::PrimitiveData primitiveData;
    Core::Renderer::MaterialInfo materialInfo = {};

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
            std::string textureFileName = baseDir + path.C_Str();
            Core::Renderer::VkTextureData textureData;
            std::future<bool> textureLoadFuture =
                Core::Renderer::Texture::loadTexture(renderData, textureData, textureFileName);

            if (textureLoadFuture.get())
            {
                primitiveData.textures[aiTextureType_DIFFUSE] = textureData;
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
            Core::Renderer::VkTextureData textureData;
            std::future<bool> textureLoadFuture = Core::Renderer::Texture::loadTexture(
                renderData, textureData, textureFileName, VK_FORMAT_R8G8B8A8_UNORM);

            if (textureLoadFuture.get())
            {
                primitiveData.textures[aiTextureType_NORMALS] = textureData;
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
            Core::Renderer::VkTextureData textureData;
            std::future<bool> textureLoadFuture = Core::Renderer::Texture::loadTexture(
                renderData, textureData, textureFileName, VK_FORMAT_R8G8B8A8_UNORM);

            if (textureLoadFuture.get())
            {
                primitiveData.textures[aiTextureType_METALNESS] = textureData;
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
            Core::Renderer::VkTextureData textureData;
            std::future<bool> textureLoadFuture = Core::Renderer::Texture::loadTexture(
                renderData, textureData, textureFileName, VK_FORMAT_R8G8B8A8_UNORM);

            if (textureLoadFuture.get())
            {
                primitiveData.textures[aiTextureType_AMBIENT_OCCLUSION] = textureData;
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
            Core::Renderer::VkTextureData textureData;
            std::future<bool> textureLoadFuture =
                Core::Renderer::Texture::loadTexture(renderData, textureData, textureFileName);

            if (textureLoadFuture.get())
            {
                primitiveData.textures[aiTextureType_EMISSIVE] = textureData;
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
        renderData.rdDescriptorLayoutCache->getLayout(Core::Renderer::DescriptorLayoutType::PBRTextures);

    if (!renderData.rdDescriptorAllocator->allocate(layout, primitiveData.materialDescriptorSet))
    {
        Logger::log(1, "Failed to allocate material descriptor set via Allocator!\n");
    }
    else
    {
        auto makeInfo = [](const Core::Renderer::VkTextureData& tex)
        {
            VkDescriptorImageInfo info{};
            info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            info.imageView = tex.imageView;
            info.sampler = tex.sampler;
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
        Core::Renderer::Vertex vertex{};
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
            aiVector3D& tangent = mesh->mTangents[i];
            vertex.tangent = glm::vec4(tangent.x, tangent.y, tangent.z, mesh->mBitangents ? 1.0f : -1.0f);
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
        processBones(primitiveData, mesh);
    }

    outPrimitives.emplace_back(std::move(primitiveData));
}

void processNodeHierarchy(Core::Utils::MeshNode& outNode, aiNode* node, const aiScene* scene,
                          Core::Renderer::VkRenderData& renderData, const std::string& baseDir)
{
    outNode.name = node->mName.C_Str();

    outNode.localTransform = glm::transpose(glm::make_mat4(&node->mTransformation.a1));

    for (size_t i = 0; i < node->mNumMeshes; ++i)
    {
        const aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        const aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        processMesh(outNode.primitives, mesh, scene, material, renderData, baseDir);
    }

    for (size_t i = 0; i < node->mNumChildren; ++i)
    {
        Core::Utils::MeshNode childNode;
        processNodeHierarchy(childNode, node->mChildren[i], scene, renderData, baseDir);
        outNode.children.push_back(std::move(childNode));
    }
}

// TODO
// add meshes cache here!
Core::Utils::MeshData Core::Utils::loadMeshFromFile(const std::string& fileName, Renderer::VkRenderData& renderData)
{
    Assimp::Importer importer{};
    importer.ReadFile(fileName, aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_JoinIdenticalVertices |
                                    aiProcess_TransformUVCoords | aiProcess_GlobalScale | aiProcess_CalcTangentSpace);

    const aiScene* scene = importer.GetScene();

    if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
    {
        Logger::log(1, "%s error: Failed to load mesh %s \n", __FUNCTION__, fileName.c_str());
        return {};
    }

    const std::filesystem::path meshPath(fileName);
    std::string baseDir = meshPath.parent_path().string();
    if (!baseDir.empty())
    {
        baseDir += "/";
    }

    MeshData mesh;
    processNodeHierarchy(mesh.rootNode, scene->mRootNode, scene, renderData, baseDir);
    mesh.skeleton.setRootNode(Animations::AnimationsUtils::buildBoneHierarchy(scene->mRootNode));
    return mesh;
}

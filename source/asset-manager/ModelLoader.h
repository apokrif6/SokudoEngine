#pragma once

#include "resources/Mesh.h"

struct aiMesh;
struct aiScene;
struct aiNode;
struct aiBone;

namespace Core::Assets
{
class ModelLoader
{
public:
    static Resources::MeshData loadMeshFromFile(const std::string& fileName, Renderer::VkRenderData& renderData);

    static void collectPrimitivesRecursive(const Resources::MeshNode& node, const glm::mat4& parentTransform,
                                           std::vector<Resources::PrimitiveData>& outAllPrimitives);

private:
    static void buildGlobalBoneIndexMap(const aiScene* scene,
                                        std::unordered_map<std::string, int>& outGlobalBoneIndexMap);

    static void fillBoneParents(const aiNode* node, int parentIndex,
                                const std::unordered_map<std::string, int>& globalBoneIndexMap,
                                std::vector<int>& outParents);

    static void processNodeHierarchy(Resources::MeshNode& outNode, aiNode* node, const aiScene* scene,
                                     Renderer::VkRenderData& renderData, const std::string& baseDir,
                                     const std::unordered_map<std::string, int>& globalBoneIndexMap);

    static void processMesh(std::vector<Resources::PrimitiveData>& outPrimitives, const aiMesh* mesh,
                            const aiMaterial* material, Renderer::VkRenderData& renderData, const std::string& baseDir,
                            const std::unordered_map<std::string, int>& globalBoneIndexMap);

    static void processBones(Resources::PrimitiveData& primitiveData, const aiMesh* mesh,
                             const std::unordered_map<std::string, int>& globalBoneIndexMap);

    static void processSingleBone(Resources::PrimitiveData& primitiveData, const aiBone* bone,
                                  const std::unordered_map<std::string, int>& globalBoneIndexMap);

    static void setVertexBoneData(Renderer::Vertex& vertex, int id, float weight);
};
} // namespace Core::Assets

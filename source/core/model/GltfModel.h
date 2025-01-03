#pragma once

#include <string>
#include <vector>
#include <memory>
#include <vulkan/vulkan.h>
#include <tiny_gltf.h>
#include "core/vk-renderer/Texture.h"
#include "core/vk-renderer/VkRenderData.h"
#include "GltfNode.h"

namespace Core::Model
{
class GltfModel
{
public:
  bool loadModel(Core::Renderer::VkRenderData& renderData, Core::Renderer::VkGltfRenderData& gltfRenderData, const std::string& modelFilename,
                 const std::string& textureFilename);

  void draw(const Core::Renderer::VkRenderData& renderData, const Core::Renderer::VkGltfRenderData& gltfRenderData);

  void cleanup(Core::Renderer::VkRenderData& renderData, Core::Renderer::VkGltfRenderData& gltfRenderData);

  void uploadVertexBuffers(Core::Renderer::VkRenderData& renderData, Core::Renderer::VkGltfRenderData& gltfRenderData);

  void uploadPositionBuffer(Core::Renderer::VkRenderData& renderData, Core::Renderer::VkGltfRenderData& gltfRenderData);

  void applyVertexSkinning(bool enableSkinning);

  void uploadIndexBuffer(Core::Renderer::VkRenderData& renderData, Core::Renderer::VkGltfRenderData& gltfRenderData);

  std::shared_ptr<Core::Renderer::VkMesh> getSkeleton(bool enableSkinning);

private:
  void createVertexBuffers(Core::Renderer::VkRenderData& renderData, Core::Renderer::VkGltfRenderData& gltfRenderData);

  void createIndexBuffer(Core::Renderer::VkRenderData& renderData, Core::Renderer::VkGltfRenderData& gltfRenderData);

  int getTriangleCount();

  void getSkeletonPerNode(const std::shared_ptr<GltfNode>& treeNode, bool enableSkinning);

  void getJointData();

  void getWeightData();

  void getInvBindMatrices();

  void getNodes(const std::shared_ptr<GltfNode>& treeNode);

  void getNodeData(const std::shared_ptr<GltfNode>& treeNode, const glm::mat4& parentNodeMatrix);

  std::vector<glm::tvec4<uint16_t>> mJointVec{};

  std::vector<glm::vec4> mWeightVec{};

  std::vector<glm::mat4> mInverseBindMatrices{};

  std::vector<glm::mat4> mJointMatrices{};

  std::vector<int> mAttribAccessors{};

  std::vector<int> mNodeToJoint{};

  std::vector<glm::vec3> mAlteredPositions{};

  std::shared_ptr<GltfNode> mRootNode = nullptr;

  std::shared_ptr<tinygltf::Model> mModel = nullptr;

  std::shared_ptr<Core::Renderer::VkMesh> mSkeletonMesh = nullptr;

  std::map<std::string, GLint> attributes = {{"POSITION", 0}, {"NORMAL", 1}, {"TEXCOORD_0", 2}};
};
}
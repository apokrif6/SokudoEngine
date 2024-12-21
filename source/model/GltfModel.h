#pragma once

#include <string>
#include <vector>
#include <memory>
#include <vulkan/vulkan.h>
#include <tiny_gltf.h>
#include "vk-renderer/Texture.h"
#include "vk-renderer/VkRenderData.h"

class GltfModel
{
  public:
    bool loadModel(VkRenderData& renderData, VkGltfRenderData& gltfRenderData, std::string modelFilename,
                   const std::string& textureFilename);

    void draw(VkRenderData& renderData, VkGltfRenderData& gltfRenderData);

    void cleanup(VkRenderData& renderData, VkGltfRenderData& gltfRenderData);

    void uploadVertexBuffers(VkRenderData& renderData, VkGltfRenderData& gltfRenderData);

    void uploadIndexBuffer(VkRenderData& renderData, VkGltfRenderData& gltfRenderData);

  private:
    void createVertexBuffers(VkRenderData& renderData, VkGltfRenderData& gltfRenderData);

    void createIndexBuffer(VkRenderData& renderData, VkGltfRenderData& gltfRenderData);

    int getTriangleCount();

    std::shared_ptr<tinygltf::Model> mModel = nullptr;

    std::map<std::string, GLint> attributes = {{"POSITION", 0}, {"NORMAL", 1}, {"TEXCOORD_0", 2}};
};
#pragma once

#include <memory>
#include "core/vk-renderer/VkRenderData.h"
#include "tiny_gltf.h"

namespace Core::Model
{
class GltfSphere
{
  public:
    bool loadModel(Core::Renderer::VkRenderData& renderData, Core::Renderer::VkGltfSphereRenderData& gltfSphereRenderData,
                   const std::string& modelFilename, const std::string& textureFilename);

    void draw(const Core::Renderer::VkRenderData& renderData, const Core::Renderer::VkGltfSphereRenderData& gltfSphereRenderData);

    void uploadVertexBuffers(Core::Renderer::VkRenderData& renderData, Core::Renderer::VkGltfSphereRenderData& gltfSphereRenderData);

    void uploadIndexBuffer(Core::Renderer::VkRenderData& renderData, Core::Renderer::VkGltfSphereRenderData& gltfSphereRenderData);

    void cleanup(Core::Renderer::VkRenderData& renderData, Core::Renderer::VkGltfSphereRenderData& gltfSphereRenderData);

  private:
    std::shared_ptr<tinygltf::Model> mModel = nullptr;

    std::vector<int> mAttribAccessors{};

    std::map<std::string, GLint> attributes = {{"POSITION", 0}, {"NORMAL", 1}, {"TEXCOORD_0", 2}};

    void createVertexBuffers(Renderer::VkRenderData& renderData, Renderer::VkGltfSphereRenderData& gltfSphereRenderData);

    void createIndexBuffer(Renderer::VkRenderData& renderData, Renderer::VkGltfSphereRenderData& gltfSphereRenderData);

    int getTriangleCount();
};
}
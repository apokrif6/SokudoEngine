#pragma once

#include "Asset.h"
#include "resources/Mesh.h"
#include "vk-renderer/VkRenderData.h"

namespace Core::Assets
{
class MeshAsset : public Asset
{
public:
    MeshAsset(const std::string& path, Renderer::VkRenderData& renderData);

    ~MeshAsset() override = default;

    [[nodiscard]] const Resources::MeshData& getMeshData() const { return mMeshData; }

private:
    Resources::MeshData mMeshData;
};
} // namespace Core::Assets

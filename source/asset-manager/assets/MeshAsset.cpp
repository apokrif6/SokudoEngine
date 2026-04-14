
#include "MeshAsset.h"

Core::Assets::MeshAsset::MeshAsset(const std::string& path, Renderer::VkRenderData& renderData) : Asset(path)
{
    mMeshData = Utils::loadMeshFromFile(path, renderData);
}

#include "MeshAsset.h"
#include "asset-manager/ModelLoader.h"

Core::Assets::MeshAsset::MeshAsset(const std::string& path, Renderer::VkRenderData& renderData) : Asset(path)
{
    mMeshData = ModelLoader::loadMeshFromFile(path, renderData);
}

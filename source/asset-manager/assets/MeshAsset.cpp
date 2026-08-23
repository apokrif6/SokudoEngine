#include "MeshAsset.h"
#include "asset-manager/ModelLoader.h"
#include "tools/Logger.h"

Core::Assets::MeshAsset::MeshAsset(const std::string& path, Renderer::VkRenderData& renderData)
    : Asset(path), mMeshData(ModelLoader::loadMeshFromFile(path, renderData))
{
    if (mMeshData.rootNode.primitives.empty())
    {
        Logger::log(1, "Failed to load AnimationAsset: %s", path.c_str());
    }
}

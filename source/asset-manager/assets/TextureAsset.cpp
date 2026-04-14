#include "TextureAsset.h"

#include "tools/Logger.h"
#include "vk-renderer/Texture.h"

Core::Assets::TextureAsset::TextureAsset(const std::string& path, Renderer::VkRenderData& renderData, VkFormat format)
    : Asset(path), mRenderData(renderData)
{
    auto future = Renderer::Texture::loadTexture(mRenderData, mTextureData, path, format);
    // TODO
    // make it proper async loading
    if (!future.get())
    {
        Logger::log(1, "Failed to load TextureAsset: %s", path.c_str());
    }
}

Core::Assets::TextureAsset::~TextureAsset()
{
    Logger::log(1, "AssetManager: TextureAsset %s is being freed", mPath.c_str());
    Renderer::Texture::cleanup(mRenderData, mTextureData);
}
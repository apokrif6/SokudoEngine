#pragma once

#include "Asset.h"
#include "vk-renderer/VkRenderData.h"

namespace Core::Assets
{
class TextureAsset : public Asset
{
public:
    TextureAsset(const std::string& path, Renderer::VkRenderData& renderData, VkFormat format);

    ~TextureAsset() override;

    [[nodiscard]] Renderer::VkTextureData& getTextureData() { return mTextureData; }

private:
    Renderer::VkTextureData mTextureData;
    Renderer::VkRenderData& mRenderData;
};
} // namespace Core::Assets
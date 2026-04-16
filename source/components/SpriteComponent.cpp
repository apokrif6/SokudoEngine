#include "SpriteComponent.h"

#include "TransformComponent.h"
#include "asset-manager/AssetManager.h"
#include "asset-manager/assets/TextureAsset.h"
#include "utils/FileUtils.h"
#include "vk-renderer/Texture.h"

void Core::Component::SpriteComponent::update(Renderer::VkRenderData& renderData)
{
    auto* transformComponent = getOwner()->getComponent<TransformComponent>();
    if (!transformComponent)
    {
        return;
    }

    if (!mPrimitive)
    {
        return;
    }

    mPrimitive->uploadUniformBuffer(renderData, transformComponent->getWorldMatrix());
}

void Core::Component::SpriteComponent::draw(Renderer::VkRenderData& renderData)
{
    if (!mPrimitive)
    {
        return;
    }

    mPrimitive->draw(renderData, Renderer::PrimitiveRenderType::Sprite);
}

void Core::Component::SpriteComponent::cleanup(Renderer::VkRenderData& renderData) { mPrimitive->cleanup(renderData); }

YAML::Node Core::Component::SpriteComponent::serialize() const
{
    YAML::Node node;

    node["spriteFile"] = mSpriteFilePath;

    return node;
}

void Core::Component::SpriteComponent::deserialize(const YAML::Node& node)
{
    if (node["spriteFile"])
    {
        const std::string path = node["spriteFile"].as<std::string>();
        loadSpriteFromFile(path);
    }
}

void Core::Component::SpriteComponent::loadSpriteFromFile(const std::string& path)
{
    mSpriteFilePath = mSpriteFilePath = Utils::FileUtils::getRelativePath(path);

    auto& renderData = Engine::getInstance().getRenderData();

    Resources::PrimitiveData data;

    createSpritePrimitiveData(path, renderData, data);

    mPrimitive = std::make_unique<Renderer::Primitive>(data.vertices, data.indices, data.textures, data.material,
                                                       data.materialDescriptorSet, Animations::BonesInfo{}, renderData);

    mPrimitive->uploadVertexBuffer(renderData);
    mPrimitive->uploadIndexBuffer(renderData);
}

void Core::Component::SpriteComponent::createSpritePrimitiveData(const std::string& spritePath,
                                                                 Renderer::VkRenderData& renderData,
                                                                 Resources::PrimitiveData& outPrimitiveData)
{
    outPrimitiveData.vertices = {
        {{-0.5f, -0.5f, 0.0f}, {0, 0, 1}, {0, 0, 0, 1}, {1, 1, 1, 1}, {0.0f, 0.0f}}, // LB
        {{0.5f, -0.5f, 0.0f}, {0, 0, 1}, {0, 0, 0, 1}, {1, 1, 1, 1}, {1.0f, 0.0f}},  // RB
        {{0.5f, 0.5f, 0.0f}, {0, 0, 1}, {0, 0, 0, 1}, {1, 1, 1, 1}, {1.0f, 1.0f}},   // RT
        {{-0.5f, 0.5f, 0.0f}, {0, 0, 1}, {0, 0, 0, 1}, {1, 1, 1, 1}, {0.0f, 1.0f}}   // LT
    };
    outPrimitiveData.indices = {0, 1, 2, 2, 3, 0};

    auto textureAsset = Assets::AssetManager::getInstance().getOrCreate<Assets::TextureAsset>(spritePath, renderData,
                                                                                              VK_FORMAT_R8G8B8A8_SRGB);

    if (textureAsset)
    {
        outPrimitiveData.textures[aiTextureType_DIFFUSE] = textureAsset;
    }

    if (auto layout = renderData.rdDescriptorLayoutCache->getLayout(Renderer::DescriptorLayoutType::SingleTexture);
        renderData.rdDescriptorAllocator->allocate(layout, outPrimitiveData.materialDescriptorSet))
    {
        const auto& textureData = textureAsset->getTextureData();

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = textureData.imageView;
        imageInfo.sampler = textureData.sampler;

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = outPrimitiveData.materialDescriptorSet;
        write.dstBinding = 0;
        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write.descriptorCount = 1;
        write.pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(renderData.rdVkbDevice.device, 1, &write, 0, nullptr);
    }
}
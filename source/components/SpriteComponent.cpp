#include "SpriteComponent.h"

#include "TransformComponent.h"
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

    Utils::createSpritePrimitiveData(path, renderData, data);

    mPrimitive = std::make_unique<Renderer::Primitive>(data.vertices, data.indices, data.textures, data.material,
                                                       data.materialDescriptorSet, Animations::BonesInfo{}, renderData);

    mPrimitive->uploadVertexBuffer(renderData);
    mPrimitive->uploadIndexBuffer(renderData);
}

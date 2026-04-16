#pragma once

#include "Component.h"
#include "engine/Engine.h"

namespace Core::Component
{
class SpriteComponent : public Component
{
public:
    [[nodiscard]] std::string_view getTypeName() const override { return "SpriteComponent"; }

    void update(Renderer::VkRenderData& renderData) override;

    void draw(Renderer::VkRenderData& renderData) override;

    void cleanup(Renderer::VkRenderData& renderData) override;

    [[nodiscard]] YAML::Node serialize() const override;
    void deserialize(const YAML::Node& node) override;

    void loadSpriteFromFile(const std::string& path);

private:
    std::unique_ptr<Renderer::Primitive> mPrimitive;

    std::string mSpriteFilePath;

    void createSpritePrimitiveData(const std::string& spritePath, Renderer::VkRenderData& renderData,
                                   Resources::PrimitiveData& outPrimitiveData);
};
} // namespace Core::Component

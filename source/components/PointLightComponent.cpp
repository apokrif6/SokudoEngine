#include "PointLightComponent.h"
#include "TransformComponent.h"
#include "scene/objects/SceneObject.h"

void Core::Component::PointLightComponent::update(Renderer::VkRenderData& renderData)
{
    if (!mOwner)
    {
        return;
    }

    const auto* transform = mOwner->getComponent<TransformComponent>();
    if (!transform)
    {
        return;
    }

    if (int& count = renderData.rdGlobalSceneData.lightCount.x; count < Renderer::maxNumberOfLights)
    {
        renderData.rdGlobalSceneData.lights[count].position = glm::vec4(transform->getPosition(), mRadius);
        renderData.rdGlobalSceneData.lights[count].color = glm::vec4(mColor, mIntensity);

        ++count;
    }
}

YAML::Node Core::Component::PointLightComponent::serialize() const
{
    YAML::Node node;
    node["color"] = std::vector{mColor.r, mColor.g, mColor.b};
    node["intensity"] = mIntensity;
    node["radius"] = mRadius;
    return node;
}

void Core::Component::PointLightComponent::deserialize(const YAML::Node& node)
{

    auto colorNode = node["color"];
    mColor = glm::vec3(colorNode[0].as<float>(), colorNode[1].as<float>(), colorNode[2].as<float>());

    mIntensity = node["intensity"].as<float>();

    mRadius = node["radius"].as<float>();
}
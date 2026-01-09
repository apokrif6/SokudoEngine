#pragma once

#include <yaml-cpp/node/node.h>

#include "Component.h"
#include "core/scene/Transform.h"

// TODO
// add serialization
namespace Core::Component
{
class TransformComponent : public Component
{
public:
    Scene::Transform transform;

    std::string_view getTypeName() const override { return "TransformComponent"; }

    YAML::Node serialize() const override;
    void deserialize(const YAML::Node& node) override;
};
} // namespace Core::Component
#pragma once

#include "Component.h"

namespace Core::Component
{

class IKTargetComponent : public Component
{
public:
    IKTargetComponent() = default;
    ~IKTargetComponent() override = default;

    [[nodiscard]] std::string_view getTypeName() const override { return "IKTargetComponent"; }

    [[nodiscard]] YAML::Node serialize() const override { return {}; }
    void deserialize(const YAML::Node& node) override {}

    [[nodiscard]] glm::vec3 getTargetWorldPosition() const;
};
} // namespace Core::Component
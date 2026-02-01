#pragma once

#include <functional>
#include <memory>
#include "TransformComponent.h"
#include "MeshComponent.h"
#include "RotatingComponent.h"

namespace Core::Component
{
class ComponentFactory
{
public:
    using Creator = std::function<std::unique_ptr<Component>()>;

    static void registerAll()
    {
        registerComponent("TransformComponent", [] { return std::make_unique<TransformComponent>(); });
        registerComponent("MeshComponent", [] { return std::make_unique<MeshComponent>(); });
        registerComponent("RotatingComponent", [] { return std::make_unique<RotatingComponent>(); });
    }

    static void registerComponent(const std::string& name, Creator creator) { getMap()[name] = std::move(creator); }

    static std::unique_ptr<Component> create(const std::string& name) { return getMap().at(name)(); }

private:
    static std::unordered_map<std::string, Creator>& getMap()
    {
        static std::unordered_map<std::string, Creator> map;
        return map;
    }
};
} // namespace Core::Component
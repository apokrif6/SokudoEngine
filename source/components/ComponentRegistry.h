#pragma once

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <utility>

#include "Component.h"
#include "scene/objects/SceneObject.h"

namespace Core::Scene
{
class SceneObject;
}

namespace Core::Component
{
class ComponentRegistry
{
public:
    using Creator = std::function<std::unique_ptr<Component>()>;

    using ContainsFunction = std::function<bool(const Scene::SceneObject&)>;

    using AddFunction = std::function<void(Scene::SceneObject&)>;

    using InspectorRenderer = std::function<void()>;

    struct Descriptor
    {
        std::string name;
        std::type_index type{typeid(void)};

        Creator create;
        ContainsFunction contains;
        AddFunction add;
        InspectorRenderer renderInspector;

        bool addable = true;
    };

    template <typename T>
    static void registerComponent(std::string name, InspectorRenderer renderer, bool addable = true)
    {
        static_assert(std::is_base_of_v<Component, T>, "T must derive from Component");

        Descriptor descriptor;

        descriptor.name = std::move(name);
        descriptor.type = std::type_index(typeid(T));

        descriptor.create = []() -> std::unique_ptr<Component> { return std::make_unique<T>(); };

        descriptor.contains = [](const Scene::SceneObject& object) { return object.getComponent<T>() != nullptr; };

        descriptor.add = [](Scene::SceneObject& object) { object.addComponent<T>(); };

        descriptor.renderInspector = std::move(renderer);
        descriptor.addable = addable;

        registerDescriptor(std::move(descriptor));
    }

    static void registerDescriptor(Descriptor descriptor)
    {
        const auto name = descriptor.name;

        if (const auto it = getIndices().find(name); it != getIndices().end())
        {
            getDescriptors()[it->second] = std::move(descriptor);
            return;
        }

        const auto index = getDescriptors().size();

        getIndices().emplace(name, index);
        getDescriptors().push_back(std::move(descriptor));
    }

    static const std::vector<Descriptor>& getAll() { return getDescriptors(); }

    static const Descriptor* find(std::string_view name)
    {
        const auto it = getIndices().find(std::string(name));

        if (it == getIndices().end())
        {
            return nullptr;
        }

        return &getDescriptors()[it->second];
    }

    static std::unique_ptr<Component> create(std::string_view name)
    {
        const auto* descriptor = find(name);

        if (!descriptor)
        {
            throw std::runtime_error("Unknown component type: " + std::string(name));
        }

        return descriptor->create();
    }

private:
    static std::vector<Descriptor>& getDescriptors()
    {
        static std::vector<Descriptor> descriptors;
        return descriptors;
    }

    static std::unordered_map<std::string, std::size_t>& getIndices()
    {
        static std::unordered_map<std::string, std::size_t> indices;
        return indices;
    }
};
} // namespace Core::Component

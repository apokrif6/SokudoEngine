#pragma once

#include "scene/Scene.h"
#include "uuid.h"

namespace Core
{
template <typename T> class ComponentReference
{
public:
    ComponentReference() = default;

    explicit ComponentReference(const uuids::uuid& uuid) : mUUID(uuid) {}

    explicit ComponentReference(T* component) { set(component); }

    void set(T* component)
    {
        if (component)
        {
            mUUID = component->getUUID();
        }
        else
        {
            mUUID = uuids::uuid{};
        }
    }

    [[nodiscard]]
    T* resolve(Scene::Scene* scene) const
    {
        if (mUUID.is_nil())
        {
            return nullptr;
        }

        auto* component = scene->findComponentByUUID<Component::Component>(mUUID);
        if (!component)
        {
            return nullptr;
        }

        return dynamic_cast<T*>(component);
    }

    [[nodiscard]]
    const uuids::uuid& uuid() const
    {
        return mUUID;
    }

    [[nodiscard]]
    bool isValid() const
    {
        return !mUUID.is_nil();
    }

    explicit operator bool() const { return isValid(); }

private:
    uuids::uuid mUUID{};
};
} // namespace Core
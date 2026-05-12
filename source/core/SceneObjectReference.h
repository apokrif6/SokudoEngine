#pragma once

#include "scene/Scene.h"
#include "uuid.h"

namespace Core
{
template <typename T> class SceneObjectReference
{
public:
    SceneObjectReference() = default;

    explicit SceneObjectReference(const uuids::uuid& uuid) : mUUID(uuid) {}

    explicit SceneObjectReference(T* object) { set(object); }

    void set(T* sceneObject)
    {
        if (sceneObject)
        {
            mUUID = sceneObject->getUUID();
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

        return scene->findSceneObjectByUUID<T>(mUUID);
    }

    [[nodiscard]]
    const uuids::uuid& uuid() const
    {
        return mUUID;
    }

    [[nodiscard]]
    bool isValid() const
    {
        return mUUID != uuids::uuid{};
    }

    explicit operator bool() const { return isValid(); }

private:
    uuids::uuid mUUID{};
};
} // namespace Core

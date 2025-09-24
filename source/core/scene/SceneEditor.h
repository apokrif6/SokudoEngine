#pragma once

#include <memory>
#include "core/scene/objects/SceneObject.h"

namespace Core::Scene
{
struct SceneObjectSelection
{
    std::shared_ptr<Core::Scene::SceneObject> selectedObject;
};
}
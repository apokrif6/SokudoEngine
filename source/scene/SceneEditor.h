#pragma once

#include <memory>
#include "scene/objects/SceneObject.h"

namespace Core::Scene
{
struct SceneObjectSelection
{
    std::weak_ptr<SceneObject> selectedObject;
};
} // namespace Core::Scene
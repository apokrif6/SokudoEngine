#pragma once

#include <memory>
#include <string_view>

namespace Core::Animations
{
class Skeleton;
}

namespace Core::Utils
{
struct MeshNode;
}

namespace Core::Scene
{
class SceneObject;

class SceneImporter
{
public:
    static std::shared_ptr<SceneObject> createObjectFromNode(const Utils::MeshNode& node,
                                                             const Animations::Skeleton& skeleton,
                                                             const std::string_view& filePath);
};
} // namespace Core::Scene
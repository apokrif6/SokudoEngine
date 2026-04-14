#pragma once

#include <memory>
#include <string_view>
#include <vector>
#include <glm/fwd.hpp>

namespace Core::Animations
{
class Skeleton;
}

namespace Core::Utils
{
struct SkeletonData;
struct PrimitiveData;
struct MeshNode;
} // namespace Core::Utils

namespace Core::Scene
{
class SceneObject;

class SceneImporter
{
public:
    static std::shared_ptr<SceneObject> createObjectFromNode(const Utils::MeshNode& node,
                                                             const Utils::SkeletonData& skeletonData,
                                                             const std::string_view& filePath, bool shouldMergeMeshes);
};
} // namespace Core::Scene
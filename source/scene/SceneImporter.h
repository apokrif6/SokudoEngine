#pragma once

#include <memory>
#include <string_view>

namespace Core::Resources
{
struct SkeletonData;
struct MeshNode;
} // namespace Core::Resources
namespace Core::Animations
{
class Skeleton;
}

namespace Core::Scene
{
class SceneObject;

class SceneImporter
{
public:
    static std::shared_ptr<SceneObject> createObjectFromNode(const Resources::MeshNode& node,
                                                             const Resources::SkeletonData& skeletonData,
                                                             const std::string_view& filePath, bool shouldMergeMeshes);
};
} // namespace Core::Scene
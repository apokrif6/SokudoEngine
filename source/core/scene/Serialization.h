#pragma once

#include "Scene.h"
#include "yaml-cpp/yaml.h"

namespace Core::Scene
{
class Serialization
{
public:
    static YAML::Node serialize(const Scene& scene);
    static Scene deserializeScene(const YAML::Node& node);

    static void saveSceneToFile(const Scene& scene, const std::string& filename);
    static Scene loadSceneFromFile(const std::string& filename);
};
} // namespace Core::Scene
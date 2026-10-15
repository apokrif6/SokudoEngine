#pragma once

#include "Scene.h"
#include "yaml-cpp/yaml.h"

namespace Core::Scene
{
class Serialization
{
public:
    static YAML::Node serialize(const Scene& scene);
    static void deserializeScene(Scene& scene, const YAML::Node& node);

    static void saveSceneToFile(const Scene& scene, const std::string& filename);
    static bool loadSceneFromFile(Scene& scene, const std::string& filename);
};
} // namespace Core::Scene
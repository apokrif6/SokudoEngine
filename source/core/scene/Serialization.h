#pragma once

#include "Scene.h"
#include "yaml-cpp/yaml.h"

namespace Core::Scene
{
class Serialization
{
public:
    YAML::Node serialize(const Scene& scene);
    Scene deserializeScene(const YAML::Node& node);

    YAML::Node serialize(const SceneObject& object);
    SceneObject deserializeSceneObject(const YAML::Node& node);

    void saveSceneToFile(const Scene& scene, const std::string& filename);
    Scene loadSceneFromFile(const std::string& filename);
};
}
#include "SceneImporter.h"
#include "components/MeshComponent.h"
#include "components/TransformComponent.h"
#include "engine/Engine.h"
#include "utils/ShapeUtils.h"
#include "objects/SceneObject.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>

std::shared_ptr<Core::Scene::SceneObject>
Core::Scene::SceneImporter::createObjectFromNode(const Utils::MeshNode& node, const Animations::Skeleton& skeleton,
                                                 const std::string_view& filePath)
{
    auto sceneObject = std::make_shared<SceneObject>(node.name);

    auto* transformComponent = sceneObject->addComponent<Component::TransformComponent>();
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;

    glm::decompose(node.localTransform, scale, rotation, translation, skew, perspective);

    transformComponent->setPosition(translation);
    transformComponent->setRotation(rotation);
    transformComponent->setScale(scale);

    if (node.primitives.size() > 1)
    {
        for (uint32_t i = 0; i < node.primitives.size(); ++i)
        {
            std::string partName = node.name + "_index_" + std::to_string(i);
            auto partObject = std::make_shared<SceneObject>(partName);

            partObject->addComponent<Component::TransformComponent>();

            auto* meshComp = partObject->addComponent<Component::MeshComponent>(skeleton);
            meshComp->setSourceMesh(filePath, i);

            const auto& primitive = node.primitives[i];
            meshComp->addPrimitive(primitive.vertices, primitive.indices, primitive.textures,
                                   Engine::getInstance().getRenderData(), primitive.material, primitive.bones,
                                   primitive.materialDescriptorSet);

            sceneObject->addChild(partObject);
        }
    }
    else if (node.primitives.size() == 1)
    {
        auto* meshComp = sceneObject->addComponent<Component::MeshComponent>(skeleton);
        meshComp->setSourceMesh(filePath, 0);
        const auto& prim = node.primitives[0];
        meshComp->addPrimitive(prim.vertices, prim.indices, prim.textures, Engine::getInstance().getRenderData(),
                               prim.material, prim.bones, prim.materialDescriptorSet);
    }

    for (const auto& childNode : node.children)
    {
        sceneObject->addChild(createObjectFromNode(childNode, skeleton, filePath));
    }

    return sceneObject;
}

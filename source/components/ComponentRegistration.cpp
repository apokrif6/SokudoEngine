#include "ComponentRegistry.h"

#include "ui/inspector/TransformInspectorUIWindow.h"
#include "ui/inspector/RotatingComponentInspectorUIWindow.h"
#include "ui/inspector/MeshComponentInspectorUIWindow.h"
#include "ui/inspector/SpriteComponentInspectorUIWindow.h"
#include "ui/inspector/PointLightComponentInspectorUIWindow.h"
#include "ui/inspector/IKTargetComponentInspectorUIWindow.h"

namespace Core::Component
{
void registerBuiltinComponents()
{
    ComponentRegistry::registerComponent<TransformComponent>(
        "TransformComponent", [] { UI::TransformInspectorUIWindow::renderBody(); }, false);

    ComponentRegistry::registerComponent<RotatingComponent>("RotatingComponent", []
                                                            { UI::RotatingComponentInspectorUIWindow::renderBody(); });

    ComponentRegistry::registerComponent<MeshComponent>("MeshComponent",
                                                        [] { UI::MeshComponentInspectorUIWindow::renderBody(); });

    ComponentRegistry::registerComponent<SpriteComponent>("SpriteComponent",
                                                          [] { UI::SpriteComponentInspectorUIWindow::renderBody(); });

    ComponentRegistry::registerComponent<PointLightComponent>(
        "PointLightComponent", [] { UI::PointLightComponentInspectorUIWindow::renderBody(); });

    ComponentRegistry::registerComponent<IKTargetComponent>("IKTargetComponent", []
                                                            { UI::IKTargetComponentInspectorUIWindow::renderBody(); });
}
} // namespace Core::Component

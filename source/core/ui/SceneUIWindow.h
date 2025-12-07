#pragma once

#include <vector>
#include <memory>
#include "UIWindow.h"
#include "string"
#include "imgui.h"
#include "ImGuizmo.h"
#include "core/scene/SceneEditor.h"

namespace Core::UI
{
class SceneUIWindow : public UIWindow<SceneUIWindow>
{
public:
    static bool getBody();

private:
    static inline int selectedSceneObjectIndex = 0;

    static inline bool showSaveDialog = false;
    static inline bool showLoadDialog = false;
    static inline std::string saveFilename;
    static inline std::vector<std::string> sceneFileList;
    static inline std::string selectedSceneFile;

    static void refreshSceneFiles();
    static void drawSceneObjectNode(std::shared_ptr<Core::Scene::SceneObject> object, Core::Scene::SceneObjectSelection& selection);
};
} // namespace Core::UI

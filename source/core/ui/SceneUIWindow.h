#pragma once

#include <vector>
#include <memory>
#include "UIWindow.h"
#include "string"
#include "core/scene/SceneEditor.h"

namespace Core::UI
{
class SceneUIWindow : public UIWindow<SceneUIWindow>
{
    friend class UIWindow;

    static bool getBody();

    static inline int selectedSceneObjectIndex = 0;

    static inline bool showSaveDialog = false;
    static inline bool showLoadDialog = false;
    static inline std::string saveFilename;
    static inline std::vector<std::string> sceneFileList;
    static inline std::string selectedSceneFile;

    static void refreshSceneFiles();
    static void drawSceneObjectNode(std::shared_ptr<Scene::SceneObject> object, Scene::SceneObjectSelection& selection);
};
} // namespace Core::UI

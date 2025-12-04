#pragma once

#include <vector>
#include "UIWindow.h"
#include "string"
#include "imgui.h"
#include "ImGuizmo.h"

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
};
} // namespace Core::UI

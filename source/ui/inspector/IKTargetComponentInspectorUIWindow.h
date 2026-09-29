#pragma once

namespace Core::UI
{
class IKTargetComponentInspectorUIWindow : public UIPermanentWindow<IKTargetComponentInspectorUIWindow>
{
    friend class UIWindow;
    friend class UIPermanentWindow;

    static bool getBody()
    {
        ImGui::Text("IK Target Component Test");

        return true;
    }
};
} // namespace Core::UI

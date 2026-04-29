#pragma once

#include "components/IKTargetComponent.h"
#include "ui/UIWindow.h"
#include "engine/Engine.h"

namespace Core::UI
{
class IKTargetComponentInspectorUIWindow : public UIWindow<IKTargetComponentInspectorUIWindow>
{
    friend class UIWindow;

    static bool getBody() { return true; }
};
} // namespace Core::UI

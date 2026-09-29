#pragma once

#include "UIWindow.h"

namespace Core::UI
{
template <typename Derived> class UIPermanentWindow : public UIWindow<Derived>
{
protected:
    struct ValidateInterface
    {
        ValidateInterface() { (void)&Derived::getBody; }
    };

    inline static const ValidateInterface validator{};
};
} // namespace Core::UI
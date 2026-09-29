#pragma once

#include "UIWindow.h"

namespace Core::UI
{
template <typename Derived> class UIManagedWindow : public UIWindow<Derived>
{
public:
    static void open() { mIsOpen = true; }

    static void close() { mIsOpen = false; }

    static void toggle() { mIsOpen = !mIsOpen; }

    [[nodiscard]] static bool isOpen() { return mIsOpen; }

protected:
    static bool& openState() { return mIsOpen; }

private:
    inline static bool mIsOpen = false;

    struct ValidateInterface
    {
        ValidateInterface() { (void)&Derived::draw; }
    };

    inline static const ValidateInterface validator{};
};
} // namespace Core::UI
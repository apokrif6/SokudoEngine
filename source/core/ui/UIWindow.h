#pragma once

#include <type_traits>

namespace Core::UI
{
template <typename Derived> class UIWindow
{
public:
    template <typename... Args> static bool renderBody(Args&&... args)
    {
        static_assert(std::is_invocable_v<decltype(&Derived::getBody), Args...>,
                      "Derived class must implement 'static bool getBody(...)' with matching arguments!");

        return Derived::getBody(std::forward<Args>(args)...);
    }

protected:
    struct ValidateInterface
    {
        ValidateInterface() { (void)&Derived::getBody; }
    };

    inline static const ValidateInterface validator{};
};
} // namespace Core::UI
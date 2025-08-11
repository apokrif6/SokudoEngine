#pragma once

#include <type_traits>

namespace Core::UI
{
template <typename Derived>
class UIWindow
{
  public:
    static bool renderBody()
    {
        return Derived::getBody();
    }

  protected:
    struct ValidateInterface {
        ValidateInterface() {
            static_assert(std::is_same_v<decltype(&Derived::getBody), bool(*)()>,
                          "Derived class must implement static bool getBody()");
        }
    };

    inline static const ValidateInterface validator{};
};
}
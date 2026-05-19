#pragma once

#include <string>
#include <vector>
#include <functional>

namespace Editor::UI
{
struct PopupRequest
{
    std::string id;
    std::vector<std::string> items;
    std::function<void(int)> onSelect;
};
} // namespace Editor::UI
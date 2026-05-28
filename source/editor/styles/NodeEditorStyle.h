#pragma once

namespace Editor::UI
{
class NodeEditorStyle
{
public:
    static void push();

    static void pop();
};
} // namespace Editor::UI
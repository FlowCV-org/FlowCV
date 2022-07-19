# include "widgets.h"
#include <iostream>

void ax::Widgets::Icon(const ImVec2& size, const ImRect& nodeSize, IconType type, bool filled, const ImVec4& color, const ImVec4& innerColor, bool isInput)
{
    if (ImGui::IsRectVisible(size))
    {
        auto cursorPos = ImGui::GetCursorScreenPos();
        auto drawList  = ImGui::GetWindowDrawList();
        if (!isInput) {
            cursorPos.x = (nodeSize.Max.x - size.x) - 12;
        }
        ax::Drawing::DrawIcon(drawList, cursorPos , cursorPos + size, type, filled, ImColor(color), ImColor(innerColor));
    }

    ImGui::Dummy(size);
}


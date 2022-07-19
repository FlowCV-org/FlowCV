#pragma once
# define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include "drawing.h"

namespace ax {
namespace Widgets {

using Drawing::IconType;

void Icon(const ImVec2& size, const ImRect& nodeSize, IconType type, bool filled, const ImVec4& color = ImVec4(1, 1, 1, 1), const ImVec4& innerColor = ImVec4(0, 0, 0, 0), bool isInput = false);

} // namespace Widgets
} // namespace ax
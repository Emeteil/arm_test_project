#pragma once

#include <imgui.h>

namespace ArmProject::Gui
{
    /// @brief Применяет шокирующую современную тему к ImGui.
    void ApplyTheme();

    /// @brief Рисует анимированный градиентный фон в области viewport.
    void DrawAnimatedBackground(ImDrawList* dl, ImVec2 a, ImVec2 b, float t);

    /// @brief Рисует мягкое свечение под прямоугольником.
    void DrawGlow(ImDrawList* dl, ImVec2 a, ImVec2 b, ImU32 color, float radius);

    /// @brief HSV → RGB в формате ImU32.
    ImU32 HsvColor(float h, float s, float v, float a = 1.0f);
}

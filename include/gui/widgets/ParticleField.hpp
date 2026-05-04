#pragma once

#include <imgui.h>
#include <vector>

namespace ArmProject::Gui
{
    class NodeField
    {
    public:
        NodeField();
        /// @brief Обновляет позиции узлов.
        void Update(float dt, ImVec2 area);
        /// @brief Рисует узлы и линии в указанном draw-list.
        void Draw(ImDrawList* dl, ImVec2 origin, ImVec2 size) const;

    private:
        struct Node
        {
            float x, y;
            float vx, vy;
            float pulse;
        };
        std::vector<Node> _nodes;
        float _time;
    };
}

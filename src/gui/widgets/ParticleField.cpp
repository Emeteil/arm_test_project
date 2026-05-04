#include "gui/widgets/ParticleField.hpp"
#include "core/Config.hpp"

#include <cmath>
#include <random>

namespace ArmProject::Gui
{
    namespace
    {
        std::mt19937& Rng()
        {
            static std::mt19937 r{0xA1B2C3D4u};
            return r;
        }
        float Frand(float a, float b)
        {
            std::uniform_real_distribution<float> d(a, b);
            return d(Rng());
        }
    }

    NodeField::NodeField() : _time(0.0f)
    {
        _nodes.reserve(Config::NODE_COUNT);
        for (int i = 0; i < Config::NODE_COUNT; ++i)
        {
            Node n;
            n.x = Frand(0.0f, 1.0f);
            n.y = Frand(0.0f, 1.0f);
            float ang = Frand(0.0f, 6.28318f);
            float sp = Frand(Config::NODE_MIN_SPEED, Config::NODE_MAX_SPEED);
            n.vx = std::cos(ang) * sp;
            n.vy = std::sin(ang) * sp;
            n.pulse = Frand(0.0f, 6.28318f);
            _nodes.push_back(n);
        }
    }

    void NodeField::Update(float dt, ImVec2 area)
    {
        _time += dt;
        for (auto& n : _nodes)
        {
            n.x += (n.vx / area.x) * dt;
            n.y += (n.vy / area.y) * dt;
            if (n.x < 0.0f) { n.x = 0.0f; n.vx = -n.vx; }
            if (n.x > 1.0f) { n.x = 1.0f; n.vx = -n.vx; }
            if (n.y < 0.0f) { n.y = 0.0f; n.vy = -n.vy; }
            if (n.y > 1.0f) { n.y = 1.0f; n.vy = -n.vy; }
        }
    }

    void NodeField::Draw(ImDrawList* dl, ImVec2 origin, ImVec2 size) const
    {
        const auto& A = Config::ACCENT_PRIMARY;
        ImU32 lineBase = IM_COL32((int)(A.r * 255), (int)(A.g * 255), (int)(A.b * 255), 255);
        ImU32 nodeBase = IM_COL32(200, 210, 240, 255);

        const float linkDist = Config::NODE_LINK_DISTANCE;
        const float linkDist2 = linkDist * linkDist;

        for (std::size_t i = 0; i < _nodes.size(); ++i)
        {
            const auto& a = _nodes[i];
            ImVec2 pa(origin.x + a.x * size.x, origin.y + a.y * size.y);
            for (std::size_t j = i + 1; j < _nodes.size(); ++j)
            {
                const auto& b = _nodes[j];
                ImVec2 pb(origin.x + b.x * size.x, origin.y + b.y * size.y);
                float dx = pa.x - pb.x;
                float dy = pa.y - pb.y;
                float d2 = dx * dx + dy * dy;
                if (d2 > linkDist2) continue;
                float t = 1.0f - std::sqrt(d2) / linkDist;
                int alpha = (int)(t * t * 70.0f);
                ImU32 c = (lineBase & 0x00FFFFFFu) | ((unsigned)alpha << 24);
                dl->AddLine(pa, pb, c, 1.0f);
            }
        }

        for (const auto& n : _nodes)
        {
            ImVec2 p(origin.x + n.x * size.x, origin.y + n.y * size.y);
            float pulse = 0.5f + 0.5f * std::sin(_time * 1.2f + n.pulse);
            int alpha = 90 + (int)(pulse * 80);
            ImU32 c = (nodeBase & 0x00FFFFFFu) | ((unsigned)alpha << 24);
            dl->AddCircleFilled(p, Config::NODE_RADIUS, c, 10);
        }
    }
}

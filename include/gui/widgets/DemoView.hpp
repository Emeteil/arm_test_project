#pragma once

#include <cstdint>
#include <vector>

namespace ArmProject::Gui
{
    class DemoView
    {
    public:
        DemoView();
        ~DemoView();
        void Render(float dt);

    private:
        void Regenerate();
        void RecomputeEnergy();

        std::vector<std::int32_t> _data;
        int _size;
        float _freq;
        float _phase;
        std::int64_t _energyScalar;
        std::int64_t _energyNeon;
        double _msScalar;
        double _msNeon;
    };
}

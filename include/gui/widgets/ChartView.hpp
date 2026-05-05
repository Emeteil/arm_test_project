#pragma once

#include "core/bench/Benchmark.hpp"

#include <atomic>
#include <future>
#include <vector>
#include <cstddef>

namespace ArmProject::Gui
{
    class ChartView
    {
    public:
        ChartView();
        ~ChartView();

        void Render(float dt);

    private:
        void StartBench();
        void DrawChart();

        std::vector<Bench::Suite> _suites;
        std::vector<std::size_t>  _sizes;
        std::future<std::vector<Bench::Suite>> _future;
        std::atomic<float> _progress;
        std::atomic<bool>  _running;
        int _repeats;
        float _animTime;
    };
}

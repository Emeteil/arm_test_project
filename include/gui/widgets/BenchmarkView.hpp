#pragma once

#include "core/bench/Benchmark.hpp"

#include <atomic>
#include <future>
#include <vector>

namespace ArmProject::Gui
{
    class BenchmarkView
    {
    public:
        BenchmarkView();
        ~BenchmarkView();

        void Render(float dt);

    private:
        void StartBench();
        void DrawBars(const Bench::Suite& suite, float dt);

        std::vector<Bench::Suite> _suites;
        std::future<std::vector<Bench::Suite>> _future;
        std::atomic<float> _progress;
        std::atomic<bool> _running;
        int _selectedSize;
        int _repeats;
        float _animTime;
    };
}

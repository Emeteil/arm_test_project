#include "core/bench/Benchmark.hpp"
#include "core/algo/SignalEnergy.hpp"
#include "core/Config.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <new>
#include <random>

namespace ArmProject::Bench
{
    std::int32_t* GenerateAlignedData(std::size_t n, std::uint64_t seed)
    {
        const std::size_t bytes = ((n * sizeof(std::int32_t) + Config::DATA_ALIGNMENT - 1) /
                                   Config::DATA_ALIGNMENT) * Config::DATA_ALIGNMENT;
        void* raw = nullptr;
#if defined(_WIN32)
        raw = _aligned_malloc(bytes, Config::DATA_ALIGNMENT);
#else
        if (posix_memalign(&raw, Config::DATA_ALIGNMENT, bytes) != 0)
            raw = nullptr;
#endif
        if (!raw)
            throw std::bad_alloc();

        auto* p = static_cast<std::int32_t*>(raw);
        std::mt19937_64 rng(seed);
        std::uniform_int_distribution<std::int32_t> dist(-100000, 100000);
        for (std::size_t i = 0; i < n; ++i)
            p[i] = dist(rng);
        return p;
    }

    void FreeAlignedData(std::int32_t* p)
    {
        if (!p) return;
#if defined(_WIN32)
        _aligned_free(p);
#else
        std::free(p);
#endif
    }

    Sample RunAlgorithm(const std::string& name, const Algorithm& algo,
                        const std::int32_t* data, std::size_t n,
                        int repeats, int warmup)
    {
        std::int64_t result = 0;
        for (int i = 0; i < warmup; ++i)
            result = algo(data, n);

        std::vector<double> times;
        times.reserve(static_cast<std::size_t>(repeats));

        for (int i = 0; i < repeats; ++i)
        {
            auto t0 = std::chrono::high_resolution_clock::now();
            result = algo(data, n);
            auto t1 = std::chrono::high_resolution_clock::now();
            double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
            times.push_back(ms);
        }

        std::sort(times.begin(), times.end());
        double sum = 0.0;
        for (double t : times) sum += t;
        double mean = sum / static_cast<double>(times.size());
        double var = 0.0;
        for (double t : times) var += (t - mean) * (t - mean);
        var /= static_cast<double>(times.size());

        return Sample{name, n, mean, times.front(), times.back(), std::sqrt(var), result};
    }

    Suite RunSuite(std::size_t n, int repeats, int warmup, std::uint64_t seed)
    {
        Suite suite;
        suite.n = n;
        std::int32_t* data = GenerateAlignedData(n, seed);

        suite.samples.push_back(RunAlgorithm("Scalar",       Algo::SignalEnergyScalar,       data, n, repeats, warmup));
        suite.samples.push_back(RunAlgorithm("NEON",         Algo::SignalEnergyNeon,         data, n, repeats, warmup));
        suite.samples.push_back(RunAlgorithm("NEON unrolled",Algo::SignalEnergyNeonUnrolled, data, n, repeats, warmup));

        FreeAlignedData(data);
        return suite;
    }
}

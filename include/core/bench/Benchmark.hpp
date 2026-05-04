#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace ArmProject::Bench
{
    using Algorithm = std::function<std::int64_t(const std::int32_t*, std::size_t)>;

    struct Sample
    {
        std::string name;
        std::size_t n;
        double meanMs;
        double minMs;
        double maxMs;
        double stddevMs;
        std::int64_t result;
    };

    struct Suite
    {
        std::size_t n;
        std::vector<Sample> samples;
    };

    /// @brief Генерирует выровненный массив случайных int32 с заданным сидом.
    std::int32_t* GenerateAlignedData(std::size_t n, std::uint64_t seed);

    /// @brief Освобождает память, выделенную GenerateAlignedData.
    void FreeAlignedData(std::int32_t* p);

    /// @brief Запускает один алгоритм и возвращает агрегированный сэмпл.
    Sample RunAlgorithm(const std::string& name, const Algorithm& algo,
                        const std::int32_t* data, std::size_t n,
                        int repeats, int warmup);

    /// @brief Запускает полный набор алгоритмов на массиве размера n.
    Suite RunSuite(std::size_t n, int repeats, int warmup, std::uint64_t seed);
}

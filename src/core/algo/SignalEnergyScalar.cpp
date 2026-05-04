#include "core/algo/SignalEnergy.hpp"

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC optimize("no-tree-vectorize")
#endif

namespace ArmProject::Algo
{
#if defined(__GNUC__) || defined(__clang__)
    __attribute__((optimize("no-tree-vectorize")))
#endif
    std::int64_t SignalEnergyScalar(const std::int32_t* data, std::size_t n) noexcept
    {
        std::int64_t sum = 0;
        for (std::size_t i = 0; i < n; ++i)
        {
            const std::int32_t v = data[i];
            const std::int32_t s = v >> 31;
            sum += static_cast<std::int64_t>((v ^ s) - s);
        }
        return sum;
    }
}

#include "core/algo/SignalEnergy.hpp"
#include "core/Platform.hpp"

#if ARMP_HAS_NEON
    #include <arm_neon.h>
#endif
namespace ArmProject::Algo
{

#if ARMP_HAS_NEON

    std::int64_t SignalEnergyNeon(const std::int32_t* data, std::size_t n) noexcept
    {
        int32x4_t acc = vdupq_n_s32(0);
        std::size_t i = 0;
        const int32x4_t zero = vdupq_n_s32(0);

        for (; i + 4 <= n; i += 4)
        {
            int32x4_t vec = vld1q_s32(data + i);
            uint32x4_t maskPos = vcgtq_s32(vec, zero);
            uint32x4_t maskNeg = vcltq_s32(vec, zero);
            int32x4_t sign = vshrq_n_s32(vec, 31);
            int32x4_t absVal = vsubq_s32(veorq_s32(vec, sign), sign);
            int32x4_t posPart = vandq_s32(vec, vreinterpretq_s32_u32(maskPos));
            int32x4_t negPart = vandq_s32(absVal, vreinterpretq_s32_u32(maskNeg));
            acc = vaddq_s32(acc, vorrq_s32(posPart, negPart));
        }

#if defined(__aarch64__)
        std::int64_t sum = vaddlvq_s32(acc);
#else
        int32x2_t lo = vget_low_s32(acc);
        int32x2_t hi = vget_high_s32(acc);
        int32x2_t pp = vpadd_s32(lo, hi);
        pp = vpadd_s32(pp, pp);
        std::int64_t sum = static_cast<std::int64_t>(vget_lane_s32(pp, 0)) +
                           static_cast<std::int64_t>(vget_lane_s32(pp, 1));
#endif

        for (; i < n; ++i)
        {
            const std::int32_t v = data[i];
            const std::int32_t s = v >> 31;
            sum += static_cast<std::int64_t>((v ^ s) - s);
        }
        return sum;
    }

    std::int64_t SignalEnergyNeonUnrolled(const std::int32_t* data, std::size_t n) noexcept
    {
        int32x4_t acc0 = vdupq_n_s32(0);
        int32x4_t acc1 = vdupq_n_s32(0);
        std::size_t i = 0;

        for (; i + 8 <= n; i += 8)
        {
            __builtin_prefetch(data + i + 64);
            int32x4_t v0 = vld1q_s32(data + i);
            int32x4_t v1 = vld1q_s32(data + i + 4);
            int32x4_t s0 = vshrq_n_s32(v0, 31);
            int32x4_t s1 = vshrq_n_s32(v1, 31);
            int32x4_t a0 = vsubq_s32(veorq_s32(v0, s0), s0);
            int32x4_t a1 = vsubq_s32(veorq_s32(v1, s1), s1);
            acc0 = vaddq_s32(acc0, a0);
            acc1 = vaddq_s32(acc1, a1);
        }

        int32x4_t acc = vaddq_s32(acc0, acc1);
#if defined(__aarch64__)
        std::int64_t sum = vaddlvq_s32(acc);
#else
        int32x2_t lo = vget_low_s32(acc);
        int32x2_t hi = vget_high_s32(acc);
        int32x2_t pp = vpadd_s32(lo, hi);
        pp = vpadd_s32(pp, pp);
        std::int64_t sum = static_cast<std::int64_t>(vget_lane_s32(pp, 0)) +
                           static_cast<std::int64_t>(vget_lane_s32(pp, 1));
#endif

        for (; i < n; ++i)
        {
            const std::int32_t v = data[i];
            const std::int32_t s = v >> 31;
            sum += static_cast<std::int64_t>((v ^ s) - s);
        }
        return sum;
    }

#else

    std::int64_t SignalEnergyNeon(const std::int32_t* data, std::size_t n) noexcept
    {
        return SignalEnergyScalar(data, n);
    }

    std::int64_t SignalEnergyNeonUnrolled(const std::int32_t* data, std::size_t n) noexcept
    {
        return SignalEnergyScalar(data, n);
    }

#endif
}

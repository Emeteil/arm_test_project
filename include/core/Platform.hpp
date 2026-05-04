#pragma once

#include <string>

#if defined(__aarch64__) || defined(__arm__) || defined(_M_ARM) || defined(_M_ARM64)
    #define ARMP_IS_ARM 1
#else
    #define ARMP_IS_ARM 0
#endif

#if defined(__ARM_NEON) || defined(__ARM_NEON__) || (defined(_M_ARM) && _M_ARM >= 7) || defined(_M_ARM64)
    #define ARMP_HAS_NEON 1
#else
    #define ARMP_HAS_NEON 0
#endif

#if ARMP_HAS_NEON
    #define ARMP_NEON_ONLY(x) x
#else
    #define ARMP_NEON_ONLY(x)
#endif

namespace ArmProject
{
    struct PlatformInfo
    {
        bool isArm;
        bool hasNeon;
        std::string archName;
        std::string compilerName;
        std::string osName;
        std::string note;
    };

    /// @brief Возвращает информацию о платформе, на которой запущена программа.
    PlatformInfo DetectPlatform();
}

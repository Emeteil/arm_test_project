#include "core/Platform.hpp"

namespace ArmProject
{
    PlatformInfo DetectPlatform()
    {
        PlatformInfo info;
        info.isArm = ARMP_IS_ARM != 0;
        info.hasNeon = ARMP_HAS_NEON != 0;

#if defined(__aarch64__) || defined(_M_ARM64)
        info.archName = "ARM64 (AArch64)";
#elif defined(__arm__) || defined(_M_ARM)
        info.archName = "ARM32";
#elif defined(__x86_64__) || defined(_M_X64)
        info.archName = "x86_64";
#elif defined(__i386__) || defined(_M_IX86)
        info.archName = "x86";
#else
        info.archName = "Unknown";
#endif

#if defined(__clang__)
        info.compilerName = "Clang";
#elif defined(__GNUC__)
        info.compilerName = "GCC";
#elif defined(_MSC_VER)
        info.compilerName = "MSVC";
#else
        info.compilerName = "Unknown";
#endif

#if defined(__linux__)
        info.osName = "Linux";
#elif defined(_WIN32)
        info.osName = "Windows";
#elif defined(__APPLE__)
        info.osName = "macOS";
#else
        info.osName = "Unknown";
#endif

        if (!info.hasNeon)
        {
            info.note = "Платформа не ARM/NEON: NEON-функции заменены на скалярные. "
                        "Результаты бенчмарка будут идентичны скалярной версии.";
        }
        else
        {
            info.note = "Платформа поддерживает ARM NEON: используются векторные интринсики.";
        }
        return info;
    }
}

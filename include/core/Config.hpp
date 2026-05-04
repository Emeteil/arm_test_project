#pragma once

#include <cstddef>
#include <cstdint>
#include <array>
#include <string_view>

namespace ArmProject::Config
{
    inline constexpr const char* WINDOW_TITLE = "ARM NEON | Signal Energy Analyzer";

    inline constexpr int WINDOW_DEFAULT_WIDTH = 1280;
    inline constexpr int WINDOW_DEFAULT_HEIGHT = 900;

    inline constexpr int WINDOW_MIN_WIDTH = 800;
    inline constexpr int WINDOW_MIN_HEIGHT = 600;

    inline constexpr int VSYNC_INTERVAL = 1;
    inline constexpr int SOFTWARE_TARGET_FPS = 60;

    inline constexpr std::array<std::size_t, 6> BENCH_DEFAULT_SIZES = {
        1024, 8192, 65536, 262144, 1048576, 4194304
    };

    inline constexpr int BENCH_REPEATS = 7;
    inline constexpr std::uint64_t DEFAULT_SEED = 0xC0FFEEULL;

    inline constexpr float DEMO_NOISE_LEVEL = 0.15f;
    inline constexpr std::uint64_t DEMO_NOISE_SEED = 0xD15EA5EULL;

    struct Color { float r, g, b, a; };
    inline constexpr Color ACCENT_PRIMARY   = {0.42f, 0.62f, 1.00f, 1.0f};
    inline constexpr Color ACCENT_SECONDARY = {0.56f, 0.45f, 0.92f, 1.0f};
    inline constexpr Color ACCENT_TERTIARY  = {0.32f, 0.78f, 0.78f, 1.0f};
    inline constexpr Color BG_DEEP          = {0.035f, 0.04f, 0.055f, 1.0f};
    inline constexpr Color BG_MID           = {0.065f, 0.075f, 0.10f, 1.0f};
    inline constexpr Color BG_SURFACE       = {0.10f, 0.115f, 0.15f, 1.0f};
    inline constexpr Color TEXT_PRIMARY     = {0.92f, 0.93f, 0.96f, 1.0f};
    inline constexpr Color TEXT_MUTED       = {0.55f, 0.58f, 0.66f, 1.0f};

    inline constexpr float ANIM_SPEED = 0.6f;
    inline constexpr int NODE_COUNT = 60;
    inline constexpr float NODE_MIN_SPEED = 4.0f;
    inline constexpr float NODE_MAX_SPEED = 14.0f;
    inline constexpr float NODE_LINK_DISTANCE = 180.0f;
    inline constexpr float NODE_RADIUS = 1.8f;

    inline constexpr bool FOG_ANIMATION = false;

    inline constexpr const char* FONT_PRIMARY = "assets/fonts/NotoSans-Bold.ttf";
    inline constexpr const char* FONT_EMOJI   = "assets/fonts/NotoEmoji.ttf";
    inline constexpr float FONT_SIZE = 18.0f;

    inline constexpr std::string_view MODE_GL = "OpenGL";
    inline constexpr std::string_view MODE_SOFT = "Software";
    inline constexpr std::string_view MODE_CLI = "CLI";

    /// Количество прогревочных запусков (warm-up).
    inline constexpr int BENCH_WARMUP = 2;

    /// Выравнивание массивов (для NEON-выровненных загрузок).
    inline constexpr std::size_t DATA_ALIGNMENT = 64;
}

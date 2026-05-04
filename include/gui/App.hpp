#pragma once

namespace ArmProject::Gui
{
    struct AppOptions
    {
        bool forceSoftware = false;
    };

    /// @brief Пытается запустить GUI: сначала OpenGL/GLES, затем Software.
    /// @return 0 при успехе, ненулевое значение, если ни один режим не сработал.
    int RunApp(const AppOptions& options);
}

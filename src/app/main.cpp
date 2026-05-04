#include "core/Platform.hpp"
#include "cli/CliApp.hpp"

#ifdef ARMP_HAVE_GUI
    #include "gui/App.hpp"
#endif

#include <cstring>
#include <iostream>

int main(int argc, char** argv)
{
    bool forceCli = false;
    bool forceSoft = false;
    for (int i = 1; i < argc; ++i)
    {
        if (std::strcmp(argv[i], "--cli") == 0) forceCli = true;
        else if (std::strcmp(argv[i], "--software") == 0) forceSoft = true;
        else if (std::strcmp(argv[i], "--help") == 0)
        {
            std::cout << "Usage: " << argv[0] << " [--cli] [--software]\n";
            return 0;
        }
    }

#ifdef ARMP_HAVE_GUI
    if (!forceCli)
    {
        ArmProject::Gui::AppOptions opts;
        opts.forceSoftware = forceSoft;
        int rc = ArmProject::Gui::RunApp(opts);
        if (rc == 0) return 0;
        std::cerr << "[GUI] Не удалось инициализировать графический режим, "
                     "переключаюсь на CLI...\n";
    }
#else
    (void)forceSoft;
#endif

    return ArmProject::Cli::Run();
}

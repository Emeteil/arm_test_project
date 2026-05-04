#include "cli/CliApp.hpp"
#include "core/Config.hpp"
#include "core/Platform.hpp"
#include "core/bench/Benchmark.hpp"
#include "core/algo/SignalEnergy.hpp"

#include <chrono>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>
#include <cmath>
#include <random>
#include <future>

#if defined(ARMP_HAVE_TUI)
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/canvas.hpp>
#include <ftxui/dom/table.hpp>
#endif

namespace ArmProject::Cli
{
#if !defined(ARMP_HAVE_TUI)

    namespace
    {
        constexpr const char* C_RESET   = "\x1b[0m";
        constexpr const char* C_BOLD    = "\x1b[1m";
        constexpr const char* C_DIM     = "\x1b[2m";
        constexpr const char* C_PINK    = "\x1b[38;5;205m";
        constexpr const char* C_CYAN    = "\x1b[38;5;87m";
        constexpr const char* C_YELLOW  = "\x1b[38;5;221m";
        constexpr const char* C_GREEN   = "\x1b[38;5;120m";
        constexpr const char* C_GREY    = "\x1b[38;5;245m";

        void PrintBanner()
        {
            std::cout << C_PINK << C_BOLD;
            std::cout << "  ╔══════════════════════════════════════════════════════════════════╗\n";
            std::cout << "  ║      " << C_CYAN << "ARM NEON SHOWCASE" << C_PINK
                      << "  ::  " << C_YELLOW << "Signal Energy Analyzer" << C_PINK << "      ║\n";
            std::cout << "  ╚══════════════════════════════════════════════════════════════════╝\n";
            std::cout << C_RESET;
        }

        void PrintPlatform(const PlatformInfo& info)
        {
            std::cout << C_BOLD << "Платформа:\n" << C_RESET;
            std::cout << "  " << C_GREY << "Архитектура: " << C_RESET << info.archName << "\n";
            std::cout << "  " << C_GREY << "ОС:          " << C_RESET << info.osName << "\n";
            std::cout << "  " << C_GREY << "Компилятор:  " << C_RESET << info.compilerName << "\n";
            std::cout << "  " << C_GREY << "ARM:         " << C_RESET
                      << (info.isArm ? "да" : "нет") << "\n";
            std::cout << "  " << C_GREY << "NEON:        " << C_RESET
                      << (info.hasNeon ? "доступен" : "ОТСУТСТВУЕТ - фолбэк на скалярную реализацию") << "\n";
            std::cout << C_DIM << "  " << info.note << C_RESET << "\n\n";
        }

        std::string Bar(double frac, int width)
        {
            if (frac < 0) frac = 0;
            if (frac > 1) frac = 1;
            int filled = static_cast<int>(frac * width + 0.5);
            std::string s;
            for (int i = 0; i < width; ++i)
                s += (i < filled) ? "█" : "░";
            return s;
        }

        void Spinner(const std::string& text, int totalMs)
        {
            const char* frames[] = {"⠋","⠙","⠹","⠸","⠼","⠴","⠦","⠧","⠇","⠏"};
            int n = sizeof(frames)/sizeof(frames[0]);
            auto t0 = std::chrono::steady_clock::now();
            int i = 0;
            while (true)
            {
                auto el = std::chrono::duration_cast<std::chrono::milliseconds>(
                              std::chrono::steady_clock::now() - t0).count();
                if (el >= totalMs) break;
                std::cout << "\r  " << C_CYAN << frames[i % n] << C_RESET << "  " << text << std::flush;
                std::this_thread::sleep_for(std::chrono::milliseconds(80));
                ++i;
            }
            std::cout << "\r  " << C_GREEN << "✔" << C_RESET << "  " << text << "          \n";
        }

        void PrintSuite(const Bench::Suite& s, double maxMean)
        {
            std::cout << C_BOLD << "  N = " << s.n << C_RESET << "\n";
            for (const auto& sm : s.samples)
            {
                double frac = (maxMean > 0.0) ? sm.meanMs / maxMean : 0.0;
                const char* col = (sm.name == "Scalar") ? C_YELLOW : C_PINK;
                std::cout << "    " << col << std::left << std::setw(15) << sm.name << C_RESET
                          << " " << Bar(frac, 28)
                          << "  " << std::right << std::setw(9) << std::fixed << std::setprecision(3)
                          << sm.meanMs << " ms"
                          << C_DIM << "  ±" << std::setw(6) << sm.stddevMs << C_RESET
                          << "\n";
            }
            std::cout << "\n";
        }
    }

    int Run()
    {
        PrintBanner();
        std::cout << "\n";
        PrintPlatform(DetectPlatform());

        std::cout << C_BOLD << "Запуск бенчмарка...\n\n" << C_RESET;

        for (auto n : Config::BENCH_DEFAULT_SIZES)
        {
            std::ostringstream label;
            label << "обработка массива N = " << n;
            Spinner(label.str(), 250);

            auto suite = Bench::RunSuite(n, Config::BENCH_REPEATS, Config::BENCH_WARMUP,
                                         Config::DEFAULT_SEED);

            double maxMean = 0.0;
            for (const auto& s : suite.samples) if (s.meanMs > maxMean) maxMean = s.meanMs;
            PrintSuite(suite, maxMean);

            double scalar = suite.samples.front().meanMs;
            std::cout << C_GREEN << "    Ускорение vs Scalar:";
            for (std::size_t i = 1; i < suite.samples.size(); ++i)
            {
                double sp = scalar / suite.samples[i].meanMs;
                std::cout << "  " << suite.samples[i].name << " ×" << std::fixed
                          << std::setprecision(2) << sp;
            }
            std::cout << C_RESET << "\n\n";
        }

        std::cout << C_BOLD << C_GREEN << "Готово.\n" << C_RESET;
        return 0;
    }

#else

    using namespace ftxui;

    int Run()
    {
        auto screen = ScreenInteractive::Fullscreen();
        auto info = DetectPlatform();

        auto fmt = [](double v, int precision=3) {
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(precision) << v;
            return ss.str();
        };

        int demo_size = 2048;
        int demo_freq_int = 40;
        std::vector<int32_t> demo_data;
        int64_t energy_scalar = 0;
        int64_t energy_neon = 0;
        double ms_scalar = 0;
        double ms_neon = 0;

        auto RegenerateDemo = [&]() {
            float freq = demo_freq_int / 10.0f;
            demo_data.assign(demo_size, 0);
            std::mt19937_64 rng(Config::DEMO_NOISE_SEED);
            std::uniform_real_distribution<float> n(-1.0f, 1.0f);
            for (int i = 0; i < demo_size; ++i) {
                float t = static_cast<float>(i) / demo_size;
                float s = std::sin(t * 6.28318f * freq) +
                          0.4f * std::sin(t * 6.28318f * freq * 2.7f);
                float v = s + Config::DEMO_NOISE_LEVEL * n(rng);
                demo_data[i] = static_cast<int32_t>(v * 1.0e6f);
            }
            
            auto run = [&](auto fn) {
                auto t0 = std::chrono::high_resolution_clock::now();
                int64_t v = 0;
                for (int k = 0; k < 64; ++k) v = fn(demo_data.data(), demo_data.size());
                auto t1 = std::chrono::high_resolution_clock::now();
                return std::pair{v, std::chrono::duration<double, std::milli>(t1 - t0).count() / 64.0};
            };
            
            auto [s1, t1] = run(Algo::SignalEnergyScalar);
            auto [s2, t2] = run(Algo::SignalEnergyNeon);
            energy_scalar = s1; ms_scalar = t1;
            energy_neon = s2; ms_neon = t2;
        };

        RegenerateDemo();

        Component demo_size_slider = Slider("Размер N:", &demo_size, 256, 16384, 256);
        Component demo_freq_slider = Slider("Частота x10:", &demo_freq_int, 5, 200, 5);
        Component demo_regen_btn = Button("Перегенерировать и замерить", RegenerateDemo);

        Component demo_controls = Container::Vertical({
            demo_size_slider,
            demo_freq_slider,
            demo_regen_btn
        });

        auto demo_renderer = Renderer(demo_controls, [&] {
            auto c_renderer = canvas([&](Canvas& c) {
                int w = c.width();
                int h = c.height();
                if (w <= 0 || h <= 0 || demo_data.empty()) return;

                int step = std::max(1, demo_size / w);
                for(int i = 0; i < demo_size - step; i += step) {
                    float y1 = demo_data[i] / 2.0e6f;
                    float y2 = demo_data[i+step] / 2.0e6f;
                    
                    int x1 = (i * w) / demo_size;
                    int x2 = ((i+step) * w) / demo_size;
                    
                    int cy1 = h / 2 - static_cast<int>(y1 * h / 2.0f);
                    int cy2 = h / 2 - static_cast<int>(y2 * h / 2.0f);
                    
                    c.DrawPointLine(x1, cy1, x2, cy2, Color::Cyan);
                }
            }) | flex;
            
            std::string speedup = ms_neon > 0 ? "x" + fmt(ms_scalar / ms_neon, 2) : "N/A";
            
            return vbox({
                text("ДЕМОНСТРАЦИЯ") | bold | color(Color::Cyan),
                text("Аудио-PCM сигнал -> энергия Σ|x|") | dim,
                separator(),
                demo_controls->Render(),
                separator(),
                c_renderer | border,
                separator(),
                hbox({
                    vbox({
                        text("Скалярная версия") | bold,
                        text("Энергия: " + std::to_string(energy_scalar)),
                        text("Время:   " + fmt(ms_scalar) + " ms")
                    }) | flex,
                    vbox({
                        text("NEON-версия") | bold | color(Color::Green),
                        text("Энергия: " + std::to_string(energy_neon)),
                        text("Время:   " + fmt(ms_neon) + " ms"),
                        text("Ускорение: " + speedup)
                    }) | flex
                })
            });
        });

        int bench_repeats = Config::BENCH_REPEATS;
        float bench_progress = 0.0f;
        std::atomic<bool> bench_running{false};
        std::vector<Bench::Suite> bench_suites;
        std::future<std::vector<Bench::Suite>> bench_future;

        auto StartBenchmark = [&]() {
            if (bench_running) return;
            bench_running = true;
            bench_progress = 0.0f;
            int repeats = bench_repeats;
            
            bench_future = std::async(std::launch::async, [repeats, &screen, &bench_progress, &bench_running]() {
                std::vector<Bench::Suite> out;
                const auto& sizes = Config::BENCH_DEFAULT_SIZES;
                int total = static_cast<int>(sizes.size());
                for (int i = 0; i < total; ++i) {
                    out.push_back(Bench::RunSuite(sizes[i], repeats, Config::BENCH_WARMUP, Config::DEFAULT_SEED));
                    bench_progress = static_cast<float>(i + 1) / static_cast<float>(total);
                    screen.PostEvent(Event::Custom);
                }
                bench_running = false;
                screen.PostEvent(Event::Custom);
                return out;
            });
        };

        Component bench_repeats_slider = Slider("Повторов:", &bench_repeats, 1, 30, 1);
        Component bench_btn = Button("▶ Запустить полный бенчмарк", StartBenchmark);

        Component bench_controls = Container::Vertical({
            bench_repeats_slider,
            bench_btn
        });

        int bench_tab_selected = 0;
        std::vector<std::string> bench_tab_entries;

        Component bench_size_toggle = Toggle(&bench_tab_entries, &bench_tab_selected);

        Component bench_container = Container::Vertical({
            bench_controls,
            bench_size_toggle
        });

        auto bench_renderer = Renderer(bench_container, [&] {
            if (bench_future.valid() && !bench_running.load() && 
                bench_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                bench_suites = bench_future.get();
                bench_tab_entries.clear();
                for (auto& s : bench_suites) bench_tab_entries.push_back("N=" + std::to_string(s.n));
            }

            auto title = vbox({
                text("БЕНЧМАРК АЛГОРИТМОВ") | bold | color(Color::Cyan),
                text("Signal Energy = Σ |x_i|") | dim,
                separator(),
            });
            
            auto controls = vbox({
                bench_controls->Render(),
                bench_running.load() ? gauge(bench_progress) : emptyElement()
            });
            
            Element content = text("Нажмите кнопку, чтобы прогнать алгоритмы...");
            if (!bench_suites.empty() && static_cast<std::size_t>(bench_tab_selected) < bench_suites.size()) {
                const auto& suite = bench_suites[bench_tab_selected];
                double maxMean = 0;
                double scalarMean = suite.samples.front().meanMs;
                for (const auto& s : suite.samples) if (s.meanMs > maxMean) maxMean = s.meanMs;
                if (maxMean <= 0) maxMean = 1.0;
                
                Elements bars;
                for (const auto& s : suite.samples) {
                    bars.push_back(hbox({
                        text(s.name) | size(WIDTH, EQUAL, 15),
                        gauge(static_cast<float>(s.meanMs / maxMean)) | color(s.name == "Scalar" ? Color::Yellow : Color::Green) | flex,
                        text(" " + fmt(s.meanMs) + " ms (x" + fmt(scalarMean / s.meanMs, 2) + ")") | size(WIDTH, EQUAL, 25)
                    }));
                }
                
                content = vbox({
                    bench_size_toggle->Render(),
                    separator(),
                    text("Результаты для N=" + std::to_string(suite.n)) | bold,
                    separator(),
                    vbox(std::move(bars))
                });
            }

            return vbox({
                title,
                controls,
                separator(),
                content
            });
        });

        int main_tab = 0;
        std::vector<std::string> main_tab_entries = {"Демо", "Бенчмарк"};
        Component main_toggle = Toggle(&main_tab_entries, &main_tab);

        Component main_container = Container::Tab({
            demo_renderer,
            bench_renderer
        }, &main_tab);

        auto exit_btn = Button("Выход", [&]{ screen.Exit(); });
        Component header_with_exit = Container::Horizontal({
            main_toggle,
            exit_btn
        });

        Component main_layout = Container::Vertical({
            header_with_exit,
            main_container
        });

        auto main_view = Renderer(main_layout, [&] {
            return vbox({
                vbox({
                    text("ARM NEON SHOWCASE") | bold | color(Color::Cyan),
                    text("Signal Energy Analyzer") | color(Color::Pink1),
                    text("Платформа: " + info.archName + " | " + info.osName + " | " + info.compilerName) | dim,
                    text(info.hasNeon ? "NEON ACTIVE" : "NEON OFF - fallback to scalar") | color(info.hasNeon ? Color::Green : Color::Red),
                    separator()
                }),
                hbox({
                    main_toggle->Render(),
                    filler(),
                    exit_btn->Render()
                }),
                separator(),
                main_container->Render() | flex
            }) | border;
        });

        screen.Loop(main_view);

        return 0;
    }

#endif
}

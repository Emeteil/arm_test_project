#include "gui/App.hpp"
#include "gui/Theme.hpp"
#include "gui/widgets/BenchmarkView.hpp"
#include "gui/widgets/ChartView.hpp"
#include "gui/widgets/DemoView.hpp"
#include "gui/widgets/ParticleField.hpp"
#include "core/Config.hpp"
#include "core/Platform.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

namespace ArmProject::Gui
{
    namespace
    {
        bool g_glLoaded = false;
        std::string g_renderMode = "OpenGL";

        void GlfwErrorCallback(int err, const char* msg)
        {
            std::fprintf(stderr, "[GLFW] %d: %s\n", err, msg);
        }

        GLFWwindow* TryCreateWindow(bool useGLES)
        {
            glfwDefaultWindowHints();
            if (useGLES)
            {
                glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
                glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
            }
            else
            {
                glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
                glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if defined(__APPLE__)
                glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
            }
            glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
            return glfwCreateWindow(Config::WINDOW_DEFAULT_WIDTH,
                                    Config::WINDOW_DEFAULT_HEIGHT,
                                    Config::WINDOW_TITLE, nullptr, nullptr);
        }

        const char* GlslFor(bool useGLES)
        {
            return useGLES ? "#version 300 es" : "#version 330";
        }

        void Header(const PlatformInfo& info)
        {
            ImDrawList* dl = ImGui::GetWindowDrawList();
            ImVec2 a = ImGui::GetCursorScreenPos();
            float w = ImGui::GetContentRegionAvail().x;
            ImVec2 b(a.x + w, a.y + 78);

            const auto& MID = Config::BG_MID;
            const auto& BG = Config::BG_DEEP;
            ImU32 cTop = IM_COL32((int)(MID.r * 255), (int)(MID.g * 255), (int)(MID.b * 255), 200);
            ImU32 cBot = IM_COL32((int)(BG.r * 255), (int)(BG.g * 255), (int)(BG.b * 255), 200);
            dl->AddRectFilledMultiColor(a, b, cTop, cTop, cBot, cBot);

            const auto& AC = Config::ACCENT_PRIMARY;
            ImU32 acc = IM_COL32((int)(AC.r * 255), (int)(AC.g * 255), (int)(AC.b * 255), 220);
            dl->AddLine(ImVec2(a.x, b.y - 1), ImVec2(b.x, b.y - 1), acc, 1.5f);

            ImGui::Dummy(ImVec2(0, 3));
            ImGui::Indent(8);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.94f, 0.95f, 0.97f, 1.0f));
            ImGui::TextUnformatted("ARM NEON | Signal Energy Analyzer");
            ImGui::PopStyleColor();
            ImGui::TextDisabled("Демонстрация SIMD, баррельного шифтера и безветвевых вычислений");
            ImGui::Unindent(8);
            ImGui::Dummy(ImVec2(0, 14));

            const char* badge = info.hasNeon ? "  NEON ACTIVE  "
                                             : "  NEON OFF - fallback to scalar  ";
            ImVec4 badgeBg = info.hasNeon ? ImVec4(AC.r * 0.22f, AC.g * 0.22f, AC.b * 0.30f, 1.0f)
                                          : ImVec4(0.30f, 0.20f, 0.15f, 1.0f);
            ImVec4 badgeText = info.hasNeon ? ImVec4(AC.r, AC.g, AC.b, 1.0f)
                                            : ImVec4(0.96f, 0.70f, 0.45f, 1.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, badgeBg);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, badgeBg);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, badgeBg);
            ImGui::PushStyleColor(ImGuiCol_Text, badgeText);
            ImGui::SmallButton(badge);
            ImGui::PopStyleColor(4);
            ImGui::SameLine();
            ImGui::TextDisabled("%s · %s · %s · render: %s",
                                info.archName.c_str(), info.osName.c_str(),
                                info.compilerName.c_str(), g_renderMode.c_str());

            if (!info.hasNeon)
            {
                ImGui::Spacing();
                ImGui::TextDisabled("Платформа без NEON: SIMD-функции заменены на скалярные. "
                                    "Времена обеих версий будут идентичны.");
            }
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
        }

        int RunMainLoop(GLFWwindow* window, bool useGLES)
        {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

            bool fontLoaded = false;
            if (FILE* f = std::fopen(Config::FONT_PRIMARY, "rb"))
            {
                std::fclose(f);
                ImFontConfig fc;
                fc.OversampleH = 3;
                fc.OversampleV = 3;
                io.Fonts->AddFontFromFileTTF(Config::FONT_PRIMARY, Config::FONT_SIZE, &fc,
                                             io.Fonts->GetGlyphRangesCyrillic());
                fontLoaded = true;
            }
            if (!fontLoaded)
                io.Fonts->AddFontDefault();

            if (FILE* fe = std::fopen(Config::FONT_EMOJI, "rb"))
            {
                std::fclose(fe);
                static const ImWchar emojiRanges[] = {
                    0x2000, 0x2BFF, 0x2600, 0x27BF, 0
                };
                ImFontConfig ec;
                ec.MergeMode = true;
                ec.OversampleH = 1;
                ec.OversampleV = 1;
                io.Fonts->AddFontFromFileTTF(Config::FONT_EMOJI, Config::FONT_SIZE, &ec,
                                             emojiRanges);
            }

            ApplyTheme();

            ImGui_ImplGlfw_InitForOpenGL(window, true);
            ImGui_ImplOpenGL3_Init(GlslFor(useGLES));

            BenchmarkView bench;
            ChartView chart;
            DemoView demo;
            NodeField nodes;
            float lastTime = static_cast<float>(glfwGetTime());

            auto info = DetectPlatform();

            while (!glfwWindowShouldClose(window))
            {
                glfwPollEvents();
                float now = static_cast<float>(glfwGetTime());
                float dt = now - lastTime;
                lastTime = now;

                ImGui_ImplOpenGL3_NewFrame();
                ImGui_ImplGlfw_NewFrame();
                ImGui::NewFrame();

                ImGuiViewport* vp = ImGui::GetMainViewport();
                ImDrawList* bg = ImGui::GetBackgroundDrawList();

                if (Config::FOG_ANIMATION)
                {
                    DrawAnimatedBackground(bg, vp->Pos, ImVec2(vp->Pos.x + vp->Size.x, vp->Pos.y + vp->Size.y), now);
                }

                nodes.Update(dt, vp->Size);
                nodes.Draw(bg, vp->Pos, vp->Size);

                ImGui::SetNextWindowPos(vp->Pos);
                ImGui::SetNextWindowSize(vp->Size);
                ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
                ImGui::Begin("##root", nullptr,
                             ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                             ImGuiWindowFlags_NoBringToFrontOnFocus);

                Header(info);

                if (ImGui::BeginTabBar("##main"))
                {
                    if (ImGui::BeginTabItem("Бенчмарк"))
                    {
                        ImGui::BeginChild("bench_pane", ImVec2(0, 0), true);
                        bench.Render(dt);
                        ImGui::EndChild();
                        ImGui::EndTabItem();
                    }
                    if (ImGui::BeginTabItem("График"))
                    {
                        ImGui::BeginChild("chart_pane", ImVec2(0, 0), true);
                        chart.Render(dt);
                        ImGui::EndChild();
                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }

                ImGui::End();
                ImGui::PopStyleColor();

                ImGui::Render();
                int dw, dh;
                glfwGetFramebufferSize(window, &dw, &dh);
                static auto glViewportPtr =
                    reinterpret_cast<void(*)(int,int,int,int)>(glfwGetProcAddress("glViewport"));
                static auto glClearColorPtr =
                    reinterpret_cast<void(*)(float,float,float,float)>(glfwGetProcAddress("glClearColor"));
                static auto glClearPtr =
                    reinterpret_cast<void(*)(unsigned)>(glfwGetProcAddress("glClear"));
                if (glViewportPtr && glClearColorPtr && glClearPtr)
                {
                    glViewportPtr(0, 0, dw, dh);
                    glClearColorPtr(Config::BG_DEEP.r, Config::BG_DEEP.g,
                                    Config::BG_DEEP.b, 1.0f);
                    glClearPtr(0x00004000u);
                }
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
                glfwSwapBuffers(window);
            }

            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
            return 0;
        }
    }

    int RunApp(const AppOptions& options)
    {
        glfwSetErrorCallback(GlfwErrorCallback);

        if (options.forceSoftware)
        {
#if defined(__linux__)
            setenv("LIBGL_ALWAYS_SOFTWARE", "1", 1);
            setenv("GALLIUM_DRIVER", "llvmpipe", 0);
#endif
            g_renderMode = "Software";
        }

        if (!glfwInit())
        {
            std::cerr << "[GUI] glfwInit failed.\n";
            return 1;
        }

#if defined(__arm__) || defined(__aarch64__)
        bool tryGLES = true;
#else
        bool tryGLES = false;
#endif

        GLFWwindow* window = TryCreateWindow(tryGLES);
        if (!window && !tryGLES)
            window = TryCreateWindow(true);

        if (!window)
        {
            std::cerr << "[GUI] OpenGL не доступен - пробую software-режим (llvmpipe)...\n";
#if defined(__linux__)
            setenv("LIBGL_ALWAYS_SOFTWARE", "1", 1);
            setenv("GALLIUM_DRIVER", "llvmpipe", 0);
#endif
            g_renderMode = "Software";
            window = TryCreateWindow(false);
            if (!window) window = TryCreateWindow(true);
        }

        if (!window)
        {
            std::cerr << "[GUI] Не удалось создать окно ни в OpenGL, ни в software-режиме.\n";
            glfwTerminate();
            return 2;
        }

        glfwMakeContextCurrent(window);
        glfwSwapInterval(Config::VSYNC_INTERVAL);
        g_glLoaded = true;

        int rc = RunMainLoop(window, tryGLES);

        glfwDestroyWindow(window);
        glfwTerminate();
        return rc;
    }
}

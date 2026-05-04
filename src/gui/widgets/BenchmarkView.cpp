#include "gui/widgets/BenchmarkView.hpp"
#include "gui/Theme.hpp"
#include "core/Config.hpp"
#include "core/Platform.hpp"

#include <algorithm>
#include <cstdio>
#include <imgui.h>
#include <thread>

namespace ArmProject::Gui
{
    BenchmarkView::BenchmarkView()
        : _progress(0.0f), _running(false), _selectedSize(3),
          _repeats(Config::BENCH_REPEATS), _animTime(0.0f)
    {
    }

    BenchmarkView::~BenchmarkView()
    {
        if (_future.valid()) _future.wait();
    }

    void BenchmarkView::StartBench()
    {
        if (_running.load()) return;
        _running.store(true);
        _progress.store(0.0f);
        int repeats = _repeats;
        _future = std::async(std::launch::async, [this, repeats]() {
            std::vector<Bench::Suite> out;
            const auto& sizes = Config::BENCH_DEFAULT_SIZES;
            int total = static_cast<int>(sizes.size());
            int idx = 0;
            for (auto n : sizes)
            {
                out.push_back(Bench::RunSuite(n, repeats, Config::BENCH_WARMUP,
                                              Config::DEFAULT_SEED));
                ++idx;
                _progress.store(static_cast<float>(idx) / static_cast<float>(total));
            }
            _running.store(false);
            return out;
        });
    }

    void BenchmarkView::DrawBars(const Bench::Suite& suite, float /*dt*/)
    {
        if (suite.samples.empty()) return;

        double maxMean = 0.0;
        double scalar = suite.samples.front().meanMs;
        for (const auto& s : suite.samples) if (s.meanMs > maxMean) maxMean = s.meanMs;
        if (maxMean <= 0.0) maxMean = 1.0;

        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 cursor = ImGui::GetCursorScreenPos();
        float avail = ImGui::GetContentRegionAvail().x;
        float barH = 38.0f;
        float gap = 14.0f;
        float labelW = 160.0f;
        float numW = 180.0f;
        float chartW = avail - labelW - numW - 30.0f;
        if (chartW < 100) chartW = 100;

        for (std::size_t i = 0; i < suite.samples.size(); ++i)
        {
            const auto& s = suite.samples[i];
            float y = cursor.y + i * (barH + gap);
            float frac = static_cast<float>(s.meanMs / maxMean);
            float w = chartW * frac;

            ImVec2 a(cursor.x + labelW, y);
            ImVec2 b(cursor.x + labelW + chartW, y + barH);
            dl->AddRectFilled(a, b, IM_COL32(20, 22, 32, 200), 8.0f);

            const auto& AC = Config::ACCENT_PRIMARY;
            const auto& AC2 = Config::ACCENT_SECONDARY;
            const Config::Color& C = (s.name == "Scalar") ? AC2 : AC;
            ImU32 c0 = IM_COL32((int)(C.r * 255), (int)(C.g * 255), (int)(C.b * 255), 235);
            ImU32 c1 = IM_COL32((int)(C.r * 180), (int)(C.g * 180), (int)(C.b * 180), 235);
            ImVec2 fb(a.x + w, b.y);
            dl->AddRectFilledMultiColor(a, fb, c0, c1, c1, c0);

            char label[64];
            std::snprintf(label, sizeof(label), "%s", s.name.c_str());
            dl->AddText(ImVec2(cursor.x, y + barH * 0.5f - 8), IM_COL32(230,230,240,255), label);

            char num[128];
            double speedup = scalar / s.meanMs;
            std::snprintf(num, sizeof(num), "%.3f ms   ×%.2f", s.meanMs, speedup);
            dl->AddText(ImVec2(b.x + 12, y + barH * 0.5f - 8), IM_COL32(220,220,230,255), num);
        }

        ImGui::Dummy(ImVec2(avail, suite.samples.size() * (barH + gap)));
    }

    void BenchmarkView::Render(float dt)
    {
        _animTime += dt;

        if (_future.valid() && !_running.load() &&
            _future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
        {
            _suites = _future.get();
        }

        const auto& AC = Config::ACCENT_PRIMARY;
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(AC.r, AC.g, AC.b, 1.0f));
        ImGui::TextUnformatted("БЕНЧМАРК АЛГОРИТМОВ");
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::TextDisabled("    Signal Energy = Σ |x_i|");
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::PushItemWidth(280);
        ImGui::SliderInt("Повторов", &_repeats, 1, 30);
        ImGui::PopItemWidth();
        ImGui::SameLine();

        if (_running.load())
        {
            ImGui::BeginDisabled();
            ImGui::Button("Идёт замер...");
            ImGui::EndDisabled();
            ImGui::SameLine();
            ImGui::ProgressBar(_progress.load(), ImVec2(-1, 0));
        }
        else
        {
            if (ImGui::Button("▶ Запустить полный бенчмарк"))
                StartBench();
        }

        ImGui::Spacing();

        if (_suites.empty())
        {
            ImGui::Dummy(ImVec2(0, 30));
            ImGui::TextDisabled("Нажмите кнопку, чтобы прогнать алгоритмы на разных размерах массива.");
            return;
        }

        if (ImGui::BeginTabBar("##sizes"))
        {
            for (std::size_t i = 0; i < _suites.size(); ++i)
            {
                char tab[32];
                std::snprintf(tab, sizeof(tab), "N=%zu", _suites[i].n);
                if (ImGui::BeginTabItem(tab))
                {
                    _selectedSize = static_cast<int>(i);
                    ImGui::Spacing();
                    DrawBars(_suites[i], dt);
                    ImGui::EndTabItem();
                }
            }
            ImGui::EndTabBar();
        }

        ImGui::Spacing();
        if (ImGui::CollapsingHeader("Сводная таблица по размерам"))
        {
            if (ImGui::BeginTable("summary", 1 + (int)_suites.front().samples.size(),
                                  ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
            {
                ImGui::TableSetupColumn("N");
                for (const auto& s : _suites.front().samples)
                    ImGui::TableSetupColumn(s.name.c_str());
                ImGui::TableHeadersRow();

                for (const auto& suite : _suites)
                {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("%zu", suite.n);
                    for (const auto& s : suite.samples)
                    {
                        ImGui::TableNextColumn();
                        ImGui::Text("%.3f ms", s.meanMs);
                    }
                }
                ImGui::EndTable();
            }
        }
    }
}

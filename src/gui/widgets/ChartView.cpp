#include "gui/widgets/ChartView.hpp"
#include "core/Config.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <imgui.h>

namespace ArmProject::Gui
{
    static std::vector<std::size_t> BuildSizes()
    {
        std::vector<std::size_t> sizes;
        const double lo = std::log2(512.0);
        const double hi = std::log2(4194304.0);
        const int    n  = 80;
        for (int i = 0; i < n; ++i)
        {
            double t   = (double)i / (double)(n - 1);
            double exp = lo + t * (hi - lo);
            std::size_t s = static_cast<std::size_t>(std::round(std::pow(2.0, exp)));
            s = (s + 15) & ~std::size_t(15);
            if (s < 16) s = 16;
            sizes.push_back(s);
        }
        return sizes;
    }

    ChartView::ChartView()
        : _sizes(BuildSizes()), _progress(0.0f), _running(false), _repeats(3), _animTime(0.0f)
    {
    }

    ChartView::~ChartView()
    {
        if (_future.valid()) _future.wait();
    }

    void ChartView::StartBench()
    {
        if (_running.load()) return;
        _running.store(true);
        _progress.store(0.0f);
        _suites.clear();
        auto sizes = _sizes;
        int repeats = _repeats;
        _future = std::async(std::launch::async, [this, sizes, repeats]() {
            std::vector<Bench::Suite> out;
            out.reserve(sizes.size());
            int total = static_cast<int>(sizes.size());
            int idx   = 0;
            for (auto n : sizes)
            {
                out.push_back(Bench::RunSuite(n, repeats, 1, Config::DEFAULT_SEED));
                ++idx;
                _progress.store((float)idx / (float)total);
            }
            _running.store(false);
            return out;
        });
    }

    void ChartView::DrawChart()
    {
        if (_suites.size() < 2) return;

        std::size_t numAlgo = _suites.front().samples.size();
        if (numAlgo == 0) return;

        ImDrawList* dl     = ImGui::GetWindowDrawList();
        ImVec2 origin      = ImGui::GetCursorScreenPos();
        float avail        = ImGui::GetContentRegionAvail().x;
        float availH       = ImGui::GetContentRegionAvail().y - 32.0f;
        float chartH       = std::max(300.0f, availH);
        float padL         = 72.0f;
        float padR         = 24.0f;
        float padT         = 48.0f;
        float padB         = 44.0f;
        float W            = avail - padL - padR;
        float H            = chartH - padT - padB;

        ImVec2 tl(origin.x + padL, origin.y + padT);
        ImVec2 br(origin.x + padL + W, origin.y + padT + H);

        dl->AddRectFilled(ImVec2(origin.x, origin.y),
                          ImVec2(origin.x + avail, origin.y + chartH),
                          IM_COL32(10, 12, 20, 230), 12.0f);
        dl->AddRect(ImVec2(origin.x, origin.y),
                    ImVec2(origin.x + avail, origin.y + chartH),
                    IM_COL32(55, 60, 90, 130), 12.0f, ImDrawFlags_None, 1.2f);

        double maxMs = 0.0;
        for (const auto& suite : _suites)
            if (suite.n >= 65536)
                for (const auto& s : suite.samples)
                    if (s.meanMs > maxMs) maxMs = s.meanMs;
        if (maxMs <= 0.0) maxMs = 1.0;

        double yStep = 0.5;
        if (maxMs > 8.0)  yStep = 2.0;
        else if (maxMs > 4.0) yStep = 1.0;
        {
            double yTick = 0.0;
            while (yTick <= maxMs + 1e-9)
            {
                float fy = tl.y + H * (1.0f - (float)(yTick / maxMs));
                if (fy >= tl.y - 1 && fy <= br.y + 1)
                {
                    bool maj = (yTick < 1e-9 || std::fmod(yTick, (yStep * 2)) < 1e-9);
                    dl->AddLine(ImVec2(tl.x, fy), ImVec2(br.x, fy),
                                maj ? IM_COL32(80, 85, 115, 160) : IM_COL32(38, 42, 62, 110),
                                maj ? 1.1f : 0.7f);
                    char lbl[32];
                    if (yTick < 1e-9)
                        std::snprintf(lbl, sizeof(lbl), "0");
                    else if (yStep < 1.0)
                        std::snprintf(lbl, sizeof(lbl), "%.1f ms", yTick);
                    else
                        std::snprintf(lbl, sizeof(lbl), "%.0f ms", yTick);
                    ImVec2 ts = ImGui::CalcTextSize(lbl);
                    dl->AddText(ImVec2(tl.x - ts.x - 6, fy - 7),
                                IM_COL32(130, 136, 160, 200), lbl);
                }
                yTick += yStep;
            }
        }

        const std::size_t N = _suites.size();

        const double logXMin   = std::log10(65536.0);
        const double logXMax   = std::log10((double)_suites.back().n);
        const double logXRange = logXMax - logXMin;

        auto logFx = [&](double sz) -> float {
            double t = (std::log10(sz) - logXMin) / logXRange;
            return tl.x + (float)t * W;
        };

        const double xTickValues[] = {
            80000, 100000, 200000, 500000, 1000000, 2000000, 4000000
        };
        for (double sz : xTickValues)
        {
            float fx = logFx(sz);
            if (fx < tl.x - 1 || fx > br.x + 1) continue;
            dl->AddLine(ImVec2(fx, tl.y), ImVec2(fx, br.y),
                        IM_COL32(40, 44, 66, 110), 0.7f);
            char lbl[32];
            if (sz >= 1e6)
                std::snprintf(lbl, sizeof(lbl), "%.0fM", sz / 1e6);
            else
                std::snprintf(lbl, sizeof(lbl), "%.0fK", sz / 1000.0);
            ImVec2 ts = ImGui::CalcTextSize(lbl);
            dl->AddText(ImVec2(fx - ts.x * 0.5f, br.y + 7),
                        IM_COL32(140, 146, 170, 210), lbl);
        }

        struct LineStyle { ImU32 col; ImU32 glow; float thick; };
        const Config::Color& A1 = Config::ACCENT_SECONDARY;
        const Config::Color& A2 = Config::ACCENT_PRIMARY;
        const Config::Color& A3 = Config::ACCENT_TERTIARY;
        LineStyle styles[3] = {
            { IM_COL32((int)(A1.r*255),(int)(A1.g*255),(int)(A1.b*255),245),
              IM_COL32((int)(A1.r*255),(int)(A1.g*255),(int)(A1.b*255), 45), 2.2f },
            { IM_COL32((int)(A2.r*255),(int)(A2.g*255),(int)(A2.b*255),245),
              IM_COL32((int)(A2.r*255),(int)(A2.g*255),(int)(A2.b*255), 45), 2.2f },
            { IM_COL32((int)(A3.r*255),(int)(A3.g*255),(int)(A3.b*255),245),
              IM_COL32((int)(A3.r*255),(int)(A3.g*255),(int)(A3.b*255), 45), 2.2f },
        };

        for (std::size_t a = 0; a < numAlgo && a < 3; ++a)
        {
            std::vector<ImVec2> pts;
            pts.reserve(N);
            for (std::size_t i = 0; i < N; ++i)
            {
                if (_suites[i].n < 65536) continue;
                float fx = logFx(_suites[i].n);
                float fy = tl.y + H * (1.0f - (float)(_suites[i].samples[a].meanMs / maxMs));
                fy = std::max(tl.y, std::min(br.y, fy));
                pts.push_back(ImVec2(fx, fy));
            }
            dl->AddPolyline(pts.data(), (int)pts.size(), styles[a].glow,  ImDrawFlags_None, 9.0f);
            dl->AddPolyline(pts.data(), (int)pts.size(), styles[a].col,   ImDrawFlags_None, styles[a].thick);
        }

        dl->AddLine(ImVec2(tl.x, br.y), ImVec2(br.x, br.y), IM_COL32(90, 95, 125, 200), 1.5f);
        dl->AddLine(ImVec2(tl.x, tl.y), ImVec2(tl.x, br.y), IM_COL32(90, 95, 125, 200), 1.5f);

        float legX = tl.x + 18;
        float legY = tl.y - 34;
        for (std::size_t a = 0; a < numAlgo && a < 3; ++a)
        {
            const char* name = _suites.front().samples[a].name.c_str();
            float lineW = 28.0f;
            float lineY = legY + 7.0f;
            dl->AddLine(ImVec2(legX, lineY), ImVec2(legX + lineW, lineY),
                        styles[a].col, 2.5f);
            dl->AddCircleFilled(ImVec2(legX + lineW * 0.5f, lineY), 4.0f, styles[a].col);
            dl->AddText(ImVec2(legX + lineW + 6, legY - 7),
                        IM_COL32(210, 215, 232, 235), name);
            legX += lineW + 6 + ImGui::CalcTextSize(name).x + 28;
        }

        dl->AddText(ImVec2(origin.x + 4, origin.y + chartH * 0.5f - 36),
                    IM_COL32(110, 116, 140, 160), "ms");

        ImGui::Dummy(ImVec2(avail, chartH + 4));
    }

    void ChartView::Render(float dt)
    {
        _animTime += dt;

        if (_future.valid() && !_running.load() &&
            _future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
        {
            _suites = _future.get();
        }

        const auto& AC = Config::ACCENT_PRIMARY;
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(AC.r, AC.g, AC.b, 1.0f));
        ImGui::TextUnformatted("СРАВНИТЕЛЬНЫЙ ГРАФИК АЛГОРИТМОВ");
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::TextDisabled("    80 точек · логарифмический шаг по N");
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::PushItemWidth(240);
        ImGui::SliderInt("Повторов##chart", &_repeats, 1, 10);
        ImGui::PopItemWidth();
        ImGui::SameLine();

        if (_running.load())
        {
            ImGui::BeginDisabled();
            ImGui::Button("Замер...");
            ImGui::EndDisabled();
            ImGui::SameLine();
            ImGui::ProgressBar(_progress.load(), ImVec2(-1, 0));
        }
        else
        {
            if (ImGui::Button("▶ Построить график"))
                StartBench();
        }

        ImGui::Spacing();

        if (_suites.empty())
        {
            ImGui::Dummy(ImVec2(0, 40));
            ImGui::TextDisabled("Нажмите кнопку — алгоритмы будут прогнаны на 80 размерах "
                                "от 512 до 4M элементов.");
            return;
        }

        DrawChart();
    }
}

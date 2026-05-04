#include "gui/widgets/DemoView.hpp"
#include "gui/Theme.hpp"
#include "core/algo/SignalEnergy.hpp"
#include "core/Config.hpp"

#include <chrono>
#include <cmath>
#include <cstdio>
#include <imgui.h>
#include <random>

namespace ArmProject::Gui
{
    DemoView::DemoView()
        : _size(2048), _freq(4.0f), _phase(0.0f),
          _energyScalar(0), _energyNeon(0), _msScalar(0), _msNeon(0)
    {
        Regenerate();
    }

    DemoView::~DemoView() = default;

    void DemoView::Regenerate()
    {
        _data.assign(static_cast<std::size_t>(_size), 0);
        std::mt19937_64 rng(Config::DEMO_NOISE_SEED);
        std::uniform_real_distribution<float> n(-1.0f, 1.0f);
        for (int i = 0; i < _size; ++i)
        {
            float t = static_cast<float>(i) / static_cast<float>(_size);
            float s = std::sin(t * 6.28318f * _freq) +
                      0.4f * std::sin(t * 6.28318f * _freq * 2.7f);
            float v = s + Config::DEMO_NOISE_LEVEL * n(rng);
            _data[i] = static_cast<std::int32_t>(v * 1.0e6f);
        }
        RecomputeEnergy();
    }

    void DemoView::RecomputeEnergy()
    {
        auto run = [&](auto fn) {
            auto t0 = std::chrono::high_resolution_clock::now();
            std::int64_t v = 0;
            for (int k = 0; k < 64; ++k) v = fn(_data.data(), _data.size());
            auto t1 = std::chrono::high_resolution_clock::now();
            return std::pair{v, std::chrono::duration<double, std::milli>(t1 - t0).count() / 64.0};
        };
        auto [s1, t1] = run(Algo::SignalEnergyScalar);
        auto [s2, t2] = run(Algo::SignalEnergyNeon);
        _energyScalar = s1; _msScalar = t1;
        _energyNeon = s2;   _msNeon = t2;
    }

    void DemoView::Render(float dt)
    {
        _phase += dt * 1.2f;

        const auto& AC = Config::ACCENT_PRIMARY;
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(AC.r, AC.g, AC.b, 1.0f));
        ImGui::TextUnformatted("ДЕМОНСТРАЦИЯ");
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::TextDisabled("    Аудио-PCM сигнал -> энергия Σ|x|");
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::TextWrapped(
            "Здесь моделируется один кадр аудио-сигнала длиной N отсчётов int32. "
            "Сигнал - сумма двух синусоид с указанной частотой; именно такие данные "
            "обычно поступают с АЦП. После генерации обе версии алгоритма "
            "(скалярная и NEON) считают сумму |x_i| - абсолютную энергию сигнала. "
            "Результат и время выполнения показаны ниже, ось графика - амплитуда отсчётов.");
        ImGui::Spacing();

        bool dirty = false;
        ImGui::PushItemWidth(280);
        dirty |= ImGui::SliderInt("Размер N (отсчётов)", &_size, 256, 16384);
        dirty |= ImGui::SliderFloat("Частота (циклов на кадр)", &_freq, 0.5f, 20.0f, "%.2f");
        ImGui::PopItemWidth();
        if (dirty) Regenerate();
        if (ImGui::Button("Перегенерировать"))
            Regenerate();
        ImGui::SameLine();
        if (ImGui::Button("Замерить заново"))
            RecomputeEnergy();

        ImGui::Spacing();

        ImVec2 avail = ImGui::GetContentRegionAvail();
        ImVec2 origin = ImGui::GetCursorScreenPos();
        ImVec2 size(avail.x, 200);
        ImDrawList* dl = ImGui::GetWindowDrawList();

        const auto& MID = Config::BG_MID;
        ImU32 bg = IM_COL32((int)(MID.r * 255), (int)(MID.g * 255), (int)(MID.b * 255), 235);
        dl->AddRectFilled(origin, ImVec2(origin.x + size.x, origin.y + size.y), bg, 8.0f);
        ImU32 border = IM_COL32(40, 46, 60, 220);
        dl->AddRect(origin, ImVec2(origin.x + size.x, origin.y + size.y), border, 8.0f);

        float midY = origin.y + size.y * 0.5f;
        ImU32 grid = IM_COL32(50, 56, 72, 180);
        dl->AddLine(ImVec2(origin.x, midY), ImVec2(origin.x + size.x, midY), grid, 1.0f);
        for (int k = 1; k < 4; ++k)
        {
            float gy = origin.y + size.y * k / 4.0f;
            dl->AddLine(ImVec2(origin.x, gy), ImVec2(origin.x + size.x, gy),
                        IM_COL32(36, 42, 54, 120), 1.0f);
        }

        ImU32 lineCol = IM_COL32((int)(AC.r * 255), (int)(AC.g * 255), (int)(AC.b * 255), 235);
        int step = std::max(1, _size / static_cast<int>(size.x));
        ImVec2 prev{};
        bool has = false;
        for (int i = 0; i < _size; i += step)
        {
            float xt = static_cast<float>(i) / static_cast<float>(_size);
            float yt = static_cast<float>(_data[i]) / 2.0e6f;
            ImVec2 p(origin.x + xt * size.x, midY - yt * size.y * 0.42f);
            if (has) dl->AddLine(prev, p, lineCol, 1.5f);
            prev = p;
            has = true;
        }

        ImGui::Dummy(size);
        ImGui::Spacing();

        ImGui::Columns(2, nullptr, false);
        ImGui::TextDisabled("Скалярная версия");
        ImGui::Text("Энергия: %lld", static_cast<long long>(_energyScalar));
        ImGui::Text("Время:   %.4f ms", _msScalar);
        ImGui::NextColumn();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(AC.r, AC.g, AC.b, 1.0f));
        ImGui::TextUnformatted("NEON-версия");
        ImGui::PopStyleColor();
        ImGui::Text("Энергия: %lld", static_cast<long long>(_energyNeon));
        ImGui::Text("Время:   %.4f ms", _msNeon);
        if (_msNeon > 0)
            ImGui::Text("Ускорение: ×%.2f", _msScalar / _msNeon);
        ImGui::Columns(1);

        if (_energyScalar != _energyNeon)
            ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "ВНИМАНИЕ: результаты не совпадают!");
    }
}

#include "gui/Theme.hpp"
#include "core/Config.hpp"

#include <cmath>

namespace ArmProject::Gui
{
    static ImVec4 ToVec(const Config::Color& c) { return ImVec4(c.r, c.g, c.b, c.a); }
    static ImVec4 ToVec(const Config::Color& c, float a) { return ImVec4(c.r, c.g, c.b, a); }

    void ApplyTheme()
    {
        ImGuiStyle& s = ImGui::GetStyle();
        s.WindowRounding    = 10.0f;
        s.ChildRounding     = 8.0f;
        s.FrameRounding     = 6.0f;
        s.GrabRounding      = 6.0f;
        s.PopupRounding     = 8.0f;
        s.ScrollbarRounding = 8.0f;
        s.TabRounding       = 6.0f;
        s.WindowBorderSize  = 0.0f;
        s.ChildBorderSize   = 1.0f;
        s.FrameBorderSize   = 0.0f;
        s.WindowPadding     = ImVec2(20, 18);
        s.FramePadding      = ImVec2(12, 7);
        s.ItemSpacing       = ImVec2(12, 10);
        s.ItemInnerSpacing  = ImVec2(8, 6);
        s.ScrollbarSize     = 10.0f;
        s.GrabMinSize       = 10.0f;

        ImVec4* c = s.Colors;
        const auto& A   = Config::ACCENT_PRIMARY;
        const auto& A2  = Config::ACCENT_SECONDARY;
        const auto& A3  = Config::ACCENT_TERTIARY;
        const auto& BG  = Config::BG_DEEP;
        const auto& MID = Config::BG_MID;
        const auto& SF  = Config::BG_SURFACE;
        const auto& TX  = Config::TEXT_PRIMARY;
        const auto& TM  = Config::TEXT_MUTED;

        c[ImGuiCol_Text]                  = ToVec(TX);
        c[ImGuiCol_TextDisabled]          = ToVec(TM);
        c[ImGuiCol_WindowBg]              = ImVec4(BG.r, BG.g, BG.b, 0.0f);
        c[ImGuiCol_ChildBg]               = ToVec(MID, 0.65f);
        c[ImGuiCol_PopupBg]               = ToVec(MID, 0.98f);
        c[ImGuiCol_Border]                = ImVec4(0.18f, 0.20f, 0.26f, 0.6f);
        c[ImGuiCol_FrameBg]               = ToVec(SF);
        c[ImGuiCol_FrameBgHovered]        = ImVec4(0.13f, 0.15f, 0.20f, 1.0f);
        c[ImGuiCol_FrameBgActive]         = ImVec4(0.16f, 0.18f, 0.24f, 1.0f);
        c[ImGuiCol_TitleBg]               = ToVec(BG);
        c[ImGuiCol_TitleBgActive]         = ToVec(MID);
        c[ImGuiCol_CheckMark]             = ToVec(A);
        c[ImGuiCol_SliderGrab]            = ToVec(A);
        c[ImGuiCol_SliderGrabActive]      = ToVec(A2);
        c[ImGuiCol_Button]                = ToVec(SF);
        c[ImGuiCol_ButtonHovered]         = ImVec4(0.14f, 0.17f, 0.23f, 1.0f);
        c[ImGuiCol_ButtonActive]          = ImVec4(A.r * 0.6f, A.g * 0.6f, A.b * 0.7f, 1.0f);
        c[ImGuiCol_Header]                = ImVec4(0.12f, 0.14f, 0.20f, 1.0f);
        c[ImGuiCol_HeaderHovered]         = ImVec4(0.16f, 0.19f, 0.26f, 1.0f);
        c[ImGuiCol_HeaderActive]          = ImVec4(A.r * 0.4f, A.g * 0.4f, A.b * 0.55f, 1.0f);
        c[ImGuiCol_Separator]             = ImVec4(0.16f, 0.18f, 0.24f, 1.0f);
        c[ImGuiCol_SeparatorHovered]      = ToVec(A, 0.5f);
        c[ImGuiCol_SeparatorActive]       = ToVec(A);
        c[ImGuiCol_Tab]                   = ImVec4(0.07f, 0.08f, 0.11f, 1.0f);
        c[ImGuiCol_TabHovered]            = ImVec4(0.12f, 0.14f, 0.20f, 1.0f);
        c[ImGuiCol_TabActive]             = ImVec4(0.10f, 0.12f, 0.17f, 1.0f);
        c[ImGuiCol_TabUnfocused]          = ImVec4(0.06f, 0.07f, 0.10f, 1.0f);
        c[ImGuiCol_TabUnfocusedActive]    = ImVec4(0.08f, 0.09f, 0.13f, 1.0f);
        c[ImGuiCol_PlotLines]             = ToVec(A);
        c[ImGuiCol_PlotLinesHovered]      = ToVec(A3);
        c[ImGuiCol_PlotHistogram]         = ToVec(A);
        c[ImGuiCol_PlotHistogramHovered]  = ToVec(A3);
        c[ImGuiCol_TextSelectedBg]        = ToVec(A, 0.30f);
        c[ImGuiCol_ScrollbarBg]           = ImVec4(0.04f, 0.05f, 0.07f, 0.50f);
        c[ImGuiCol_ScrollbarGrab]         = ImVec4(0.18f, 0.20f, 0.26f, 1.0f);
        c[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.22f, 0.24f, 0.32f, 1.0f);
        c[ImGuiCol_ScrollbarGrabActive]   = ToVec(A);
        c[ImGuiCol_TableHeaderBg]         = ToVec(MID);
        c[ImGuiCol_TableBorderStrong]     = ImVec4(0.16f, 0.18f, 0.24f, 1.0f);
        c[ImGuiCol_TableBorderLight]      = ImVec4(0.12f, 0.14f, 0.18f, 1.0f);
        c[ImGuiCol_TableRowBgAlt]         = ImVec4(1.0f, 1.0f, 1.0f, 0.02f);
    }

    ImU32 HsvColor(float h, float s, float v, float a)
    {
        float r, g, b;
        ImGui::ColorConvertHSVtoRGB(h, s, v, r, g, b);
        return IM_COL32((int)(r * 255), (int)(g * 255), (int)(b * 255), (int)(a * 255));
    }

    void DrawAnimatedBackground(ImDrawList* dl, ImVec2 a, ImVec2 b, float t)
    {
        const auto& BG = Config::BG_DEEP;
        ImU32 base = IM_COL32((int)(BG.r * 255), (int)(BG.g * 255), (int)(BG.b * 255), 255);
        dl->AddRectFilled(a, b, base);

        const auto& A = Config::ACCENT_PRIMARY;
        const auto& A2 = Config::ACCENT_SECONDARY;
        float w = b.x - a.x;
        float h = b.y - a.y;

        for (int i = 0; i < 2; ++i)
        {
            float ang = t * 0.10f + i * 3.14f;
            float cx = a.x + w * (0.30f + 0.40f * (i == 0 ? 0.0f : 1.0f)) +
                       std::cos(ang) * 80.0f;
            float cy = a.y + h * 0.50f + std::sin(ang * 1.2f) * 60.0f;
            float r = std::min(w, h) * 0.55f;
            const auto& C = (i == 0) ? A : A2;
            for (int k = 18; k > 0; --k)
            {
                float fr = (float)k / 18.0f;
                int alpha = (int)((1.0f - fr) * 14.0f);
                ImU32 col = IM_COL32((int)(C.r * 255), (int)(C.g * 255), (int)(C.b * 255), alpha);
                dl->AddCircleFilled(ImVec2(cx, cy), r * fr, col, 48);
            }
        }
    }

    void DrawGlow(ImDrawList* dl, ImVec2 a, ImVec2 b, ImU32 color, float radius)
    {
        for (int i = 1; i <= 4; ++i)
        {
            ImU32 col = (color & 0x00FFFFFFu) | ((unsigned)((5 - i) * 10) << 24);
            dl->AddRect(ImVec2(a.x - i, a.y - i), ImVec2(b.x + i, b.y + i),
                        col, radius + i, 0, 1.0f);
        }
    }
}
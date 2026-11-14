// BotWRecomp: replaces Cemu's LatteOverlay. Drawn with Cemu's ImGui pass right after the TV image
// (LatteRenderTarget_copyToBackbuffer, GPU thread): an optional FPS counter and the in-game settings
// menu (runtime/src/host/menu.cpp). ImGui's built-in font is used.
#include <imgui.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <pthread.h>

#include "../src/host/menu.h"
#include "../src/host/soft_keyboard.h"
#include "../src/host/touch.h"
#include "../src/host/touch_draw.h"
#include "../src/os.h"
#include "../src/settings.h"
#include "Cafe/HW/Latte/Core/LatteOverlay.h"

void log_write(const char* fmt, ...);
namespace threads {
std::string main_wait_report();
}

namespace {
std::atomic<float> g_fps{0};
std::atomic<int> g_draws{0};
std::atomic<float> g_gpu_ms_f{0};
std::atomic<float> g_us_per_draw{0};
}  // namespace

// Cemu's GPU thread (LatteThread), for the host loop's slow-frame sampler (window.mm).
std::atomic<pthread_t> g_gpu_thread{nullptr};

void LatteOverlay_updateStats(double fps, sint32 drawcalls, sint32 fastDrawcalls) {
    g_gpu_thread = pthread_self();  // stats are updated on the GPU thread
    g_fps = (float)fps;
    g_draws = drawcalls;
    using clock = std::chrono::steady_clock;
    static auto s_prev_tp = clock::now();
    auto now_tp = clock::now();
    double dt_s = std::chrono::duration<double>(now_tp - s_prev_tp).count();
    s_prev_tp = now_tp;
    uint64_t gpu_us = g_gpu_hw_time_us.exchange(0, std::memory_order_relaxed);
    double gpu_ms_f = 0.0, us_per_draw = 0.0;
    if (dt_s > 0.05 && fps > 0.5) {
        gpu_ms_f = ((double)gpu_us / 1000.0) / (fps * dt_s);
        if (drawcalls > 0) us_per_draw = (gpu_ms_f * 1000.0) / (double)drawcalls;
    }
    g_gpu_ms_f.store((float)gpu_ms_f, std::memory_order_relaxed);
    g_us_per_draw.store((float)us_per_draw, std::memory_order_relaxed);
    if (!g_boot_world_ready.load(std::memory_order_relaxed)) {
        static int s_world_ticks = 0;
        // Title/loading screens stay <= ~150 draws/frame; 3D overworld/shrines sustain >= 600.
        if (fps > 0.0 && drawcalls >= 600) {
            if (++s_world_ticks >= 3) {
                g_boot_world_ready.store(true, std::memory_order_release);
                log_write("fps: 3D world ready (%d draws/frame), unlocking 60 fps", drawcalls);
            }
        } else {
            s_world_ticks = 0;
        }
    }
    static int n = 0;
    if (fps > 0 && (n++ % 3) == 0)
        log_write("[gpu] %.1f fps, %d draws/frame (%d fast), gpu_hw=%.2fms/f (%.2fus/d), %s lp=%d; main thread waits: %s",
                  fps, drawcalls, fastDrawcalls, gpu_ms_f, us_per_draw,
                  botw_thermal_state_name(botw_thermal_state()), (int)botw_low_power_mode(),
                  threads::main_wait_report().c_str());
}

void LatteOverlay_render(bool padView) {
    if (padView) return;
    swkbd_ui_render();  // on-screen keyboard for nn_swkbd, above the game image
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    const float ui = std::max(1.0f, io.DisplaySize.y / 540.0f);
    // Font atlas is baked at 36 px (imgui_font.mm); scale relative to 36 px so Retina text is razor-sharp.
    const float font_scale = (14.5f * ui) / 36.0f;

    float px, py;
    bool down;
    menu::pointer_state(&px, &py, &down);
    io.AddMousePosEvent(px, py);
    io.AddMouseButtonEvent(0, down);

    const ImGuiWindowFlags overlay = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                                     ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
                                     ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;
    const int show_mode = settings::get().show_fps;
    if (show_mode > 0) {
        float sl, st, sr, sb;  // top-right, inside the safe area with some padding
        menu::safe_area(&sl, &st, &sr, &sb);
        ImVec2 pos{io.DisplaySize.x - sr - 16 * ui, st + 12 * ui}, pivot{1.0f, 0.0f};
        const touch::DrawState ts = touch::draw_state();
        if (ts.visible && !ts.collapsed && ts.w > 0)  // on-screen ZR is up there: centered under the top button row
            for (const touch::DrawItem& it : ts.items)
                if (it.shape == touch::DrawItem::kMenu) {
                    const float k = io.DisplaySize.x / ts.w;
                    pos = {it.x * k, (it.y + it.r + 10 * ts.u) * k}, pivot = {0.5f, 0.0f};
                }
        ImGui::SetNextWindowPos(pos, ImGuiCond_Always, pivot);
        ImGui::SetNextWindowBgAlpha(0.58f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f * ui);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {10.0f * ui, 5.0f * ui});
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.86f, 0.71f, 0.34f, 0.35f));
        if (ImGui::Begin("##fps", nullptr, overlay | ImGuiWindowFlags_NoInputs)) {
            ImGui::SetWindowFontScale(font_scale * 0.90f);
            ImGui::Text("%.0f FPS  ·  %d Draws", (double)g_fps, (int)g_draws);
            if (show_mode >= 2) {
                const float gpu_ms = g_gpu_ms_f.load(std::memory_order_relaxed);
                const float us_d = g_us_per_draw.load(std::memory_order_relaxed);
                const int draws = g_draws.load(std::memory_order_relaxed);
                const char* gpu_state = "";
                ImVec4 gpu_col(0.75f, 0.78f, 0.82f, 1.0f);
                if (draws >= 2500 && us_d > 0.0f) {
                    if (us_d <= 3.85f) {
                        gpu_state = "(满速)";
                        gpu_col = ImVec4(0.45f, 0.92f, 0.55f, 1.0f);
                    } else if (us_d <= 4.65f) {
                        gpu_state = "(轻度降频)";
                        gpu_col = ImVec4(0.98f, 0.85f, 0.35f, 1.0f);
                    } else {
                        gpu_state = "(降频)";
                        gpu_col = ImVec4(1.00f, 0.35f, 0.25f, 1.0f);
                    }
                }
                ImGui::SameLine(0, 8.0f * ui);
                ImGui::TextColored(gpu_col, "GPU:%.1fms%s%s", (double)gpu_ms, gpu_state,
                                   botw_low_power_mode() ? " [省电]" : "");
            }
        }
        ImGui::End();
        ImGui::PopStyleColor(1);
        ImGui::PopStyleVar(2);
    }

    menu::View v = menu::view();
    if (!v.open) touch::draw();
    if (!v.open) return;

    ImGui::SetNextWindowPos({io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f}, ImGuiCond_Always, {0.5f, 0.5f});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 16.0f * ui);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.5f * ui);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {20.0f * ui, 14.0f * ui});
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 7.0f * ui);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {8.0f * ui, 4.0f * ui});
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {8.0f * ui, 4.5f * ui});

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.06f, 0.08f, 0.11f, 0.93f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.86f, 0.71f, 0.34f, 0.55f));
    ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.86f, 0.71f, 0.34f, 0.28f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.16f, 0.21f, 0.29f, 0.92f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.27f, 0.17f, 0.95f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.45f, 0.38f, 0.19f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.86f, 0.71f, 0.34f, 0.20f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.86f, 0.71f, 0.34f, 0.28f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.86f, 0.71f, 0.34f, 0.36f));

    if (ImGui::Begin("##menu", nullptr, overlay)) {
        const float label_w = 135.0f * ui;
        const float btn_w = 34.0f * ui;
        const float val_w = 195.0f * ui;
        const float row_w = label_w + btn_w + val_w + btn_w + 18.0f * ui;

        ImGui::SetWindowFontScale(font_scale * 1.15f);
        ImGui::TextColored(ImVec4(0.95f, 0.80f, 0.42f, 1.0f), "塞尔达传说：旷野之息 · 设置");
        const float close_w = 52.0f * ui;
        const float close_h = ImGui::GetFrameHeight();
        ImGui::SameLine(ImGui::GetStyle().WindowPadding.x + row_w - close_w);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.32f, 0.15f, 0.17f, 0.88f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.50f, 0.20f, 0.22f, 0.96f));
        if (ImGui::Button("关闭", {close_w, close_h})) menu::close();
        ImGui::PopStyleColor(2);
        const ImVec2 xa = ImGui::GetItemRectMin(), xb = ImGui::GetItemRectMax();

        ImGui::Spacing();
        ImGui::Separator();

        for (int i = 0; i < (int)v.rows.size(); i++) {
            const menu::Row& r = v.rows[i];
            if (r.section) {
                ImGui::Spacing();
                ImGui::SetWindowFontScale(font_scale * 0.84f);
                ImGui::TextColored(ImVec4(0.54f, 0.80f, 0.90f, 0.92f), "%s", r.section);
            }
            ImGui::SetWindowFontScale(font_scale);
            ImGui::PushID(i);
            const bool sel = i == v.cursor;
            const float cur_h = ImGui::GetFrameHeight();
            if (r.action) {
                ImGui::Spacing();
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.22f, 0.28f, 0.36f, 0.92f));
                if (ImGui::Button(r.label.c_str(), {row_w, cur_h * 1.05f})) menu::click(i, 0);
                ImGui::PopStyleColor();
            } else {
                ImGui::AlignTextToFramePadding();
                if (sel) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.98f, 0.84f, 0.44f, 1.0f));
                if (ImGui::Selectable(r.label.c_str(), sel, 0, {label_w, cur_h})) menu::click(i, 0);
                if (sel) ImGui::PopStyleColor();

                ImGui::SameLine(0, 6.0f * ui);
                if (ImGui::Button("<", {btn_w, cur_h})) menu::click(i, -1);

                ImGui::SameLine(0, 6.0f * ui);
                const float vx0 = ImGui::GetCursorPosX();
                const float tw = ImGui::CalcTextSize(r.value.c_str()).x;
                const float offset_x = std::max(0.0f, (val_w - tw) * 0.5f);
                ImGui::SetCursorPosX(vx0 + offset_x);
                ImGui::AlignTextToFramePadding();
                ImGui::TextColored(sel ? ImVec4(1.0f, 0.96f, 0.84f, 1.0f) : ImVec4(0.88f, 0.91f, 0.95f, 1.0f), "%s", r.value.c_str());

                ImGui::SameLine(vx0 + val_w + 6.0f * ui);
                if (ImGui::Button(">", {btn_w, cur_h})) menu::click(i, +1);
            }
            ImGui::PopID();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::SetWindowFontScale(font_scale * 0.82f);
        ImGui::TextColored(ImVec4(0.75f, 0.82f, 0.88f, 0.95f), "%s", v.note.c_str());
        ImGui::TextColored(ImVec4(0.58f, 0.65f, 0.72f, 0.90f), "点击左右箭头或选项切换  ·  方向键选择 / A确认 / B或点外部关闭");

        // for touch (host side, see menu::touch_close): panel and X in fractions of the screen
        const ImVec2 wp = ImGui::GetWindowPos(), ws = ImGui::GetWindowSize(), d = io.DisplaySize;
        const float panel[4] = {wp.x / d.x, wp.y / d.y, (wp.x + ws.x) / d.x, (wp.y + ws.y) / d.y};
        const float xbtn[4] = {xa.x / d.x, xa.y / d.y, xb.x / d.x, xb.y / d.y};
        menu::set_rects(panel, xbtn);
    }
    ImGui::End();
    ImGui::PopStyleColor(9);
    ImGui::PopStyleVar(6);
}

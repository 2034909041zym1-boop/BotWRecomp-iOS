// On-screen keyboard for nn_swkbd, drawn with the ImGui overlay pass (LatteOverlay_render) and
// driven by the gamepad: the stick / d-pad move the highlight, A types, B deletes, + confirms.
// See runtime/src/host/soft_keyboard.h for why a UIKit overlay cannot be used on this port.
#include <algorithm>
#include <cfloat>
#include <cstdio>
#include <cstring>
#include <mutex>

#include <imgui.h>

#include "../src/host/soft_keyboard.h"
#include "../src/input/input.h"

namespace {

constexpr int kCols = 7;
constexpr int kRows = 6;
constexpr int kMaxText = 127;

// 0 marks an unused cell, '\b' is delete and '\n' is confirm.
const char kKeys[kRows][kCols] = {
    {'a', 'b', 'c', 'd', 'e', 'f', 'g'},
    {'h', 'i', 'j', 'k', 'l', 'm', 'n'},
    {'o', 'p', 'q', 'r', 's', 't', 'u'},
    {'v', 'w', 'x', 'y', 'z', '0', '1'},
    {'2', '3', '4', '5', '6', '7', '8'},
    {'9', ' ', '\b', '\n', 0, 0, 0},
};

std::mutex g_mu;
char g_text[kMaxText + 1];
uint32_t g_maxLen = 20;
int g_row = 0;
int g_col = 0;
int g_result = 0;
bool g_shown = false;
uint32_t g_prevButtons = 0;
int g_prevDx = 0;
int g_prevDy = 0;

void append(char ch) {
    const size_t n = strlen(g_text);
    if (n >= g_maxLen || n + 1 >= sizeof(g_text)) return;
    g_text[n] = ch;
    g_text[n + 1] = 0;
}

void backspace() {
    const size_t n = strlen(g_text);
    if (n) g_text[n - 1] = 0;
}

void move_to_valid(int dr, int dc) {
    int r = g_row, c = g_col;
    for (int i = 0; i < kRows * kCols; i++) {
        r = (r + dr + kRows) % kRows;
        c = (c + dc + kCols) % kCols;
        if (kKeys[r][c]) {
            g_row = r;
            g_col = c;
            return;
        }
    }
}

}  // namespace

void swkbd_ui_precreate(void) {}
void swkbd_ui_pump(void) {}

void swkbd_ui_show(const char* initial, int max_len) {
    std::lock_guard<std::mutex> l(g_mu);
    g_maxLen = max_len > 0 ? (uint32_t)max_len : 20;
    if (g_maxLen > kMaxText) g_maxLen = kMaxText;
    strncpy(g_text, initial ? initial : "", sizeof(g_text) - 1);
    g_text[sizeof(g_text) - 1] = 0;
    g_result = 0;
    g_row = 0;
    g_col = 0;
    g_prevButtons = 0;
    g_prevDx = g_prevDy = 0;
    g_shown = true;
}

void swkbd_ui_hide(void) {
    std::lock_guard<std::mutex> l(g_mu);
    g_shown = false;
}

void swkbd_ui_input(uint32_t buttons, float lx, float ly) {
    std::lock_guard<std::mutex> l(g_mu);
    if (!g_shown) return;
    const uint32_t pressed = buttons & ~g_prevButtons;
    g_prevButtons = buttons;

    const float dz = 0.55f;
    int dy = 0, dx = 0;
    if (buttons & input::BTN_UP) dy = -1;
    else if (buttons & input::BTN_DOWN) dy = +1;
    if (buttons & input::BTN_LEFT) dx = -1;
    else if (buttons & input::BTN_RIGHT) dx = +1;
    if (!dy) dy = ly > dz ? -1 : ly < -dz ? +1 : 0;
    if (!dx) dx = lx > dz ? +1 : lx < -dz ? -1 : 0;
    if (dy && dy != g_prevDy) move_to_valid(dy, 0);
    if (dx && dx != g_prevDx) move_to_valid(0, dx);
    g_prevDy = dy;
    g_prevDx = dx;

    if (pressed & input::BTN_B) backspace();  // B acts as backspace, like the Switch keyboard
    if (pressed & input::BTN_PLUS) g_result = 1;
    if (pressed & input::BTN_A) {
        const char k = kKeys[g_row][g_col];
        if (k == '\b') backspace();
        else if (k == '\n') g_result = 1;
        else if (k) append(k);
    }
}

void swkbd_ui_render(void) {
    std::lock_guard<std::mutex> l(g_mu);
    if (!g_shown) return;

    ImDrawList* dl = ImGui::GetForegroundDrawList();
    ImGuiIO& io = ImGui::GetIO();
    const float W = io.DisplaySize.x;
    const float H = io.DisplaySize.y;
    if (W <= 0 || H <= 0) return;

    const float ui = std::max(1.0f, H / 540.0f);
    const float top = H * 0.38f;
    const float pad = 6.0f * ui;
    const float grid_w = W * 0.94f;
    const float grid_h = (H - top) * 0.88f;
    const float cell_w = (grid_w - pad * (kCols + 1)) / kCols;
    const float cell_h = (grid_h - pad * (kRows + 1)) / kRows;
    const float x0 = (W - grid_w) * 0.5f;
    const float y0 = top + (H - top - grid_h) * 0.5f;

    dl->AddRectFilled(ImVec2(0, top), ImVec2(W, H), IM_COL32(0, 0, 0, 195));
    dl->AddLine(ImVec2(0, top), ImVec2(W, top), IM_COL32(255, 255, 255, 40), 2.0f * ui);

    ImFont* font = ImGui::GetFont();
    const float fs = std::min(cell_h * 0.52f, 26.0f * ui);

    for (int r = 0; r < kRows; r++) {
        for (int c = 0; c < kCols; c++) {
            const char k = kKeys[r][c];
            if (!k) continue;
            const ImVec2 a(x0 + pad + c * (cell_w + pad), y0 + pad + r * (cell_h + pad));
            const ImVec2 b(a.x + cell_w, a.y + cell_h);
            const bool sel = (r == g_row && c == g_col);
            dl->AddRectFilled(a, b, sel ? IM_COL32(214, 172, 70, 240) : IM_COL32(255, 255, 255, 34), 8.0f * ui);
            dl->AddRect(a, b, sel ? IM_COL32(255, 228, 150, 255) : IM_COL32(255, 255, 255, 70), 8.0f * ui, 0,
                        2.0f * ui);
            char single[2] = {k, 0};
            const char* label = k == '\b' ? "DEL" : k == '\n' ? "OK" : k == ' ' ? "SP" : single;
            const ImVec2 tsz = font->CalcTextSizeA(fs, FLT_MAX, 0.0f, label);
            dl->AddText(font, fs, ImVec2(a.x + (cell_w - tsz.x) * 0.5f, a.y + (cell_h - tsz.y) * 0.5f),
                        sel ? IM_COL32(24, 18, 6, 255) : IM_COL32(255, 255, 255, 236), label);
        }
    }

    char line[kMaxText + 8];
    snprintf(line, sizeof(line), "%s_", g_text);
    dl->AddText(font, 28.0f * ui, ImVec2(x0 + pad, top - 44.0f * ui), IM_COL32(255, 255, 255, 255), line);
    dl->AddText(font, 15.0f * ui, ImVec2(x0 + pad, top - 16.0f * ui), IM_COL32(210, 214, 220, 170),
                "摇杆/十字键移动   A 选择   B 删除   + 完成");
}

int swkbd_ui_active(void) {
    std::lock_guard<std::mutex> l(g_mu);
    return g_shown ? 1 : 0;
}

int swkbd_ui_poll(void) {
    std::lock_guard<std::mutex> l(g_mu);
    return g_result;
}

void swkbd_ui_text(char* out, int cap) {
    if (!out || cap <= 0) return;
    std::lock_guard<std::mutex> l(g_mu);
    strncpy(out, g_text, (size_t)cap - 1);
    out[cap - 1] = 0;
}

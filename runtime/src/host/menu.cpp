#include "menu.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdio>
#include <mutex>

#include "../settings.h"

extern float g_render_scale;  // main.cpp: the scale this run renders at

namespace menu {
namespace {
std::mutex g_lock;
std::atomic<bool> g_open{false};
int g_cursor = 0;

enum RowId {
    kFps, kFrameGen, kScale, kMetalFX, kAA, kFilter, kShadowHalf, kAniso4x, kGrassLod, kDisplay, kShowFps,
    kGyro, kRumble, kLayout, kTouch, kStickFixed, kLookSens, kLookAccel, kTouchAlpha, kMacroBtns,
    kVolume, kClose, kRows
};
const float kScales[] = {0, 0.5f, 0.75f, 0.8f, 1, 1.5f, 2, 2.5f, 3};  // 0 = fit the display
const float kLookSensValues[] = {1.0f, 1.5f, 2.0f, 3.0f, 4.0f, 5.0f};
const int kTouchAlphas[] = {25, 50, 75, 100};
const char* kDisplays[] = {"原始 16:9 (黑边)", "全屏拉伸", "全屏填充 (裁剪)", "宽屏 hor+ (重启后生效)"};
const char* kFilters[] = {"双线性 (Bilinear)", "双三次 (Bicubic)", "埃尔米特 (Hermite)", "邻近采样 (Nearest)"};
const char* kAAModes[] = {"关闭 (原生)", "FXAA 抗锯齿", "FXAA + CAS 锐化", "CAS 锐化 (无模糊)"};
const char* kShowFpsModes[] = {"关闭", "一档: 仅帧率", "二档: 帧率 + GPU状态"};

std::string scale_text(float s) {
    char b[64];
    if (s <= 0) return "自动 (适配屏幕)";
    snprintf(b, sizeof b, "%gx (%dx%d)", s, (int)lroundf(1280 * s), (int)lroundf(720 * s));
    return b;
}

void step(Settings& s, int row, int dir) {
    auto wrap = [](int v, int n) { return (v % n + n) % n; };
    switch (row) {
        case kFps: {
            static constexpr int kFpsVals[] = {30, 40, 60};
            int i = 0;
            for (int k = 0; k < 3; k++)
                if (kFpsVals[k] == s.fps) i = k;
            s.fps = kFpsVals[wrap(i + dir, 3)];
            break;
        }
        case kFrameGen: s.frame_gen = s.frame_gen ? 0 : 1; break;
        case kScale: {
            int i = 0;
            for (int k = 0; k < (int)std::size(kScales); k++)
                if (fabsf(kScales[k] - s.scale) < 0.01f) i = k;
            s.scale = kScales[wrap(i + dir, (int)std::size(kScales))];
            break;
        }
        case kMetalFX: s.metalfx = wrap(s.metalfx + dir, 3); break;
        case kAA: s.aa = wrap(s.aa + dir, 4); break;
        case kDisplay: s.display = wrap(s.display + dir, 4); break;
        case kFilter: s.filter = wrap(s.filter + dir, 4); break;
        case kShadowHalf: s.shadow_half = s.shadow_half ? 0 : 1; break;
        case kAniso4x: s.aniso_4x = s.aniso_4x ? 0 : 1; break;
        case kGrassLod: s.grass_lod = wrap(s.grass_lod + dir, 3); break;
        case kGyro: s.gyro = !s.gyro; break;
        case kRumble: s.rumble = !s.rumble; break;
        case kLayout: s.layout = !s.layout; break;
        case kTouch: s.touch = wrap(s.touch + dir, 3), s.touch_hidden = false; break;
        case kStickFixed: s.stick_fixed = s.stick_fixed ? 0 : 1; break;
        case kLookSens: {
            int i = 2;
            for (int k = 0; k < (int)std::size(kLookSensValues); k++)
                if (fabsf(kLookSensValues[k] - s.look_sens) < 0.05f) i = k;
            s.look_sens = kLookSensValues[wrap(i + dir, (int)std::size(kLookSensValues))];
            break;
        }
        case kLookAccel: s.look_accel = s.look_accel ? 0 : 1; break;
        case kTouchAlpha: {
            int i = 2;
            for (int k = 0; k < (int)std::size(kTouchAlphas); k++)
                if (kTouchAlphas[k] == s.touch_alpha) i = k;
            s.touch_alpha = kTouchAlphas[wrap(i + dir, (int)std::size(kTouchAlphas))];
            break;
        }
        case kMacroBtns: s.macro_btns = s.macro_btns ? 0 : 1; break;
        case kShowFps: s.show_fps = wrap(s.show_fps + dir, 3); break;
        case kVolume: s.volume = std::clamp(s.volume + dir * 10, 0, 100); break;
    }
}
}  // namespace

std::atomic<float> g_px{-1}, g_py{-1};
std::atomic<bool> g_pdown{false};
void pointer(float x, float y, bool down) {
    g_px = x;
    g_py = y;
    g_pdown = down;
}
void pointer_state(float* x, float* y, bool* down) {
    *x = g_px;
    *y = g_py;
    *down = g_pdown;
}

std::atomic<float> g_safe[4] = {0, 0, 0, 0};
void set_safe_area(float l, float t, float r, float b) {
    g_safe[0] = l, g_safe[1] = t, g_safe[2] = r, g_safe[3] = b;
}
void safe_area(float* l, float* t, float* r, float* b) {
    *l = g_safe[0], *t = g_safe[1], *r = g_safe[2], *b = g_safe[3];
}

std::atomic<float> g_panel[4] = {0, 0, 0, 0}, g_xbtn[4] = {0, 0, 0, 0};
void set_rects(const float p[4], const float x[4]) {
    for (int i = 0; i < 4; i++) g_panel[i] = p[i], g_xbtn[i] = x[i];
}
void rects(float p[4], float x[4]) {
    for (int i = 0; i < 4; i++) p[i] = g_panel[i], x[i] = g_xbtn[i];
}
bool touch_close(float x, float y) {
    const float slop = 0.04f;  // the X is small for a thumb
    bool outside = x < g_panel[0] || y < g_panel[1] || x > g_panel[2] || y > g_panel[3];
    bool on_x = x > g_xbtn[0] - slop && y > g_xbtn[1] - slop && x < g_xbtn[2] + slop && y < g_xbtn[3] + slop;
    if (!g_open || !(outside || on_x)) return false;
    close();
    return true;
}

bool is_open() { return g_open; }
void toggle() { g_open = !g_open; }
void close() { g_open = false; }

void move(int dy) {
    std::lock_guard<std::mutex> l(g_lock);
    g_cursor = (g_cursor + dy + kRows) % kRows;
}

void change(int dir) {
    int row;
    {
        std::lock_guard<std::mutex> l(g_lock);
        row = g_cursor;
    }
    if (row == kClose) return;
    Settings s = settings::get();
    step(s, row, dir);
    settings::set(s);
}

void activate() {
    int row;
    {
        std::lock_guard<std::mutex> l(g_lock);
        row = g_cursor;
    }
    if (row == kClose) close();
    else change(+1);
}

void click(int row, int dir) {
    if (row < 0 || row >= kRows) return;
    {
        std::lock_guard<std::mutex> l(g_lock);
        g_cursor = row;
    }
    if (dir == 0) activate();
    else change(dir);
}

View view() {
    View v;
    v.open = g_open;
    {
        std::lock_guard<std::mutex> l(g_lock);
        v.cursor = g_cursor;
    }
    Settings s = settings::get();
    char b[128];
    v.rows.resize(kRows);
    const char* fps_label = (s.fps >= 60) ? "60 帧 (高负载)" : (s.fps >= 40 ? "40 帧 (均衡/配插帧80)" : "30 帧 (流畅省电)");
    v.rows[kFps] = {"帧率模式", fps_label, false, "画质与性能"};
    v.rows[kFrameGen] = {"帧生成 (实验性)", s.frame_gen ? "2x 光流插帧 (建议配30帧)" : "关闭"};
    v.rows[kScale] = {"渲染分辨率", scale_text(s.scale)};
    static const char* kMetalFxModes[] = {"自动 (<1x开启)", "全局开启 (含1x)", "关闭"};
    v.rows[kMetalFX] = {"MetalFX 超分", kMetalFxModes[std::clamp(s.metalfx, 0, 2)]};
    v.rows[kAA] = {"抗锯齿", kAAModes[std::clamp(s.aa, 0, 3)]};
    v.rows[kFilter] = {"缩放滤镜", s.aa > 0 ? "由抗锯齿接管" : kFilters[std::clamp(s.filter, 0, 3)]};
    v.rows[kShadowHalf] = {"阴影分辨率", s.shadow_half ? "50% 省电 (重启生效)" : "原生 100%"};
    v.rows[kAniso4x] = {"各向异性过滤", s.aniso_4x ? "限制 4x 省电 (重启生效)" : "原生 (最高16x)"};
    static const char* kGrassLodModes[] = {"原生 (默认)", "低密度 (省电)", "极低密度 (最省电)"};
    v.rows[kGrassLod] = {"草地与远景LOD", kGrassLodModes[std::clamp(s.grass_lod, 0, 2)]};
    v.rows[kDisplay] = {"画面比例", kDisplays[std::clamp(s.display, 0, 3)]};
    v.rows[kShowFps] = {"性能角标", kShowFpsModes[std::clamp(s.show_fps, 0, 2)]};
    v.rows[kGyro] = {"体感瞄准", s.gyro ? "开启" : "关闭", false, "操控与音频"};
    v.rows[kRumble] = {"手柄震动", s.rumble ? "开启" : "关闭"};
    v.rows[kLayout] = {"按键布局", s.layout ? "Xbox (A在下)" : "任天堂 (A在右)"};
    v.rows[kTouch] = {"虚拟按键", s.touch == 1 ? "常开" : s.touch == 2 ? "关闭" : "自动 (连手柄隐藏)"};
    v.rows[kStickFixed] = {"左摇杆模式", s.stick_fixed ? "落点定心 (推荐)" : "持续跟随"};
    snprintf(b, sizeof b, "%gx", s.look_sens);
    v.rows[kLookSens] = {"视角灵敏度", b};
    v.rows[kLookAccel] = {"视角非线性加速", s.look_accel ? "开启 (慢稳快甩)" : "关闭 (线性)"};
    snprintf(b, sizeof b, "%d%%", s.touch_alpha);
    v.rows[kTouchAlpha] = {"按键透明度", b};
    v.rows[kMacroBtns] = {"快捷宏按钮", s.macro_btns ? "显示 (风弹/盾反/林克)" : "隐藏"};
    snprintf(b, sizeof b, "%d%%", s.volume);
    v.rows[kVolume] = {"游戏音量", b};
    v.rows[kClose] = {"关闭设置菜单", "", true};
    snprintf(b, sizeof b, "当前以 %gx 渲染  ·  分辨率与宽屏 hor+ 改动将在重启后生效，其余选项实时生效", g_render_scale);
    v.note = b;
    return v;
}
}  // namespace menu

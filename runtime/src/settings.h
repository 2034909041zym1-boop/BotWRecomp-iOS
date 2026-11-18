// User settings: changed from the in-game menu (host/menu.cpp) and kept in settings.ini in the user
// folder. Command-line flags override them for one run.
#pragma once
#include <TargetConditionals.h>

#include <string>

struct Settings {
#if TARGET_OS_IOS
    int fps = 30;          // 30 or 60 (native 30 fps keeps A18 Pro cool and steady)
    float scale = 1;       // render scale (phones: native 720p); 0 = fit the display; applies at the next launch
    int display = 1;       // 0 fit (letterbox), 1 stretch, 2 fill (crop), 3 wide (hor+, next launch)
#else
    int fps = 60;          // 30 or 60
    float scale = 0;       // render scale; 0 = fit the display (takes effect at the next launch)
    int display = 0;       // 0 fit (letterbox), 1 stretch, 2 fill (crop), 3 wide (hor+, next launch)
#endif
    int filter = 1;        // output scaling: 0 bilinear, 1 bicubic, 2 hermite, 3 nearest
    int aa = 2;            // post-process AA: 0 off, 1 FXAA, 2 FXAA + CAS sharpen
    bool gyro = true;      // controller gyro -> GamePad motion
    bool rumble = true;
    int layout = 0;        // 0 Nintendo positions (A = right face button), 1 labels (A = bottom face button)
    int show_fps = 0;      // 0 off, 1 FPS only, 2 FPS + thermal state
    int volume = 100;      // percent
    int touch = 0;         // on-screen controls: 0 auto (while no controller), 1 on, 2 off
    bool touch_hidden = false;  // collapsed to the show/hide button (that button toggles it)
    int stick_fixed = 1;   // 1 = fixed left stick base, 0 = floating (follows thumb)
    float look_sens = 1.6f;// right-half camera drag sensitivity multiplier
    int look_accel = 1;    // 1 = mild non-linear camera acceleration, 0 = linear
    int touch_alpha = 75;  // virtual button opacity percent (25..100)
    int macro_btns = 1;    // 1 = show quick macro buttons (WB / PR / BT), 0 = hide
    int frame_gen = 0;     // 0 = off, 1 = 2x optical flow frame generation (experimental)
    int shadow_half = 0;   // 0 = native 1024x1024 shadows, 1 = 512x512 half-res shadows (restart)
    int aniso_4x = 0;      // 0 = native (up to 16x), 1 = cap anisotropic filtering at 4x (restart)
    int grass_lod = 0;     // 0 = native, 1 = low grass/tree LOD, 2 = very low grass/tree LOD (live)
    int metalfx = 0;       // 0 = auto (<1x scale), 1 = always on (including 1x -> screen), 2 = off
};

namespace settings {
Settings get();
void set(const Settings& s, bool save = true);  // apply to the running game (and save to settings.ini)
void load(const std::string& path);  // and apply; missing file = defaults
void apply();                         // push the current values into the runtime
}  // namespace settings

// User settings: changed from the in-game menu (host/menu.cpp) and kept in settings.ini in the user
// folder. Command-line flags override them for one run.
#pragma once
#include <TargetConditionals.h>

#include <string>

struct Settings {
#if TARGET_OS_IOS
    int fps = 30;          // 30 or 60 (native 30 fps keeps A18 Pro cool and steady)
    float scale = 1;       // render scale (phones: native 720p); 0 = fit the display; applies at the next launch
    int display = 1;       // 0 fit (letterbox), 1 stretch, 2 fill (crop)
#else
    int fps = 60;          // 30 or 60
    float scale = 0;       // render scale; 0 = fit the display (takes effect at the next launch)
    int display = 0;       // 0 fit (letterbox), 1 stretch, 2 fill (crop)
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
};

namespace settings {
Settings get();
void set(const Settings& s, bool save = true);  // apply to the running game (and save to settings.ini)
void load(const std::string& path);  // and apply; missing file = defaults
void apply();                         // push the current values into the runtime
}  // namespace settings

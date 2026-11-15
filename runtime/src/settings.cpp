#include "settings.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>

#include "audio/output.h"
#include "host/touch.h"
#include "host/window.h"
#include "latte/latte.h"
#include "log.h"
#include "os.h"

namespace settings {
namespace {
std::mutex g_lock;
Settings g_s;
std::string g_path;

void save_locked() {
    if (g_path.empty()) return;
    FILE* f = fopen(g_path.c_str(), "w");
    if (!f) return;
    fprintf(f, "# BotWRecomp settings (also editable from the in-game menu: touchpad, L3+R3 or F1)\n");
    fprintf(f, "fps=%d\nscale=%g\ndisplay=%d\nfilter=%d\naa=%d\ngyro=%d\nrumble=%d\nlayout=%d\nshow_fps=%d\nvolume=%d\ntouch=%d\ntouch_hidden=%d\nstick_fixed=%d\nlook_sens=%g\nlook_accel=%d\ntouch_alpha=%d\nmacro_btns=%d\nframe_gen=%d\n", g_s.fps,
            g_s.scale, g_s.display, g_s.filter, g_s.aa, g_s.gyro, g_s.rumble, g_s.layout, g_s.show_fps, g_s.volume, g_s.touch, g_s.touch_hidden,
            g_s.stick_fixed, g_s.look_sens, g_s.look_accel, g_s.touch_alpha, g_s.macro_btns, g_s.frame_gen);
    fclose(f);
}
}  // namespace

Settings get() {
    std::lock_guard<std::mutex> l(g_lock);
    return g_s;
}

void apply() {
    Settings s = get();
    g_target_fps = s.fps;
    latte::set_display_mode(s.display);
    latte::set_output_filter(s.filter);
    latte::set_aa_mode(s.aa);
    latte::set_frame_gen(s.frame_gen);
    window::set_gyro(s.gyro);
    window::set_rumble(s.rumble);
    window::set_layout(s.layout);
    audio::set_volume(s.volume / 100.0f);
    touch::set_mode(s.touch);
    touch::set_hidden(s.touch_hidden);
    touch::set_stick_fixed(s.stick_fixed != 0);
    touch::set_look_params(s.look_sens, s.look_accel != 0);
    touch::set_alpha(s.touch_alpha / 100.0f);
    touch::set_macro_buttons(s.macro_btns != 0);
}

void set(const Settings& s, bool save) {
    {
        std::lock_guard<std::mutex> l(g_lock);
        g_s = s;
        if (save) save_locked();
    }
    apply();
}

void load(const std::string& path) {
    {
        std::lock_guard<std::mutex> l(g_lock);
        g_path = path;
        if (FILE* f = fopen(path.c_str(), "r")) {
            char line[256];
            while (fgets(line, sizeof line, f)) {
                char* eq = strchr(line, '=');
                if (line[0] == '#' || !eq) continue;
                *eq = 0;
                const char* v = eq + 1;
                if (!strcmp(line, "fps")) g_s.fps = atoi(v) >= 60 ? 60 : 30;
                else if (!strcmp(line, "scale")) g_s.scale = (float)atof(v);
                else if (!strcmp(line, "display")) g_s.display = atoi(v);
                else if (!strcmp(line, "filter")) g_s.filter = atoi(v);
                else if (!strcmp(line, "aa")) g_s.aa = atoi(v);
                else if (!strcmp(line, "gyro")) g_s.gyro = atoi(v) != 0;
                else if (!strcmp(line, "rumble")) g_s.rumble = atoi(v) != 0;
                else if (!strcmp(line, "layout")) g_s.layout = atoi(v);
                else if (!strcmp(line, "show_fps")) g_s.show_fps = std::clamp(atoi(v), 0, 2);
                else if (!strcmp(line, "volume")) g_s.volume = atoi(v);
                else if (!strcmp(line, "touch")) g_s.touch = atoi(v);
                else if (!strcmp(line, "touch_hidden")) g_s.touch_hidden = atoi(v) != 0;
                else if (!strcmp(line, "stick_fixed")) g_s.stick_fixed = atoi(v) != 0 ? 1 : 0;
                else if (!strcmp(line, "look_sens")) g_s.look_sens = std::clamp((float)atof(v), 0.5f, 4.0f);
                else if (!strcmp(line, "look_accel")) g_s.look_accel = atoi(v) != 0 ? 1 : 0;
                else if (!strcmp(line, "touch_alpha")) g_s.touch_alpha = std::clamp(atoi(v), 25, 100);
                else if (!strcmp(line, "macro_btns")) g_s.macro_btns = atoi(v) != 0 ? 1 : 0;
                else if (!strcmp(line, "frame_gen")) g_s.frame_gen = atoi(v) != 0 ? 1 : 0;
            }
            fclose(f);
            LOG("settings: %s", path.c_str());
        }
    }
    apply();
}
}  // namespace settings

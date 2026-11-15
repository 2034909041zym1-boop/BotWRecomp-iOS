// On-screen touch controls (iPhone / iPad, or a touchscreen Mac). Host side (window.mm) feeds SDL finger
// events and merges the virtual pad into the GamePad state; the overlay (cemu_shims/overlay_menu.cpp)
// calls draw() inside its ImGui pass on the GPU thread.
#pragma once
#include <SDL3/SDL_events.h>

#include "../input/input.h"

namespace touch {
void set_mode(int mode);              // 0 auto (shown while no controller is connected), 1 on, 2 off
void set_controller_connected(bool connected);
void set_hidden(bool hidden);          // collapsed: only the show/hide button is shown and live
void set_stick_fixed(bool fixed);
void set_look_params(float sens, bool accel);
void set_alpha(float alpha);
void set_macro_buttons(bool show);
bool active();                        // controls currently shown and live
void event(const SDL_Event& e);       // SDL_EVENT_FINGER_* (host main thread)
void apply(input::PadState& s);       // add virtual buttons / sticks (host main thread, every loop)
void draw();                          // GPU thread, inside the overlay's ImGui frame
}  // namespace touch

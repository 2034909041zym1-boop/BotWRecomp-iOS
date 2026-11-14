// On-screen keyboard for nn_swkbd, drawn by the ImGui overlay pass and driven by the gamepad.
//
// BotW's KeyBoradTextArea_00 only draws the typed text; the keys came from the Wii U system
// keyboard. A UIKit overlay cannot be used here: the emulator owns the main thread, so UIKit never
// commits the layer tree and the keys never received touches. Drawing through the existing ImGui
// overlay (LatteOverlay_render) reuses the Metal pass that already draws the FPS counter and menu.
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Opens the keyboard with the given initial text, clamped to max_len characters.
void swkbd_ui_show(const char* initial, int max_len);
void swkbd_ui_hide(void);

// Feeds one frame of gamepad state; the keyboard consumes the d-pad / stick / A / B / +.
void swkbd_ui_input(uint32_t buttons, float lx, float ly);

// Draws the keyboard; called from LatteOverlay_render on the GPU thread.
void swkbd_ui_render(void);

int swkbd_ui_active(void);
int swkbd_ui_poll(void);              // 1 once the player confirmed
void swkbd_ui_text(char* out, int cap);

// Kept for the guest-side call sites that predate the overlay implementation.
void swkbd_ui_precreate(void);
void swkbd_ui_pump(void);

#ifdef __cplusplus
}
#endif

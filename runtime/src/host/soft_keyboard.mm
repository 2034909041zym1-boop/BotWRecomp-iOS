// Text entry for nn_swkbd via SDL's own iOS text input.
//
// A hand-rolled UITextField never received the on-screen key presses: the emulator owns the main
// thread, so UIKit's event delivery for the keyboard window did not reach it. SDL already implements
// this path (it owns a hidden SDLUITextField and translates keystrokes into SDL_EVENT_TEXT_INPUT),
// and those events arrive through the normal SDL event queue that the game loop already polls.
// BotW draws the answer text itself from the swkbd form, so the guest copy is synced every frame.
#include <TargetConditionals.h>

#if TARGET_OS_IOS

#import <UIKit/UIKit.h>

#include <SDL3/SDL.h>
#include <string.h>

#include "../log.h"

namespace {

constexpr int kMaxText = 127;

char g_text[kMaxText + 1];
uint32_t g_maxLen = 20;
int g_shown = 0;
int g_result = 0;  // 0 none, 1 accepted

}  // namespace

// Kept for the caller's startup hook; SDL already created its text field with the window.
extern "C" void swkbd_ui_precreate(void) {}

extern "C" void swkbd_ui_show(const char* initial, int max_len) {
    if (g_shown) return;
    g_maxLen = max_len > 0 ? (uint32_t)max_len : 20;
    if (g_maxLen > kMaxText) g_maxLen = kMaxText;
    strncpy(g_text, initial ? initial : "", sizeof(g_text) - 1);
    g_text[sizeof(g_text) - 1] = 0;
    g_result = 0;
    g_shown = 1;
    // Starting text input calls becomeFirstResponder, which FBSScene only allows on the main run
    // loop's serial queue, so hand it to the main queue (serviced by the frame pump).
    dispatch_async(dispatch_get_main_queue(), ^{
        SDL_StartTextInput();
        LOG("[swkbd-ui] SDL text input started");
    });
    LOG("[swkbd-ui] show init='%s' max=%u", g_text, g_maxLen);
}

extern "C" void swkbd_ui_hide(void) {
    g_shown = 0;
    dispatch_async(dispatch_get_main_queue(), ^{ SDL_StopTextInput(); });
}

// Called from the SDL event loop for SDL_EVENT_TEXT_INPUT.
extern "C" void swkbd_ui_append(const char* text) {
    if (!g_shown || !text) return;
    size_t n = strlen(g_text);
    for (const unsigned char* p = (const unsigned char*)text; *p; p++) {
        if (*p < 0x20 || *p > 0x7E) {
            LOG("[swkbd-ui] dropped byte 0x%02X", *p);
            continue;
        }
        if (n >= g_maxLen || n + 1 >= sizeof(g_text)) break;
        g_text[n++] = (char)*p;
    }
    g_text[n] = 0;
    LOG("[swkbd-ui] text='%s'", g_text);
}

// Called from the SDL event loop when return is pressed on the keyboard.
extern "C" void swkbd_ui_confirm(void) {
    if (g_shown) g_result = 1;
}

// Lets UIKit deliver the keyboard's touches while the emulator keeps the main thread.
extern "C" void swkbd_ui_pump(void) {
    if (!g_shown) return;
    CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.004, false);
    [CATransaction flush];
}

extern "C" int swkbd_ui_active(void) { return g_shown; }

extern "C" int swkbd_ui_poll(void) { return g_result; }

extern "C" void swkbd_ui_text(char* out, int cap) {
    if (!out || cap <= 0) return;
    strncpy(out, g_text, (size_t)cap - 1);
    out[cap - 1] = 0;
}

#endif  // TARGET_OS_IOS

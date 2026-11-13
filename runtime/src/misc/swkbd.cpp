// nn_swkbd: the Wii U on-screen keyboard. BotW opens it when naming a horse; the iOS port used to
// ship swkbd as an empty header, so the first export lookup came back unimplemented and the guest
// was killed ("FATAL: call to an unimplemented dynamically-loaded export at 03c91bb8").
//
// There is no emulated Wii U keyboard, so the input form is backed by the host: on iOS host/
// text_input.mm presents a native text field, and this file runs the swkbd state machine around it.
// When no host is available (macOS test builds), the form reports Done immediately with a default.
#include <TargetConditionals.h>

#include "../hle.h"

#if TARGET_OS_IOS
extern "C" {
void swkbd_host_begin(const char* initial);
int swkbd_host_poll(char* out, int cap);
void swkbd_host_cancel(void);
}
constexpr bool kHasHostInput = true;
#else
constexpr bool kHasHostInput = false;
static void swkbd_host_begin(const char*) {}
static int swkbd_host_poll(char*, int) { return 0; }
static void swkbd_host_cancel() {}
#endif

namespace {

// swkbd::State
enum : uint32_t { kAppear = 0, kVisible = 1, kDisappear = 2, kDone = 3 };

const uint16_t kDefaultName[] = {'E', 'p', 'o', 'n', 'a', 0};

uint32_t g_state = kDone;
uint32_t g_ticks = 0;
uint32_t g_stringAddr = 0;
bool g_waiting = false;  // waiting for the host text field

// Wide string handed to SwkbdSetInputFormString (UTF-16, already in host order).
uint16_t g_seed[128];
uint32_t g_seedLen = 0;

// Text accepted by the host, again in host order.
uint16_t g_result[128];
uint32_t g_resultLen = 0;

uint32_t utf8_to_utf16(const char* s, uint16_t* out, uint32_t cap) {
    uint32_t n = 0;
    const unsigned char* p = (const unsigned char*)s;
    while (*p && n + 1 < cap) {
        uint32_t cp;
        if (p[0] < 0x80) {
            cp = p[0];
            p += 1;
        } else if ((p[0] & 0xE0) == 0xC0 && p[1]) {
            cp = ((p[0] & 0x1F) << 6) | (p[1] & 0x3F);
            p += 2;
        } else if ((p[0] & 0xF0) == 0xE0 && p[1] && p[2]) {
            cp = ((p[0] & 0x0F) << 12) | ((p[1] & 0x3F) << 6) | (p[2] & 0x3F);
            p += 3;
        } else if ((p[0] & 0xF8) == 0xF0 && p[1] && p[2] && p[3]) {
            cp = ((p[0] & 0x07) << 18) | ((p[1] & 0x3F) << 12) | ((p[2] & 0x3F) << 6) | (p[3] & 0x3F);
            p += 4;
        } else {
            cp = '?';
            p += 1;
        }
        if (cp >= 0x10000) {
            cp -= 0x10000;
            out[n++] = (uint16_t)(0xD800 + (cp >> 10));
            if (n + 1 < cap) out[n++] = (uint16_t)(0xDC00 + (cp & 0x3FF));
        } else {
            out[n++] = (uint16_t)cp;
        }
    }
    out[n] = 0;
    return n;
}

uint32_t utf16_to_utf8(const uint16_t* s, uint32_t n, char* out, uint32_t cap) {
    uint32_t o = 0;
    for (uint32_t i = 0; i < n; i++) {
        uint32_t cp = s[i];
        if (cp >= 0xD800 && cp < 0xDC00 && i + 1 < n) {
            cp = 0x10000 + ((cp - 0xD800) << 10) + (s[++i] - 0xDC00);
        }
        if (cp < 0x80) {
            if (o + 1 >= cap) break;
            out[o++] = (char)cp;
        } else if (cp < 0x800) {
            if (o + 2 >= cap) break;
            out[o++] = (char)(0xC0 | (cp >> 6));
            out[o++] = (char)(0x80 | (cp & 0x3F));
        } else if (cp < 0x10000) {
            if (o + 3 >= cap) break;
            out[o++] = (char)(0xE0 | (cp >> 12));
            out[o++] = (char)(0x80 | ((cp >> 6) & 0x3F));
            out[o++] = (char)(0x80 | (cp & 0x3F));
        } else {
            if (o + 4 >= cap) break;
            out[o++] = (char)(0xF0 | (cp >> 18));
            out[o++] = (char)(0x80 | ((cp >> 12) & 0x3F));
            out[o++] = (char)(0x80 | ((cp >> 6) & 0x3F));
            out[o++] = (char)(0x80 | (cp & 0x3F));
        }
    }
    if (cap) out[o < cap ? o : cap - 1] = 0;
    return o;
}

void write_result() {
    if (!g_stringAddr) g_stringAddr = mem::sys_alloc(256, 2);
    const uint16_t* src = g_resultLen ? g_result : kDefaultName;
    const uint32_t n = g_resultLen ? g_resultLen : 5;
    for (uint32_t i = 0; i < n && i < 127; i++) mem::w16(g_stringAddr + i * 2, src[i]);
    mem::w16(g_stringAddr + n * 2, 0);
}

// Advance the state machine. While the host text field is up this only polls it; once the player
// accepts or cancels, the form closes and the game sees Done a couple of frames later.
void tick() {
    if (g_waiting) {
        char buf[256];
        int r = swkbd_host_poll(buf, (int)sizeof(buf));
        if (r == 1) {
            g_resultLen = utf8_to_utf16(buf, g_result, 127);
            g_waiting = false;
            g_state = kDisappear;
            g_ticks = 0;
        } else if (r == 2) {
            g_resultLen = 0;
            g_waiting = false;
            g_state = kDisappear;
            g_ticks = 0;
        }
        return;
    }
    g_ticks++;
    if (g_state == kAppear && g_ticks >= 3) {
        g_state = kVisible;
        g_ticks = 0;
    } else if (g_state == kVisible && g_ticks >= 24) {
        g_state = kDisappear;
        g_ticks = 0;
    } else if (g_state == kDisappear && g_ticks >= 2) {
        g_state = kDone;
        g_ticks = 0;
    }
}

void open_form() {
    g_resultLen = 0;
    g_ticks = 0;
    if (kHasHostInput) {
        char initial[256];
        utf16_to_utf8(g_seed, g_seedLen, initial, sizeof(initial));
        g_state = kVisible;
        g_waiting = true;
        swkbd_host_begin(initial);
    } else {
        // No host keyboard: behave like the previous headless shim and finish on its own.
        g_state = kAppear;
        g_waiting = false;
    }
}

void reset() {
    g_state = kDone;
    g_ticks = 0;
    g_waiting = false;
    g_seedLen = 0;
    g_resultLen = 0;
    swkbd_host_cancel();
}

}  // namespace

// ---- lifecycle ------------------------------------------------------------------------------------
HLE_EXPORT(swkbd, SwkbdCreate__3RplFPUcQ3_2nn5swkbd10RegionTypeUiP8FSClient) { reset(); }
HLE_EXPORT(swkbd, SwkbdDestroy__3RplFv) { reset(); }
HLE_EXPORT(swkbd, SwkbdSetVersion__3RplFi) {}

// ---- showing / hiding ------------------------------------------------------------------------------
HLE_EXPORT(swkbd, SwkbdAppearInputForm__3RplFRCQ3_2nn5swkbd9AppearArg) {
    open_form();
    RET(1);
}
HLE_EXPORT(swkbd, SwkbdAppearKeyboard__3RplFRCQ3_2nn5swkbd11KeyboardArg) {
    open_form();
    RET(1);
}
HLE_EXPORT(swkbd, SwkbdDisappearInputForm__3RplFv) {
    swkbd_host_cancel();
    g_waiting = false;
    g_state = kDisappear;
    g_ticks = 0;
}
HLE_EXPORT(swkbd, SwkbdDisappearKeyboard__3RplFv) {
    swkbd_host_cancel();
    g_waiting = false;
    g_state = kDisappear;
    g_ticks = 0;
}

// ---- state / result --------------------------------------------------------------------------------
HLE_EXPORT(swkbd, SwkbdCalc__3RplFRCQ3_2nn5swkbd14ControllerInfo) { tick(); }
HLE_EXPORT(swkbd, SwkbdGetStateInputForm__3RplFv) {
    tick();
    RET(g_state);
}
HLE_EXPORT(swkbd, SwkbdGetStateKeyboard__3RplFv) {
    tick();
    RET(g_state);
}
HLE_EXPORT(swkbd, SwkbdGetInputFormString__3RplFv) {
    write_result();
    RET(g_stringAddr);
}
HLE_EXPORT(swkbd, SwkbdSetInputFormString__3RplFPCw) {
    uint32_t src = ARG(0);
    g_seedLen = 0;
    if (!src) return;
    for (uint32_t i = 0; i < 127; i++) {
        uint16_t ch = mem::r16(src + i * 2);
        g_seed[i] = ch;
        if (!ch) return;
        g_seedLen = i + 1;
    }
}

// ---- queries the game polls while the form is open -------------------------------------------------
HLE_EXPORT(swkbd, SwkbdIsNeedCalcSubThreadFont__3RplFv) { RET(0); }
HLE_EXPORT(swkbd, SwkbdIsNeedCalcSubThreadPredict__3RplFv) { RET(0); }
HLE_EXPORT(swkbd, SwkbdCalcSubThreadFont__3RplFv) {}
HLE_EXPORT(swkbd, SwkbdCalcSubThreadPredict__3RplFv) {}
HLE_EXPORT(swkbd, SwkbdIsSelectCursorActive__3RplFv) { RET(0); }
HLE_EXPORT(swkbd, SwkbdIsCoveredWithSubWindow__3RplFv) { RET(0); }
HLE_EXPORT(swkbd, SwkbdIsKeyboardTarget__3RplFPQ3_2nn5swkbd14IEventReceiver) { RET(0); }
HLE_EXPORT(swkbd, SwkbdIsDecideOkButton__3RplFPb) {
    if (ARG(0)) mem::w8(ARG(0), 0);
    RET(0);
}
HLE_EXPORT(swkbd, SwkbdIsDecideCancelButton__3RplFPb) {
    if (ARG(0)) mem::w8(ARG(0), 0);
    RET(0);
}
HLE_EXPORT(swkbd, SwkbdGetKeyboardCondition__3RplFPQ3_2nn5swkbd17KeyboardCondition) {
    if (ARG(0)) memset(mem::host(ARG(0)), 0, 0x20);
}
HLE_EXPORT(swkbd, SwkbdGetDrawStringInfo__3RplFPQ3_2nn5swkbd14DrawStringInfo) {
    if (ARG(0)) memset(mem::host(ARG(0)), 0, 0x40);
}

// ---- drawing (the host draws its own form) ---------------------------------------------------------
HLE_EXPORT(swkbd, SwkbdDrawTV__3RplFv) {}
HLE_EXPORT(swkbd, SwkbdDrawDRC__3RplFv) {}

// ---- setters ---------------------------------------------------------------------------------------
HLE_EXPORT(swkbd, SwkbdConfirmUnfixAll__3RplFv) {}
HLE_EXPORT(swkbd, SwkbdInactivateSelectCursor__3RplFv) {}
HLE_EXPORT(swkbd, SwkbdInitLearnDic__3RplFPv) {}
HLE_EXPORT(swkbd, SwkbdMuteAllSound__3RplFb) {}
HLE_EXPORT(swkbd, SwkbdSetControllerRemo__3RplFQ3_2nn5swkbd14ControllerType) {}
HLE_EXPORT(swkbd, SwkbdSetCursorPos__3RplFi) {}
HLE_EXPORT(swkbd, SwkbdSetEnableOkButton__3RplFb) {}
HLE_EXPORT(swkbd, SwkbdSetReceiver__3RplFRCQ3_2nn5swkbd11ReceiverArg) {}
HLE_EXPORT(swkbd, SwkbdSetSelectFrom__3RplFi) {}
HLE_EXPORT(swkbd, SwkbdSetUserControllerEventObj__3RplFPQ3_2nn5swkbd19IControllerEventObj) {}
HLE_EXPORT(swkbd, SwkbdSetUserSoundObj__3RplFPQ3_2nn5swkbd9ISoundObj) {}

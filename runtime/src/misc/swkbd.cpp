// nn_swkbd: the Wii U software keyboard. BotW opens it when naming a horse; the iOS port used to
// ship swkbd as an empty header, so the first export lookup came back unimplemented and the guest
// was killed ("FATAL: call to an unimplemented dynamically-loaded export at 03c91bb8").
//
// State handling mirrors Cemu (src/Cafe/OS/libs/swkbd/swkbd.cpp): the form reports DISPLAYED while
// active and BLANK once the game closes it, and SwkbdCalc is a no-op. The keys are drawn by
// host/soft_keyboard.mm as an overlay on the game surface, since BotW only draws the answer text.
#include <TargetConditionals.h>

#include "../hle.h"

#if TARGET_OS_IOS
extern "C" {
void swkbd_ui_show(const char* initial, int max_len);
void swkbd_ui_hide(void);
int swkbd_ui_poll(void);
void swkbd_ui_text(char* out, int cap);
}
constexpr bool kHasHostKeyboard = true;
#else
constexpr bool kHasHostKeyboard = false;
static void swkbd_ui_show(const char*, int) {}
static void swkbd_ui_hide() {}
static int swkbd_ui_poll() { return 1; }  // no host keyboard: accept straight away
static void swkbd_ui_text(char* out, int cap) {
    if (cap > 0) out[0] = 0;
}
#endif

namespace {

// swkbd::State as used by Cemu: only "blank" and "displayed" are ever reported.
constexpr uint32_t kStateBlank = 0;
constexpr uint32_t kStateDisplayed = 2;

constexpr uint32_t kMaxChars = 4096;

const uint16_t kDefaultName[] = {'E', 'p', 'o', 'n', 'a', 0};

bool g_active = false;
bool g_decide = false;

// Current text, host order.
uint16_t g_form[kMaxChars];
uint32_t g_formLen = 0;
uint32_t g_maxLen = kMaxChars - 1;

// Big-endian copy handed back to the game.
uint32_t g_stringAddr = 0;

void ensure_string_buffer() {
    if (!g_stringAddr) g_stringAddr = mem::sys_alloc(kMaxChars * 2, 2);
}

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

void set_text_from_guest(uint32_t src) {
    g_formLen = 0;
    if (!src) {
        g_form[0] = 0;
        return;
    }
    for (uint32_t i = 0; i < g_maxLen; i++) {
        uint16_t ch = mem::r16(src + i * 2);
        g_form[i] = ch;
        if (!ch) break;
        g_formLen = i + 1;
    }
    if (g_formLen == 0) g_form[0] = 0;
}

// Pull the text the player typed on the host keyboard and treat accept/cancel as "decided" so the
// game always moves on.
void poll_host_keyboard() {
    if (!kHasHostKeyboard || !g_active) return;
    const int r = swkbd_ui_poll();
    if (r == 1 || r == 2) {
        char buf[512];
        swkbd_ui_text(buf, (int)sizeof(buf));
        g_formLen = utf8_to_utf16(buf, g_form, g_maxLen);
        if (g_formLen == 0) {
            for (uint32_t i = 0; i < 5; i++) g_form[i] = kDefaultName[i];
            g_formLen = 5;
        }
        g_decide = true;
    }
}

void reset() {
    swkbd_ui_hide();
    g_active = false;
    g_decide = false;
    g_formLen = 0;
    g_form[0] = 0;
    g_maxLen = kMaxChars - 1;
}

}  // namespace

// ---- lifecycle ------------------------------------------------------------------------------------
HLE_EXPORT(swkbd, SwkbdCreate__3RplFPUcQ3_2nn5swkbd10RegionTypeUiP8FSClient) { reset(); }
HLE_EXPORT(swkbd, SwkbdDestroy__3RplFv) { reset(); }
HLE_EXPORT(swkbd, SwkbdSetVersion__3RplFi) {}

// ---- showing / hiding ------------------------------------------------------------------------------
// swkbdAppearArg: +0xC8 initialText (utf16be*), +0xD0 maxTextLength
HLE_EXPORT(swkbd, SwkbdAppearInputForm__3RplFRCQ3_2nn5swkbd9AppearArg) {
    const uint32_t arg = ARG(0);
    g_decide = false;
    ensure_string_buffer();

    uint32_t maxLen = arg ? mem::r32(arg + 0xD0) : 0;
    if (maxLen == 0 || maxLen > kMaxChars - 1) maxLen = kMaxChars - 1;
    g_maxLen = maxLen;

    set_text_from_guest(arg ? mem::r32(arg + 0xC8) : 0);

    char initial[512];
    utf16_to_utf8(g_form, g_formLen, initial, sizeof(initial));
    g_active = true;
    swkbd_ui_show(initial, (int)(g_maxLen > 20 ? 20 : g_maxLen));
    RET(1);
}
HLE_EXPORT(swkbd, SwkbdAppearKeyboard__3RplFRCQ3_2nn5swkbd11KeyboardArg) {
    g_decide = false;
    ensure_string_buffer();
    g_formLen = 0;
    g_form[0] = 0;
    g_active = true;
    swkbd_ui_show("", (int)(g_maxLen > 20 ? 20 : g_maxLen));
    RET(1);
}
HLE_EXPORT(swkbd, SwkbdDisappearInputForm__3RplFv) {
    swkbd_ui_hide();
    g_active = false;
    RET(1);
}
HLE_EXPORT(swkbd, SwkbdDisappearKeyboard__3RplFv) {
    swkbd_ui_hide();
    g_active = false;
    RET(1);
}

// ---- state / result --------------------------------------------------------------------------------
HLE_EXPORT(swkbd, SwkbdCalc__3RplFRCQ3_2nn5swkbd14ControllerInfo) { poll_host_keyboard(); }
HLE_EXPORT(swkbd, SwkbdGetStateInputForm__3RplFv) {
    poll_host_keyboard();
    RET(g_active ? kStateDisplayed : kStateBlank);
}
HLE_EXPORT(swkbd, SwkbdGetStateKeyboard__3RplFv) {
    poll_host_keyboard();
    RET(g_active ? kStateDisplayed : kStateBlank);
}
HLE_EXPORT(swkbd, SwkbdGetInputFormString__3RplFv) {
    // Keep the game's copy current so its own answer field shows what is being typed.
    if (kHasHostKeyboard && g_active) {
        char buf[512];
        swkbd_ui_text(buf, (int)sizeof(buf));
        if (buf[0]) g_formLen = utf8_to_utf16(buf, g_form, g_maxLen);
        else g_formLen = 0;
    }
    ensure_string_buffer();
    for (uint32_t i = 0; i < g_formLen; i++) mem::w16(g_stringAddr + i * 2, g_form[i]);
    mem::w16(g_stringAddr + g_formLen * 2, 0);
    RET(g_stringAddr);
}
HLE_EXPORT(swkbd, SwkbdSetInputFormString__3RplFPCw) { set_text_from_guest(ARG(0)); }
HLE_EXPORT(swkbd, SwkbdIsDecideOkButton__3RplFPb) { RET(g_decide ? 1 : 0); }
HLE_EXPORT(swkbd, SwkbdIsDecideCancelButton__3RplFPb) { RET(0); }

// ---- queries the game polls while the form is open -------------------------------------------------
HLE_EXPORT(swkbd, SwkbdIsNeedCalcSubThreadFont__3RplFv) { RET(0); }
HLE_EXPORT(swkbd, SwkbdIsNeedCalcSubThreadPredict__3RplFv) { RET(0); }
HLE_EXPORT(swkbd, SwkbdCalcSubThreadFont__3RplFv) {}
HLE_EXPORT(swkbd, SwkbdCalcSubThreadPredict__3RplFv) {}
HLE_EXPORT(swkbd, SwkbdIsSelectCursorActive__3RplFv) { RET(0); }
HLE_EXPORT(swkbd, SwkbdIsCoveredWithSubWindow__3RplFv) { RET(0); }
HLE_EXPORT(swkbd, SwkbdIsKeyboardTarget__3RplFPQ3_2nn5swkbd14IEventReceiver) { RET(0); }
HLE_EXPORT(swkbd, SwkbdGetKeyboardCondition__3RplFPQ3_2nn5swkbd17KeyboardCondition) {
    if (ARG(0)) memset(mem::host(ARG(0)), 0, 0x20);
}
// Cemu fills this with -1 and the game still works, so mirror that.
HLE_EXPORT(swkbd, SwkbdGetDrawStringInfo__3RplFPQ3_2nn5swkbd14DrawStringInfo) {
    const uint32_t p = ARG(0);
    if (p) {
        for (int i = 0; i < 6; i++) mem::w32(p + i * 4, 0xFFFFFFFF);
        mem::w8(p + 0x18, 0);
    }
    RET(0);
}

// ---- drawing (the keys live in the host overlay) -----------------------------------------------------
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

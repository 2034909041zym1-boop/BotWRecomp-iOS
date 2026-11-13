// nn_swkbd: the Wii U on-screen keyboard. BotW opens it when naming a horse; the iOS port used to
// ship swkbd as an empty header, so the first export lookup came back unimplemented and the guest
// was killed ("FATAL: call to an unimplemented dynamically-loaded export at 03c91bb8").
//
// This is a headless implementation: the input form appears, runs its state machine for a few
// frames, then reports Done with whatever SwkbdSetInputFormString supplied - or a default name when
// the game opened it empty. The naming flow therefore completes instead of crashing.
#include "../hle.h"

namespace {

// swkbd::State
enum : uint32_t { kAppear = 0, kVisible = 1, kDisappear = 2, kDone = 3 };

const uint16_t kDefaultName[] = {'E', 'p', 'o', 'n', 'a', 0};

uint32_t g_state = kDone;
uint32_t g_ticks = 0;
uint32_t g_stringAddr = 0;

// Wide string handed to SwkbdSetInputFormString (UTF-16, already in host order).
uint16_t g_seed[128];
uint32_t g_seedLen = 0;

// Long enough for one frame of "open, accept, close"; both SwkbdCalc and SwkbdGetState advance it
// so the flow still finishes even if the game only polls the state.
void tick() {
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

void write_result() {
    if (!g_stringAddr) g_stringAddr = mem::sys_alloc(256, 2);
    const uint16_t* src = g_seedLen ? g_seed : kDefaultName;
    const uint32_t n = g_seedLen ? g_seedLen : 5;
    for (uint32_t i = 0; i < n && i < 127; i++) mem::w16(g_stringAddr + i * 2, src[i]);
    mem::w16(g_stringAddr + n * 2, 0);
}

void reset() {
    g_state = kDone;
    g_ticks = 0;
    g_seedLen = 0;
}

}  // namespace

// ---- lifecycle ------------------------------------------------------------------------------------
HLE_EXPORT(swkbd, SwkbdCreate__3RplFPUcQ3_2nn5swkbd10RegionTypeUiP8FSClient) { reset(); }
HLE_EXPORT(swkbd, SwkbdDestroy__3RplFv) { reset(); }
HLE_EXPORT(swkbd, SwkbdSetVersion__3RplFi) {}

// ---- showing / hiding ------------------------------------------------------------------------------
HLE_EXPORT(swkbd, SwkbdAppearInputForm__3RplFRCQ3_2nn5swkbd9AppearArg) {
    g_state = kAppear;
    g_ticks = 0;
    RET(1);
}
HLE_EXPORT(swkbd, SwkbdAppearKeyboard__3RplFRCQ3_2nn5swkbd11KeyboardArg) {
    g_state = kAppear;
    g_ticks = 0;
    RET(1);
}
HLE_EXPORT(swkbd, SwkbdDisappearInputForm__3RplFv) {
    g_state = kDisappear;
    g_ticks = 0;
}
HLE_EXPORT(swkbd, SwkbdDisappearKeyboard__3RplFv) {
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

// ---- drawing (nothing to draw headless) ------------------------------------------------------------
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

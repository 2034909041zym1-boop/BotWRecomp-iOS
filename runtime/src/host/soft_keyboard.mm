// Text entry for nn_swkbd using the iOS system keyboard.
//
// A custom-drawn key panel never worked in this port: the emulator owns the main thread, so UIKit
// never committed the layer tree and touches were never delivered (the panel existed, was unhidden
// and log-confirmed, yet stayed invisible). The system keyboard is drawn by the keyboard process and
// only needs a first responder, so a hidden UITextField is enough. BotW's KeyBoradTextArea_00 draws
// the answer text itself from the swkbd form, so the player sees what they typed inside the game.
#include <TargetConditionals.h>

#if TARGET_OS_IOS

#import <UIKit/UIKit.h>

#include <string.h>

#include "../log.h"

extern void* botw_ui_root_view();

namespace {

constexpr int kMaxText = 127;

char g_text[kMaxText + 1];
uint32_t g_maxLen = 20;
int g_shown = 0;
int g_result = 0;  // 0 none, 1 accepted

UITextField* g_field = nil;

void copy_from_field() {
    if (!g_field) return;
    const char* s = g_field.text.UTF8String;
    strncpy(g_text, s ? s : "", sizeof(g_text) - 1);
    g_text[sizeof(g_text) - 1] = 0;
}

}  // namespace

// The Wii U keyboard only offers printable ASCII. Rejecting anything else here keeps unencodable
// characters out of the guest entirely, which is what used to upset the game's text renderer.
@interface SwkbdField : UITextField <UITextFieldDelegate>
@end

@implementation SwkbdField

- (BOOL)textField:(UITextField*)field
    shouldChangeCharactersInRange:(NSRange)range
                replacementString:(NSString*)string {
    for (NSUInteger i = 0; i < string.length; i++) {
        const unichar c = [string characterAtIndex:i];
        if (c < 0x20 || c > 0x7E) {
            LOG("[swkbd-ui] rejected character U+%04X", (unsigned)c);
            return NO;
        }
    }
    const NSUInteger length = field.text.length - range.length + string.length;
    return length <= g_maxLen;
}

- (BOOL)textFieldShouldReturn:(UITextField*)field {
    [field resignFirstResponder];
    g_result = 1;
    return YES;
}

@end

extern "C" void swkbd_ui_precreate(void) {
    if (g_field) return;
    UIView* host = (__bridge UIView*)botw_ui_root_view();
    if (!host || host.bounds.size.width <= 0) {
        LOG("[swkbd-ui] precreate deferred (host=%p)", (__bridge void*)host);
        return;
    }
    SwkbdField* field = [[SwkbdField alloc] initWithFrame:CGRectMake(0, 0, 1, 1)];
    field.delegate = field;
    field.alpha = 0.01;  // must stay visible enough to become first responder
    field.autocorrectionType = UITextAutocorrectionTypeNo;
    field.autocapitalizationType = UITextAutocapitalizationTypeNone;
    field.spellCheckingType = UITextSpellCheckingTypeNo;
    field.returnKeyType = UIReturnKeyDone;
    // The Wii U keyboard is ASCII-only, and a CJK IME would leave the text empty while composing
    // pinyin, so ask for an ASCII keyboard outright.
    field.keyboardType = UIKeyboardTypeASCIICapable;
    field.textColor = [UIColor clearColor];
    [field addAction:[UIAction actionWithHandler:^(UIAction*) { copy_from_field(); }]
        forControlEvents:UIControlEventEditingChanged];
    [host addSubview:field];
    g_field = field;
    LOG("[swkbd-ui] field ready %.0fx%.0f", host.bounds.size.width, host.bounds.size.height);
}

extern "C" void swkbd_ui_show(const char* initial, int max_len) {
    if (g_shown) return;
    g_maxLen = max_len > 0 ? (uint32_t)max_len : 20;
    if (g_maxLen > kMaxText) g_maxLen = kMaxText;

    if (!g_field) swkbd_ui_precreate();
    if (!g_field) return;

    g_field.text = [NSString stringWithUTF8String:initial ? initial : ""];
    copy_from_field();
    g_result = 0;
    g_shown = 1;
    // becomeFirstResponder goes through FBSScene, which asserts that it runs on the main run loop's
    // serial queue. Calling it straight from the guest thread traps (SIGTRAP in
    // FBSMainRunLoopSerialQueue assertBarrierOnQueue), so hand it to the main queue; the frame pump
    // services it a frame later.
    dispatch_async(dispatch_get_main_queue(), ^{
        if (g_shown && g_field) {
            [g_field becomeFirstResponder];
            LOG("[swkbd-ui] keyboard up responder=%d", (int)g_field.isFirstResponder);
        }
    });
    LOG("[swkbd-ui] show init='%s' max=%u", g_text, g_maxLen);
}

extern "C" void swkbd_ui_hide(void) {
    g_shown = 0;
    dispatch_async(dispatch_get_main_queue(), ^{
        if (g_field && g_field.isFirstResponder) [g_field resignFirstResponder];
    });
}

// Lets UIKit deliver typing and keyboard events while the emulator keeps the main thread. The
// timeout must be non-zero: with 0 the run loop returns before any asynchronously posted touch
// event arrives, which is why key presses did nothing.
extern "C" void swkbd_ui_pump(void) {
    if (!g_shown) return;
    CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.003, false);
    [CATransaction flush];
}

extern "C" int swkbd_ui_active(void) { return g_shown; }

extern "C" int swkbd_ui_poll(void) { return g_result; }

extern "C" void swkbd_ui_text(char* out, int cap) {
    if (!out || cap <= 0) return;
    copy_from_field();  // read the field directly so a missed editingChanged cannot lose keystrokes
    strncpy(out, g_text, (size_t)cap - 1);
    out[cap - 1] = 0;
}

#endif  // TARGET_OS_IOS

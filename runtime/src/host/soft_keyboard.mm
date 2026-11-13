// On-screen keyboard for nn_swkbd, drawn as a plain overlay view on top of the SDL surface.
//
// BotW's KeyBoradTextArea_00 only draws the answer text and the cursor; the keys themselves come
// from the Wii U system keyboard (Cemu draws them with ImGui). This port has no ImGui overlay, and
// presenting a UIAlertController from the emulator main loop crashed, so this is a self-drawn
// button grid added as a subview - no system keyboard, no modal presentation.
#include <TargetConditionals.h>

#if TARGET_OS_IOS

#import <UIKit/UIKit.h>

#include <string.h>

namespace {

constexpr int kMaxText = 127;

char g_text[kMaxText + 1];
uint32_t g_maxLen = 20;
int g_shown = 0;
int g_result = 0;  // 0 none, 1 accepted, 2 cancelled

UIView* g_panel = nil;
UILabel* g_field = nil;

UIViewController* root_view_controller() {
    UIWindow* key = nil;
    for (UIWindow* w in UIApplication.sharedApplication.windows) {
        if (w.isKeyWindow) {
            key = w;
            break;
        }
    }
    if (!key) key = UIApplication.sharedApplication.windows.firstObject;
    UIViewController* vc = key.rootViewController;
    while (vc.presentedViewController) vc = vc.presentedViewController;
    return vc;
}

void refresh_label() {
    if (g_field) g_field.text = [NSString stringWithUTF8String:g_text];
}

void append_char(char ch) {
    size_t n = strlen(g_text);
    if (n >= g_maxLen || n + 1 >= sizeof(g_text)) return;
    g_text[n] = ch;
    g_text[n + 1] = 0;
    refresh_label();
}

void backspace() {
    size_t n = strlen(g_text);
    if (!n) return;
    g_text[n - 1] = 0;
    refresh_label();
}

void close_panel() {
    if (g_panel) {
        [g_panel removeFromSuperview];
        g_panel = nil;
        g_field = nil;
    }
    g_shown = 0;
}

}  // namespace

extern "C" void swkbd_ui_show(const char* initial, int max_len) {
    if (g_shown) return;
    g_maxLen = max_len > 0 ? (uint32_t)max_len : 20;
    if (g_maxLen > kMaxText) g_maxLen = kMaxText;
    strncpy(g_text, initial ? initial : "", sizeof(g_text) - 1);
    g_text[sizeof(g_text) - 1] = 0;
    g_result = 0;

    UIViewController* vc = root_view_controller();
    if (!vc) return;
    UIView* host = vc.view;
    const CGRect bounds = host.bounds;
    const CGFloat w = bounds.size.width;
    const CGFloat h = bounds.size.height;
    if (w <= 0 || h <= 0) return;

    UIView* panel = [[UIView alloc] initWithFrame:CGRectMake(0, h * 0.30, w, h * 0.70)];
    panel.backgroundColor = [UIColor colorWithWhite:0.0 alpha:0.88];
    panel.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleTopMargin;

    const CGFloat pad = 8;
    const CGFloat fieldH = 40;
    const CGFloat rowH = (panel.bounds.size.height - fieldH - pad * 6) / 4;

    CGFloat y = pad;
    UILabel* field = [[UILabel alloc] initWithFrame:CGRectMake(24, y, w - 48, fieldH)];
    field.textColor = [UIColor whiteColor];
    field.backgroundColor = [UIColor colorWithWhite:1.0 alpha:0.12];
    field.font = [UIFont systemFontOfSize:26];
    field.textAlignment = NSTextAlignmentLeft;
    field.text = [NSString stringWithUTF8String:g_text];
    [panel addSubview:field];
    g_field = field;
    y += fieldH + pad;

    // Four rows of keys: a-z, then space / delete / cancel / ok. Send actions are added inline so
    // no selector table is needed.
    const char* rows[] = {"abcdefghij", "klmnopqrst", "uvwxyz"};
    for (int r = 0; r < 3; r++) {
        const char* keys = rows[r];
        const size_t count = strlen(keys);
        const CGFloat bw = (w - pad * (count + 1)) / count;
        for (size_t i = 0; i < count; i++) {
            char label[2] = {keys[i], 0};
            const char ch = keys[i];
            UIButton* b = [UIButton buttonWithType:UIButtonTypeSystem];
            b.frame = CGRectMake(pad + i * (bw + pad), y, bw, rowH);
            [b setTitle:[NSString stringWithUTF8String:label] forState:UIControlStateNormal];
            b.titleLabel.font = [UIFont systemFontOfSize:24];
            b.backgroundColor = [UIColor colorWithWhite:1.0 alpha:0.18];
            [b addAction:[UIAction actionWithHandler:^(UIAction*) { append_char(ch); }]
                forControlEvents:UIControlEventTouchUpInside];
            [panel addSubview:b];
        }
        y += rowH + pad;
    }

    const CGFloat bw = (w - pad * 5) / 4;
    struct { const char* title; int kind; } actions[4] = {
        {"空格", 0}, {"删除", 1}, {"取消", 2}, {"确定", 3}};
    for (int i = 0; i < 4; i++) {
        const int kind = actions[i].kind;
        UIButton* b = [UIButton buttonWithType:UIButtonTypeSystem];
        b.frame = CGRectMake(pad + i * (bw + pad), y, bw, rowH);
        [b setTitle:[NSString stringWithUTF8String:actions[i].title] forState:UIControlStateNormal];
        b.titleLabel.font = [UIFont systemFontOfSize:22];
        b.backgroundColor = [UIColor colorWithWhite:1.0 alpha:0.18];
        [b addAction:[UIAction actionWithHandler:^(UIAction*) {
            if (kind == 0) append_char(' ');
            else if (kind == 1) backspace();
            else if (kind == 2) { g_result = 2; close_panel(); }
            else { g_result = 1; close_panel(); }
        }] forControlEvents:UIControlEventTouchUpInside];
        [panel addSubview:b];
    }

    [host addSubview:panel];
    g_panel = panel;
    g_shown = 1;
}

extern "C" void swkbd_ui_hide(void) { close_panel(); }

// 0 = still open, 1 = accepted, 2 = cancelled.
extern "C" int swkbd_ui_poll(void) { return g_result; }

extern "C" void swkbd_ui_text(char* out, int cap) {
    if (!out || cap <= 0) return;
    strncpy(out, g_text, (size_t)cap - 1);
    out[cap - 1] = 0;
}

#endif  // TARGET_OS_IOS

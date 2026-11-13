// Host text entry for nn_swkbd. The Wii U software keyboard is not emulated, so when the game opens
// its input form we present a native alert with a text field and hand the result back to swkbd.cpp.
//
// Everything here runs on the main thread (the emulator loop calls SDL_PollEvent, which drives the
// UIKit run loop), so the alert can be presented and dismissed directly.
#include <TargetConditionals.h>

#if TARGET_OS_IOS

#import <UIKit/UIKit.h>

#include <string.h>

namespace {

char g_text[256];
int g_state = 0;  // 0 idle, 1 pending, 2 accepted, 3 cancelled
UIAlertController* g_alert = nil;

UIViewController* top_view_controller() {
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

}  // namespace

extern "C" void swkbd_host_begin(const char* initial) {
    if (g_state == 1) return;  // already showing
    g_text[0] = 0;
    g_state = 1;

    UIViewController* vc = top_view_controller();
    if (!vc || vc.presentedViewController) {
        g_state = 0;
        return;
    }

    NSString* start = [NSString stringWithUTF8String:initial ? initial : ""];
    UIAlertController* alert =
        [UIAlertController alertControllerWithTitle:@"输入名字" message:nil
                                     preferredStyle:UIAlertControllerStyleAlert];
    __weak UIAlertController* weakAlert = alert;
    [alert addTextFieldWithConfigurationHandler:^(UITextField* tf) {
        tf.text = start;
        tf.autocorrectionType = UITextAutocorrectionTypeNo;
        tf.autocapitalizationType = UITextAutocapitalizationTypeNone;
        tf.clearButtonMode = UITextFieldViewModeWhileEditing;
    }];
    [alert addAction:[UIAlertAction actionWithTitle:@"确定"
                                              style:UIAlertActionStyleDefault
                                            handler:^(UIAlertAction*) {
        UITextField* tf = weakAlert.textFields.firstObject;
        const char* s = tf.text.UTF8String;
        strncpy(g_text, s ? s : "", sizeof(g_text) - 1);
        g_text[sizeof(g_text) - 1] = 0;
        g_alert = nil;
        g_state = 2;
    }]];
    [alert addAction:[UIAlertAction actionWithTitle:@"取消"
                                              style:UIAlertActionStyleCancel
                                            handler:^(UIAlertAction*) {
        g_text[0] = 0;
        g_alert = nil;
        g_state = 3;
    }]];

    g_alert = alert;
    [vc presentViewController:alert animated:YES completion:nil];
}

// 0 = still waiting, 1 = accepted (copies the text), 2 = cancelled.
extern "C" int swkbd_host_poll(char* out, int cap) {
    if (g_state == 2) {
        if (out && cap > 0) {
            strncpy(out, g_text, (size_t)cap - 1);
            out[cap - 1] = 0;
        }
        g_state = 0;
        return 1;
    }
    if (g_state == 3) {
        g_state = 0;
        return 2;
    }
    return 0;
}

extern "C" void swkbd_host_cancel(void) {
    if (g_alert) {
        [g_alert dismissViewControllerAnimated:NO completion:nil];
        g_alert = nil;
    }
    g_state = 0;
}

#endif  // TARGET_OS_IOS

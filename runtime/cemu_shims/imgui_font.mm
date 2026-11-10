#include <imgui.h>
#include "imgui/imgui_extension.h"
#include <unistd.h>

#if defined(__APPLE__)
#import <Foundation/Foundation.h>
#import <CoreText/CoreText.h>
#include <TargetConditionals.h>
#endif

void ImGui_LoadChineseFont(ImFontAtlas* atlas) {
    // Add default font first so there is always a base font
    atlas->AddFontDefault();

#if defined(__APPLE__)
    @autoreleasepool {
        NSArray<NSString*>* fontNames = @[
            @"PingFangSC-Regular",
            @".PingFangSC-Regular",
            @".PingFang-SC-Regular",
            @"STHeitiSC-Light"
        ];
        for (NSString* fontName in fontNames) {
            CTFontRef ctFont = CTFontCreateWithName((__bridge CFStringRef)fontName, 15.0, NULL);
            if (!ctFont) continue;
            CTFontDescriptorRef desc = CTFontCopyFontDescriptor(ctFont);
            if (desc) {
                CFURLRef url = (CFURLRef)CTFontDescriptorCopyAttribute(desc, kCTFontURLAttribute);
                if (url) {
                    NSString* path = [(__bridge NSURL*)url path];
                    if (path && access([path UTF8String], R_OK) == 0) {
                        ImFontConfig cfg;
                        cfg.FontNo = 0;
                        cfg.MergeMode = true;
                        cfg.OversampleH = 1;
                        cfg.OversampleV = 1;
                        atlas->AddFontFromFileTTF([path UTF8String], 15.0f, &cfg, atlas->GetGlyphRangesChineseSimplifiedCommon());
                        CFRelease(url);
                        CFRelease(desc);
                        CFRelease(ctFont);
                        return;
                    }
                    CFRelease(url);
                }
                CFRelease(desc);
            }
            CFRelease(ctFont);
        }
    }
#endif
}

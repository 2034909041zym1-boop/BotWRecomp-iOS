#include <imgui.h>
#include "imgui/imgui_extension.h"
#include <unistd.h>

#if defined(__APPLE__)
#import <Foundation/Foundation.h>
#import <CoreText/CoreText.h>
#include <TargetConditionals.h>
#endif

void ImGui_LoadChineseFont(ImFontAtlas* atlas) {
    // Bake PingFangSC at 36px (native Retina scale on iPhone) so glyphs stay sharp instead of
    // upscaling a 15px bitmap, and explicitly include every Chinese/punctuation character used by
    // the in-game overlay UI so nothing ever falls back to '?'.
    atlas->TexDesiredWidth = 2048;

    static ImVector<ImWchar> s_ranges;
    if (s_ranges.empty()) {
        ImFontGlyphRangesBuilder builder;
        builder.AddRanges(atlas->GetGlyphRangesDefault());
        builder.AddRanges(atlas->GetGlyphRangesChineseSimplifiedCommon());
        builder.AddText(
            "塞尔达传说：旷野之息设置画质与渲染操控音频帧率模式流畅省电高负载"
            "渲染分辨率自动适配屏幕抗锯齿关闭原生柔和锐化推荐缩放滤镜双线性双三次"
            "埃尔米特邻近采样画面比例原始黑边全屏拉伸填充裁剪性能角标显示角标"
            "体感瞄准手柄震动按键布局任天堂在右下虚拟按键常开连隐藏游戏音量"
            "关闭菜单当前以改动将在重启后生效其余选项实时方向键选择调节确认返回"
            "由抗锯齿接管点击左右箭头或标题切换外部画质模糊清晰度默认重置，。：；（）！？、·•×—【】「」…←→<>%");
        builder.BuildRanges(&s_ranges);
    }

#if defined(__APPLE__)
    @autoreleasepool {
        NSArray<NSString*>* fontNames = @[
            @"PingFangSC-Regular",
            @".PingFangSC-Regular",
            @".PingFang-SC-Regular",
            @"STHeitiSC-Light"
        ];
        for (NSString* fontName in fontNames) {
            CTFontRef ctFont = CTFontCreateWithName((__bridge CFStringRef)fontName, 36.0, NULL);
            if (!ctFont) continue;
            CTFontDescriptorRef desc = CTFontCopyFontDescriptor(ctFont);
            if (desc) {
                CFURLRef url = (CFURLRef)CTFontDescriptorCopyAttribute(desc, kCTFontURLAttribute);
                if (url) {
                    NSString* path = [(__bridge NSURL*)url path];
                    if (path && access([path UTF8String], R_OK) == 0) {
                        ImFontConfig cfg;
                        cfg.FontNo = 0;
                        cfg.MergeMode = false;
                        cfg.OversampleH = 1;
                        cfg.OversampleV = 1;
                        cfg.PixelSnapH = true;
                        ImFont* loaded = atlas->AddFontFromFileTTF([path UTF8String], 36.0f, &cfg, s_ranges.Data);
                        CFRelease(url);
                        CFRelease(desc);
                        CFRelease(ctFont);
                        if (loaded) return;
                    } else {
                        CFRelease(url);
                    }
                }
                CFRelease(desc);
            }
            CFRelease(ctFont);
        }
    }
#endif
    atlas->AddFontDefault();
}

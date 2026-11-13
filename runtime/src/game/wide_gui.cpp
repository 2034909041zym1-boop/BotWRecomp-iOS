// Port of Cemu's BotW v208 ultrawide GUI patch (src/BreathOfTheWild/Graphics/patch_UltrawideAspectRatio
// gui_aspect.asm + gui_screens.asm, moduleMatches 0x6267BFD0). The .blyt UI is laid out for a 16:9
// surface; with hor+ the surface is wider, so each named pane is rescaled/repositioned to keep the HUD
// at the screen edges ("Edge HUD", $ultrawideHUDMode = 1, the pack default). Only runs when hor+ is on.
#include <math.h>
#include <string.h>

#include "hooks.h"

extern float g_aspect_extra;   // main.cpp: horizontal widening factor (1.0 = 16:9)
extern float g_render_scale;   // main.cpp: render scale (1.0 in hor+ mode)

namespace {

constexpr int kNameLen = 34;

// Thread-local copies of the current screen name and the enclosing "Pa_" sub-panel name
// (the pack's copyScreenString / copySubPanelString codecave buffers).
thread_local char g_screen[40];
thread_local char g_subPanel[40];

// Guest address of the map pane's X position (0x1C field), written by scaleMapAndStore and
// overridden by the setMapPosition hook to switch between full map (0) and minimap (right edge).
uint32_t g_mapXPosAddr = 0;

// --- constants derived from g_aspect_extra (the pack's const_* values) ---
inline float ar()  { return 1.0f / g_aspect_extra; }                            // const_AspectRatio
inline float rar() { return g_aspect_extra; }                                   // const_ReverseAspectRatio
inline float padRight()    { return 640.0f - 640.0f / g_aspect_extra; }        // const_PaddingRightSide
inline float padLeft()     { return -(640.0f - 640.0f / g_aspect_extra); }     // const_PaddingLeftSide
inline float altPadRight() { return 640.0f * g_render_scale - 640.0f / g_aspect_extra; }  // const_AltPaddingRightSide
inline float altPadLeft()  { return -(640.0f * g_render_scale - 640.0f / g_aspect_extra); } // const_AltPaddingLeftSide

// Custom keep-position offsets (const_* values evaluated for $ultrawideHUDMode == 1).
constexpr float kTimeOffset       = 342.0f * 0.85f;   // const_TimeOffset
constexpr float kSensorsOffset    = 392.0f;           // const_SensorsOffset / const_EnergyMeterOffset
constexpr float kItemGetPlusMinus = -514.0f;          // const_ItemGetPlusMinusOffset
constexpr float kWolfHeartOffset  = -468.0f;          // const_WolfHeartOffset
constexpr float kSkipButtonOffset = 560.0f;           // const_SkipButtonOffset

// --- pane field accessors (float layout confirmed from the disassembly at 0x03C49680) ---
inline const char* name(uint32_t p) { return mem::str(p + 0x80); }
inline bool isPane(uint32_t p, const char* n) { const char* s = name(p); return s && strcmp(s, n) == 0; }
inline float gx(uint32_t p)  { return mem::rf32(p + 0x1C); }   // X position
inline void  sx(uint32_t p, float v) { mem::wf32(p + 0x1C, v); }
inline float gxs(uint32_t p) { return mem::rf32(p + 0x34); }   // X scale
inline void  sxs(uint32_t p, float v) { mem::wf32(p + 0x34, v); }
inline float gys(uint32_t p) { return mem::rf32(p + 0x38); }   // Y scale
inline void  sys(uint32_t p, float v) { mem::wf32(p + 0x38, v); }

// --- generic pane transforms (mode is always 1 = edge HUD) ---

// [x] + ((640 - [x]) * (1 - ar)) for x >= 0, mirrored for x < 0. Mirrors the pack's
// scalePaneAndKeepPos / moveKeepPos / scalePaneAndKeepCustomPos arithmetic.
inline float keepPos(float x) {
    if (x >= 0.0f) return x + (640.0f - x) * (1.0f - ar());
    return x - (640.0f + x) * (1.0f - ar());
}

void scalePaneNormal(uint32_t p, uint32_t) { sxs(p, gxs(p) * ar()); }
void scalePaneAndPos(uint32_t p, uint32_t) {
    sxs(p, gxs(p) * ar());
    sx(p, gx(p) * ar());
}
void scaleOnlyPos(uint32_t p, uint32_t) { sx(p, gx(p) * ar()); }
void scalePaneToLeftSide(uint32_t p, uint32_t) {
    sxs(p, gxs(p) * ar());
    sx(p, gx(p) + padLeft());
}
void scalePaneToRightSide(uint32_t p, uint32_t) {
    sxs(p, gxs(p) * ar());
    sx(p, gx(p) + padRight());
}
void scalePaneReverse(uint32_t p, uint32_t) { sxs(p, gxs(p) * rar()); }
void scalePaneOnYScale(uint32_t p, uint32_t) { sys(p, gys(p) * rar()); }
void movePaneToRightSide(uint32_t p, uint32_t) { sx(p, gx(p) + padRight()); }
void movePaneToRightSideAlt(uint32_t p, uint32_t) { sx(p, gx(p) + altPadRight()); }
void scalePaneAndKeepPos(uint32_t p, uint32_t) {
    sxs(p, gxs(p) * ar());
    sx(p, keepPos(gx(p)));
}
void moveKeepPos(uint32_t p, uint32_t) { sx(p, keepPos(gx(p))); }
void scalePaneAndKeepCustomPos(uint32_t p, uint32_t, float off) {
    sxs(p, gxs(p) * ar());
    sx(p, keepPos(gx(p) + off) - off);
}
void scaleOnlySizeReverse(uint32_t p, uint32_t sizePtr) {
    mem::wf32(sizePtr, mem::rf32(sizePtr) * rar());   // scales 0x0(r27), the pane's size width
}
void scaleMapAndStore(uint32_t p, uint32_t) {
    sxs(p, gxs(p) * ar());
    sx(p, gx(p) + padRight());  // edge HUD: initialise the map at the right edge
    g_mapXPosAddr = p + 0x1C;
}

// --- screen-level strategies (mirror the pack's scale*Panes labels) ---

void scaleRootToCenter(uint32_t p, uint32_t sz) {
    if (isPane(p, "RootPane")) scalePaneNormal(p, sz);
}
void scaleInOutToCenter(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_InOut_00")) scalePaneNormal(p, sz);
}
void scaleTopInOutToCenter(uint32_t p, uint32_t sz) {
    if (g_subPanel[0] != 0) return;
    if (isPane(p, "N_InOut_00")) scalePaneNormal(p, sz);
}
void scaleTopInOutToCenterAndPos(uint32_t p, uint32_t sz) {
    if (g_subPanel[0] != 0) return;
    if (isPane(p, "N_InOut_00")) scalePaneAndPos(p, sz);
}
void scaleInOutScreenToLeftSide(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_InOut_00")) scalePaneToLeftSide(p, sz);
}
void scaleInOutScreenToRightSide(uint32_t p, uint32_t sz) {
    if (isPane(p, "RootPane")) movePaneToRightSide(p, sz);
    else if (isPane(p, "N_InOut_00")) scalePaneNormal(p, sz);
}

void scaleMainScreenPanes(uint32_t p, uint32_t sz);
void scaleMainScreenRunePointingPanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_SunLight_00")) scalePaneNormal(p, sz);
}
void scaleMainScreenCameraPointerPanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_InOut_01") || isPane(p, "N_Dungeon_00")) scalePaneNormal(p, sz);
}
void scaleMainScreenScopeFramePanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_TimeLineDemo_00") || isPane(p, "N_IconSeek_00")) scalePaneNormal(p, sz);
}
void scaleMainScreenGaugePanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_All_00")) scalePaneNormal(p, sz);
}
void scaleMainScreenTimePanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_All_00")) scalePaneAndKeepCustomPos(p, sz, kTimeOffset);
}
void scaleSensorMeterPanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_InOut_00")) scalePaneAndKeepCustomPos(p, sz, kSensorsOffset);
}
void scaleSoundMeterPanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_InOut_00")) scalePaneAndKeepCustomPos(p, sz, kSensorsOffset);
}
void scaleTempMeterPanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_InOut_00")) scalePaneAndKeepCustomPos(p, sz, kSensorsOffset);
}
void scaleMainScreenWeatherPanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_InOut_00")) scalePaneAndKeepPos(p, sz);
}
void scaleMainScreenInformationTextPanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "Pa_Information_00")) scalePaneNormal(p, sz);
}
void scaleMainScreenLocationTextPanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "Pa_LocationName_00")) scalePaneNormal(p, sz);
}
void scaleMainScreenQuestTextPanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "Pa_QuestName_00")) scalePaneNormal(p, sz);
    else if (isPane(p, "P_Base_01") || isPane(p, "P_Base_00")) scalePaneReverse(p, sz);
}
void scaleMainScreenSmallLocationTextPanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "Pa_LocationNameS_00")) scalePaneToLeftSide(p, sz);
}
void scaleMainScreenBossGaugePanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_All_00")) scalePaneNormal(p, sz);
}

void scaleMainScreenPanes(uint32_t p, uint32_t sz) {
    // Dispatch on the enclosing "Pa_" sub-panel name first.
    if (strcmp(g_subPanel, "Pa_ArrowPointer_00") == 0) { scaleInOutToCenter(p, sz); return; }
    if (strcmp(g_subPanel, "Pa_ItemPointer_00") == 0) { scaleMainScreenRunePointingPanes(p, sz); return; }
    if (strcmp(g_subPanel, "Pa_ThrowingPointer_00") == 0) { scaleInOutToCenter(p, sz); return; }
    if (strcmp(g_subPanel, "Pa_CameraPointer_00") == 0) { scaleMainScreenCameraPointerPanes(p, sz); return; }
    if (strcmp(g_subPanel, "Pa_BtnZR_00") == 0) { scaleMainScreenScopeFramePanes(p, sz); return; }
    if (strcmp(g_subPanel, "Pa_Deco_00") == 0) { scaleMainScreenScopeFramePanes(p, sz); return; }
    if (strcmp(g_subPanel, "Pa_Gauge_00") == 0) { scaleMainScreenGaugePanes(p, sz); return; }
    if (strcmp(g_subPanel, "Pa_Time_00") == 0) { scaleMainScreenTimePanes(p, sz); return; }
    if (strcmp(g_subPanel, "Pa_Sensor_00") == 0) { scaleSensorMeterPanes(p, sz); return; }
    if (strcmp(g_subPanel, "Pa_SoundGauge_00") == 0) { scaleSoundMeterPanes(p, sz); return; }
    if (strcmp(g_subPanel, "Pa_TempMeter_00") == 0) { scaleTempMeterPanes(p, sz); return; }
    if (strcmp(g_subPanel, "Pa_Weather_00") == 0) { scaleMainScreenWeatherPanes(p, sz); return; }
    if (strcmp(g_subPanel, "Pa_Information_00") == 0) { scaleMainScreenInformationTextPanes(p, sz); return; }
    if (strcmp(g_subPanel, "Pa_LocationName_00") == 0) { scaleMainScreenLocationTextPanes(p, sz); return; }
    if (strcmp(g_subPanel, "Pa_LocationNameS_00") == 0) { scaleMainScreenSmallLocationTextPanes(p, sz); return; }
    if (strcmp(g_subPanel, "Pa_QuestName_00") == 0) { scaleMainScreenQuestTextPanes(p, sz); return; }
    if (strcmp(g_subPanel, "Pa_BossGauge_00") == 0) { scaleMainScreenBossGaugePanes(p, sz); return; }

    // Fall through: dispatch on the pane's own name.
    if (isPane(p, "N_State_00")) { scalePaneToLeftSide(p, sz); return; }
    if (isPane(p, "Pa_SinJu_00")) { scalePaneAndKeepPos(p, sz); return; }
    if (isPane(p, "Pa_SinJu_01")) { scalePaneAndKeepPos(p, sz); return; }
    if (isPane(p, "Pa_SinJu_02")) { scalePaneAndKeepPos(p, sz); return; }
    if (isPane(p, "Pa_SinJu_03")) { scalePaneAndKeepPos(p, sz); return; }
}

void scaleMainDungeonPanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "Pa_LocationNameS_00")) scalePaneToLeftSide(p, sz);
    else if (isPane(p, "Pa_Message_00") || isPane(p, "Pa_Title_00")) scalePaneNormal(p, sz);
}

void scaleSimpleItemGetPanes(uint32_t p, uint32_t sz);
void scaleSimpleItemGetContinue(uint32_t p, uint32_t sz) {
    if (strcmp(g_subPanel, "PaPlusMinus_00") == 0) {
        if (isPane(p, "N_InOut_00")) scalePaneAndKeepCustomPos(p, sz, kItemGetPlusMinus);
        return;
    }
    if (isPane(p, "W_Base_00") || isPane(p, "T_Num_00") || isPane(p, "T_Time_00") ||
        isPane(p, "T_Rupee_00") || isPane(p, "T_KeyNum_00") || isPane(p, "P_Icon_00") ||
        isPane(p, "N_Icon_00") || isPane(p, "P_KologNuts_00")) {
        scalePaneAndKeepPos(p, sz);
    }
}
void scaleSimpleItemGetPanes(uint32_t p, uint32_t sz) {
    if (g_subPanel[0] != 0) {
        scaleSimpleItemGetContinue(p, sz);
    } else {
        if (isPane(p, "N_Capture_00")) scalePaneAndKeepPos(p, sz);
        else scaleSimpleItemGetContinue(p, sz);
    }
}

void scaleWolfLinkHeartPanes(uint32_t p, uint32_t sz) {
    if (strcmp(g_subPanel, "Pa_HeartGauge_00") == 0) {
        if (isPane(p, "T_Name_00")) scalePaneAndKeepPos(p, sz);
        else if (isPane(p, "N_InOut_00")) scalePaneAndKeepCustomPos(p, sz, kWolfHeartOffset);
        return;
    }
    if (g_subPanel[0] == 0) {
        if (isPane(p, "P_Sh_00") || isPane(p, "P_Illust_00")) scalePaneAndKeepPos(p, sz);
    }
}

void scaleMainScreenMasterSwordPanes(uint32_t p, uint32_t sz) {
    if (g_subPanel[0] == 0) {
        if (isPane(p, "N_InOut_00")) scalePaneNormal(p, sz);
    }
}

void scaleMainScreen3DPanes(uint32_t p, uint32_t sz);
void scaleMainScreen3DStaminaPanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_All_00")) scalePaneNormal(p, sz);
}
void scaleMainScreen3DBonusStaminaPanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_Size_05")) scalePaneNormal(p, sz);
}
void scaleMainScreen3DPickupPanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_Capture_00") || isPane(p, "N_ModeItem_00")) scalePaneNormal(p, sz);
}
void scaleMainScreen3DEnemyNoticePanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_Change_00") || isPane(p, "N_Change_01")) scalePaneNormal(p, sz);
}
void scaleMainScreen3DEnemyGaugePanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_Gauge_00")) scalePaneNormal(p, sz);
}
void scaleMainScreen3DPanes(uint32_t p, uint32_t sz) {
    if (strcmp(g_subPanel, "Pa_NoticeItem_00") == 0) { scaleMainScreen3DPickupPanes(p, sz); return; }
    if (strcmp(g_subPanel, "Pa_Ganbari_00") == 0) { scaleMainScreen3DStaminaPanes(p, sz); return; }
    if (strcmp(g_subPanel, "Pa_EnemyMark_00") == 0) { scaleMainScreen3DEnemyNoticePanes(p, sz); return; }
    if (strcmp(g_subPanel, "Pa_EnemyGauge_00") == 0) { scaleMainScreen3DEnemyGaugePanes(p, sz); return; }
    if (strcmp(g_subPanel, "Pa_NoticeZ_00") == 0) { scaleInOutToCenter(p, sz); return; }
    if (strcmp(g_subPanel, "Pa_GanbariEx_00") == 0) { scaleMainScreen3DBonusStaminaPanes(p, sz); return; }
}

void scaleMainShortCutPanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_MainAll_00")) scalePaneAndKeepPos(p, sz);
    else if (isPane(p, "N_Cut_00")) scalePaneNormal(p, sz);
    else if (isPane(p, "N_Capture_00") || isPane(p, "N_Capture_01") ||
             isPane(p, "P_CaptureMask_00") || isPane(p, "P_CaptureMask_01")) scalePaneAndPos(p, sz);
    else if (isPane(p, "N_PartsSize_00")) scalePaneReverse(p, sz);
}

void scaleHorseStaminaPanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_ExUse_01")) scalePaneNormal(p, sz);
}

void scaleMainHardModePanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_InOut_00") || isPane(p, "N_Glow_00")) scalePaneAndKeepPos(p, sz);
}

void scalePauseMenuPanes(uint32_t p, uint32_t sz);
void scalePauseMenuQuestPanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_Cut_00")) scalePaneReverse(p, sz);
}
void scalePauseMenuPanes(uint32_t p, uint32_t sz) {
    if (g_subPanel[0] == 0) {
        if (isPane(p, "B_Page20_00") || isPane(p, "N_InOut_00")) scalePaneAndPos(p, sz);
        return;
    }
    if (strcmp(g_subPanel, "Pa_Quest_00") == 0) { scalePauseMenuQuestPanes(p, sz); return; }
    if (isPane(p, "N_Slide_02")) scalePaneNormal(p, sz);
    else if (isPane(p, "N_InCap_01") || isPane(p, "N_InOutGear_01")) scalePaneAndPos(p, sz);
    else if (isPane(p, "N_InOutQuest_02") || isPane(p, "Pa_Page_00")) scalePaneNormal(p, sz);
}

void scalePauseHomePanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_IconSeek_00") || isPane(p, "N_TimeLineDemo_00")) scalePaneNormal(p, sz);
}
void scalePauseRunePanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_InOut_00")) scalePaneNormal(p, sz);
    else if (isPane(p, "N_NoContents_00")) movePaneToRightSide(p, sz);
}
void scalePauseAlbumPanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "Pa_Page_00") || isPane(p, "N_InOut_02") || isPane(p, "Pa_Photo_00")) scalePaneNormal(p, sz);
    else if (isPane(p, "N_NoContents_00")) movePaneToRightSide(p, sz);
}
void scalePausePictureBookPanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "Pa_Page_00") || isPane(p, "N_Contents_00")) scalePaneNormal(p, sz);
    else if (isPane(p, "N_PartsSize_00")) scalePaneReverse(p, sz);
    else if (isPane(p, "N_InOut_02")) scalePaneNormal(p, sz);
    else if (isPane(p, "N_NoContents_00")) movePaneToRightSide(p, sz);
    else if (isPane(p, "N_MapPos_00")) moveKeepPos(p, sz);
}

void scaleSystemWindowInventoryPanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_TypeBtn_01")) scalePaneAndPos(p, sz);
    else if (isPane(p, "N_InOut_01")) scalePaneNormal(p, sz);
}
void scalePauseMenuAbilityControlsPanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_Option_00") || isPane(p, "N_Align_00")) scalePaneNormal(p, sz);
}
void scalePauseMenuBGPanes(uint32_t p, uint32_t sz) { (void)p; (void)sz; }  // pack leaves this empty
void scalePauseMenuInfoPanes(uint32_t p, uint32_t sz) {
    if (g_subPanel[0] == 0) return;
    if (isPane(p, "N_InOut_01")) scalePaneNormal(p, sz);
    else if (isPane(p, "N_Slide_00")) scaleOnlyPos(p, sz);
    else if (isPane(p, "Pa_RotateGuide_00")) scalePaneAndKeepPos(p, sz);
    else if (isPane(p, "N_State_00") || isPane(p, "N_State_03") ||
             isPane(p, "N_TitleBar_00") || isPane(p, "N_TitleBarDown_00")) scalePaneNormal(p, sz);
}

void scaleShopHorsePanes(uint32_t p, uint32_t sz) {
    if (g_subPanel[0] != 0) {
        if (isPane(p, "N_Info_00")) scalePaneNormal(p, sz);
    } else {
        if (isPane(p, "N_InOut_00")) scalePaneNormal(p, sz);
    }
}
void scaleShop05Panes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_InOut_01") || isPane(p, "N_InOut_02")) scalePaneNormal(p, sz);
}
void scaleShop20Panes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_Cut_00")) scaleOnlyPos(p, sz);
    else if (isPane(p, "Pa_Page_4x5_00") || isPane(p, "N_InOut_02")) scalePaneNormal(p, sz);
}

void scaleMapPanes(uint32_t p, uint32_t sz) {
    if (g_subPanel[0] == 0) {
        if (isPane(p, "N_InOut_00")) scaleMapAndStore(p, sz);
        else if (isPane(p, "N_Cut_00")) scaleOnlySizeReverse(p, sz);
        else if (isPane(p, "Black8_01")) scalePaneReverse(p, sz);
        return;
    }
    if (strcmp(g_subPanel, "Pa_Map_00") == 0) {
        if (isPane(p, "B_MapCapture_00")) scaleOnlySizeReverse(p, sz);
    }
}

void scaleDungeonMapPanes(uint32_t p, uint32_t sz) {
    if (g_subPanel[0] == 0) {
        if (isPane(p, "N_Born_00")) scalePaneNormal(p, sz);
        return;
    }
    if (strcmp(g_subPanel, "Pa_SensorBox_00") == 0 || strcmp(g_subPanel, "Pa_SubBtnChange_00") == 0 ||
        strcmp(g_subPanel, "Pa_Btn_00") == 0) {
        if (isPane(p, "Pa_SensorBox_00")) scalePaneReverse(p, sz);
        else if (isPane(p, "N_BtnPos")) scalePaneNormal(p, sz);
        else if (isPane(p, "N_SubIn_00")) scalePaneAndPos(p, sz);
        else if (isPane(p, "P_Sh_00")) scalePaneNormal(p, sz);
        else if (isPane(p, "T_SensorTitle_00")) scalePaneNormal(p, sz);
    }
}

void scaleCameraPanes(uint32_t p, uint32_t sz);
void scaleCameraFramePanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_TimeLineDemo_00") || isPane(p, "N_IconSeek_00")) scalePaneNormal(p, sz);
}
void scaleCameraPanes(uint32_t p, uint32_t sz) {
    if (strcmp(g_subPanel, "Pa_BtnZR_00") == 0 || strcmp(g_subPanel, "Pa_Deco_00") == 0) {
        scaleCameraFramePanes(p, sz);
        return;
    }
    if (isPane(p, "N_ModeCamera_00") || isPane(p, "N_FirstOut_00") || isPane(p, "N_Limit_00")) scalePaneNormal(p, sz);
    else if (isPane(p, "P_Capture_00")) scalePaneReverse(p, sz);
}

void scaleDoCommandPanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_MS_00")) scalePaneNormal(p, sz);
}
void scalePickupPanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "Pa_PickUpWin_00")) scalePaneAndKeepPos(p, sz);
}
void scaleRuneGuidePanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_Align_00")) scalePaneNormal(p, sz);
}

void scaleMessageDialogPanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_Pos_00")) scalePaneNormal(p, sz);
    else if (isPane(p, "N_Cursor_00") || isPane(p, "B_Hit_00")) movePaneToRightSideAlt(p, sz);
}
void scaleMessageSpPanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_Seeker_00") || isPane(p, "N_Pos_00") || isPane(p, "N_BtnNum_00")) scalePaneNormal(p, sz);
    else if (isPane(p, "N_Cursor_00") || isPane(p, "B_Hit_00")) movePaneToRightSideAlt(p, sz);
}

void scaleMessageGetPanes(uint32_t p, uint32_t sz);
void scaleMessageGetBackgroundAndIconPanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "W_Base_00")) scalePaneNormal(p, sz);
    else if (isPane(p, "N_Icon_00")) scalePaneAndPos(p, sz);
    else if (isPane(p, "N_State_00")) scalePaneNormal(p, sz);
    else if (isPane(p, "P_Capture_00") || isPane(p, "P_Capture_03")) scalePaneNormal(p, sz);
    else if (isPane(p, "Nt_DecoLT_00") || isPane(p, "Nt_DecoLT_01") || isPane(p, "Nt_DecoRT_00") ||
             isPane(p, "Nt_DecoRT_01") || isPane(p, "Nt_DecoLB_00") || isPane(p, "Nt_DecoLB_01")) scalePaneAndPos(p, sz);
    else if (isPane(p, "Nt_DecoRB_00") || isPane(p, "Nt_DecoRB_01")) scalePaneNormal(p, sz);
}
void scaleMessageGetPanes(uint32_t p, uint32_t sz) {
    if (strcmp(g_subPanel, "Pa_Star_00") == 0) { scaleMessageGetBackgroundAndIconPanes(p, sz); return; }
    if (isPane(p, "N_DecideOut_00") || isPane(p, "P_Capture_01") || isPane(p, "P_Capture_04") ||
        isPane(p, "P_Capture_02")) scalePaneNormal(p, sz);
}

void scaleMessage3DPanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_Capture_00") || isPane(p, "N_All_00")) scalePaneNormal(p, sz);
}

void scaleRuntimeMessagePanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "W_Base_00") || isPane(p, "T_Text_00") || isPane(p, "T_Text_00_JPja") ||
        isPane(p, "N_GuideOn_00")) scalePaneAndKeepPos(p, sz);
}

void scaleCursorPanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_Porch_00") || isPane(p, "N_Seek_00") || isPane(p, "N_Porch_01") ||
        isPane(p, "N_Seek_01") || isPane(p, "N_Porch_02") || isPane(p, "N_Seek_02") ||
        isPane(p, "N_Porch_03") || isPane(p, "N_Seek_03")) scalePaneNormal(p, sz);
}

void scaleSkipButtonPanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_InOut_00") || isPane(p, "N_Glow_00")) scalePaneAndKeepCustomPos(p, sz, kSkipButtonOffset);
}
void scaleLoadSaveIconPanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_InOut_00") || isPane(p, "N_Glow_00")) scalePaneAndKeepPos(p, sz);
}

void scaleLoadingPanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_RegionTypePos_00")) scalePaneToLeftSide(p, sz);
    else if (isPane(p, "Pa_GuideA_00")) scalePaneAndKeepPos(p, sz);
    else if (isPane(p, "N_LogoInOut_02")) scalePaneToRightSide(p, sz);
    else if (isPane(p, "T_TipsTitle_00") || isPane(p, "T_TipsTitle_00_JPja") ||
             isPane(p, "T_TipsTitle_00_KR") || isPane(p, "T_TipsTitle_00_CN") ||
             isPane(p, "T_TipsTitle_00_TW") || isPane(p, "T_Tips_00") ||
             isPane(p, "T_Tips_00_JPja") || isPane(p, "T_Tips_00_KR") ||
             isPane(p, "T_Tips_00_CN") || isPane(p, "T_Tips_00_TW")) scalePaneAndKeepPos(p, sz);
}
void scaleLoadingStatusPanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_ParamInOut_00")) scalePaneToRightSide(p, sz);
}

void scaleGameOverPanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_Aka_00") || isPane(p, "N_Ao_00") || isPane(p, "N_Kiiro_00") ||
        isPane(p, "Pa_Btn_00") || isPane(p, "Pa_Btn_02") || isPane(p, "T_SubText_00")) scalePaneNormal(p, sz);
}

void scaleSystemWindowPanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_SlideArea_00") || isPane(p, "N_InOut_03") || isPane(p, "N_FileSelect_01")) scalePaneNormal(p, sz);
    else if (isPane(p, "N_HardModePos_00")) scalePaneToLeftSide(p, sz);
}
void scaleAppSystemWindowPanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_BtnNum_00") || isPane(p, "N_BtnPos_00")) scalePaneNormal(p, sz);
}
void scaleOptionsMenuPanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_Option_00") || isPane(p, "N_Option_01")) scalePaneNormal(p, sz);
}
void scaleDLCWindowPanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_Option_00")) scalePaneNormal(p, sz);
}

void scaleTitlePanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_RegionType_00") || isPane(p, "N_RegionType_00_JPja") || isPane(p, "P_Blur_08")) scalePaneToRightSide(p, sz);
    else if (isPane(p, "N_HardMode_03")) scalePaneToLeftSide(p, sz);
    else if (isPane(p, "N_CopyRight_00") || isPane(p, "N_BtnIn_00")) scalePaneToRightSide(p, sz);
    else if (isPane(p, "N_1_Arrow_00") || isPane(p, "N_5_Gardian_00") || isPane(p, "N_2_Horse_00") ||
             isPane(p, "N_4_SenakaLink_00")) scalePaneToLeftSide(p, sz);
    else if (isPane(p, "N_3_Gake_00") || isPane(p, "N_0_MainV_00") || isPane(p, "N_6_MainVE_00")) scalePaneOnYScale(p, sz);
}

void scaleDemoNameEnemyPanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_Capture_00") || isPane(p, "N_Capture_01") || isPane(p, "N_In_00") ||
        isPane(p, "P_BigTextSh_00") || isPane(p, "P_BigText_00") || isPane(p, "P_Light_01") ||
        isPane(p, "P_BigText_01") || isPane(p, "P_BigText_02")) scalePaneNormal(p, sz);
}

void scaleLastCompletePanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_Capture_00") || isPane(p, "P_Base_03") || isPane(p, "P_Base_01") ||
        isPane(p, "N_In_00") || isPane(p, "N_In_01")) scalePaneNormal(p, sz);
}

void scaleCreditPanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_PicFade_00") || isPane(p, "N_PicFade_01") || isPane(p, "P_Sh_00") ||
        isPane(p, "P_Sh_01") || isPane(p, "P_Text_01") || isPane(p, "N_StaffRoll_00") ||
        isPane(p, "N_PostOne_03") || isPane(p, "N_PostOne_02") || isPane(p, "N_PostOne_01") ||
        isPane(p, "N_PostOne_00") || isPane(p, "N_NameTriple_00") || isPane(p, "N_NameOne_00") ||
        isPane(p, "N_CopyRight_00")) scalePaneNormal(p, sz);
}

void scaleDLCCreditPanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_CaptureText_00") || isPane(p, "N_In_00")) scalePaneNormal(p, sz);
    else if (isPane(p, "P_BG_00")) scalePaneReverse(p, sz);
}

void scaleThanksPanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_In_00") || isPane(p, "N_Capture_00") || isPane(p, "N_Capture_01")) scalePaneNormal(p, sz);
}

void scaleGameTitlePanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_Region_01") || isPane(p, "N_Region_01_JPja")) scalePaneAndKeepPos(p, sz);
}

void scaleMessageTipsPanes(uint32_t p, uint32_t sz);
void scaleMessageTipsSubTipsPanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_InOut_01")) scalePaneNormal(p, sz);
}
void scaleMessageTipsPanes(uint32_t p, uint32_t sz) {
    if (strcmp(g_subPanel, "Pa_Tips_00") == 0 || strcmp(g_subPanel, "Pa_TipsAmiibo_00") == 0) {
        scaleMessageTipsSubTipsPanes(p, sz);
        return;
    }
    if (isPane(p, "N_DecideOut_00")) scalePaneNormal(p, sz);
}

void scaleEnergyMeterDLCPanes(uint32_t p, uint32_t sz) {
    if (isPane(p, "N_InOut_00")) scalePaneNormal(p, sz);
}

// --- main dispatch (mirrors _scalePaneGUI) ---
void scalePaneGUI(uint32_t pane, uint32_t sizePtr) {
    const char* n = name(pane);

    // Remember the enclosing "Pa_" sub-panel name for the strategies above.
    if (n && n[0] == 'P' && n[1] == 'a') {
        strncpy(g_subPanel, n, kNameLen);
        g_subPanel[kNameLen] = 0;
    }

    // Screen-name dispatch.
    const char* s = g_screen;
    LOG("[gui] scr='%s' pane='%s' sub='%s'", s, n ? n : "?", g_subPanel);
    if (strcmp(s, "BootUp_00") == 0) { scaleInOutToCenter(pane, sizePtr); return; }
    if (strcmp(s, "ChangeController_00") == 0 || strcmp(s, "ChangeControllerDRC_00") == 0) { scaleInOutScreenToRightSide(pane, sizePtr); return; }
    if (strcmp(s, "GamePadBG_00") == 0) { scaleRootToCenter(pane, sizePtr); return; }
    if (strcmp(s, "Title_00") == 0) { scaleTitlePanes(pane, sizePtr); return; }
    if (strcmp(s, "SystemWindow_01") == 0) { scaleSystemWindowPanes(pane, sizePtr); return; }
    if (strcmp(s, "AppSystemWindow_00") == 0) { scaleAppSystemWindowPanes(pane, sizePtr); return; }
    if (strcmp(s, "AppSystemWindowNoBtn_00") == 0) { scaleInOutScreenToRightSide(pane, sizePtr); return; }
    if (strcmp(s, "OptionWindow_00") == 0) { scaleOptionsMenuPanes(pane, sizePtr); return; }
    if (strcmp(s, "AmiiboWindow_00") == 0) { scaleInOutToCenter(pane, sizePtr); return; }
    if (strcmp(s, "Fade") == 0) { scaleLoadingPanes(pane, sizePtr); return; }
    if (strcmp(s, "GameOver_00") == 0) { scaleGameOverPanes(pane, sizePtr); return; }
    if (strcmp(s, "FadeStatus_00") == 0) { scaleLoadingStatusPanes(pane, sizePtr); return; }
    if (strcmp(s, "Skip_00") == 0) { scaleSkipButtonPanes(pane, sizePtr); return; }
    if (strcmp(s, "LoadSaveIcon_00") == 0) { scaleLoadSaveIconPanes(pane, sizePtr); return; }
    if (strcmp(s, "MainScreen_00") == 0) { scaleMainScreenPanes(pane, sizePtr); return; }
    if (strcmp(s, "MainDungeon_00") == 0) { scaleMainDungeonPanes(pane, sizePtr); return; }
    if (strcmp(s, "MainScreenMS_00") == 0) { scaleMainScreenMasterSwordPanes(pane, sizePtr); return; }
    if (strcmp(s, "WolfLinkHeartGauge_00") == 0) { scaleWolfLinkHeartPanes(pane, sizePtr); return; }
    if (strcmp(s, "Rupee_00") == 0 || strcmp(s, "AkashNum_00") == 0 || strcmp(s, "KologNum_00") == 0 ||
        strcmp(s, "MamoNum_00") == 0 || strcmp(s, "KeyNum_00") == 0 || strcmp(s, "Time_00") == 0) { scaleSimpleItemGetPanes(pane, sizePtr); return; }
    if (strcmp(s, "MainScreen3D_00") == 0) { scaleMainScreen3DPanes(pane, sizePtr); return; }
    if (strcmp(s, "MainShortCut_00") == 0) { scaleMainShortCutPanes(pane, sizePtr); return; }
    if (strcmp(s, "MainHorse_00") == 0) { scaleHorseStaminaPanes(pane, sizePtr); return; }
    if (strcmp(s, "MainHardMode_00") == 0) { scaleMainHardModePanes(pane, sizePtr); return; }
    if (strcmp(s, "PauseMenu_00") == 0) { scalePauseMenuPanes(pane, sizePtr); return; }
    if (strcmp(s, "AppHome_00") == 0) { scalePauseHomePanes(pane, sizePtr); return; }
    if (strcmp(s, "AppTool_00") == 0) { scalePauseRunePanes(pane, sizePtr); return; }
    if (strcmp(s, "AppAlbum_00") == 0) { scalePauseAlbumPanes(pane, sizePtr); return; }
    if (strcmp(s, "AppPictureBook_00") == 0) { scalePausePictureBookPanes(pane, sizePtr); return; }
    if (strcmp(s, "SystemWindow_00") == 0) { scaleSystemWindowInventoryPanes(pane, sizePtr); return; }
    if (strcmp(s, "ControllerWindow_00") == 0) { scalePauseMenuAbilityControlsPanes(pane, sizePtr); return; }
    if (strcmp(s, "PauseMenuBG_00") == 0) { scalePauseMenuBGPanes(pane, sizePtr); return; }
    if (strcmp(s, "PauseMenuInfo_00") == 0) { scalePauseMenuInfoPanes(pane, sizePtr); return; }
    if (strcmp(s, "PauseMenuRecipe_00") == 0 || strcmp(s, "PauseMenuMantan_00") == 0) { scaleTopInOutToCenterAndPos(pane, sizePtr); return; }
    if (strcmp(s, "ShopHorse_00") == 0) { scaleShopHorsePanes(pane, sizePtr); return; }
    if (strcmp(s, "ShopBtnList5_00") == 0) { scaleShop05Panes(pane, sizePtr); return; }
    if (strcmp(s, "ShopBtnList15_00") == 0) { scaleTopInOutToCenter(pane, sizePtr); return; }
    if (strcmp(s, "ShopBtnList20_00") == 0) { scaleShop20Panes(pane, sizePtr); return; }
    if (strcmp(s, "ShopInfo_00") == 0) { scaleTopInOutToCenter(pane, sizePtr); return; }
    if (strcmp(s, "AppMap_00") == 0) { scaleMapPanes(pane, sizePtr); return; }
    if (strcmp(s, "AppMapDungeon_00") == 0) { scaleDungeonMapPanes(pane, sizePtr); return; }
    if (strcmp(s, "AppCamera_00") == 0) { scaleCameraPanes(pane, sizePtr); return; }
    if (strcmp(s, "DoCommand_00") == 0) { scaleDoCommandPanes(pane, sizePtr); return; }
    if (strcmp(s, "PickUp_00") == 0) { scalePickupPanes(pane, sizePtr); return; }
    if (strcmp(s, "SousaGuide_00") == 0) { scaleRuneGuidePanes(pane, sizePtr); return; }
    if (strcmp(s, "MessageDialog") == 0) { scaleMessageDialogPanes(pane, sizePtr); return; }
    if (strcmp(s, "MessageSp_00") == 0 || strcmp(s, "MessageSp_00_NoTop") == 0) { scaleMessageSpPanes(pane, sizePtr); return; }
    if (strcmp(s, "MessageGet_00") == 0) { scaleMessageGetPanes(pane, sizePtr); return; }
    if (strcmp(s, "Message3D_00") == 0) { scaleMessage3DPanes(pane, sizePtr); return; }
    if (strcmp(s, "DemoMessage") == 0 || strcmp(s, "DemoMessage_00") == 0 ||
        strcmp(s, "DemoName") == 0 || strcmp(s, "DemoName_00") == 0) { scaleInOutToCenter(pane, sizePtr); return; }
    if (strcmp(s, "DemoNameEnemy") == 0 || strcmp(s, "DemoNameEnemy_00") == 0) { scaleDemoNameEnemyPanes(pane, sizePtr); return; }
    if (strcmp(s, "LastComplete_00") == 0) { scaleLastCompletePanes(pane, sizePtr); return; }
    if (strcmp(s, "StaffRoll_00") == 0) { scaleCreditPanes(pane, sizePtr); return; }
    if (strcmp(s, "StaffRollDLC_00") == 0) { scaleDLCCreditPanes(pane, sizePtr); return; }
    if (strcmp(s, "Thanks_00") == 0) { scaleThanksPanes(pane, sizePtr); return; }
    if (strcmp(s, "End_00") == 0 || strcmp(s, "HardMode_00") == 0) { scaleInOutToCenter(pane, sizePtr); return; }
    if (strcmp(s, "DLCWindow_00") == 0) { scaleDLCWindowPanes(pane, sizePtr); return; }
    if (strcmp(s, "BoxCursorTV") == 0) { scaleCursorPanes(pane, sizePtr); return; }
    if (strcmp(s, "KeyBoradTextArea_00") == 0) { scaleInOutToCenter(pane, sizePtr); return; }
    if (strcmp(s, "GameTitle_00") == 0) { scaleGameTitlePanes(pane, sizePtr); return; }
    if (strcmp(s, "HardModeTextDLC_00") == 0) { scaleInOutScreenToLeftSide(pane, sizePtr); return; }
    if (strcmp(s, "MessageTipsRunTime_00") == 0) { scaleRuntimeMessagePanes(pane, sizePtr); return; }
    if (strcmp(s, "MessageTips_00") == 0) { scaleMessageTipsPanes(pane, sizePtr); return; }
    if (strcmp(s, "OPtext_00") == 0) { scaleInOutToCenter(pane, sizePtr); return; }
    if (strcmp(s, "ChallengeWin_00") == 0) { scaleInOutScreenToRightSide(pane, sizePtr); return; }
    if (strcmp(s, "EnergyMeterDLC_00") == 0) { scaleEnergyMeterDLCPanes(pane, sizePtr); return; }
}

}  // namespace

// Main pane-scaling hook: runs after `lwz r0,36(r30)` at 0x03C496B8. r31 = pane, r27 = size ptr.
BOTW_HOOK(0x03c496b8) {
    if (g_aspect_extra <= 1.0f) return;
    scalePaneGUI(c->r[31], c->r[27]);
}

// New-screen hook: runs after `lwz r11,12(r30)` at 0x03A3EDC4. r24 = screen name string.
BOTW_HOOK(0x03a3edc4) {
    if (g_aspect_extra <= 1.0f) return;
    const char* s = mem::str(c->r[24]);
    if (s) {
        strncpy(g_screen, s, kNameLen);
        g_screen[kNameLen] = 0;
    }
    g_subPanel[0] = 0;  // a new screen invalidates the previous sub-panel name
}

// Map-position hook: runs after `cmpi cr0,r3,0` at 0x02F60204. r31 == 0 selects the minimap.
BOTW_HOOK(0x02f60204) {
    if (g_aspect_extra <= 1.0f) return;
    if (g_mapXPosAddr == 0) return;
    float v = (c->r[31] == 0) ? padRight() : 0.0f;
    mem::wf32(g_mapXPosAddr, v);
}

// Weather-pane projection hook: runs after `stw r0,16(r4)` at 0x03C56CD0. r4 = projection object.
BOTW_HOOK(0x03c56cd0) {
    if (g_aspect_extra <= 1.0f) return;
    LOG("[gui-proj] sub='%s'", g_subPanel);
    if (strcmp(g_subPanel, "Pa_Weather_00") == 0) {
        mem::wf32(c->r[4] + 0x8, ar());  // proj_ScaleX
    }
}

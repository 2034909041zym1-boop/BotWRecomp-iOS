// Wide-screen (hor+) support. The camera descriptor keeps its projection aspect in a field read back
// through the getAspect() virtual (U-King.rpx 1.5.0, v208). f_036ff220 seeds that field from the
// 16:9 .data slot 0x1030a57c; forcing getAspect() to return the display aspect widens the horizontal
// FOV while the vertical FOV stays as authored. The render targets are widened to the display aspect
// (graphic_pack_shim.cpp) and LatteRenderTarget presents the result 1:1 (kHorPlus).
#include <atomic>

#include "hooks.h"

namespace {
constexpr uint32_t kAspectFrustum = 0x1030a57c;  // seeds the camera descriptor aspect via f_036ff220
constexpr uint32_t kAspectCamera = 0x1036dd4c;   // camera init default aspect (f_03ae58f8)

std::atomic<uint32_t> g_aspectLogs{0};
}  // namespace

extern float g_aspect_extra;

void wide_apply(float aspect) {
    const uint32_t slots[] = {kAspectFrustum, kAspectCamera};
    for (uint32_t addr : slots) {
        const float old = mem::rf32(addr);
        mem::wf32(addr, aspect);
        LOG("[wide] guest aspect slot %08x: %.6f -> %.6f", addr, old, aspect);
    }
}

// getAspect() virtual: `lfs f1,172(r3); blr`. Runs right after the lfs; overwrite f1 so the caller
// (the projection-matrix builder) receives the display aspect instead of 16:9.
BOTW_HOOK(0x030c1974) {
    if (g_aspect_extra <= 1.0f) return;
    const float prev = c->f[1];
    c->f[1] = g_aspect_extra * (16.0f / 9.0f);
    if (g_aspectLogs.fetch_add(1, std::memory_order_relaxed) % 300 == 0)
        LOG("[wide] getAspect: %.4f -> %.4f (obj=%08x)", prev, c->f[1], c->r[3]);
}

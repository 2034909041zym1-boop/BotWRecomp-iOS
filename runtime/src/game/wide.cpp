// Wide-screen (hor+) support. BotW's TV projection uses the 16:9 aspect ratio stored in two .data
// slots (U-King.rpx 1.5.0, v208). Rewriting those floats widens the horizontal FOV while the
// vertical FOV stays as authored; the render targets are widened to the display aspect
// (graphic_pack_shim.cpp) and LatteRenderTarget presents the result 1:1 (kHorPlus).
#include <atomic>

#include "hooks.h"

namespace {
constexpr uint32_t kAspectFrustum = 0x1030a57c;  // loaded by f_036ff220 when it builds frustum parameters
constexpr uint32_t kAspectCamera = 0x1036dd4c;   // loaded by f_03ae58f8 (camera descriptor: near/far/fov/aspect)

std::atomic<uint32_t> g_poseLogs{0};
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

// Diagnostic: the camera pose keeps fov/aspect at +36/+40 (see tree_diag.cpp). Logging it confirms
// whether the projection reads the aspect from there, which is the next place to patch if the
// .data constants turn out not to drive the projection. Rate limited, wide mode only.
BOTW_HOOK(0x02c07ca8) {
    if (g_aspect_extra <= 1.0f) return;
    if (g_poseLogs.fetch_add(1, std::memory_order_relaxed) % 300 != 0) return;
    const uint32_t cam = c->r[3] + 1584;  // current pose
    LOG("[wide] pose fov=%.4f aspect=%.4f near=%.4f far=%.4f", mem::rf32(cam + 36), mem::rf32(cam + 40),
        mem::rf32(cam + 44), mem::rf32(cam + 48));
}

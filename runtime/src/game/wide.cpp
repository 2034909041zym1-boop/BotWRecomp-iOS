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
thread_local uint32_t g_camera_base = 0;  // f_02c07c84's r3 (camera object), captured in the prologue
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

// Run in f_02c07c84's prologue while r3 still holds the camera object (r31 gets overwritten by the
// virtual-call results later in the function). Capture r3 here; the body hook uses it as the pose base.
BOTW_HOOK(0x02c07ca8) {
    g_camera_base = c->r[3];
    if (g_aspect_extra <= 1.0f) return;
    if (g_poseLogs.fetch_add(1, std::memory_order_relaxed) % 300 != 0) return;
    const uint32_t cam = c->r[3] + 1584;  // current pose
    LOG("[wide] pose fov=%.4f f40=%.4f aspect=%.4f near=%.4f far=%.4f r3=%08x", mem::rf32(cam + 36),
        mem::rf32(cam + 40), mem::rf32(cam + 44), mem::rf32(cam + 48), mem::rf32(cam + 52), c->r[3]);
}

// f_02c07c84 copies the target pose into the current pose field by field; run right after the +44
// copy and overwrite only the destination aspect (offset 1628) so the projection is built with the
// display aspect. Using the captured r3, not r31.
BOTW_HOOK(0x02c08628) {
    if (g_aspect_extra <= 1.0f || !g_camera_base) return;
    mem::wf32(g_camera_base + 1628, g_aspect_extra * (16.0f / 9.0f));
}

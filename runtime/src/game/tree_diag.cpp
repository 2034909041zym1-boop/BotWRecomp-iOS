// Tree-top freeze: the camera target pose at cam+1472..1492 (eye -> look-at) turns NaN, f_02c07c84 copies
// it into the current pose (cam+1584..), the NaN ray then defeats Havok BVH culling in the camera collision
// cast and every frame takes ~3.3 s. The NaN also feeds back into the camera controller, so it never recovers.
//
// 1) Writer watch: hooks after every store group that writes cam+1472..1492 and logs the first writer
//    that leaves a non-finite value (temporary diagnostic, rate limited).
// 2) Guard: at f_02c07c84 entry, a non-finite target/current pose is replaced with the last finite one,
//    which breaks the feedback loop instead of freezing the game.
#include <atomic>
#include <cmath>
#include <cstdint>

#include "hooks.h"

namespace {
constexpr int kPoseFloats = 14;  // +0..+52: eye, look-at, up, fov, aspect, near...

bool pose_finite(uint32_t base) {
    for (int i = 0; i < 6; i++)
        if (!std::isfinite(mem::rf32(base + i * 4))) return false;
    return true;
}

std::atomic<int> g_writer_logs{0};

void check_writer(const char* tag, PPCContext* c, int base_reg) {
    uint32_t cam = c->r[base_reg];
    if (pose_finite(cam + 1472)) return;
    if (g_writer_logs.fetch_add(1, std::memory_order_relaxed) >= 16) return;
    LOG("[nan-writer] %s cam=%08x lr=%08x pose1472=(%.4g,%.4g,%.4g)->(%.4g,%.4g,%.4g) "
        "cur1584=(%.4g,%.4g,%.4g)->(%.4g,%.4g,%.4g) "
        "f0..f13=%.4g %.4g %.4g %.4g %.4g %.4g %.4g %.4g %.4g %.4g %.4g %.4g %.4g %.4g "
        "f29..f31=%.4g %.4g %.4g r3=%08x r4=%08x r5=%08x",
        tag, cam, c->lr,
        mem::rf32(cam + 1472), mem::rf32(cam + 1476), mem::rf32(cam + 1480),
        mem::rf32(cam + 1484), mem::rf32(cam + 1488), mem::rf32(cam + 1492),
        mem::rf32(cam + 1584), mem::rf32(cam + 1588), mem::rf32(cam + 1592),
        mem::rf32(cam + 1596), mem::rf32(cam + 1600), mem::rf32(cam + 1604),
        c->f[0], c->f[1], c->f[2], c->f[3], c->f[4], c->f[5], c->f[6], c->f[7], c->f[8], c->f[9],
        c->f[10], c->f[11], c->f[12], c->f[13], c->f[29], c->f[30], c->f[31],
        c->r[3], c->r[4], c->r[5]);
}

struct GoodPose {
    uint32_t cam = 0;
    bool valid = false;
    float target[kPoseFloats]{};
    float current[kPoseFloats]{};
};
thread_local GoodPose tl_good;
std::atomic<int> g_guard_logs{0};

void save(uint32_t base, float* out) {
    for (int i = 0; i < kPoseFloats; i++) out[i] = mem::rf32(base + i * 4);
}
void restore(uint32_t base, const float* in) {
    for (int i = 0; i < kPoseFloats; i++) mem::wf32(base + i * 4, in[i]);
}
}  // namespace

// ---- writer watch (each hook runs right after the last store of a cam+1472..1492 group) ----------
BOTW_HOOK(0x02c03ce4) { check_writer("02c03ce4", c, 31); }
BOTW_HOOK(0x02c040c4) { check_writer("02c040c4", c, 31); }
BOTW_HOOK(0x02c04400) { check_writer("02c04400", c, 29); }
BOTW_HOOK(0x02c046cc) { check_writer("02c046cc", c, 29); }
BOTW_HOOK(0x02c04ae0) { check_writer("02c04ae0", c, 31); }
BOTW_HOOK(0x02c04f04) { check_writer("02c04f04", c, 30); }
BOTW_HOOK(0x02c054fc) { check_writer("02c054fc", c, 31); }
BOTW_HOOK(0x02c05590) { check_writer("02c05590", c, 31); }
BOTW_HOOK(0x02c098b4) { check_writer("02c098b4", c, 31); }
BOTW_HOOK(0x02c0b3dc) { check_writer("02c0b3dc", c, 30); }
BOTW_HOOK(0x02b8eb68) { check_writer("02b8eb68", c, 29); }
BOTW_HOOK(0x02bafd50) { check_writer("02bafd50", c, 30); }
BOTW_HOOK(0x02bb71a4) { check_writer("02bb71a4", c, 27); }
BOTW_HOOK(0x02b8faec) { check_writer("02b8faec", c, 3); }
BOTW_HOOK(0x02b203b0) { check_writer("02b203b0", c, 28); }

// ---- guard: f_02c07c84 entry (stwu r1, -640(r1)); r3 = camera ------------------------------------
BOTW_HOOK(0x02c07c84) {
    uint32_t cam = c->r[3];
    bool t_ok = pose_finite(cam + 1472);
    bool c_ok = pose_finite(cam + 1584);
    if (t_ok && c_ok) {
        tl_good.cam = cam;
        tl_good.valid = true;
        save(cam + 1472, tl_good.target);
        save(cam + 1584, tl_good.current);
        return;
    }
    if (!tl_good.valid || tl_good.cam != cam) return;
    if (g_guard_logs.fetch_add(1, std::memory_order_relaxed) < 8)
        LOG("[nan-guard] cam=%08x target_ok=%d current_ok=%d -> restoring last finite pose", cam, t_ok, c_ok);
    if (!t_ok) restore(cam + 1472, tl_good.target);
    if (!c_ok) restore(cam + 1584, tl_good.current);
}

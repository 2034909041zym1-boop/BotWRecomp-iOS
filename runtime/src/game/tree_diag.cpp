// Temporary diagnostic hooks for tracing the first NaN origin in camera shape-cast (f_02c07c84)
// and confirming BVH traversal behavior (f_04147e30).
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>

#include "hooks.h"

namespace {
struct CamSnapshot {
    uint32_t cam = 0;
    float f1472[14]{};
    float f1584[14]{};
    float v1768[3]{};
    float ang1792 = 0, ang1796 = 0, ang1804 = 0;
};

bool any_nan(const float* p, int n) {
    for (int i = 0; i < n; i++) {
        if (!std::isfinite(p[i])) return true;
    }
    return false;
}

bool cam_has_nan(uint32_t cam, const char** which) {
    for (int i = 0; i < 14; i++) {
        if (!std::isfinite(mem::rf32(cam + 1472 + i * 4))) { *which = "1472"; return true; }
    }
    for (int i = 0; i < 14; i++) {
        if (!std::isfinite(mem::rf32(cam + 1584 + i * 4))) { *which = "1584"; return true; }
    }
    for (int i = 0; i < 3; i++) {
        if (!std::isfinite(mem::rf32(cam + 1768 + i * 4))) { *which = "1768"; return true; }
    }
    if (!std::isfinite(mem::rf32(cam + 1792))) { *which = "1792"; return true; }
    if (!std::isfinite(mem::rf32(cam + 1796))) { *which = "1796"; return true; }
    if (!std::isfinite(mem::rf32(cam + 1804))) { *which = "1804"; return true; }
    return false;
}

void log_cam(const char* tag, uint32_t cam, uint32_t r1, const char* which) {
    LOG("[nan-trace] %s (%s) cam=%08x "
        "1472=[(%.4g,%.4g,%.4g)->(%.4g,%.4g,%.4g) fov=%.4g a=%.4g d=%.4g] "
        "1584=[(%.4g,%.4g,%.4g)->(%.4g,%.4g,%.4g) fov=%.4g a=%.4g d=%.4g] "
        "1768=(%.4g,%.4g,%.4g) ang=(%.4g,%.4g,%.4g) 1344=%.4g 1832=%.4g r1_60=(%.4g,%.4g,%.4g) r1_96=(%.4g,%.4g,%.4g)",
        tag, which ? which : "-", cam,
        mem::rf32(cam + 1472), mem::rf32(cam + 1476), mem::rf32(cam + 1480),
        mem::rf32(cam + 1484), mem::rf32(cam + 1488), mem::rf32(cam + 1492),
        mem::rf32(cam + 1508), mem::rf32(cam + 1516), mem::rf32(cam + 1520),
        mem::rf32(cam + 1584), mem::rf32(cam + 1588), mem::rf32(cam + 1592),
        mem::rf32(cam + 1596), mem::rf32(cam + 1600), mem::rf32(cam + 1604),
        mem::rf32(cam + 1620), mem::rf32(cam + 1628), mem::rf32(cam + 1632),
        mem::rf32(cam + 1768), mem::rf32(cam + 1772), mem::rf32(cam + 1776),
        mem::rf32(cam + 1792), mem::rf32(cam + 1796), mem::rf32(cam + 1804),
        mem::rf32(cam + 1344), mem::rf32(cam + 1832),
        r1 ? mem::rf32(r1 + 60) : 0.f, r1 ? mem::rf32(r1 + 64) : 0.f, r1 ? mem::rf32(r1 + 68) : 0.f,
        r1 ? mem::rf32(r1 + 96) : 0.f, r1 ? mem::rf32(r1 + 100) : 0.f, r1 ? mem::rf32(r1 + 104) : 0.f);
}

thread_local CamSnapshot tl_prev;
thread_local bool tl_has_prev = false;
thread_local uint32_t tl_frame_nan_mask = 0;
std::atomic<int> g_nan_logs{0};

void check_point(int bit, const char* tag, uint32_t cam, uint32_t r1, bool chk_s60 = false, bool chk_s96 = false) {
    if (tl_frame_nan_mask != 0) return;
    const char* which = nullptr;
    bool bad = cam_has_nan(cam, &which);
    if (!bad && chk_s60 && r1) {
        float s60[3] = {mem::rf32(r1 + 60), mem::rf32(r1 + 64), mem::rf32(r1 + 68)};
        if (any_nan(s60, 3)) { bad = true; which = "stack60"; }
    }
    if (!bad && chk_s96 && r1) {
        float s96[3] = {mem::rf32(r1 + 96), mem::rf32(r1 + 100), mem::rf32(r1 + 104)};
        if (any_nan(s96, 3)) { bad = true; which = "stack96"; }
    }
    if (!bad) return;
    tl_frame_nan_mask = 1u << bit;
    int n = g_nan_logs.fetch_add(1, std::memory_order_relaxed);
    if (n < 12) {
        if (tl_has_prev && tl_prev.cam == cam) {
            LOG("[nan-trace] PREV_OK 1472=[(%.4g,%.4g,%.4g)->(%.4g,%.4g,%.4g) fov=%.4g] 1584=[(%.4g,%.4g,%.4g)->(%.4g,%.4g,%.4g) fov=%.4g] 1768=(%.4g,%.4g,%.4g) ang=(%.4g,%.4g,%.4g)",
                tl_prev.f1472[0], tl_prev.f1472[1], tl_prev.f1472[2],
                tl_prev.f1472[3], tl_prev.f1472[4], tl_prev.f1472[5], tl_prev.f1472[9],
                tl_prev.f1584[0], tl_prev.f1584[1], tl_prev.f1584[2],
                tl_prev.f1584[3], tl_prev.f1584[4], tl_prev.f1584[5], tl_prev.f1584[9],
                tl_prev.v1768[0], tl_prev.v1768[1], tl_prev.v1768[2],
                tl_prev.ang1792, tl_prev.ang1796, tl_prev.ang1804);
        }
        log_cam(tag, cam, r1, which);
    }
}
}  // namespace

// 0: Entry of f_02c07c84 (stwu r1, -640(r1)); c->r[3] is cam
BOTW_HOOK(0x02c07c84) {
    tl_frame_nan_mask = 0;
    check_point(0, "0_entry", c->r[3], 0);
}

// 1: Right after vtable[1316] camera update (lwz r0, 3396(r31))
BOTW_HOOK(0x02c07cec) { check_point(1, "1_after_vt1316", c->r[31], c->r[1]); }

// 2: After frsqrte block 0x02c07d18..0x02c07de0 (psq_l f0, 1584(r31))
BOTW_HOOK(0x02c07de4) { check_point(2, "2_after_frsqrte", c->r[31], c->r[1]); }

// 3: After 0x02e557d0 #1 on 1584->1604 into r1+60 (lfs f11, 1480(r31))
BOTW_HOOK(0x02c07e2c) { check_point(3, "3_after_sph1", c->r[31], c->r[1], true, false); }

// 4: After 0x02e557d0 #2 on 1472->1492 into r1+96 (lfs f13, 64(r1))
BOTW_HOOK(0x02c07e70) { check_point(4, "4_after_sph2", c->r[31], c->r[1], true, true); }

// 5: After storing 1792/1796/1804 (stfs f8, 1804(r31))
BOTW_HOOK(0x02c07ea8) { check_point(5, "5_after_angdiff", c->r[31], c->r[1]); }

// 6: After 0x02c08340..0x02c0837c fdivs+0x02b8f2c4 (lfs f1, 1344(r31))
BOTW_HOOK(0x02c08380) {
    if (tl_frame_nan_mask == 0 && !std::isfinite(c->f[2])) {
        tl_frame_nan_mask = 1u << 6;
        if (g_nan_logs.fetch_add(1, std::memory_order_relaxed) < 12) {
            LOG("[nan-trace] 6_f2_bad f2=%g f28=%g f29=%g f1=%g", c->f[2], c->f[28], c->f[29], c->f[1]);
            log_cam("6_f2_bad", c->r[31], c->r[1], "f2");
        }
        return;
    }
    check_point(6, "6_after_timer_lerp", c->r[31], c->r[1]);
}

// 7: After 0x02b8f350 + 0x02e5527c loop (lfs f7, 1768(r31))
BOTW_HOOK(0x02c08424) { check_point(7, "7_after_ang_lerp", c->r[31], c->r[1]); }

// 8: After 0x02c085ec bl 0x02c06368 (addi r3, r31, 3248)
BOTW_HOOK(0x02c085f0) { check_point(8, "8_after_build1584", c->r[31], c->r[1]); }

// 9: After 0x02c0878c bl 0x02e53de4 (or 29, 9, 29)
BOTW_HOOK(0x02c08790) {
    uint32_t r1 = c->r[1];
    if (tl_frame_nan_mask == 0) {
        float s[6] = {mem::rf32(r1 + 224), mem::rf32(r1 + 228), mem::rf32(r1 + 232),
                      mem::rf32(r1 + 320), mem::rf32(r1 + 324), mem::rf32(r1 + 328)};
        if (!std::isfinite(c->f[1]) || any_nan(s, 6)) {
            tl_frame_nan_mask = 1u << 9;
            if (g_nan_logs.fetch_add(1, std::memory_order_relaxed) < 12) {
                LOG("[nan-trace] 9_after_de4 f1=%g s224=(%.4g,%.4g,%.4g) e320=(%.4g,%.4g,%.4g)",
                    c->f[1], s[0], s[1], s[2], s[3], s[4], s[5]);
                log_cam("9_after_de4", c->r[31], r1, "de4_or_e98");
            }
            return;
        }
    }
    check_point(9, "9_after_de4", c->r[31], r1);
}

// 10: After shape_cast bctrl + 0x02c08894 hit update (psq_l f9, 1596(r31))
BOTW_HOOK(0x02c0892c) { check_point(10, "10_after_hit_upd", c->r[31], c->r[1]); }

// 11: After 0x02c08994 + 1596..1604 update (addi r3, r31, 1584)
BOTW_HOOK(0x02c089e8) {
    check_point(11, "11_after_post_sph", c->r[31], c->r[1]);
    if (tl_frame_nan_mask == 0) {
        uint32_t cam = c->r[31];
        tl_prev.cam = cam;
        for (int i = 0; i < 14; i++) {
            tl_prev.f1472[i] = mem::rf32(cam + 1472 + i * 4);
            tl_prev.f1584[i] = mem::rf32(cam + 1584 + i * 4);
        }
        for (int i = 0; i < 3; i++) tl_prev.v1768[i] = mem::rf32(cam + 1768 + i * 4);
        tl_prev.ang1792 = mem::rf32(cam + 1792);
        tl_prev.ang1796 = mem::rf32(cam + 1796);
        tl_prev.ang1804 = mem::rf32(cam + 1804);
        tl_has_prev = true;
    }
}

// 12: Near end of f_02c07c84 after 0x02c08b84..0x02c08c1c stores (stfs f6, 1464(r31))
BOTW_HOOK(0x02c08c1c) { check_point(12, "12_tail", c->r[31], c->r[1]); }

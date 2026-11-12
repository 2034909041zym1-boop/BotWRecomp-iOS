// Temporary diagnostic hooks for the tree-top 1-FPS freeze inside hkpWorld BVH cast (f_04147e30).
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>

#include "hooks.h"

namespace {
struct CallerCastState {
    bool valid = false;
    uint64_t seq = 0;
    float start[3]{};
    float end[3]{};
    float shape_a[3]{};
    float shape_b[3]{};
};

struct BvhDiagState {
    int depth = 0;
    uint32_t iters = 0;
    uint32_t leaves = 0;
    uint32_t r3 = 0;
    uint32_t r4 = 0;
    uint32_t r5 = 0;
    uint32_t lr = 0;
    uint32_t bad_leaf_hits = 0;
    float start[3]{};
    float end[3]{};
    float half[3]{};
    std::chrono::steady_clock::time_point t0{};
};

thread_local CallerCastState tl_caller;
thread_local BvhDiagState tl_bvh;
std::atomic<uint64_t> g_last_log_ms{0};
std::atomic<uint64_t> g_seq{0};

bool rate_limit(uint64_t min_interval_ms) {
    uint64_t now = (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::steady_clock::now().time_since_epoch())
                       .count();
    uint64_t prev = g_last_log_ms.load(std::memory_order_relaxed);
    if (now - prev < min_interval_ms) return false;
    return g_last_log_ms.compare_exchange_strong(prev, now, std::memory_order_relaxed);
}
}  // namespace

// f_02c07c84 just before bctrl to f_034d171c (addi r3, r1, 440)
BOTW_HOOK(0x02c087f0) {
    uint32_t r1 = c->r[1];
    tl_caller.valid = true;
    tl_caller.seq = g_seq.fetch_add(1, std::memory_order_relaxed) + 1;
    for (int i = 0; i < 3; i++) {
        tl_caller.start[i] = mem::rf32(r1 + 224 + i * 4);
        tl_caller.end[i] = mem::rf32(r1 + 320 + i * 4);
        tl_caller.shape_a[i] = mem::rf32(r1 + 440 + 8 + i * 4);
        tl_caller.shape_b[i] = mem::rf32(r1 + 440 + 32 + i * 4);
    }
}

// Entry of f_04147e30 (or r11, r1, r1)
BOTW_HOOK(0x04147e30) {
    if (tl_bvh.depth++ > 0) return;
    tl_bvh.iters = 0;
    tl_bvh.leaves = 0;
    tl_bvh.bad_leaf_hits = 0;
    tl_bvh.r3 = c->r[3];
    tl_bvh.r4 = c->r[4];
    tl_bvh.r5 = c->r[5];
    tl_bvh.lr = c->lr;
    tl_bvh.t0 = std::chrono::steady_clock::now();
    uint32_t r4 = c->r[4];
    for (int i = 0; i < 3; i++) {
        tl_bvh.start[i] = mem::rf32(r4 + i * 4);
        tl_bvh.end[i] = mem::rf32(r4 + 16 + i * 4);
        tl_bvh.half[i] = mem::rf32(r4 + 32 + i * 4);
    }
}

// BVH loop head inside f_04147e30 (lhz r9, 30(r6))
BOTW_HOOK(0x04148418) {
    if (tl_bvh.depth != 1) return;
    uint32_t n = ++tl_bvh.iters;
    if (n == 50000 || (n > 50000 && (n % 2000000) == 0)) {
        if (!rate_limit(1500)) return;
        uint32_t r1 = c->r[1];
        uint32_t r6 = c->r[6];
        double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - tl_bvh.t0).count();
        LOG("[tree-diag] MID iters=%u leaves=%u %.1fms lr=%08x r27=%u sp_cnt=%u r6=%08x (c0=%u c1=%u) "
            "ray_start=(%.4g,%.4g,%.4g) ray_end=(%.4g,%.4g,%.4g) half=(%.4g,%.4g,%.4g) "
            "orig=(%.4g,%.4g,%.4g) inv=(%.4g,%.4g,%.4g) frac=%.6g node_min=(%.4g,%.4g,%.4g) node_max=(%.4g,%.4g,%.4g)",
            n, tl_bvh.leaves, ms, tl_bvh.lr, c->r[27], mem::r32(r1 + 24), r6,
            mem::r16(r6 + 28), mem::r16(r6 + 30),
            tl_bvh.start[0], tl_bvh.start[1], tl_bvh.start[2],
            tl_bvh.end[0], tl_bvh.end[1], tl_bvh.end[2],
            tl_bvh.half[0], tl_bvh.half[1], tl_bvh.half[2],
            mem::rf32(r1 + 128), mem::rf32(r1 + 132), mem::rf32(r1 + 136),
            mem::rf32(r1 + 160), mem::rf32(r1 + 164), mem::rf32(r1 + 168),
            mem::rf32(r1 + 156),
            mem::rf32(r6 + 0), mem::rf32(r6 + 4), mem::rf32(r6 + 8),
            mem::rf32(r6 + 16), mem::rf32(r6 + 20), mem::rf32(r6 + 24));
        if (tl_caller.valid) {
            LOG("[tree-diag] CALLER seq=%llu start=(%.4g,%.4g,%.4g) end=(%.4g,%.4g,%.4g) sa=(%.4g,%.4g,%.4g) sb=(%.4g,%.4g,%.4g)",
                (unsigned long long)tl_caller.seq,
                tl_caller.start[0], tl_caller.start[1], tl_caller.start[2],
                tl_caller.end[0], tl_caller.end[1], tl_caller.end[2],
                tl_caller.shape_a[0], tl_caller.shape_a[1], tl_caller.shape_a[2],
                tl_caller.shape_b[0], tl_caller.shape_b[1], tl_caller.shape_b[2]);
        }
    }
}

// Right after leaf callback bctrl inside f_04147e30 (ps_merge00 f13, f1, f1)
BOTW_HOOK(0x041488b0) {
    if (tl_bvh.depth != 1) return;
    tl_bvh.leaves++;
    if (!std::isfinite(c->f[1]) && tl_bvh.bad_leaf_hits < 3) {
        tl_bvh.bad_leaf_hits++;
        LOG("[tree-diag] BAD_LEAF_RET f1=%g bits=%016llx iters=%u leaves=%u",
            c->f[1], (unsigned long long)dbits(c->f[1]), tl_bvh.iters, tl_bvh.leaves);
    }
}

// Exit of f_04147e30 (addi r11, r29, 72)
BOTW_HOOK(0x041489e8) {
    if (--tl_bvh.depth != 0) return;
    double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - tl_bvh.t0).count();
    if (tl_bvh.iters >= 5000 || ms >= 30.0) {
        if (rate_limit(1000)) {
            uint32_t r1 = c->r[1];
            LOG("[tree-diag] DONE iters=%u leaves=%u %.1fms bad_ret=%u lr=%08x frac=%.6g "
                "ray_start=(%.4g,%.4g,%.4g) ray_end=(%.4g,%.4g,%.4g) half=(%.4g,%.4g,%.4g)",
                tl_bvh.iters, tl_bvh.leaves, ms, tl_bvh.bad_leaf_hits, tl_bvh.lr,
                mem::rf32(r1 + 156),
                tl_bvh.start[0], tl_bvh.start[1], tl_bvh.start[2],
                tl_bvh.end[0], tl_bvh.end[1], tl_bvh.end[2],
                tl_bvh.half[0], tl_bvh.half[1], tl_bvh.half[2]);
            if (tl_caller.valid) {
                LOG("[tree-diag] DONE_CALLER seq=%llu start=(%.4g,%.4g,%.4g) end=(%.4g,%.4g,%.4g) sa=(%.4g,%.4g,%.4g) sb=(%.4g,%.4g,%.4g)",
                    (unsigned long long)tl_caller.seq,
                    tl_caller.start[0], tl_caller.start[1], tl_caller.start[2],
                    tl_caller.end[0], tl_caller.end[1], tl_caller.end[2],
                    tl_caller.shape_a[0], tl_caller.shape_a[1], tl_caller.shape_a[2],
                    tl_caller.shape_b[0], tl_caller.shape_b[1], tl_caller.shape_b[2]);
            }
        }
    }
}

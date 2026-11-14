#include "os.h"

#include <unistd.h>

#include <atomic>
#include <chrono>
#include <thread>

#include "hle.h"
#include "host/window.h"
#include "thread.h"

std::atomic<uint64_t> g_game_frames{0};
std::atomic<bool> g_boot_world_ready{false};
std::atomic<uint64_t> g_gpu_hw_time_us{0};
std::atomic<uint64_t> g_gpu_render_passes{0};
std::atomic<uint64_t> g_gpu_blit_splits{0};
std::atomic<uint64_t> g_gpu_cmd_buffers{0};

extern "C" void botw_record_gpu_time_us(uint64_t us) { g_gpu_hw_time_us.fetch_add(us, std::memory_order_relaxed); }
extern "C" void botw_record_gpu_render_pass() { g_gpu_render_passes.fetch_add(1, std::memory_order_relaxed); }
extern "C" void botw_record_gpu_blit_split() { g_gpu_blit_splits.fetch_add(1, std::memory_order_relaxed); }
extern "C" void botw_record_gpu_cmd_buffer() { g_gpu_cmd_buffers.fetch_add(1, std::memory_order_relaxed); }

namespace os {
namespace {
std::atomic<int> g_exit_code{-1};
}

void init() {
    init_heaps();
    init_time();
}

uint32_t build_argv(const char* rpx) {
    uint32_t argv = mem::sys_alloc(16, 4);
    mem::w32(argv, mem::sys_strdup(rpx));
    mem::w32(argv + 4, 0);
    return argv;
}

void request_exit(int code) {
    g_exit_code = code;
    // give the log a moment, then leave without running host static destructors
    fflush(stderr);
    _exit(code);
}

void host_main_loop() {
    window::run();
    if (g_run_seconds) threads::dump();  // test runs: where every thread was at the end
    fflush(stderr);
    _exit(0);
}
}  // namespace os

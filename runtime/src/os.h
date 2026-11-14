// Cafe OS process-level state shared by the HLE libraries.
#pragma once
#include <stdint.h>

#include <string>

extern std::string g_game_root, g_user_root;
extern bool g_headless;
extern int g_run_seconds;
extern int g_target_fps;
#include <atomic>
extern std::atomic<uint64_t> g_game_frames;  // GX2SwapScanBuffers calls (watchdog)
extern std::atomic<bool> g_boot_world_ready;  // 3D world settled after boot; delays 60fps unlock until after world init
extern std::atomic<uint64_t> g_gpu_hw_time_us;  // accumulated Metal command buffer GPU execution time (microseconds)
extern std::atomic<uint64_t> g_gpu_render_passes;
extern std::atomic<uint64_t> g_gpu_blit_splits;
extern std::atomic<uint64_t> g_gpu_cmd_buffers;
extern uint64_t g_trace_from, g_trace_to;

extern "C" int botw_thermal_state();
extern "C" const char* botw_thermal_state_name(int state);
extern "C" bool botw_low_power_mode();
extern "C" void botw_record_gpu_time_us(uint64_t us);
extern "C" void botw_record_gpu_render_pass();
extern "C" void botw_record_gpu_blit_split();
extern "C" void botw_record_gpu_cmd_buffer();

namespace os {
void init();                              // heaps, FS, devices, per-library init
uint32_t build_argv(const char* rpx);     // guest argv array
void host_main_loop();                    // window/event pump on the host main thread
void request_exit(int code);
// default heaps (coreinit MEM)
uint32_t default_heap_alloc(uint32_t size, int32_t align);
void default_heap_free(uint32_t ptr);
uint32_t mem2_bound_start();
void init_heaps();
void init_time();
}  // namespace os

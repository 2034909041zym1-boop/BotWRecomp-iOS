// iOS has no gamepad sensors, so the phone itself stands in for the Wii U GamePad: BotW's gyro
// aiming and the gyro shrines read the motion values pad_hle.cpp fills from input/motion.cpp.
// Feed that filter from CoreMotion using the same samples a DualSense would report through SDL
// (rad/s gyro, m/s^2 accelerometer including gravity), so the Mahony filter and the VPAD unit
// conversion stay untouched.
//
// No axis remapping: SDL and CoreMotion both report the device-local frame, and the filter
// recovers the attitude from gravity, so landscape hold works out the same way it does with a
// physically rotated gamepad.
#import <CoreMotion/CoreMotion.h>
#import <Foundation/Foundation.h>

#include <atomic>

#include "../input/input.h"
#include "../log.h"

namespace {
CMMotionManager* g_manager = nil;
NSOperationQueue* g_queue = nil;
std::atomic<bool> g_running{false};
}  // namespace

extern "C" bool ios_motion_running() { return g_running.load(std::memory_order_relaxed); }

extern "C" void ios_motion_start() {
    if (g_running.load(std::memory_order_relaxed)) return;
    g_manager = [[CMMotionManager alloc] init];
    if (!g_manager.deviceMotionAvailable) {
        LOG("[motion] device motion is not available on this device");
        g_manager = nil;
        return;
    }
    g_manager.deviceMotionUpdateInterval = 1.0 / 60.0;
    g_queue = [[NSOperationQueue alloc] init];
    g_queue.maxConcurrentOperationCount = 1;
    g_queue.qualityOfService = NSQualityOfServiceUserInteractive;
    g_running.store(true, std::memory_order_relaxed);
    [g_manager startDeviceMotionUpdatesToQueue:g_queue
                                   withHandler:^(CMDeviceMotion* m, NSError* e) {
        if (!m || e) return;
        const uint64_t ts = (uint64_t)(m.timestamp * 1e9);
        const float gyro[3] = {(float)m.rotationRate.x, (float)m.rotationRate.y, (float)m.rotationRate.z};
        // CoreMotion reports gravity separately from the user acceleration; the SDL sensor stream
        // that motion.cpp consumes is the raw specific force, so add the two back together.
        const float acc[3] = {(float)(m.gravity.x + m.userAcceleration.x) * 9.81f,
                              (float)(m.gravity.y + m.userAcceleration.y) * 9.81f,
                              (float)(m.gravity.z + m.userAcceleration.z) * 9.81f};
        input::motion_sensor(true, gyro, ts);
        input::motion_sensor(false, acc, ts);
    }];
    LOG("[motion] CoreMotion device motion started");
}

extern "C" void ios_motion_stop() {
    if (!g_running.exchange(false)) return;
    if (g_manager) [g_manager stopDeviceMotionUpdates];
    g_manager = nil;
    g_queue = nil;
    input::motion_reset();
    LOG("[motion] CoreMotion device motion stopped");
}

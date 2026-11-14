// iOS has no gamepad sensors, so the phone itself stands in for the Wii U GamePad: BotW's gyro
// aiming and the gyro shrines read the motion values pad_hle.cpp fills from input/motion.cpp.
// Feed that filter from CoreMotion using the same samples a DualSense would report through SDL
// (rad/s gyro, m/s^2 accelerometer), so the Mahony filter and the VPAD unit conversion stay
// untouched.
//
// Sign convention: an accelerometer at rest reports the specific force (pointing away from the
// ground), while CMMotionManager splits that into gravity (pointing down) and userAcceleration.
// So the sample SDL would hand over is (userAcceleration - gravity), not (gravity + userAcceleration);
// feeding the gravity direction inverted the whole attitude and swapped pitch/roll.
#import <CoreMotion/CoreMotion.h>
#import <Foundation/Foundation.h>

#include <atomic>
#include <cmath>

#include "../input/input.h"
#include "../log.h"

namespace {
CMMotionManager* g_manager = nil;
NSOperationQueue* g_queue = nil;
std::atomic<bool> g_running{false};
std::atomic<uint32_t> g_samples{0};
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
    g_samples.store(0, std::memory_order_relaxed);
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
        // Specific force at rest points up; gravity points down, so subtract it.
        const float acc[3] = {(float)(m.userAcceleration.x - m.gravity.x) * 9.81f,
                              (float)(m.userAcceleration.y - m.gravity.y) * 9.81f,
                              (float)(m.userAcceleration.z - m.gravity.z) * 9.81f};

        if ((g_samples.fetch_add(1, std::memory_order_relaxed) % 30) == 0) {
            input::Motion out;
            const bool has = input::motion(&out);
            LOG("[motion] up=(%.2f %.2f %.2f) gyro=(%.2f %.2f %.2f) out=(%.3f %.3f %.3f)", acc[0] / 9.81f,
                acc[1] / 9.81f, acc[2] / 9.81f, gyro[0], gyro[1], gyro[2],
                has ? out.gyro_change[0] : 0.0f, has ? out.gyro_change[1] : 0.0f,
                has ? out.gyro_change[2] : 0.0f);
        }

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

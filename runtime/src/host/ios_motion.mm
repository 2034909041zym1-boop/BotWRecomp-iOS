// iOS has no gamepad sensors, so the phone itself stands in for the Wii U GamePad: BotW's gyro
// aiming and the gyro shrines read the motion values pad_hle.cpp fills from input/motion.cpp.
// Feed that filter from CoreMotion using the same samples a DualSense would report through SDL
// (rad/s gyro, m/s^2 accelerometer), so the Mahony filter and the VPAD unit conversion stay
// untouched.
//
// Two transforms are needed before handing samples to motion.cpp:
// 1. Portrait -> Landscape: CoreMotion always reports in iPhone hardware (portrait) coordinates,
//    whereas the game runs in LandscapeLeft/LandscapeRight. Without rotating (x, y) by +-90 deg,
//    pitch and roll are swapped.
// 2. Hold-pose recentering: motion.cpp treats a controller lying flat on a table (up = (0, 0, 1))
//    as neutral. Whenever the game needs the gyroscope (VPADSetGyroDirection / aim triggers),
//    ios_motion_recenter() captures the phone's current gravity vector and rotates samples so the
//    player's current handheld posture maps to (0, 0, 1).
#import <CoreMotion/CoreMotion.h>
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

#include <atomic>
#include <cmath>
#include <cstring>
#include <mutex>

#include "../input/input.h"
#include "../log.h"

namespace {
CMMotionManager* g_manager = nil;
NSOperationQueue* g_queue = nil;
std::atomic<bool> g_running{false};
std::atomic<uint32_t> g_samples{0};
std::atomic<int> g_ui_orient{UIInterfaceOrientationLandscapeRight};

std::mutex g_cal_lock;
float g_latest_up[3] = {0, 0, 1};
bool g_has_up = false;
bool g_calibrated = false;
float g_rot[9] = {1, 0, 0, 0, 1, 0, 0, 0, 1};  // row-major rotation: landscape screen -> flat GamePad

void refresh_interface_orientation() {
    dispatch_async(dispatch_get_main_queue(), ^{
        for (UIScene* scene in UIApplication.sharedApplication.connectedScenes) {
            if ([scene isKindOfClass:[UIWindowScene class]]) {
                UIInterfaceOrientation o = ((UIWindowScene*)scene).interfaceOrientation;
                if (o == UIInterfaceOrientationLandscapeLeft || o == UIInterfaceOrientationLandscapeRight) {
                    g_ui_orient.store((int)o, std::memory_order_relaxed);
                    return;
                }
            }
        }
    });
}

void to_landscape(int orient, const float dev[3], float scr[3]) {
    if (orient == UIInterfaceOrientationLandscapeLeft) {
        // Dynamic Island on right: screen +X = +devY, screen +Y = -devX, screen +Z = +devZ
        scr[0] = dev[1];
        scr[1] = -dev[0];
        scr[2] = dev[2];
    } else {
        // LandscapeRight (Dynamic Island on left): screen +X = -devY, screen +Y = +devX, screen +Z = +devZ
        scr[0] = -dev[1];
        scr[1] = dev[0];
        scr[2] = dev[2];
    }
}

void apply_rot(const float R[9], const float in[3], float out[3]) {
    out[0] = R[0] * in[0] + R[1] * in[1] + R[2] * in[2];
    out[1] = R[3] * in[0] + R[4] * in[1] + R[5] * in[2];
    out[2] = R[6] * in[0] + R[7] * in[1] + R[8] * in[2];
}

// Shortest-arc rotation taking measured up vector `v` onto GamePad flat-up (0, 0, 1).
void solve_rotation_from(const float up[3], float R[9]) {
    float v[3] = {up[0], up[1], up[2]};
    const float len = sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    if (len < 1e-4f) {
        for (int i = 0; i < 9; i++) R[i] = (i % 4 == 0) ? 1.0f : 0.0f;
        return;
    }
    for (float& c : v) c /= len;
    // kUp = (0, 0, 1), so v x kUp = (v[1], -v[0], 0), dot(v, kUp) = v[2]
    const float ax[3] = {v[1], -v[0], 0.0f};
    const float s = sqrtf(ax[0] * ax[0] + ax[1] * ax[1]);
    const float c = v[2];
    if (s < 1e-4f) {
        if (c > 0.0f) {
            for (int i = 0; i < 9; i++) R[i] = (i % 4 == 0) ? 1.0f : 0.0f;
        } else {
            // Held face-down: half turn about screen X
            const float r[9] = {1, 0, 0, 0, -1, 0, 0, 0, -1};
            memcpy(R, r, sizeof r);
        }
        return;
    }
    const float a[3] = {ax[0] / s, ax[1] / s, 0.0f};
    const float t = 1.0f - c;
    R[0] = c + t * a[0] * a[0];
    R[1] = t * a[0] * a[1];
    R[2] = s * a[1];
    R[3] = t * a[1] * a[0];
    R[4] = c + t * a[1] * a[1];
    R[5] = -s * a[0];
    R[6] = -s * a[1];
    R[7] = s * a[0];
    R[8] = c;
}
}  // namespace

extern "C" bool ios_motion_running() { return g_running.load(std::memory_order_relaxed); }

extern "C" void ios_motion_recenter(const char* reason) {
    if (!g_running.load(std::memory_order_relaxed)) return;
    float up[3];
    bool has_up = false;
    {
        std::lock_guard<std::mutex> l(g_cal_lock);
        if (g_has_up) {
            memcpy(up, g_latest_up, sizeof up);
            solve_rotation_from(up, g_rot);
            g_calibrated = true;
            has_up = true;
        } else {
            g_calibrated = false;
        }
    }
    input::motion_reset(true);
    if (has_up) {
        LOG("[motion] recenter (%s): orient=%d up_scr=(%.2f %.2f %.2f)", reason ? reason : "?",
            g_ui_orient.load(std::memory_order_relaxed), up[0], up[1], up[2]);
    }
}

extern "C" void ios_motion_start() {
    if (g_running.load(std::memory_order_relaxed)) return;
    g_manager = [[CMMotionManager alloc] init];
    if (!g_manager.deviceMotionAvailable) {
        LOG("[motion] device motion is not available on this device");
        g_manager = nil;
        return;
    }
    refresh_interface_orientation();
    {
        std::lock_guard<std::mutex> l(g_cal_lock);
        g_has_up = false;
        g_calibrated = false;
        for (int i = 0; i < 9; i++) g_rot[i] = (i % 4 == 0) ? 1.0f : 0.0f;
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
        const int orient = g_ui_orient.load(std::memory_order_relaxed);

        const float dev_gyro[3] = {(float)m.rotationRate.x, (float)m.rotationRate.y, (float)m.rotationRate.z};
        // Specific force at rest points up; gravity points down, so subtract it.
        const float dev_acc[3] = {(float)(m.userAcceleration.x - m.gravity.x) * 9.81f,
                                  (float)(m.userAcceleration.y - m.gravity.y) * 9.81f,
                                  (float)(m.userAcceleration.z - m.gravity.z) * 9.81f};
        const float dev_up[3] = {-(float)m.gravity.x, -(float)m.gravity.y, -(float)m.gravity.z};

        float scr_gyro[3], scr_acc[3], scr_up[3];
        to_landscape(orient, dev_gyro, scr_gyro);
        to_landscape(orient, dev_acc, scr_acc);
        to_landscape(orient, dev_up, scr_up);

        float R[9];
        {
            std::lock_guard<std::mutex> l(g_cal_lock);
            memcpy(g_latest_up, scr_up, sizeof scr_up);
            g_has_up = true;
            if (!g_calibrated) {
                solve_rotation_from(scr_up, g_rot);
                g_calibrated = true;
            }
            memcpy(R, g_rot, sizeof R);
        }

        float gyro[3], acc[3];
        apply_rot(R, scr_gyro, gyro);
        apply_rot(R, scr_acc, acc);

        const uint32_t n = g_samples.fetch_add(1, std::memory_order_relaxed);
        if ((n % 30) == 0) {
            refresh_interface_orientation();
            input::Motion out;
            const bool has = input::motion(&out);
            LOG("[motion] o=%d scr_up=(%.2f %.2f %.2f) gp_up=(%.2f %.2f %.2f) gp_g=(%.2f %.2f %.2f) out=(%.3f %.3f %.3f)",
                orient, scr_up[0], scr_up[1], scr_up[2], acc[0] / 9.81f, acc[1] / 9.81f, acc[2] / 9.81f,
                gyro[0], gyro[1], gyro[2], has ? out.gyro_change[0] : 0.0f,
                has ? out.gyro_change[1] : 0.0f, has ? out.gyro_change[2] : 0.0f);
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

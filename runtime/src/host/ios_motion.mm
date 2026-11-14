// iOS has no gamepad sensors, so the phone itself stands in for the Wii U GamePad: BotW's gyro
// aiming and the gyro shrines read the motion values pad_hle.cpp fills from input/motion.cpp.
// Feed that filter from CoreMotion using the same samples a DualSense would report through SDL
// (rad/s gyro, m/s^2 accelerometer including gravity), so the Mahony filter and the VPAD unit
// conversion stay untouched.
//
// Hold-pose calibration: the GamePad is used flat and input/motion.cpp therefore treats the axis
// pointing at the sky as the camera's turn axis. A phone is held upright in landscape, which puts
// the sky axis on the short edge instead, so tilting left/right came out as camera pitch and the
// real pitch produced roll (which BotW ignores). Measure the pose once at startup and rotate the
// samples so the player's actual up axis becomes the filter's up (0, 1, 0); turning on the spot is
// then always yaw and the two tilts land on pitch and roll regardless of how the phone is held.
#import <CoreMotion/CoreMotion.h>
#import <Foundation/Foundation.h>

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

constexpr int kCalSamples = 60;          // ~1 s at 60 Hz, averaged while the phone is held still
const float kUp[3] = {0, 1, 0};          // the filter's "up" (Mahony starts from a pose aligned here)

std::mutex g_cal_lock;
float g_cal_sum[3] = {0, 0, 0};
int g_cal_count = 0;
bool g_calibrated = false;
float g_rot[9] = {1, 0, 0, 0, 1, 0, 0, 0, 1};  // row-major rotation applied to every sample

void apply_rot(const float R[9], const float in[3], float out[3]) {
    out[0] = R[0] * in[0] + R[1] * in[1] + R[2] * in[2];
    out[1] = R[3] * in[0] + R[4] * in[1] + R[5] * in[2];
    out[2] = R[6] * in[0] + R[7] * in[1] + R[8] * in[2];
}

// Shortest-arc rotation taking the measured up direction onto kUp.
void solve_rotation() {
    float v[3] = {g_cal_sum[0], g_cal_sum[1], g_cal_sum[2]};
    const float len = sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
    if (len < 0.5f) {  // not enough clean samples (phone kept moving); try again
        g_cal_sum[0] = g_cal_sum[1] = g_cal_sum[2] = 0;
        g_cal_count = 0;
        return;
    }
    for (float& c : v) c /= len;
    const float ax[3] = {v[1] * kUp[2] - v[2] * kUp[1], v[2] * kUp[0] - v[0] * kUp[2],
                         v[0] * kUp[1] - v[1] * kUp[0]};
    const float s = sqrtf(ax[0] * ax[0] + ax[1] * ax[1] + ax[2] * ax[2]);
    const float c = v[0] * kUp[0] + v[1] * kUp[1] + v[2] * kUp[2];
    if (s < 1e-4f) {
        if (c > 0) {
            for (int i = 0; i < 9; i++) g_rot[i] = (i % 4 == 0) ? 1.0f : 0.0f;
        } else {  // held upside down: half turn about x
            const float r[9] = {1, 0, 0, 0, -1, 0, 0, 0, -1};
            memcpy(g_rot, r, sizeof r);
        }
    } else {
        const float a[3] = {ax[0] / s, ax[1] / s, ax[2] / s};
        const float t = 1.0f - c;
        g_rot[0] = c + t * a[0] * a[0];
        g_rot[1] = t * a[0] * a[1] - s * a[2];
        g_rot[2] = t * a[0] * a[2] + s * a[1];
        g_rot[3] = t * a[1] * a[0] + s * a[2];
        g_rot[4] = c + t * a[1] * a[1];
        g_rot[5] = t * a[1] * a[2] - s * a[0];
        g_rot[6] = t * a[2] * a[0] - s * a[1];
        g_rot[7] = t * a[2] * a[1] + s * a[0];
        g_rot[8] = c + t * a[2] * a[2];
    }
    g_calibrated = true;
    LOG("[motion] hold pose calibrated: device up=(%.3f %.3f %.3f)", v[0], v[1], v[2]);
}

void reset_calibration() {
    std::lock_guard<std::mutex> l(g_cal_lock);
    g_cal_sum[0] = g_cal_sum[1] = g_cal_sum[2] = 0;
    g_cal_count = 0;
    g_calibrated = false;
    for (int i = 0; i < 9; i++) g_rot[i] = (i % 4 == 0) ? 1.0f : 0.0f;
}
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
    reset_calibration();
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
        float gyro[3] = {(float)m.rotationRate.x, (float)m.rotationRate.y, (float)m.rotationRate.z};
        // CoreMotion reports gravity separately from the user acceleration; the SDL sensor stream
        // that motion.cpp consumes is the raw specific force, so add the two back together.
        float acc[3] = {(float)(m.gravity.x + m.userAcceleration.x) * 9.81f,
                        (float)(m.gravity.y + m.userAcceleration.y) * 9.81f,
                        (float)(m.gravity.z + m.userAcceleration.z) * 9.81f};

        float R[9];
        bool calibrated;
        {
            std::lock_guard<std::mutex> l(g_cal_lock);
            if (!g_calibrated) {
                const float len = sqrtf(acc[0] * acc[0] + acc[1] * acc[1] + acc[2] * acc[2]);
                if (len > 5.0f && len < 15.0f) {  // ignore free fall / bumps while averaging
                    for (int i = 0; i < 3; i++) g_cal_sum[i] += acc[i] / len;
                    if (++g_cal_count >= kCalSamples) solve_rotation();
                }
            }
            memcpy(R, g_rot, sizeof R);
            calibrated = g_calibrated;
        }
        if (calibrated) {
            float g2[3], a2[3];
            apply_rot(R, gyro, g2);
            apply_rot(R, acc, a2);
            memcpy(gyro, g2, sizeof gyro);
            memcpy(acc, a2, sizeof acc);
        }

        if ((g_samples.fetch_add(1, std::memory_order_relaxed) % 30) == 0) {
            input::Motion out;
            const bool has = input::motion(&out);
            LOG("[motion] up=(%.2f %.2f %.2f) raw=(%.2f %.2f %.2f) out=(%.3f %.3f %.3f)", acc[0] / 9.81f,
                acc[1] / 9.81f, acc[2] / 9.81f, gyro[0], gyro[1], gyro[2], has ? out.gyro_change[0] : 0.0f,
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

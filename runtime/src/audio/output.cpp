#include "output.h"

#include <SDL3/SDL.h>
#include <math.h>

#include <algorithm>
#include <atomic>
#include <climits>
#include <mutex>
#include <vector>

#include "../log.h"
#include "../os.h"

extern bool g_mute;

namespace audio {
namespace {
std::once_flag g_once;
SDL_AudioStream* g_stream = nullptr;
std::vector<int16_t> g_stereo;
uint64_t g_frames_total = 0;
double g_sum_sq = 0;
uint64_t g_level_frames = 0;
uint64_t g_clipped_samples = 0;
uint64_t g_underruns = 0;
int g_min_queued = INT_MAX;
bool g_stream_active = false;
}  // namespace

void init() {
    std::call_once(g_once, [] {
        if (g_headless || g_mute) {
            LOG("audio: muted");
            return;
        }
#ifdef SDL_PLATFORM_IOS
        // Use 512-frame (~10.7 ms) CoreAudio buffers on iOS so each pull doesn't drain a thin queue.
        SDL_SetHint(SDL_HINT_AUDIO_DEVICE_SAMPLE_FRAMES, "512");
#endif
        if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
            LOG("audio: SDL audio init failed: %s", SDL_GetError());
            return;
        }
        SDL_AudioSpec spec{SDL_AUDIO_S16, 2, 48000};
        g_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, nullptr, nullptr);
        if (!g_stream) {
            LOG("audio: no output device: %s", SDL_GetError());
            return;
        }
        SDL_AudioSpec actual = spec;
        int dev_frames = 0;
        SDL_GetAudioDeviceFormat(SDL_GetAudioStreamDevice(g_stream), &actual, &dev_frames);
        SDL_ResumeAudioStreamDevice(g_stream);
        LOG("audio: output %d Hz stereo (%d device frames)", actual.freq, dev_frames);
    });
}

std::atomic<float> g_volume{1.0f};
void set_volume(float v) { g_volume = std::clamp(v, 0.0f, 1.0f); }

void push_tv(const int16_t* in, int frames, int channels) {
    const float vol = g_volume;
    g_stereo.resize((size_t)frames * 2);
    for (int i = 0; i < frames; i++) {
        int l, r;
        if (channels >= 6) {
            // L R C LFE Ls Rs -> stereo
            const int16_t* s = in + i * channels;
            l = (int)(s[0] + 0.707f * s[2] + 0.707f * s[4]);
            r = (int)(s[1] + 0.707f * s[2] + 0.707f * s[5]);
        } else if (channels == 2) {
            l = in[i * 2];
            r = in[i * 2 + 1];
        } else {
            l = r = in[i * channels];
        }
        int lv = (int)(l * vol), rv = (int)(r * vol);
        if (l >= 32767 || l <= -32768 || r >= 32767 || r <= -32768 || lv >= 32767 || lv <= -32768 || rv >= 32767 ||
            rv <= -32768)
            g_clipped_samples++;
        g_stereo[i * 2] = (int16_t)std::clamp(lv, -32768, 32767);
        g_stereo[i * 2 + 1] = (int16_t)std::clamp(rv, -32768, 32767);
        g_sum_sq += (double)l * l;
    }
    if (g_stream) {
        int q = SDL_GetAudioStreamQueued(g_stream) / 4;
        if (q > 0) g_stream_active = true;
        if (g_stream_active) {
            if (q == 0) g_underruns++;
            if (q < g_min_queued) g_min_queued = q;
        }
        // Avoid accumulating multi-second latency if iOS pauses AudioQueue in background.
        if (q < 48000 / 2) SDL_PutAudioStreamData(g_stream, g_stereo.data(), frames * 4);
    }
    g_frames_total += frames;
    g_level_frames += frames;
    if (g_level_frames >= 48000 * 10) {
        double rms = sqrt(g_sum_sq / g_level_frames) / 32768.0;
        LOG("audio: %.1f dBFS over the last 10 s (clipped %llu, underruns %llu, min_queued %d)",
            rms > 0 ? 20 * log10(rms) : -120.0, (unsigned long long)g_clipped_samples,
            (unsigned long long)g_underruns, g_min_queued == INT_MAX ? 0 : g_min_queued);
        g_level_frames = 0;
        g_sum_sq = 0;
        g_clipped_samples = 0;
        g_underruns = 0;
        g_min_queued = INT_MAX;
    }
}

int queued_frames() { return g_stream ? SDL_GetAudioStreamQueued(g_stream) / 4 : 0; }
}  // namespace audio

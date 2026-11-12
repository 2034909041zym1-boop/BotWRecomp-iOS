// Wii U h264.rpl HLE implemented on Apple VideoToolbox (Hardware H.264 -> NV12).
// Handles all 13 h264 exports imported by BotW (cutscene MP4/H.264 playback).
#include <CoreFoundation/CoreFoundation.h>
#include <CoreMedia/CoreMedia.h>
#include <CoreVideo/CoreVideo.h>
#include <VideoToolbox/VideoToolbox.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <deque>
#include <memory>
#include <mutex>
#include <span>
#include <unordered_map>
#include <vector>

#include "../hle.h"

extern "C" bool botw_is_background();
extern "C" void botw_wait_while_background();

namespace {

// Fixed layout offsets carved inside the guest-allocated decoder work memory block (0x10000 bytes).
// FPTR_OUTPUT expects H264DECOutput { u32 frameCount, u32 decodeResultsPtr, u32 userMemory }
// where each H264DECResult has double timestamp at +0x08 and u32 imageBuffer at +0x44.
constexpr uint32_t kWorkMemSize = 0x10000;
constexpr uint32_t kGuestOutputOff = 0x1000;
constexpr uint32_t kGuestResultPtrsOff = 0x1020;
constexpr uint32_t kGuestResultsBaseOff = 0x1100;
constexpr uint32_t kGuestResultStride = 0x80;
constexpr uint32_t kMaxBatchOutputs = 16;

struct BitReader {
    const uint8_t* data;
    size_t size;
    size_t bit_pos = 0;

    bool read_bits(int n, uint32_t& out) {
        if (bit_pos + (size_t)n > size * 8) return false;
        uint32_t v = 0;
        for (int i = 0; i < n; i++) {
            size_t b = bit_pos++;
            v = (v << 1) | ((data[b >> 3] >> (7 - (b & 7))) & 1);
        }
        out = v;
        return true;
    }

    bool read_u1(uint32_t& out) { return read_bits(1, out); }

    bool read_ue(uint32_t& out) {
        int zeros = 0;
        uint32_t bit = 0;
        while (read_u1(bit) && bit == 0) {
            if (++zeros > 31) return false;
        }
        if (bit == 0) return false;
        if (zeros == 0) {
            out = 0;
            return true;
        }
        uint32_t suffix = 0;
        if (!read_bits(zeros, suffix)) return false;
        out = ((1u << zeros) - 1u) + suffix;
        return true;
    }

    bool read_se(int32_t& out) {
        uint32_t v = 0;
        if (!read_ue(v)) return false;
        out = (v & 1u) ? (int32_t)((v + 1u) >> 1) : -(int32_t)(v >> 1);
        return true;
    }
};

struct SpsInfo {
    uint32_t width = 0;
    uint32_t height = 0;
    uint8_t profile_idc = 0;
    uint32_t max_num_ref_frames = 0;
    bool valid = false;
};

std::vector<std::span<const uint8_t>> split_annexb_nalus(const uint8_t* buf, size_t len) {
    std::vector<std::span<const uint8_t>> out;
    if (!buf || len < 4) return out;

    auto find_sc = [&](size_t from, size_t& sc_pos, size_t& sc_len) -> bool {
        for (size_t i = from; i + 2 < len; i++) {
            if (buf[i] == 0 && buf[i + 1] == 0) {
                if (buf[i + 2] == 1) {
                    sc_pos = i;
                    sc_len = 3;
                    return true;
                }
                if (i + 3 < len && buf[i + 2] == 0 && buf[i + 3] == 1) {
                    sc_pos = i;
                    sc_len = 4;
                    return true;
                }
            }
        }
        return false;
    };

    size_t sc_pos = 0, sc_len = 0;
    if (!find_sc(0, sc_pos, sc_len)) return out;

    size_t nalu_start = sc_pos + sc_len;
    while (nalu_start < len) {
        size_t next_pos = 0, next_len = 0;
        if (!find_sc(nalu_start, next_pos, next_len)) {
            if (len > nalu_start) out.emplace_back(buf + nalu_start, len - nalu_start);
            break;
        }
        if (next_pos > nalu_start) out.emplace_back(buf + nalu_start, next_pos - nalu_start);
        nalu_start = next_pos + next_len;
    }
    return out;
}

std::vector<uint8_t> nal_to_rbsp(std::span<const uint8_t> nal) {
    std::vector<uint8_t> rbsp;
    if (nal.size() <= 1) return rbsp;
    rbsp.reserve(nal.size() - 1);
    int zeros = 0;
    for (size_t i = 1; i < nal.size(); i++) {
        uint8_t b = nal[i];
        if (zeros == 2 && b == 0x03) {
            zeros = 0;
            continue;
        }
        zeros = (b == 0) ? zeros + 1 : 0;
        rbsp.push_back(b);
    }
    return rbsp;
}

SpsInfo parse_sps_nal(std::span<const uint8_t> nal) {
    SpsInfo info{};
    std::vector<uint8_t> rbsp = nal_to_rbsp(nal);
    if (rbsp.size() < 4) return info;

    BitReader br{rbsp.data(), rbsp.size()};
    uint32_t prof = 0, flags = 0, lvl = 0, sps_id = 0;
    if (!br.read_bits(8, prof) || !br.read_bits(8, flags) || !br.read_bits(8, lvl) || !br.read_ue(sps_id)) return info;
    info.profile_idc = (uint8_t)prof;

    uint32_t chroma_format_idc = 1;
    uint32_t separate_colour_plane_flag = 0;
    if (prof == 100 || prof == 110 || prof == 122 || prof == 244 || prof == 44 || prof == 83 || prof == 86 ||
        prof == 118 || prof == 128 || prof == 138 || prof == 139 || prof == 134 || prof == 135) {
        if (!br.read_ue(chroma_format_idc)) return info;
        if (chroma_format_idc == 3 && !br.read_u1(separate_colour_plane_flag)) return info;
        uint32_t dummy = 0, seq_scaling = 0;
        if (!br.read_ue(dummy) || !br.read_ue(dummy) || !br.read_u1(dummy) || !br.read_u1(seq_scaling)) return info;
        if (seq_scaling) {
            int count = (chroma_format_idc != 3) ? 8 : 12;
            for (int i = 0; i < count; i++) {
                uint32_t present = 0;
                if (!br.read_u1(present)) return info;
                if (present) {
                    int sz = (i < 6) ? 16 : 64;
                    int last = 8, next = 8;
                    for (int j = 0; j < sz; j++) {
                        if (next != 0) {
                            int32_t delta = 0;
                            if (!br.read_se(delta)) return info;
                            next = (last + delta + 256) % 256;
                        }
                        last = (next == 0) ? last : next;
                    }
                }
            }
        }
    }

    uint32_t log2_max_frame_num = 0, poc_type = 0, dummy = 0;
    if (!br.read_ue(log2_max_frame_num) || !br.read_ue(poc_type)) return info;
    if (poc_type == 0) {
        if (!br.read_ue(dummy)) return info;
    } else if (poc_type == 1) {
        int32_t sdummy = 0;
        uint32_t num_ref_in_cycle = 0;
        if (!br.read_u1(dummy) || !br.read_se(sdummy) || !br.read_se(sdummy) || !br.read_ue(num_ref_in_cycle))
            return info;
        for (uint32_t i = 0; i < num_ref_in_cycle; i++) {
            if (!br.read_se(sdummy)) return info;
        }
    }

    uint32_t mb_w_m1 = 0, mb_h_m1 = 0, frame_mbs_only = 0, direct_8x8 = 0, cropping = 0;
    if (!br.read_ue(info.max_num_ref_frames) || !br.read_u1(dummy)) return info;
    if (!br.read_ue(mb_w_m1) || !br.read_ue(mb_h_m1) || !br.read_u1(frame_mbs_only)) return info;
    if (!frame_mbs_only && !br.read_u1(dummy)) return info;
    if (!br.read_u1(direct_8x8) || !br.read_u1(cropping)) return info;

    uint32_t crop_l = 0, crop_r = 0, crop_t = 0, crop_b = 0;
    if (cropping) {
        if (!br.read_ue(crop_l) || !br.read_ue(crop_r) || !br.read_ue(crop_t) || !br.read_ue(crop_b)) return info;
    }

    uint32_t w = (mb_w_m1 + 1u) * 16u;
    uint32_t h = (2u - frame_mbs_only) * (mb_h_m1 + 1u) * 16u;
    if (cropping) {
        uint32_t chroma_array_type = separate_colour_plane_flag ? 0 : chroma_format_idc;
        uint32_t sub_w = (chroma_array_type == 1 || chroma_array_type == 2) ? 2u : 1u;
        uint32_t sub_h = (chroma_array_type == 1) ? 2u : 1u;
        uint32_t unit_x = sub_w;
        uint32_t unit_y = sub_h * (2u - frame_mbs_only);
        w -= (crop_l + crop_r) * unit_x;
        h -= (crop_t + crop_b) * unit_y;
    }
    info.width = w;
    info.height = h;
    info.valid = w > 0 && h > 0;
    return info;
}

struct DecodedPicture {
    CVPixelBufferRef pb = nullptr;
    double pts = 0.0;
    uint64_t seq = 0;
};

struct VtDecodeBatch {
    std::vector<DecodedPicture> pics;
    uint64_t* next_seq = nullptr;
};

struct VtCallbackReq {
    VtDecodeBatch* batch = nullptr;
    double pts = 0.0;
};

void on_vt_decompressed(void*, void* sourceFrameRefCon, OSStatus status, VTDecodeInfoFlags,
                        CVImageBufferRef imageBuffer, CMTime, CMTime) {
    auto* req = static_cast<VtCallbackReq*>(sourceFrameRefCon);
    if (!req || !req->batch) return;
    if (status == noErr && imageBuffer && CFGetTypeID(imageBuffer) == CVPixelBufferGetTypeID()) {
        CVPixelBufferRetain(imageBuffer);
        req->batch->pics.push_back({imageBuffer, req->pts, (*req->batch->next_seq)++});
    } else if (status != noErr) {
        LOG("h264: VideoToolbox decode callback error %d", (int)status);
    }
}

struct H264Decoder {
    uint32_t mem_addr = 0;
    uint32_t fptr_output = 0;
    uint32_t output_per_frame = 0;
    uint32_t user_memory = 0;

    uint32_t bitstream_addr = 0;
    uint32_t bitstream_len = 0;
    double bitstream_pts = 0.0;

    uint32_t width = 0;
    uint32_t height = 0;
    uint8_t profile_idc = 0;
    uint32_t max_num_ref_frames = 2;

    std::vector<uint8_t> sps;
    std::vector<uint8_t> pps;
    CMFormatDescriptionRef fmt = nullptr;
    VTDecompressionSessionRef session = nullptr;

    std::deque<uint32_t> held_guest_bufs;
    std::vector<DecodedPicture> reorder_queue;
    uint64_t decode_seq = 0;

    ~H264Decoder() { destroy_session(); }

    void release_queued_pics() {
        for (auto& d : reorder_queue) {
            if (d.pb) CVPixelBufferRelease(d.pb);
        }
        reorder_queue.clear();
        held_guest_bufs.clear();
    }

    void reset_vt_objects() {
        if (session) {
            VTDecompressionSessionWaitForAsynchronousFrames(session);
            VTDecompressionSessionInvalidate(session);
            CFRelease(session);
            session = nullptr;
        }
        if (fmt) {
            CFRelease(fmt);
            fmt = nullptr;
        }
    }

    void destroy_session() {
        reset_vt_objects();
        release_queued_pics();
    }

    bool ensure_session() {
        if (sps.empty() || pps.empty()) return session != nullptr;

        const uint8_t* const ptrs[2] = {sps.data(), pps.data()};
        const size_t sizes[2] = {sps.size(), pps.size()};
        CMFormatDescriptionRef new_fmt = nullptr;
        OSStatus st = CMVideoFormatDescriptionCreateFromH264ParameterSets(
            kCFAllocatorDefault, 2, ptrs, sizes, 4, &new_fmt);
        if (st != noErr || !new_fmt) {
            LOG("h264: CMVideoFormatDescriptionCreateFromH264ParameterSets failed: %d", (int)st);
            return false;
        }

        if (session && fmt && CMFormatDescriptionEqual(fmt, new_fmt)) {
            CFRelease(new_fmt);
            return true;
        }

        reset_vt_objects();

        CFNumberRef pix_fmt = nullptr;
        uint32_t nv12 = kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange;
        pix_fmt = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &nv12);
        CFStringRef keys[1] = {kCVPixelBufferPixelFormatTypeKey};
        CFTypeRef vals[1] = {pix_fmt};
        CFDictionaryRef attrs = CFDictionaryCreate(
            kCFAllocatorDefault, (const void**)keys, (const void**)vals, 1,
            &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        CFRelease(pix_fmt);

        VTDecompressionOutputCallbackRecord cb{};
        cb.decompressionOutputCallback = on_vt_decompressed;
        cb.decompressionOutputRefCon = this;

        st = VTDecompressionSessionCreate(kCFAllocatorDefault, new_fmt, nullptr, attrs, &cb, &session);
        CFRelease(attrs);
        if (st != noErr || !session) {
            LOG("h264: VTDecompressionSessionCreate failed: %d", (int)st);
            CFRelease(new_fmt);
            return false;
        }
        fmt = new_fmt;
        return true;
    }

    size_t reorder_limit() const {
        if (output_per_frame != 0 || profile_idc == 66) return 0;
        return std::clamp<size_t>(max_num_ref_frames + 1, 3, 6);
    }
};

std::mutex g_decoders_mu;
std::unordered_map<uint32_t, std::shared_ptr<H264Decoder>> g_decoders;

std::shared_ptr<H264Decoder> get_decoder(uint32_t mem_addr, bool create = false) {
    std::lock_guard<std::mutex> l(g_decoders_mu);
    auto it = g_decoders.find(mem_addr);
    if (it != g_decoders.end()) return it->second;
    if (!create) return nullptr;
    auto dec = std::make_shared<H264Decoder>();
    dec->mem_addr = mem_addr;
    g_decoders[mem_addr] = dec;
    return dec;
}

void write_nv12_to_guest(CVPixelBufferRef pb, uint32_t dst_addr, uint32_t target_w, uint32_t target_h) {
    CVPixelBufferLockBaseAddress(pb, kCVPixelBufferLock_ReadOnly);

    const uint32_t pb_w = (uint32_t)CVPixelBufferGetWidth(pb);
    const uint32_t pb_h = (uint32_t)CVPixelBufferGetHeight(pb);
    const uint32_t w = target_w ? target_w : pb_w;
    const uint32_t h = target_h ? target_h : pb_h;
    const uint32_t dst_pitch = (w + 255u) & ~255u;

    uint8_t* dst = (uint8_t*)mem::host(dst_addr);
    const uint8_t* src_y = (const uint8_t*)CVPixelBufferGetBaseAddressOfPlane(pb, 0);
    const size_t stride_y = CVPixelBufferGetBytesPerRowOfPlane(pb, 0);
    const uint32_t copy_w = std::min(w, pb_w);
    const uint32_t copy_h = std::min(h, (uint32_t)CVPixelBufferGetHeightOfPlane(pb, 0));
    for (uint32_t y = 0; y < copy_h; y++) {
        memcpy(dst + (size_t)y * dst_pitch, src_y + (size_t)y * stride_y, copy_w);
    }

    uint8_t* dst_uv = dst + (size_t)dst_pitch * h;
    const uint8_t* src_uv = (const uint8_t*)CVPixelBufferGetBaseAddressOfPlane(pb, 1);
    const size_t stride_uv = CVPixelBufferGetBytesPerRowOfPlane(pb, 1);
    const uint32_t copy_uv_h = std::min(h / 2u, (uint32_t)CVPixelBufferGetHeightOfPlane(pb, 1));
    const uint32_t copy_uv_bytes = std::min(copy_w, (uint32_t)stride_uv);
    for (uint32_t y = 0; y < copy_uv_h; y++) {
        memcpy(dst_uv + (size_t)y * dst_pitch, src_uv + (size_t)y * stride_uv, copy_uv_bytes);
    }

    CVPixelBufferUnlockBaseAddress(pb, kCVPixelBufferLock_ReadOnly);
}

struct EmittedFrame {
    uint32_t guest_buf = 0;
    double pts = 0.0;
};

// Drains reorder_queue down to `keep_count` oldest-PTS frames into `held_guest_bufs`,
// writing NV12 pixels into guest memory and returning the metadata needed for FPTR_OUTPUT.
std::vector<EmittedFrame> drain_ordered_frames(H264Decoder& dec, size_t keep_count) {
    std::vector<EmittedFrame> ready;
    while (dec.reorder_queue.size() > keep_count && !dec.held_guest_bufs.empty() && ready.size() < kMaxBatchOutputs) {
        auto best = std::min_element(dec.reorder_queue.begin(), dec.reorder_queue.end(),
                                     [](const DecodedPicture& a, const DecodedPicture& b) {
                                         if (a.pts != b.pts) return a.pts < b.pts;
                                         return a.seq < b.seq;
                                     });
        uint32_t guest_buf = dec.held_guest_bufs.front();
        dec.held_guest_bufs.pop_front();

        write_nv12_to_guest(best->pb, guest_buf, dec.width, dec.height);
        ready.push_back({guest_buf, best->pts});

        CVPixelBufferRelease(best->pb);
        dec.reorder_queue.erase(best);
    }
    return ready;
}

void invoke_output_callback(PPCContext* c, uint32_t mem_addr, uint32_t fptr_output, uint32_t user_memory,
                            const std::vector<EmittedFrame>& frames) {
    if (!fptr_output || frames.empty()) return;

    const uint32_t count = (uint32_t)std::min<size_t>(frames.size(), kMaxBatchOutputs);
    const uint32_t out_addr = mem_addr + kGuestOutputOff;
    const uint32_t ptrs_addr = mem_addr + kGuestResultPtrsOff;
    mem::w32(out_addr + 0x00, count);
    mem::w32(out_addr + 0x04, ptrs_addr);
    mem::w32(out_addr + 0x08, user_memory);

    for (uint32_t i = 0; i < count; i++) {
        const uint32_t res_addr = mem_addr + kGuestResultsBaseOff + i * kGuestResultStride;
        memset(mem::host(res_addr), 0, kGuestResultStride);
        mem::w32(res_addr + 0x00, 1);
        uint64_t pts_bits = 0;
        memcpy(&pts_bits, &frames[i].pts, sizeof(pts_bits));
        mem::w64(res_addr + 0x08, pts_bits);
        mem::w32(res_addr + 0x44, frames[i].guest_buf);
        mem::w32(ptrs_addr + i * 4, res_addr);
    }

    guest_call(c, fptr_output, {out_addr});
}

}  // namespace

// H264DECGetImageSize(const void* buffer, uint32 totalBytes, uint32 offset, uint32* outW, uint32* outH)
HLE_EXPORT(h264, H264DECGetImageSize) {
    const uint32_t buf_addr = ARG(0);
    const uint32_t total = ARG(1);
    const uint32_t off = ARG(2);
    const uint32_t out_w = ARG(3);
    const uint32_t out_h = ARG(4);
    if (!buf_addr || off >= total || !out_w || !out_h) {
        RET(-1);
        return;
    }

    const uint8_t* ptr = (const uint8_t*)mem::host(buf_addr + off);
    auto nalus = split_annexb_nalus(ptr, total - off);
    for (auto nal : nalus) {
        if (nal.empty() || (nal[0] & 0x1F) != 7) continue;
        SpsInfo info = parse_sps_nal(nal);
        if (info.valid) {
            mem::w32(out_w, info.width);
            mem::w32(out_h, info.height);
            LOG("h264: H264DECGetImageSize -> %ux%u (profile %u)", info.width, info.height, info.profile_idc);
            RET(0);
            return;
        }
    }
    LOG("h264: H264DECGetImageSize failed to parse SPS (bytes=%u)", total - off);
    RET(-1);
}

// H264DECMemoryRequirement(profile, level, maxWidth, maxHeight, uint32* outMemSize)
HLE_EXPORT(h264, H264DECMemoryRequirement) {
    if (ARG(4)) mem::w32(ARG(4), kWorkMemSize);
    RET(0);
}

// H264DECCheckMemSegmentation(void* mem, uint32 size): 0 = no bank segmentation error
HLE_EXPORT(h264, H264DECCheckMemSegmentation) { RET(0); }

// H264DECInitParam(uint32 memSize, void* mem)
HLE_EXPORT(h264, H264DECInitParam) {
    auto dec = get_decoder(ARG(1), true);
    dec->destroy_session();
    dec->fptr_output = 0;
    dec->output_per_frame = 0;
    dec->user_memory = 0;
    RET(0);
}

HLE_EXPORT(h264, H264DECSetParam_FPTR_OUTPUT) {
    if (auto dec = get_decoder(ARG(0))) dec->fptr_output = ARG(1);
    RET(0);
}

HLE_EXPORT(h264, H264DECSetParam_OUTPUT_PER_FRAME) {
    if (auto dec = get_decoder(ARG(0))) dec->output_per_frame = ARG(1);
    RET(0);
}

HLE_EXPORT(h264, H264DECSetParam_USER_MEMORY) {
    if (auto dec = get_decoder(ARG(0))) dec->user_memory = ARG(1);
    RET(0);
}

HLE_EXPORT(h264, H264DECOpen) { RET(0); }

HLE_EXPORT(h264, H264DECBegin) {
    if (auto dec = get_decoder(ARG(0))) dec->release_queued_pics();
    RET(0);
}

// H264DECSetBitstream(void* mem, const void* bitstream, uint32 length, double timestamp)
HLE_EXPORT(h264, H264DECSetBitstream) {
    auto dec = get_decoder(ARG(0));
    if (!dec) {
        RET(-1);
        return;
    }
    dec->bitstream_addr = ARG(1);
    dec->bitstream_len = ARG(2);
    dec->bitstream_pts = ARGF(0);
    RET(0);
}

// H264DECExecute(void* mem, void* frameBuffer)
// Returns 0x80 when `frameBuffer` is retained/emitted via FPTR_OUTPUT (BotW keeps the slot occupied until rendered).
HLE_EXPORT(h264, H264DECExecute) {
    const uint32_t mem_addr = ARG(0);
    const uint32_t frame_buf = ARG(1);
    auto dec = get_decoder(mem_addr);
    if (!dec || !dec->bitstream_addr || !dec->bitstream_len || !frame_buf) {
        RET(0);
        return;
    }

    const uint8_t* raw = (const uint8_t*)mem::host(dec->bitstream_addr);
    auto nalus = split_annexb_nalus(raw, dec->bitstream_len);

    std::vector<uint8_t> new_sps, new_pps;
    std::vector<uint8_t> avcc;
    avcc.reserve(dec->bitstream_len + 64);
    bool has_vcl = false;

    for (auto nal : nalus) {
        if (nal.empty()) continue;
        const uint8_t type = nal[0] & 0x1F;
        if (type == 7) {
            new_sps.assign(nal.begin(), nal.end());
            SpsInfo info = parse_sps_nal(nal);
            if (info.valid) {
                dec->width = info.width;
                dec->height = info.height;
                dec->profile_idc = info.profile_idc;
                dec->max_num_ref_frames = info.max_num_ref_frames;
            }
            continue;
        }
        if (type == 8) {
            new_pps.assign(nal.begin(), nal.end());
            continue;
        }
        if (type == 9 || type == 12) continue;  // skip AUD / filler
        if (type >= 1 && type <= 5) has_vcl = true;

        uint32_t sz = (uint32_t)nal.size();
        uint8_t len_be[4] = {(uint8_t)(sz >> 24), (uint8_t)(sz >> 16), (uint8_t)(sz >> 8), (uint8_t)sz};
        avcc.insert(avcc.end(), std::begin(len_be), std::end(len_be));
        avcc.insert(avcc.end(), nal.begin(), nal.end());
    }

    bool params_changed = (!new_sps.empty() && new_sps != dec->sps) || (!new_pps.empty() && new_pps != dec->pps);
    if (!new_sps.empty()) dec->sps = std::move(new_sps);
    if (!new_pps.empty()) dec->pps = std::move(new_pps);
    if (params_changed || !dec->session) {
        if (!dec->ensure_session()) {
            RET(0);
            return;
        }
    }

    if (!has_vcl || avcc.empty() || !dec->session || !dec->fmt) {
        RET(0);
        return;
    }

    CMBlockBufferRef block_buf = nullptr;
    OSStatus st = CMBlockBufferCreateWithMemoryBlock(
        kCFAllocatorDefault, nullptr, avcc.size(), kCFAllocatorDefault, nullptr, 0, avcc.size(), 0, &block_buf);
    if (st != noErr || !block_buf) {
        RET(0);
        return;
    }
    st = CMBlockBufferReplaceDataBytes(avcc.data(), block_buf, 0, avcc.size());
    if (st != noErr) {
        CFRelease(block_buf);
        RET(0);
        return;
    }

    CMSampleBufferRef sample_buf = nullptr;
    const size_t sample_size = avcc.size();
    st = CMSampleBufferCreateReady(kCFAllocatorDefault, block_buf, dec->fmt, 1, 0, nullptr, 1, &sample_size, &sample_buf);
    CFRelease(block_buf);
    if (st != noErr || !sample_buf) {
        RET(0);
        return;
    }

    if (botw_is_background()) botw_wait_while_background();

    VtDecodeBatch batch{ {}, &dec->decode_seq };
    VtCallbackReq req{ &batch, dec->bitstream_pts };
    VTDecodeInfoFlags info_flags = 0;
    @autoreleasepool {
        st = VTDecompressionSessionDecodeFrame(dec->session, sample_buf, 0, &req, &info_flags);
        if (st == kVTInvalidSessionErr || st == kVTVideoDecoderMalfunctionErr) {
            LOG("h264: VideoToolbox session invalidated (%d), recreating", (int)st);
            for (auto& d : batch.pics) {
                if (d.pb) CVPixelBufferRelease(d.pb);
            }
            batch.pics.clear();
            dec->reset_vt_objects();
            if (dec->ensure_session()) {
                info_flags = 0;
                st = VTDecompressionSessionDecodeFrame(dec->session, sample_buf, 0, &req, &info_flags);
            }
        }
    }
    CFRelease(sample_buf);

    if (st != noErr || batch.pics.empty()) {
        for (auto& d : batch.pics) {
            if (d.pb) CVPixelBufferRelease(d.pb);
        }
        RET(0);
        return;
    }

    // Maintain a strict 1:1 pairing between held guest frame buffers and queued decoded pictures.
    dec->held_guest_bufs.push_back(frame_buf);
    for (size_t i = 0; i < batch.pics.size(); i++) {
        if (i > 0 && dec->held_guest_bufs.size() < dec->reorder_queue.size() + batch.pics.size() - i) {
            CVPixelBufferRelease(batch.pics[i].pb);
            continue;
        }
        dec->reorder_queue.push_back(batch.pics[i]);
    }

    const uint32_t fptr = dec->fptr_output;
    const uint32_t user_mem = dec->user_memory;
    std::vector<EmittedFrame> ready = drain_ordered_frames(*dec, dec->reorder_limit());
    invoke_output_callback(c, mem_addr, fptr, user_mem, ready);

    RET(0x80);
}

// H264DECEnd(void* mem): flush remaining delayed frames in presentation order
HLE_EXPORT(h264, H264DECEnd) {
    const uint32_t mem_addr = ARG(0);
    auto dec = get_decoder(mem_addr);
    if (!dec) {
        RET(0);
        return;
    }
    if (dec->session) VTDecompressionSessionWaitForAsynchronousFrames(dec->session);
    const uint32_t fptr = dec->fptr_output;
    const uint32_t user_mem = dec->user_memory;
    while (!dec->reorder_queue.empty() && !dec->held_guest_bufs.empty()) {
        std::vector<EmittedFrame> ready = drain_ordered_frames(*dec, 0);
        if (ready.empty()) break;
        invoke_output_callback(c, mem_addr, fptr, user_mem, ready);
    }
    dec->release_queued_pics();
    RET(0);
}

HLE_EXPORT(h264, H264DECClose) {
    const uint32_t mem_addr = ARG(0);
    std::shared_ptr<H264Decoder> dec;
    {
        std::lock_guard<std::mutex> l(g_decoders_mu);
        auto it = g_decoders.find(mem_addr);
        if (it != g_decoders.end()) {
            dec = std::move(it->second);
            g_decoders.erase(it);
        }
    }
    if (dec) dec->destroy_session();
    RET(0);
}

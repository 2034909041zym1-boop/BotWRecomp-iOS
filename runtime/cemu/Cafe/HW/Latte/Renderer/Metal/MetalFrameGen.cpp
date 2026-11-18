#include "Cafe/HW/Latte/Renderer/Metal/MetalFrameGen.h"
#include "Cafe/HW/Latte/Renderer/Metal/MetalCommon.h"

#include <algorithm>

void log_write(const char* fmt, ...);

namespace
{
struct LocalAutoreleasePool
{
    NS::AutoreleasePool* pool;
    LocalAutoreleasePool() : pool(NS::AutoreleasePool::alloc()->init()) {}
    ~LocalAutoreleasePool() { if (pool) pool->release(); }
};

struct FGParams
{
    uint32_t w;
    uint32_t h;
    uint32_t dstW;
    uint32_t dstH;
    uint32_t srcW;
    uint32_t srcH;
    uint32_t isSRGB;
    float sceneThresh;
};

constexpr const char* kFrameGenMSL = R"(
#include <metal_stdlib>
using namespace metal;

struct FGParams {
    uint w;
    uint h;
    uint dstW;
    uint dstH;
    uint srcW;
    uint srcH;
    uint isSRGB;
    float sceneThresh;
};

inline half3 linear_to_srgb(half3 c) {
    c = clamp(c, half(0.0h), half(1.0h));
    half3 lo = c * half(12.92h);
    half3 hi = half(1.055h) * pow(c, half3(half(1.0h / 2.4h))) - half(0.055h);
    return select(hi, lo, c <= half(0.0031308h));
}

inline half rgb_luma(half3 c) {
    // Perceptual sqrt-luma gives better dark-scene gradient for optical flow
    return sqrt(max(dot(c, half3(0.299h, 0.587h, 0.114h)), half(0.0h)));
}

kernel void fg_copy_frame(
    texture2d<half, access::read>  src [[texture(0)]],
    texture2d<half, access::write> dst [[texture(1)]],
    constant FGParams& p [[buffer(0)]],
    uint2 gid [[thread_position_in_grid]])
{
    if (gid.x >= p.w || gid.y >= p.h) return;
    half4 c = src.read(gid);
    if (p.isSRGB != 0u) c.rgb = linear_to_srgb(c.rgb);
    dst.write(c, gid);
}

kernel void fg_downsample_luma1(
    texture2d<half, access::sample> src [[texture(0)]],
    texture2d<half, access::write>  dst [[texture(1)]],
    sampler samp [[sampler(0)]],
    constant FGParams& p [[buffer(0)]],
    uint2 gid [[thread_position_in_grid]])
{
    if (gid.x >= p.dstW || gid.y >= p.dstH) return;
    float2 uv = (float2(gid) + 0.5f) / float2(p.dstW, p.dstH);
    half3 c = src.sample(samp, uv).rgb;
    dst.write(half4(rgb_luma(c), 0.0h, 0.0h, 1.0h), gid);
}

kernel void fg_downsample_luma(
    texture2d<half, access::sample> src [[texture(0)]],
    texture2d<half, access::write>  dst [[texture(1)]],
    sampler samp [[sampler(0)]],
    constant FGParams& p [[buffer(0)]],
    uint2 gid [[thread_position_in_grid]])
{
    if (gid.x >= p.dstW || gid.y >= p.dstH) return;
    float2 uv = (float2(gid) + 0.5f) / float2(p.dstW, p.dstH);
    half l = src.sample(samp, uv).r;
    dst.write(half4(l, 0.0h, 0.0h, 1.0h), gid);
}

inline half patch_sad(
    texture2d<half, access::read> prevTex,
    texture2d<half, access::read> currTex,
    int2 p, int2 d, int w, int h)
{
    half sad = 0.0h;
    for (int oy = -1; oy <= 1; oy++) {
        int py = clamp(p.y + oy, 0, h - 1);
        int cy = clamp(p.y + oy + d.y, 0, h - 1);
        for (int ox = -1; ox <= 1; ox++) {
            int px = clamp(p.x + ox, 0, w - 1);
            int cx = clamp(p.x + ox + d.x, 0, w - 1);
            half lp = prevTex.read(uint2(px, py)).r;
            half lc = currTex.read(uint2(cx, cy)).r;
            sad += abs(lp - lc);
        }
    }
    return sad;
}

kernel void fg_flow_coarse(
    texture2d<half, access::read>  prevLuma [[texture(0)]],
    texture2d<half, access::read>  currLuma [[texture(1)]],
    texture2d<half, access::write> outFlow  [[texture(2)]],
    constant FGParams& p [[buffer(0)]],
    uint2 gid [[thread_position_in_grid]])
{
    if (gid.x >= p.dstW || gid.y >= p.dstH) return;
    int w = int(p.dstW), h = int(p.dstH);
    int2 pos = int2(gid);

    int2 bestD = int2(0, 0);
    half bestSad = patch_sad(prevLuma, currLuma, pos, bestD, w, h) * half(0.92h);

    // 2-step hierarchical search over [-3, +3]: 8 stride-2 probes + 8 stride-1 local probes (17 vs 49)
    for (int dy = -2; dy <= 2; dy += 2) {
        for (int dx = -2; dx <= 2; dx += 2) {
            if (dx == 0 && dy == 0) continue;
            half sad = patch_sad(prevLuma, currLuma, pos, int2(dx, dy), w, h);
            sad += half(0.008h) * half(abs(dx) + abs(dy));
            if (sad < bestSad) {
                bestSad = sad;
                bestD = int2(dx, dy);
            }
        }
    }
    int2 center = bestD;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            int2 d = center + int2(dx, dy);
            half sad = patch_sad(prevLuma, currLuma, pos, d, w, h);
            sad += half(0.008h) * half(abs(d.x) + abs(d.y));
            if (sad < bestSad) {
                bestSad = sad;
                bestD = d;
            }
        }
    }
    outFlow.write(half4(half2(bestD), bestSad / half(9.0h), 1.0h), gid);
}

kernel void fg_flow_refine(
    texture2d<half, access::read>   prevLuma   [[texture(0)]],
    texture2d<half, access::read>   currLuma   [[texture(1)]],
    texture2d<half, access::sample> coarseFlow [[texture(2)]],
    texture2d<half, access::write>  outFlow    [[texture(3)]],
    sampler samp [[sampler(0)]],
    constant FGParams& p [[buffer(0)]],
    uint2 gid [[thread_position_in_grid]])
{
    if (gid.x >= p.dstW || gid.y >= p.dstH) return;
    int w = int(p.dstW), h = int(p.dstH);
    int2 pos = int2(gid);
    float2 uv = (float2(gid) + 0.5f) / float2(p.dstW, p.dstH);

    int2 guess = int2(round(float2(coarseFlow.sample(samp, uv).xy) * 2.0f));
    int2 bestD = int2(0, 0);
    half bestSad = patch_sad(prevLuma, currLuma, pos, bestD, w, h) * half(0.93h);

    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int2 d = guess + int2(dx, dy);
            if (d.x == 0 && d.y == 0) continue;
            half sad = patch_sad(prevLuma, currLuma, pos, d, w, h);
            sad += half(0.004h) * half(abs(d.x) + abs(d.y));
            if (sad < bestSad) {
                bestSad = sad;
                bestD = d;
            }
        }
    }
    // Store in full-resolution pixel units (luma2 is 1/4 resolution)
    outFlow.write(half4(half2(bestD) * half(4.0h), bestSad / half(9.0h), 1.0h), gid);
}

kernel void fg_flow_smooth(
    texture2d<half, access::read>  currLuma [[texture(0)]],
    texture2d<half, access::read>  inFlow   [[texture(1)]],
    texture2d<half, access::write> outFlow  [[texture(2)]],
    constant FGParams& p [[buffer(0)]],
    uint2 gid [[thread_position_in_grid]])
{
    if (gid.x >= p.dstW || gid.y >= p.dstH) return;
    int w = int(p.dstW), h = int(p.dstH);
    int2 pos = int2(gid);

    half lc = currLuma.read(gid).r;
    half4 centerF = inFlow.read(gid);

    half2 sumF = half2(0.0h);
    half sumW = 0.0h;
    half avgSad = 0.0h;

    for (int oy = -1; oy <= 1; oy++) {
        int ny = clamp(pos.y + oy, 0, h - 1);
        for (int ox = -1; ox <= 1; ox++) {
            int nx = clamp(pos.x + ox, 0, w - 1);
            half ln = currLuma.read(uint2(nx, ny)).r;
            half4 fn = inFlow.read(uint2(nx, ny));
            half ew = exp(-abs(lc - ln) * half(18.0h));
            sumF += fn.xy * ew;
            sumW += ew;
            avgSad += fn.z;
        }
    }
    avgSad /= half(9.0h);
    half2 smoothed = (sumW > half(0.0001h)) ? (sumF / sumW) : centerF.xy;
    half conf = half(1.0h) - smoothstep(half(p.sceneThresh * 0.55f), half(p.sceneThresh), avgSad);
    smoothed *= conf;
    outFlow.write(half4(smoothed, conf, 1.0h), gid);
}

kernel void fg_interpolate(
    texture2d<half, access::sample> prevTex  [[texture(0)]],
    texture2d<half, access::sample> currTex  [[texture(1)]],
    texture2d<half, access::sample> flowTex  [[texture(2)]],
    texture2d<half, access::write>  outTex   [[texture(3)]],
    sampler samp [[sampler(0)]],
    constant FGParams& p [[buffer(0)]],
    uint2 gid [[thread_position_in_grid]])
{
    if (gid.x >= p.w || gid.y >= p.h) return;
    float2 invRes = 1.0f / float2(p.w, p.h);
    float2 uv = (float2(gid) + 0.5f) * invRes;

    half4 flow = flowTex.sample(samp, uv);
    float2 v = float2(flow.xy) * invRes;
    half conf = flow.z;

    float2 uv0 = clamp(uv - 0.5f * v, float2(0.0f), float2(1.0f));
    float2 uv1 = clamp(uv + 0.5f * v, float2(0.0f), float2(1.0f));
    half4 c0 = prevTex.sample(samp, uv0);
    half4 c1 = currTex.sample(samp, uv1);
    half4 cCurr = currTex.sample(samp, uv);

    // Guard against disocclusion double-images and static/zero-flow ghosting
    half flowMag = length(flow.xy);
    half diff = dot(abs(c0.rgb - c1.rgb), half3(0.3333h));
    half wWarp = (flowMag >= half(0.35h)) ? conf * (half(1.0h) - smoothstep(half(0.12h), half(0.30h), diff)) : half(0.0h);
    half4 blended = half(0.5h) * (c0 + c1);
    half4 out = mix(cCurr, blended, wWarp);

    if (p.isSRGB != 0u) out.rgb = linear_to_srgb(out.rgb);
    outTex.write(out, gid);
}
)";

bool isSRGBFormat(MTL::PixelFormat fmt)
{
    return fmt == MTL::PixelFormatRGBA8Unorm_sRGB || fmt == MTL::PixelFormatBGRA8Unorm_sRGB;
}
}  // namespace

MetalFrameGen::MetalFrameGen(MTL::Device* device) : m_device(device)
{
    if (m_device) m_device->retain();
}

MetalFrameGen::~MetalFrameGen()
{
    ReleaseTextures();
    if (m_psoCopy) m_psoCopy->release();
    if (m_psoLuma1) m_psoLuma1->release();
    if (m_psoLumaDown) m_psoLumaDown->release();
    if (m_psoFlowCoarse) m_psoFlowCoarse->release();
    if (m_psoFlowRefine) m_psoFlowRefine->release();
    if (m_psoFlowSmooth) m_psoFlowSmooth->release();
    if (m_psoInterpolate) m_psoInterpolate->release();
    if (m_linearSampler) m_linearSampler->release();
    if (m_library) m_library->release();
    if (m_device) m_device->release();
}

void MetalFrameGen::ResetHistory()
{
    m_hasPrev = false;
}

bool MetalFrameGen::EnsurePipelines()
{
    if (m_psoInterpolate) return true;
    if (m_initFailed || !m_device) return false;

    LocalAutoreleasePool pool;
    log_write("fg: EnsurePipelines begin (device=%p)", m_device);
    NS::Error* error = nullptr;
    m_library = m_device->newLibrary(ToNSString(kFrameGenMSL), nullptr, &error);
    log_write("fg: newLibrary done (lib=%p, err=%p)", m_library, error);
    if (!m_library)
    {
        log_write("MetalFrameGen: failed to compile MSL library: %s",
                  error ? error->localizedDescription()->utf8String() : "unknown");
        m_initFailed = true;
        return false;
    }

    auto makePSO = [&](const char* name) -> MTL::ComputePipelineState* {
        NS_STACK_SCOPED MTL::Function* fn = m_library->newFunction(ToNSString(name));
        if (!fn) { log_write("fg: missing fn %s", name); return nullptr; }
        NS::Error* psoErr = nullptr;
        auto* pso = m_device->newComputePipelineState(fn, &psoErr);
        if (!pso)
        {
            log_write("MetalFrameGen: failed to create PSO %s: %s", name,
                      psoErr ? psoErr->localizedDescription()->utf8String() : "unknown");
        }
        return pso;
    };

    m_psoCopy = makePSO("fg_copy_frame");
    m_psoLuma1 = makePSO("fg_downsample_luma1");
    m_psoLumaDown = makePSO("fg_downsample_luma");
    m_psoFlowCoarse = makePSO("fg_flow_coarse");
    m_psoFlowRefine = makePSO("fg_flow_refine");
    m_psoFlowSmooth = makePSO("fg_flow_smooth");
    m_psoInterpolate = makePSO("fg_interpolate");
    log_write("fg: all 7 PSOs created");

    NS_STACK_SCOPED auto* sampDesc = MTL::SamplerDescriptor::alloc()->init();
    sampDesc->setMinFilter(MTL::SamplerMinMagFilterLinear);
    sampDesc->setMagFilter(MTL::SamplerMinMagFilterLinear);
    sampDesc->setSAddressMode(MTL::SamplerAddressModeClampToEdge);
    sampDesc->setTAddressMode(MTL::SamplerAddressModeClampToEdge);
    m_linearSampler = m_device->newSamplerState(sampDesc);
    log_write("fg: sampler created (%p)", m_linearSampler);

    if (!m_psoCopy || !m_psoLuma1 || !m_psoLumaDown || !m_psoFlowCoarse ||
        !m_psoFlowRefine || !m_psoFlowSmooth || !m_psoInterpolate || !m_linearSampler)
    {
        m_initFailed = true;
        return false;
    }

    log_write("MetalFrameGen: optical flow compute pipelines initialized");
    return true;
}

void MetalFrameGen::ReleaseTextures()
{
    for (int i = 0; i < 2; i++)
    {
        if (m_frameView[i]) { m_frameView[i]->release(); m_frameView[i] = nullptr; }
        if (m_frameTex[i]) { m_frameTex[i]->release(); m_frameTex[i] = nullptr; }
        if (m_luma1[i]) { m_luma1[i]->release(); m_luma1[i] = nullptr; }
        if (m_luma2[i]) { m_luma2[i]->release(); m_luma2[i] = nullptr; }
        if (m_luma3[i]) { m_luma3[i]->release(); m_luma3[i] = nullptr; }
    }
    if (m_flow3) { m_flow3->release(); m_flow3 = nullptr; }
    if (m_flow2) { m_flow2->release(); m_flow2 = nullptr; }
    if (m_flowSmooth) { m_flowSmooth->release(); m_flowSmooth = nullptr; }
    if (m_interpView) { m_interpView->release(); m_interpView = nullptr; }
    if (m_interpTex) { m_interpTex->release(); m_interpTex = nullptr; }
    m_w = m_h = 0;
    m_hasPrev = false;
}

bool MetalFrameGen::EnsureTextures(uint32_t w, uint32_t h, bool isSRGB)
{
    if (m_w == w && m_h == h && m_isSRGB == isSRGB && m_interpTex)
        return true;

    ReleaseTextures();
    m_w = w;
    m_h = h;
    m_isSRGB = isSRGB;

    const uint32_t w2 = std::max(1u, w / 2), h2 = std::max(1u, h / 2);
    const uint32_t w4 = std::max(1u, w2 / 2), h4 = std::max(1u, h2 / 2);
    const uint32_t w8 = std::max(1u, w4 / 2), h8 = std::max(1u, h4 / 2);

    auto makeTex = [&](MTL::PixelFormat fmt, uint32_t tw, uint32_t th, MTL::TextureUsage usage) -> MTL::Texture* {
        auto* td = MTL::TextureDescriptor::texture2DDescriptor(fmt, tw, th, false);
        td->setStorageMode(MTL::StorageModePrivate);
        td->setUsage(usage);
        return m_device->newTexture(td);
    };

    const MTL::TextureUsage frameUsage = MTL::TextureUsageShaderRead | MTL::TextureUsageShaderWrite | MTL::TextureUsagePixelFormatView;
    const MTL::TextureUsage rwUsage = MTL::TextureUsageShaderRead | MTL::TextureUsageShaderWrite;

    for (int i = 0; i < 2; i++)
    {
        m_frameTex[i] = makeTex(MTL::PixelFormatRGBA8Unorm, w, h, frameUsage);
        if (!m_frameTex[i]) { ReleaseTextures(); return false; }
        if (isSRGB)
        {
            m_frameView[i] = m_frameTex[i]->newTextureView(MTL::PixelFormatRGBA8Unorm_sRGB);
            if (!m_frameView[i]) { ReleaseTextures(); return false; }
        }
        m_luma1[i] = makeTex(MTL::PixelFormatR8Unorm, w2, h2, rwUsage);
        m_luma2[i] = makeTex(MTL::PixelFormatR8Unorm, w4, h4, rwUsage);
        m_luma3[i] = makeTex(MTL::PixelFormatR8Unorm, w8, h8, rwUsage);
        if (!m_luma1[i] || !m_luma2[i] || !m_luma3[i]) { ReleaseTextures(); return false; }
    }

    m_flow3 = makeTex(MTL::PixelFormatRGBA16Float, w8, h8, rwUsage);
    m_flow2 = makeTex(MTL::PixelFormatRGBA16Float, w4, h4, rwUsage);
    m_flowSmooth = makeTex(MTL::PixelFormatRGBA16Float, w4, h4, rwUsage);
    m_interpTex = makeTex(MTL::PixelFormatRGBA8Unorm, w, h, frameUsage);
    if (!m_flow3 || !m_flow2 || !m_flowSmooth || !m_interpTex) { ReleaseTextures(); return false; }

    if (isSRGB)
    {
        m_interpView = m_interpTex->newTextureView(MTL::PixelFormatRGBA8Unorm_sRGB);
        if (!m_interpView) { ReleaseTextures(); return false; }
    }

    return true;
}

bool MetalFrameGen::Process(MTL::CommandBuffer* cmdBuf, MTL::Texture* srcTex, bool generate)
{
    if (!cmdBuf || !srcTex) return false;
    LocalAutoreleasePool pool;
    static int s_logN = 0;
    const bool dbg = (s_logN < 4);
    if (dbg) s_logN++;
    const uint32_t w = (uint32_t)srcTex->width();
    const uint32_t h = (uint32_t)srcTex->height();
    if (dbg) log_write("fg: Process #%d w=%u h=%u fmt=%u gen=%d", s_logN, w, h, (uint32_t)srcTex->pixelFormat(), (int)generate);
    if (w < 64 || h < 64) return false;

    const bool isSRGB = isSRGBFormat(srcTex->pixelFormat());
    if (!EnsurePipelines()) return false;
    if (!EnsureTextures(w, h, isSRGB)) { log_write("fg: EnsureTextures failed"); return false; }
    if (dbg) log_write("fg: textures ready");

    const int curr = m_currIdx;
    const int prev = 1 - m_currIdx;

    const uint32_t w2 = std::max(1u, w / 2), h2 = std::max(1u, h / 2);
    const uint32_t w4 = std::max(1u, w2 / 2), h4 = std::max(1u, h2 / 2);
    const uint32_t w8 = std::max(1u, w4 / 2), h8 = std::max(1u, h4 / 2);

    auto* enc = cmdBuf->computeCommandEncoder();
    if (!enc) return false;

    auto dispatch = [&](uint32_t tw, uint32_t th) {
        MTL::Size tg(16, 16, 1);
        MTL::Size grid((tw + 15) / 16, (th + 15) / 16, 1);
        enc->dispatchThreadgroups(grid, tg);
    };

    FGParams p{w, h, w, h, w, h, isSRGB ? 1u : 0u, 0.14f};

    // 1. Copy srcTex into m_frameTex[curr]
    enc->setComputePipelineState(m_psoCopy);
    enc->setTexture(srcTex, 0);
    enc->setTexture(m_frameTex[curr], 1);
    enc->setBytes(&p, sizeof(p), 0);
    dispatch(w, h);

    // 2. Build luma pyramid for curr
    p.dstW = w2; p.dstH = h2; p.srcW = w; p.srcH = h;
    enc->setComputePipelineState(m_psoLuma1);
    enc->setTexture(srcTex, 0);
    enc->setTexture(m_luma1[curr], 1);
    enc->setSamplerState(m_linearSampler, 0);
    enc->setBytes(&p, sizeof(p), 0);
    dispatch(w2, h2);

    p.dstW = w4; p.dstH = h4; p.srcW = w2; p.srcH = h2;
    enc->setComputePipelineState(m_psoLumaDown);
    enc->setTexture(m_luma1[curr], 0);
    enc->setTexture(m_luma2[curr], 1);
    enc->setSamplerState(m_linearSampler, 0);
    enc->setBytes(&p, sizeof(p), 0);
    dispatch(w4, h4);

    p.dstW = w8; p.dstH = h8; p.srcW = w4; p.srcH = h4;
    enc->setTexture(m_luma2[curr], 0);
    enc->setTexture(m_luma3[curr], 1);
    enc->setBytes(&p, sizeof(p), 0);
    dispatch(w8, h8);

    bool generated = false;
    if (generate && m_hasPrev)
    {
        // 3. Coarse optical flow at 1/8 resolution
        p.dstW = w8; p.dstH = h8;
        enc->setComputePipelineState(m_psoFlowCoarse);
        enc->setTexture(m_luma3[prev], 0);
        enc->setTexture(m_luma3[curr], 1);
        enc->setTexture(m_flow3, 2);
        enc->setBytes(&p, sizeof(p), 0);
        dispatch(w8, h8);

        // 4. Refine optical flow at 1/4 resolution
        p.dstW = w4; p.dstH = h4;
        enc->setComputePipelineState(m_psoFlowRefine);
        enc->setTexture(m_luma2[prev], 0);
        enc->setTexture(m_luma2[curr], 1);
        enc->setTexture(m_flow3, 2);
        enc->setTexture(m_flow2, 3);
        enc->setSamplerState(m_linearSampler, 0);
        enc->setBytes(&p, sizeof(p), 0);
        dispatch(w4, h4);

        // 5. Edge-aware smoothing + scene-cut confidence
        enc->setComputePipelineState(m_psoFlowSmooth);
        enc->setTexture(m_luma2[curr], 0);
        enc->setTexture(m_flow2, 1);
        enc->setTexture(m_flowSmooth, 2);
        enc->setBytes(&p, sizeof(p), 0);
        dispatch(w4, h4);

        // 6. Bidirectional warp + disocclusion guard into m_interpTex
        p.dstW = w; p.dstH = h;
        MTL::Texture* prevSampleTex = m_frameView[prev] ? m_frameView[prev] : m_frameTex[prev];
        MTL::Texture* currSampleTex = m_frameView[curr] ? m_frameView[curr] : m_frameTex[curr];
        enc->setComputePipelineState(m_psoInterpolate);
        enc->setTexture(prevSampleTex, 0);
        enc->setTexture(currSampleTex, 1);
        enc->setTexture(m_flowSmooth, 2);
        enc->setTexture(m_interpTex, 3);
        enc->setSamplerState(m_linearSampler, 0);
        enc->setBytes(&p, sizeof(p), 0);
        dispatch(w, h);

        generated = true;
    }

    enc->endEncoding();
    if (dbg) log_write("fg: Process #%d endEncoding ok (generated=%d)", s_logN, (int)generated);

    m_hasPrev = true;
    m_currIdx = 1 - m_currIdx;
    return generated;
}

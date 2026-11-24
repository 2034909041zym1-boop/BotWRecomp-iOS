// Auto-generated from cemu-project/cemu_graphic_packs src/BreathOfTheWild/Graphics/*_msl.txt
{0x083930adf539d9b7ULL, 0x0000000000000000ULL, GraphicPack2::GP_SHADER_TYPE::VERTEX, R"BOTW_MSL_END(#include <metal_stdlib>
using namespace metal;

// shader 083930adf539d9b7
// Shader dumped from BotW 208, using Cemu 2193a8c
// Based on e06e20b2efe87a84_0000000000000000_vs.txt
// Used for: Horizontal Ambient-Occlusion Blur

constant float resXScale = float($width) / float($gameWidth);

#define SET_POSITION(_v)                                                       \
    out.position = _v;                                                         \
    out.position.z = (out.position.z + out.position.w) / 2.0
// start of shader inputs/outputs, predetermined by Cemu. Do not touch
struct SupportBuffer {
    int4 remapped[1];
};

struct VertexIn {
    uint4 attrDataSem0 [[attribute(0)]];
    uint4 attrDataSem1 [[attribute(1)]];
};

struct VertexOut {
    float4 position [[position]] [[invariant]];
    float4 passParameterSem0 [[user(locn0)]];
};

// end of shader inputs/outputs
template <typename TextureT, typename CoordT>
float sampleCompareEmulate(TextureT tex, sampler samplr, CoordT coord,
                           float compareValue) {
    return compareValue < tex.sample(samplr, coord).x ? 1.0 : 0.0;
}
template <typename TextureT, typename CoordT>
float2 textureCalculateLod(TextureT tex, sampler samplr, CoordT coord) {
    float lod = tex.calculate_unclamped_lod(samplr, coord);
    return float2(floor(lod), fract(lod));
}
int clampFI32(int v) {
    if (v == 0x7FFFFFFF)
        return as_type<int>(1.0);
    else if (v == 0xFFFFFFFF)
        return as_type<int>(0.0);
    return as_type<int>(clamp(as_type<float>(v), 0.0, 1.0));
}
float mul_nonIEEE(float a, float b) {
    if (a == 0.0 || b == 0.0)
        return 0.0;
    return a * b;
}
vertex VertexOut main0(uint vid [[vertex_id]], uint iid [[instance_id]],
                       VertexIn in [[stage_in]],
                       constant SupportBuffer &supportBuffer [[buffer(0)]]) {
    VertexOut out;
    float4 R0f = float4(0.0);
    float4 R1f = float4(0.0);
    float4 R2f = float4(0.0);
    uint4 attrDecoder;
    float backupReg0f, backupReg1f, backupReg2f, backupReg3f, backupReg4f;
    float PV0fx = 0.0, PV0fy = 0.0, PV0fz = 0.0, PV0fw = 0.0, PV1fx = 0.0,
          PV1fy = 0.0, PV1fz = 0.0, PV1fw = 0.0;
    float PS0f = 0.0, PS1f = 0.0;
    float4 tempf = float4(0.0);
    float tempResultf;
    int tempResulti;
    int4 ARi = int4(0);
    bool predResult = true;
    R0f = float4(vid, 0, 0, iid);
    attrDecoder.xyz = in.attrDataSem0.xyz;
    attrDecoder.xyz =
        (attrDecoder.xyz >> 24) | ((attrDecoder.xyz >> 8) & 0xFF00) |
        ((attrDecoder.xyz << 8) & 0xFF0000) | ((attrDecoder.xyz << 24));
    attrDecoder.w = 0;
    R1f = float4(
        as_type<float>(int(attrDecoder.x)), as_type<float>(int(attrDecoder.y)),
        as_type<float>(int(attrDecoder.z)), as_type<float>(0x3f800000));
    attrDecoder.xy = in.attrDataSem1.xy;
    attrDecoder.xy = (attrDecoder.xy >> 24) | ((attrDecoder.xy >> 8) & 0xFF00) |
                     ((attrDecoder.xy << 8) & 0xFF0000) |
                     ((attrDecoder.xy << 24));
    attrDecoder.z = 0;
    attrDecoder.w = 0;
    R2f = float4(as_type<float>(int(attrDecoder.x)),
                 as_type<float>(int(attrDecoder.y)), as_type<float>(0),
                 as_type<float>(0x3f800000));
    // 0
    R1f.x = R1f.x;
    R1f.x *= 2.0;
    R1f.y = R1f.y;
    R1f.y *= 2.0;
    R1f.z = -(1.0);
    R1f.w = 1.0;
    R0f.x = R2f.x;
    // 1
    backupReg0f = R2f.x;
    R2f.x = R2f.y;
    R0f.y = R2f.y;
    R2f.z =
        backupReg0f + as_type<float>(supportBuffer.remapped[0].x) / resXScale;
    R2f.w = backupReg0f;
    R2f.y = backupReg0f +
            -(as_type<float>(supportBuffer.remapped[0].x) / resXScale);
    // export
    SET_POSITION(float4(R1f.x, R1f.y, R1f.z, R1f.w));
    // export
    out.passParameterSem0 = float4(R2f.x, R2f.y, R2f.z, R2f.w);
    // export
    // skipped export to semanticId 255
    return out;
}
)BOTW_MSL_END"},
{0x0f2b9ee517917425ULL, 0xff060c183153097bULL, GraphicPack2::GP_SHADER_TYPE::PIXEL, R"BOTW_MSL_END(#include <metal_stdlib>
using namespace metal;

// shader 0f2b9ee517917425
// Shader dumped from BotW 208, using Cemu 2193a8c
// Based on 0f2b9ee517917425_00000000000003c9_ps.txt
// Used for: Restoring the native BotW Anti-Aliasing implementation in inventory
// screen

constant float resX = float($width) / float($gameWidth);
constant float resY = float($height) / float($gameHeight);
#define fxaa $fxaa

#define SET_POSITION(_v)                                                       \
    out.position = _v;                                                         \
    out.position.z = (out.position.z + out.position.w) / 2.0

// start of shader inputs/outputs, predetermined by Cemu. Do not touch
struct SupportBuffer {
    int4 remapped[2];
    float2 fragCoordScale;
};

#define GET_FRAGCOORD()                                                        \
    float4(in.position.xy *supportBuffer.fragCoordScale.xy, in.position.z,     \
           1.0 / in.position.w)
struct FragmentIn {
    float4 position [[position]];
    float4 passParameterSem2 [[user(locn0)]];
};

struct FragmentOut {
    float4 passPixelColor0 [[color(0)]];
};

// end of shader inputs/outputs
template <typename TextureT, typename CoordT>
float sampleCompareEmulate(TextureT tex, sampler samplr, CoordT coord,
                           float compareValue) {
    return compareValue < tex.sample(samplr, coord).x ? 1.0 : 0.0;
}
template <typename TextureT, typename CoordT>
float2 textureCalculateLod(TextureT tex, sampler samplr, CoordT coord) {
    float lod = tex.calculate_unclamped_lod(samplr, coord);
    return float2(floor(lod), fract(lod));
}
int clampFI32(int v) {
    if (v == 0x7FFFFFFF)
        return as_type<int>(1.0);
    else if (v == 0xFFFFFFFF)
        return as_type<int>(0.0);
    return as_type<int>(clamp(as_type<float>(v), 0.0, 1.0));
}
float mul_nonIEEE(float a, float b) {
    if (a == 0.0 || b == 0.0)
        return 0.0;
    return a * b;
}
fragment FragmentOut main0(FragmentIn in [[stage_in]],
                           float2 pointCoord [[point_coord]],
                           bool frontFacing [[front_facing]],
                           constant SupportBuffer &supportBuffer [[buffer(0)]],
                           texture2d<float> tex0 [[texture(0)]],
                           sampler samplr0 [[sampler(0)]],
                           texture2d<float> tex1 [[texture(1)]],
                           sampler samplr1 [[sampler(1)]]) {
    FragmentOut out;
#if fxaa == 0
    out.passPixelColor0 = tex0.sample(samplr0, in.passParameterSem2.xy);
#else
    int4 R0i = int4(0);
    int4 R1i = int4(0);
    int4 R2i = int4(0);
    int4 R3i = int4(0);
    int4 R4i = int4(0);
    int4 R5i = int4(0);
    int4 R6i = int4(0);
    int4 R123i = int4(0);
    int4 R124i = int4(0);
    int4 R125i = int4(0);
    int4 R126i = int4(0);
    int4 R127i = int4(0);
    int backupReg0i, backupReg1i, backupReg2i, backupReg3i, backupReg4i;
    int PV0ix = 0, PV0iy = 0, PV0iz = 0, PV0iw = 0, PV1ix = 0, PV1iy = 0,
        PV1iz = 0, PV1iw = 0;
    int PS0i = 0, PS1i = 0;
    int4 tempi = int4(0);
    float tempResultf;
    int tempResulti;
    int4 ARi = int4(0);
    bool predResult = true;
    bool activeMaskStack[2];
    bool activeMaskStackC[3];
    activeMaskStack[0] = false;
    activeMaskStackC[0] = false;
    activeMaskStackC[1] = false;
    activeMaskStack[0] = true;
    activeMaskStackC[0] = true;
    activeMaskStackC[1] = true;
    R0i = as_type<int4>(in.passParameterSem2);
    if (activeMaskStackC[1] == true) {
        R2i.xzw = as_type<int3>(
            tex1.gather(samplr1, float2(0.0001) + float2(as_type<float>(R0i.x),
                                                         as_type<float>(R0i.y)))
                .xzw);
        R1i.xz = as_type<int2>(
            tex1.gather(samplr1, float2(0.0001) + float2(as_type<float>(R0i.z),
                                                         as_type<float>(R0i.w)))
                .xz);
        R3i.xyzw =
            as_type<int4>(tex0.sample(samplr0, float2(as_type<float>(R0i.x),
                                                      as_type<float>(R0i.y)))
                              .xyzw);
        R0i.w = as_type<int>(
            tex1.sample(samplr1,
                        float2(as_type<float>(R0i.x), as_type<float>(R0i.y)),
                        int2(1, -1))
                .x);
        R1i.y = as_type<int>(
            tex1.sample(samplr1,
                        float2(as_type<float>(R0i.x), as_type<float>(R0i.y)),
                        int2(-1, 1))
                .x);
    }
    if (activeMaskStackC[1] == true) {
        activeMaskStack[1] = activeMaskStack[0];
        activeMaskStackC[2] = activeMaskStackC[1];
        // 0
        PV0ix = as_type<int>(min(as_type<float>(R1i.x), as_type<float>(R1i.z)) /
                             resX);
        PV0iy = as_type<int>(max(as_type<float>(R2i.x), as_type<float>(R2i.z)) *
                             resY);
        PV0iz = as_type<int>(max(as_type<float>(R1i.x), as_type<float>(R1i.z)) *
                             resX);
        PV0iw = as_type<int>(min(as_type<float>(R2i.x), as_type<float>(R2i.z)));
        // 1
        PV1ix = as_type<int>(max(as_type<float>(PV0iz), as_type<float>(PV0iy)));
        PV1iy = as_type<int>(min(as_type<float>(PV0ix), as_type<float>(PV0iw)));
        // 2
        PV0iz = as_type<int>(min(as_type<float>(R2i.w), as_type<float>(PV1iy)));
        PV0iw = as_type<int>(max(as_type<float>(R2i.w), as_type<float>(PV1ix)));
        // 3
        PV1ix = as_type<int>(
            mul_nonIEEE(as_type<float>(PV0iw),
                        as_type<float>(supportBuffer.remapped[0].x)));
        R1i.w = as_type<int>(as_type<float>(PV0iw) + -(as_type<float>(PV0iz)));
        // 4
        R2i.y = as_type<int>(max(as_type<float>(PV1ix),
                                 as_type<float>(supportBuffer.remapped[0].y)));
        // 5
        predResult = (as_type<float>(R1i.w) >= as_type<float>(R2i.y));
        activeMaskStack[1] = predResult;
        activeMaskStackC[2] = predResult == true && activeMaskStackC[1] == true;
    } else {
        activeMaskStack[1] = false;
        activeMaskStackC[2] = false;
    }
    if (activeMaskStackC[2] == true) {
        // 0
        R126i.x = as_type<int>(as_type<float>(R2i.z) + as_type<float>(R0i.w));
        R126i.y = as_type<int>(as_type<float>(R1i.x) + as_type<float>(R1i.y));
        R126i.z = as_type<int>(as_type<float>(R2i.z) + as_type<float>(R1i.x));
        R127i.w = as_type<int>(as_type<float>(R2i.x) + as_type<float>(R1i.z));
        R127i.y = R1i.z;
        R127i.y = as_type<int>(as_type<float>(R127i.y) * 2.0);
        // 1
        PV1ix = R2i.x;
        PV1ix = as_type<int>(as_type<float>(PV1ix) * 2.0);
        PV1iy = as_type<int>(as_type<float>(R1i.x) + as_type<float>(R0i.w));
        R127i.z = as_type<int>(-(as_type<float>(R2i.w)) * 2.0 +
                               as_type<float>(R126i.z));
        PV1iw = R127i.w;
        PS1i = as_type<int>(as_type<float>(R2i.z) + as_type<float>(R1i.y));
        // 2
        R127i.x = as_type<int>(-(as_type<float>(R2i.w)) * 2.0 +
                               as_type<float>(PV1iw));
        R1i.y = R2i.z;
        PV0iz = as_type<int>(as_type<float>(PS1i) + -(as_type<float>(PV1ix)));
        PV0iw =
            as_type<int>(as_type<float>(PV1iy) + -(as_type<float>(R127i.y)));
        PS0i = R126i.x;
        // 3
        backupReg0i = R127i.z;
        R123i.x = as_type<int>(-(as_type<float>(R1i.x)) * 2.0 +
                               as_type<float>(R126i.y));
        PV1iy =
            as_type<int>(max(as_type<float>(PV0iw), -(as_type<float>(PV0iw))));
        R127i.z =
            as_type<int>(max(as_type<float>(PV0iz), -(as_type<float>(PV0iz))));
        R123i.w =
            as_type<int>(-(as_type<float>(R1i.y)) * 2.0 + as_type<float>(PS0i));
        PS1i = as_type<int>(
            max(as_type<float>(backupReg0i), -(as_type<float>(backupReg0i))));
        PS1i = as_type<int>(as_type<float>(PS1i) * 2.0);
        // 4
        backupReg0i = R126i.y;
        PV0ix = as_type<int>(
            max(as_type<float>(R123i.x), -(as_type<float>(R123i.x))));
        R126i.y = as_type<int>(
            max(as_type<float>(R123i.w), -(as_type<float>(R123i.w))));
        PV0iz = as_type<int>(
            max(as_type<float>(R127i.x), -(as_type<float>(R127i.x))));
        PV0iz = as_type<int>(as_type<float>(PV0iz) * 2.0);
        PV0iw = as_type<int>(as_type<float>(PV1iy) + as_type<float>(PS1i));
        R126i.w =
            as_type<int>(as_type<float>(R126i.x) + as_type<float>(backupReg0i));
        // 5
        PV1ix = as_type<int>(as_type<float>(PV0ix) + as_type<float>(PV0iz));
        PV1iy = as_type<int>(as_type<float>(R127i.w) + as_type<float>(R126i.z));
        PV1iy = as_type<int>(as_type<float>(PV1iy) * 2.0);
        R127i.z = as_type<int>(as_type<float>(R127i.z) + as_type<float>(PV0iw));
        R127i.y = as_type<int>(1.0 / as_type<float>(R1i.w));
        // 6
        PV0ix = as_type<int>(as_type<float>(R126i.w) + as_type<float>(PV1iy));
        PV0iy = as_type<int>(as_type<float>(R126i.y) + as_type<float>(PV1ix));
        // 7
        PV1ix = ((as_type<float>(PV0iy) >= as_type<float>(R127i.z)) ? -1 : 0);
        PV1iy =
            as_type<int>(as_type<float>(PV0ix) * as_type<float>(0x3daaaaab));
        // 8
        PV0ix = as_type<int>(as_type<float>(R2i.w) + -(as_type<float>(PV1iy)));
        R4i.z = ((PV1ix == 0) ? (int(0x3f800000)) : (0));
        R5i.w = ((PV1ix == 0) ? (0) : (int(0x3f800000)));
        // 9
        R5i.x = as_type<int>(
            mul_nonIEEE(as_type<float>(R5i.w),
                        as_type<float>(supportBuffer.remapped[1].x)));
        PV1iy = as_type<int>(
            mul_nonIEEE(as_type<float>(R4i.z),
                        as_type<float>(supportBuffer.remapped[1].x) / resX));
        PV1iz =
            as_type<int>(max(as_type<float>(PV0ix), -(as_type<float>(PV0ix))));
        R3i.w = as_type<int>(
            mul_nonIEEE(as_type<float>(R4i.z),
                        as_type<float>(supportBuffer.remapped[1].y) / resY));
        PS1i = as_type<int>(
            mul_nonIEEE(as_type<float>(R1i.x), as_type<float>(R4i.z)));
        // 10
        R127i.x = as_type<int>(as_type<float>(PV1iz) * as_type<float>(R127i.y));
        R127i.x = clampFI32(R127i.x);
        R127i.y = as_type<int>(
            mul_nonIEEE(as_type<float>(R5i.w),
                        as_type<float>(supportBuffer.remapped[1].y)) +
            as_type<float>(PV1iy));
        R127i.z = as_type<int>(
            mul_nonIEEE(as_type<float>(R1i.z), as_type<float>(R5i.w)) +
            as_type<float>(PS1i));
        PV0iw = as_type<int>(
            mul_nonIEEE(as_type<float>(R2i.z), as_type<float>(R4i.z)));
        // 11
        R124i.x = as_type<int>(
            mul_nonIEEE(as_type<float>(R2i.x), as_type<float>(R5i.w)) +
            as_type<float>(PV0iw));
        PV1iy =
            as_type<int>(-(as_type<float>(R2i.w)) + as_type<float>(R127i.z));
        R123i.w = as_type<int>(as_type<float>(R127i.x) * 6.0 + -15.0);
        // 12
        R125i.x =
            as_type<int>(max(as_type<float>(PV1iy), -(as_type<float>(PV1iy))));
        PV0iy =
            as_type<int>(-(as_type<float>(R2i.w)) + as_type<float>(R124i.x));
        R123i.z = as_type<int>(
            mul_nonIEEE(as_type<float>(R127i.x), as_type<float>(R123i.w)) +
            10.0);
        // 13
        R126i.x =
            as_type<int>(max(as_type<float>(PV0iy), -(as_type<float>(PV0iy))));
        PV1iy = as_type<int>(
            mul_nonIEEE(as_type<float>(R127i.x), as_type<float>(R123i.z)));
        // 14
        PV0ix = as_type<int>(
            mul_nonIEEE(as_type<float>(R127i.x), as_type<float>(PV1iy)));
        R126i.w =
            ((as_type<float>(R125i.x) >= as_type<float>(R126i.x)) ? -1 : 0);
        // 15
        R6i.x = as_type<int>(((R126i.w == 0) ? (as_type<float>(R127i.y))
                                             : (-(as_type<float>(R127i.y)))));
        R123i.z = ((R126i.w == 0) ? (R126i.x) : (R125i.x));
        PV1iw = as_type<int>(
            mul_nonIEEE(as_type<float>(R127i.x), as_type<float>(PV0ix)));
        // 16
        PV0ix = as_type<int>(
            mul_nonIEEE(as_type<float>(R4i.z), as_type<float>(R6i.x)));
        PV0ix = as_type<int>(as_type<float>(PV0ix) / 2.0);
        R123i.y = ((R126i.w == 0) ? (R124i.x) : (R127i.z));
        R3i.z = as_type<int>(as_type<float>(PV1iw) * 0.75);
        PV0iw = as_type<int>(
            mul_nonIEEE(as_type<float>(R5i.w), as_type<float>(R6i.x)));
        PV0iw = as_type<int>(as_type<float>(PV0iw) / 2.0);
        R4i.x = as_type<int>(as_type<float>(R123i.z) * 0.25);
        // 17
        PV1ix = as_type<int>(as_type<float>(R0i.x) + as_type<float>(PV0ix));
        PV1iz = as_type<int>(as_type<float>(R0i.y) + as_type<float>(PV0iw));
        R4i.w = as_type<int>(as_type<float>(R2i.w) + as_type<float>(R123i.y));
        R4i.w = as_type<int>(as_type<float>(R4i.w) / 2.0);
        // 18
        R3i.x = as_type<int>(-(as_type<float>(R5i.x)) + as_type<float>(PV1ix));
        R3i.y = as_type<int>(-(as_type<float>(R3i.w)) + as_type<float>(PV1iz));
        R1i.z = as_type<int>(as_type<float>(R5i.x) + as_type<float>(PV1ix));
        R1i.w = as_type<int>(as_type<float>(R3i.w) + as_type<float>(PV1iz));
        R2i.x = as_type<int>(as_type<float>(R2i.w) + -(as_type<float>(R4i.w)));
    }
    if (activeMaskStackC[2] == true) {
        R1i.y = as_type<int>(tex1.sample(samplr1, float2(as_type<float>(R3i.x),
                                                         as_type<float>(R3i.y)))
                                 .x);
        R1i.x = as_type<int>(tex1.sample(samplr1, float2(as_type<float>(R1i.z),
                                                         as_type<float>(R1i.w)))
                                 .x);
    }
    if (activeMaskStackC[2] == true) {
        // 0
        R2i.x = ((0.0 > as_type<float>(R2i.x)) ? -1 : 0);
        PV0iz = as_type<int>(-(as_type<float>(R4i.w)) + as_type<float>(R1i.x));
        PV0iw = as_type<int>(-(as_type<float>(R4i.w)) + as_type<float>(R1i.y));
        // 1
        PV1iy =
            as_type<int>(max(as_type<float>(PV0iw), -(as_type<float>(PV0iw))));
        PV1iz =
            as_type<int>(max(as_type<float>(PV0iz), -(as_type<float>(PV0iz))));
        // 2
        PV0ix = as_type<int>(as_type<float>(R4i.x) + -(as_type<float>(PV1iy)));
        PV0iy = as_type<int>(as_type<float>(R4i.x) + -(as_type<float>(PV1iz)));
        // 3
        R123i.z = ((as_type<float>(PV0iy) >= 0.0) ? (as_type<int>(1.0)) : (0));
        R123i.w = ((as_type<float>(PV0ix) >= 0.0) ? (as_type<int>(1.0)) : (0));
        // 4
        R3i.x = as_type<int>(
            mul_nonIEEE(-(as_type<float>(R5i.x)), as_type<float>(R123i.w)) +
            as_type<float>(R3i.x));
        R3i.y = as_type<int>(
            mul_nonIEEE(-(as_type<float>(R3i.w)), as_type<float>(R123i.w)) +
            as_type<float>(R3i.y));
        R2i.z = as_type<int>(
            mul_nonIEEE(as_type<float>(R5i.x), as_type<float>(R123i.z)) +
            as_type<float>(R1i.z));
        R2i.w = as_type<int>(
            mul_nonIEEE(as_type<float>(R3i.w), as_type<float>(R123i.z)) +
            as_type<float>(R1i.w));
    }
    if (activeMaskStackC[2] == true) {
        R1i.w = as_type<int>(tex1.sample(samplr1, float2(as_type<float>(R3i.x),
                                                         as_type<float>(R3i.y)))
                                 .x);
        R1i.z = as_type<int>(tex1.sample(samplr1, float2(as_type<float>(R2i.z),
                                                         as_type<float>(R2i.w)))
                                 .x);
    }
    if (activeMaskStackC[2] == true) {
        // 0
        PV0ix = as_type<int>(-(as_type<float>(R4i.w)) + as_type<float>(R1i.w));
        PV0iy = as_type<int>(-(as_type<float>(R4i.w)) + as_type<float>(R1i.z));
        // 1
        PV1ix =
            as_type<int>(max(as_type<float>(PV0iy), -(as_type<float>(PV0iy))));
        PV1iw =
            as_type<int>(max(as_type<float>(PV0ix), -(as_type<float>(PV0ix))));
        // 2
        PV0iz = as_type<int>(as_type<float>(R4i.x) + -(as_type<float>(PV1iw)));
        PV0iw = as_type<int>(as_type<float>(R4i.x) + -(as_type<float>(PV1ix)));
        // 3
        R123i.z = ((as_type<float>(PV0iw) >= 0.0) ? (as_type<int>(1.0)) : (0));
        R123i.w = ((as_type<float>(PV0iz) >= 0.0) ? (as_type<int>(1.0)) : (0));
        // 4
        R1i.x = as_type<int>(
            mul_nonIEEE(-(as_type<float>(R5i.x)), as_type<float>(R123i.w)) +
            as_type<float>(R3i.x));
        R1i.y = as_type<int>(
            mul_nonIEEE(-(as_type<float>(R3i.w)), as_type<float>(R123i.w)) +
            as_type<float>(R3i.y));
        R2i.z = as_type<int>(
            mul_nonIEEE(as_type<float>(R5i.x), as_type<float>(R123i.z)) +
            as_type<float>(R2i.z));
        R2i.w = as_type<int>(
            mul_nonIEEE(as_type<float>(R3i.w), as_type<float>(R123i.z)) +
            as_type<float>(R2i.w));
    }
    if (activeMaskStackC[2] == true) {
        R3i.y = as_type<int>(tex1.sample(samplr1, float2(as_type<float>(R1i.x),
                                                         as_type<float>(R1i.y)))
                                 .x);
        R3i.x = as_type<int>(tex1.sample(samplr1, float2(as_type<float>(R2i.z),
                                                         as_type<float>(R2i.w)))
                                 .x);
    }
    if (activeMaskStackC[2] == true) {
        // 0
        PV0ix = as_type<int>(-(as_type<float>(R4i.w)) + as_type<float>(R3i.y));
        PV0iw = as_type<int>(-(as_type<float>(R4i.w)) + as_type<float>(R3i.x));
        // 1
        PV1iz =
            as_type<int>(max(as_type<float>(PV0iw), -(as_type<float>(PV0iw))));
        PV1iw =
            as_type<int>(max(as_type<float>(PV0ix), -(as_type<float>(PV0ix))));
        // 2
        PV0iy = as_type<int>(as_type<float>(R4i.x) + -(as_type<float>(PV1iz)));
        PV0iz = as_type<int>(as_type<float>(R4i.x) + -(as_type<float>(PV1iw)));
        // 3
        R123i.y = ((as_type<float>(PV0iz) >= 0.0) ? (as_type<int>(1.0)) : (0));
        R123i.z = ((as_type<float>(PV0iy) >= 0.0) ? (as_type<int>(1.0)) : (0));
        // 4
        R1i.x = as_type<int>(
            mul_nonIEEE(-(as_type<float>(R5i.x)), as_type<float>(R123i.y)) +
            as_type<float>(R1i.x));
        R1i.y = as_type<int>(
            mul_nonIEEE(-(as_type<float>(R3i.w)), as_type<float>(R123i.y)) +
            as_type<float>(R1i.y));
        R1i.z = as_type<int>(
            mul_nonIEEE(as_type<float>(R5i.x), as_type<float>(R123i.z)) +
            as_type<float>(R2i.z));
        R1i.w = as_type<int>(
            mul_nonIEEE(as_type<float>(R3i.w), as_type<float>(R123i.z)) +
            as_type<float>(R2i.w));
    }
    if (activeMaskStackC[2] == true) {
        R2i.w = as_type<int>(tex1.sample(samplr1, float2(as_type<float>(R1i.x),
                                                         as_type<float>(R1i.y)))
                                 .x);
        R2i.z = as_type<int>(tex1.sample(samplr1, float2(as_type<float>(R1i.z),
                                                         as_type<float>(R1i.w)))
                                 .x);
    }
    if (activeMaskStackC[2] == true) {
        // 0
        PV0ix = as_type<int>(-(as_type<float>(R4i.w)) + as_type<float>(R2i.w));
        PV0iy = as_type<int>(-(as_type<float>(R4i.w)) + as_type<float>(R2i.z));
        // 1
        PV1ix =
            as_type<int>(max(as_type<float>(PV0iy), -(as_type<float>(PV0iy))));
        PV1iw =
            as_type<int>(max(as_type<float>(PV0ix), -(as_type<float>(PV0ix))));
        // 2
        PV0iz = as_type<int>(as_type<float>(R4i.x) + -(as_type<float>(PV1iw)));
        PV0iw = as_type<int>(as_type<float>(R4i.x) + -(as_type<float>(PV1ix)));
        // 3
        R123i.z = ((as_type<float>(PV0iw) >= 0.0) ? (as_type<int>(1.0)) : (0));
        R123i.w = ((as_type<float>(PV0iz) >= 0.0) ? (as_type<int>(1.0)) : (0));
        // 4
        R3i.x = as_type<int>(
            mul_nonIEEE(-(as_type<float>(R5i.x)), as_type<float>(R123i.w)) +
            as_type<float>(R1i.x));
        R3i.y = as_type<int>(
            mul_nonIEEE(-(as_type<float>(R3i.w)), as_type<float>(R123i.w)) +
            as_type<float>(R1i.y));
        R1i.z = as_type<int>(
            mul_nonIEEE(as_type<float>(R5i.x), as_type<float>(R123i.z)) +
            as_type<float>(R1i.z));
        R1i.w = as_type<int>(
            mul_nonIEEE(as_type<float>(R3i.w), as_type<float>(R123i.z)) +
            as_type<float>(R1i.w));
    }
    if (activeMaskStackC[2] == true) {
        R1i.y = as_type<int>(tex1.sample(samplr1, float2(as_type<float>(R3i.x),
                                                         as_type<float>(R3i.y)))
                                 .x);
        R1i.x = as_type<int>(tex1.sample(samplr1, float2(as_type<float>(R1i.z),
                                                         as_type<float>(R1i.w)))
                                 .x);
    }
    if (activeMaskStackC[2] == true) {
        // 0
        PV0ix = as_type<int>(-(as_type<float>(R4i.w)) + as_type<float>(R1i.y));
        PV0iw = as_type<int>(-(as_type<float>(R4i.w)) + as_type<float>(R1i.x));
        // 1
        PV1iz =
            as_type<int>(max(as_type<float>(PV0iw), -(as_type<float>(PV0iw))));
        PV1iw =
            as_type<int>(max(as_type<float>(PV0ix), -(as_type<float>(PV0ix))));
        // 2
        PV0iy = as_type<int>(as_type<float>(R4i.x) + -(as_type<float>(PV1iz)));
        PV0iz = as_type<int>(as_type<float>(R4i.x) + -(as_type<float>(PV1iw)));
        // 3
        R123i.y = ((as_type<float>(PV0iz) >= 0.0) ? (as_type<int>(1.0)) : (0));
        R123i.z = ((as_type<float>(PV0iy) >= 0.0) ? (as_type<int>(1.0)) : (0));
        // 4
        R3i.x = as_type<int>(
            mul_nonIEEE(-(as_type<float>(R5i.x)), as_type<float>(R123i.y)) +
            as_type<float>(R3i.x));
        R3i.y = as_type<int>(
            mul_nonIEEE(-(as_type<float>(R3i.w)), as_type<float>(R123i.y)) +
            as_type<float>(R3i.y));
        R2i.z = as_type<int>(
            mul_nonIEEE(as_type<float>(R5i.x), as_type<float>(R123i.z)) +
            as_type<float>(R1i.z));
        R2i.w = as_type<int>(
            mul_nonIEEE(as_type<float>(R3i.w), as_type<float>(R123i.z)) +
            as_type<float>(R1i.w));
    }
    if (activeMaskStackC[2] == true) {
        R1i.w = as_type<int>(tex1.sample(samplr1, float2(as_type<float>(R3i.x),
                                                         as_type<float>(R3i.y)))
                                 .x);
        R1i.z = as_type<int>(tex1.sample(samplr1, float2(as_type<float>(R2i.z),
                                                         as_type<float>(R2i.w)))
                                 .x);
    }
    if (activeMaskStackC[2] == true) {
        // 0
        PV0ix = as_type<int>(-(as_type<float>(R4i.w)) + as_type<float>(R1i.w));
        PV0iy = as_type<int>(-(as_type<float>(R4i.w)) + as_type<float>(R1i.z));
        // 1
        PV1ix =
            as_type<int>(max(as_type<float>(PV0iy), -(as_type<float>(PV0iy))));
        PV1iw =
            as_type<int>(max(as_type<float>(PV0ix), -(as_type<float>(PV0ix))));
        // 2
        PV0iz = as_type<int>(as_type<float>(R4i.x) + -(as_type<float>(PV1iw)));
        PV0iw = as_type<int>(as_type<float>(R4i.x) + -(as_type<float>(PV1ix)));
        // 3
        R123i.z = ((as_type<float>(PV0iw) >= 0.0) ? (as_type<int>(1.0)) : (0));
        R123i.w = ((as_type<float>(PV0iz) >= 0.0) ? (as_type<int>(1.0)) : (0));
        // 4
        PV0ix = as_type<int>(
            mul_nonIEEE(as_type<float>(R5i.x), as_type<float>(R123i.w)));
        PV0iy = as_type<int>(
            mul_nonIEEE(as_type<float>(R3i.w), as_type<float>(R123i.z)));
        PV0iz = as_type<int>(
            mul_nonIEEE(as_type<float>(R5i.x), as_type<float>(R123i.z)));
        PV0iw = as_type<int>(
            mul_nonIEEE(as_type<float>(R3i.w), as_type<float>(R123i.w)));
        // 5
        R1i.x = as_type<int>(-(as_type<float>(PV0ix)) * 1.5 +
                             as_type<float>(R3i.x));
        R1i.y = as_type<int>(-(as_type<float>(PV0iw)) * 1.5 +
                             as_type<float>(R3i.y));
        R2i.z =
            as_type<int>(as_type<float>(PV0iz) * 1.5 + as_type<float>(R2i.z));
        R2i.w =
            as_type<int>(as_type<float>(PV0iy) * 1.5 + as_type<float>(R2i.w));
    }
    if (activeMaskStackC[2] == true) {
        R3i.y = as_type<int>(tex1.sample(samplr1, float2(as_type<float>(R1i.x),
                                                         as_type<float>(R1i.y)))
                                 .x);
        R3i.x = as_type<int>(tex1.sample(samplr1, float2(as_type<float>(R2i.z),
                                                         as_type<float>(R2i.w)))
                                 .x);
    }
    if (activeMaskStackC[2] == true) {
        // 0
        PV0iz = as_type<int>(-(as_type<float>(R4i.w)) + as_type<float>(R3i.y));
        PV0iw = as_type<int>(-(as_type<float>(R4i.w)) + as_type<float>(R3i.x));
        // 1
        PV1iy =
            as_type<int>(max(as_type<float>(PV0iz), -(as_type<float>(PV0iz))));
        PV1iz =
            as_type<int>(max(as_type<float>(PV0iw), -(as_type<float>(PV0iw))));
        // 2
        PV0ix = as_type<int>(as_type<float>(R4i.x) + -(as_type<float>(PV1iy)));
        PV0iy = as_type<int>(as_type<float>(R4i.x) + -(as_type<float>(PV1iz)));
        // 3
        R123i.x = ((as_type<float>(PV0iy) >= 0.0) ? (as_type<int>(1.0)) : (0));
        R123i.w = ((as_type<float>(PV0ix) >= 0.0) ? (as_type<int>(1.0)) : (0));
        // 4
        PV0ix = as_type<int>(
            mul_nonIEEE(as_type<float>(R5i.x), as_type<float>(R123i.x)));
        PV0ix = as_type<int>(as_type<float>(PV0ix) * 2.0);
        PV0iy = as_type<int>(
            mul_nonIEEE(as_type<float>(R3i.w), as_type<float>(R123i.x)));
        PV0iy = as_type<int>(as_type<float>(PV0iy) * 2.0);
        PV0iz = as_type<int>(
            mul_nonIEEE(as_type<float>(R3i.w), as_type<float>(R123i.w)));
        PV0iz = as_type<int>(as_type<float>(PV0iz) * 2.0);
        PV0iw = as_type<int>(
            mul_nonIEEE(as_type<float>(R5i.x), as_type<float>(R123i.w)));
        PV0iw = as_type<int>(as_type<float>(PV0iw) * 2.0);
        // 5
        R1i.x = as_type<int>(as_type<float>(R1i.x) + -(as_type<float>(PV0iw)));
        R1i.y = as_type<int>(as_type<float>(R1i.y) + -(as_type<float>(PV0iz)));
        R1i.z = as_type<int>(as_type<float>(R2i.z) + as_type<float>(PV0ix));
        R1i.w = as_type<int>(as_type<float>(R2i.w) + as_type<float>(PV0iy));
    }
    if (activeMaskStackC[2] == true) {
        R2i.w = as_type<int>(tex1.sample(samplr1, float2(as_type<float>(R1i.x),
                                                         as_type<float>(R1i.y)))
                                 .x);
        R2i.z = as_type<int>(tex1.sample(samplr1, float2(as_type<float>(R1i.z),
                                                         as_type<float>(R1i.w)))
                                 .x);
    }
    if (activeMaskStackC[2] == true) {
        // 0
        PV0iy = as_type<int>(-(as_type<float>(R4i.w)) + as_type<float>(R2i.z));
        PV0iz = as_type<int>(-(as_type<float>(R4i.w)) + as_type<float>(R2i.w));
        // 1
        PV1ix =
            as_type<int>(max(as_type<float>(PV0iy), -(as_type<float>(PV0iy))));
        PV1iy =
            as_type<int>(max(as_type<float>(PV0iz), -(as_type<float>(PV0iz))));
        // 2
        PV0ix = as_type<int>(as_type<float>(R4i.x) + -(as_type<float>(PV1iy)));
        PV0iw = as_type<int>(as_type<float>(R4i.x) + -(as_type<float>(PV1ix)));
        // 3
        R123i.z = ((as_type<float>(PV0iw) >= 0.0) ? (as_type<int>(1.0)) : (0));
        R123i.w = ((as_type<float>(PV0ix) >= 0.0) ? (as_type<int>(1.0)) : (0));
        // 4
        PV0ix = as_type<int>(
            mul_nonIEEE(as_type<float>(R5i.x), as_type<float>(R123i.w)));
        PV0ix = as_type<int>(as_type<float>(PV0ix) * 2.0);
        PV0iy = as_type<int>(
            mul_nonIEEE(as_type<float>(R3i.w), as_type<float>(R123i.z)));
        PV0iy = as_type<int>(as_type<float>(PV0iy) * 2.0);
        PV0iz = as_type<int>(
            mul_nonIEEE(as_type<float>(R5i.x), as_type<float>(R123i.z)));
        PV0iz = as_type<int>(as_type<float>(PV0iz) * 2.0);
        PV0iw = as_type<int>(
            mul_nonIEEE(as_type<float>(R3i.w), as_type<float>(R123i.w)));
        PV0iw = as_type<int>(as_type<float>(PV0iw) * 2.0);
        // 5
        R3i.x = as_type<int>(as_type<float>(R1i.x) + -(as_type<float>(PV0ix)));
        R3i.y = as_type<int>(as_type<float>(R1i.y) + -(as_type<float>(PV0iw)));
        R1i.z = as_type<int>(as_type<float>(R1i.z) + as_type<float>(PV0iz));
        R1i.w = as_type<int>(as_type<float>(R1i.w) + as_type<float>(PV0iy));
    }
    if (activeMaskStackC[2] == true) {
        R1i.y = as_type<int>(tex1.sample(samplr1, float2(as_type<float>(R3i.x),
                                                         as_type<float>(R3i.y)))
                                 .x);
        R1i.x = as_type<int>(tex1.sample(samplr1, float2(as_type<float>(R1i.z),
                                                         as_type<float>(R1i.w)))
                                 .x);
    }
    if (activeMaskStackC[2] == true) {
        // 0
        PV0iz = as_type<int>(-(as_type<float>(R4i.w)) + as_type<float>(R1i.y));
        PV0iw = as_type<int>(-(as_type<float>(R4i.w)) + as_type<float>(R1i.x));
        // 1
        PV1iy =
            as_type<int>(max(as_type<float>(PV0iz), -(as_type<float>(PV0iz))));
        PV1iz =
            as_type<int>(max(as_type<float>(PV0iw), -(as_type<float>(PV0iw))));
        // 2
        PV0ix = as_type<int>(as_type<float>(R4i.x) + -(as_type<float>(PV1iy)));
        PV0iy = as_type<int>(as_type<float>(R4i.x) + -(as_type<float>(PV1iz)));
        // 3
        R123i.x = ((as_type<float>(PV0iy) >= 0.0) ? (as_type<int>(1.0)) : (0));
        R123i.w = ((as_type<float>(PV0ix) >= 0.0) ? (as_type<int>(1.0)) : (0));
        // 4
        PV0ix = as_type<int>(
            mul_nonIEEE(as_type<float>(R5i.x), as_type<float>(R123i.x)));
        PV0ix = as_type<int>(as_type<float>(PV0ix) * 2.0);
        PV0iy = as_type<int>(
            mul_nonIEEE(as_type<float>(R3i.w), as_type<float>(R123i.x)));
        PV0iy = as_type<int>(as_type<float>(PV0iy) * 2.0);
        PV0iz = as_type<int>(
            mul_nonIEEE(as_type<float>(R3i.w), as_type<float>(R123i.w)));
        PV0iz = as_type<int>(as_type<float>(PV0iz) * 2.0);
        PV0iw = as_type<int>(
            mul_nonIEEE(as_type<float>(R5i.x), as_type<float>(R123i.w)));
        PV0iw = as_type<int>(as_type<float>(PV0iw) * 2.0);
        // 5
        R3i.x = as_type<int>(as_type<float>(R3i.x) + -(as_type<float>(PV0iw)));
        R3i.y = as_type<int>(as_type<float>(R3i.y) + -(as_type<float>(PV0iz)));
        R2i.z = as_type<int>(as_type<float>(R1i.z) + as_type<float>(PV0ix));
        R2i.w = as_type<int>(as_type<float>(R1i.w) + as_type<float>(PV0iy));
    }
    if (activeMaskStackC[2] == true) {
        R1i.w = as_type<int>(tex1.sample(samplr1, float2(as_type<float>(R3i.x),
                                                         as_type<float>(R3i.y)))
                                 .x);
        R1i.z = as_type<int>(tex1.sample(samplr1, float2(as_type<float>(R2i.z),
                                                         as_type<float>(R2i.w)))
                                 .x);
    }
    if (activeMaskStackC[2] == true) {
        // 0
        PV0iy = as_type<int>(-(as_type<float>(R4i.w)) + as_type<float>(R1i.z));
        PV0iz = as_type<int>(-(as_type<float>(R4i.w)) + as_type<float>(R1i.w));
        // 1
        PV1ix =
            as_type<int>(max(as_type<float>(PV0iy), -(as_type<float>(PV0iy))));
        PV1iy =
            as_type<int>(max(as_type<float>(PV0iz), -(as_type<float>(PV0iz))));
        // 2
        PV0ix = as_type<int>(as_type<float>(R4i.x) + -(as_type<float>(PV1iy)));
        PV0iw = as_type<int>(as_type<float>(R4i.x) + -(as_type<float>(PV1ix)));
        // 3
        R123i.z = ((as_type<float>(PV0iw) >= 0.0) ? (as_type<int>(1.0)) : (0));
        R123i.w = ((as_type<float>(PV0ix) >= 0.0) ? (as_type<int>(1.0)) : (0));
        // 4
        PV0ix = as_type<int>(
            mul_nonIEEE(as_type<float>(R5i.x), as_type<float>(R123i.w)));
        PV0ix = as_type<int>(as_type<float>(PV0ix) * 2.0);
        PV0iy = as_type<int>(
            mul_nonIEEE(as_type<float>(R3i.w), as_type<float>(R123i.z)));
        PV0iy = as_type<int>(as_type<float>(PV0iy) * 2.0);
        PV0iz = as_type<int>(
            mul_nonIEEE(as_type<float>(R5i.x), as_type<float>(R123i.z)));
        PV0iz = as_type<int>(as_type<float>(PV0iz) * 2.0);
        PV0iw = as_type<int>(
            mul_nonIEEE(as_type<float>(R3i.w), as_type<float>(R123i.w)));
        PV0iw = as_type<int>(as_type<float>(PV0iw) * 2.0);
        // 5
        R1i.x = as_type<int>(as_type<float>(R3i.x) + -(as_type<float>(PV0ix)));
        R1i.y = as_type<int>(as_type<float>(R3i.y) + -(as_type<float>(PV0iw)));
        R2i.z = as_type<int>(as_type<float>(R2i.z) + as_type<float>(PV0iz));
        R2i.w = as_type<int>(as_type<float>(R2i.w) + as_type<float>(PV0iy));
    }
    if (activeMaskStackC[2] == true) {
        R3i.y = as_type<int>(tex1.sample(samplr1, float2(as_type<float>(R1i.x),
                                                         as_type<float>(R1i.y)))
                                 .x);
        R3i.x = as_type<int>(tex1.sample(samplr1, float2(as_type<float>(R2i.z),
                                                         as_type<float>(R2i.w)))
                                 .x);
    }
    if (activeMaskStackC[2] == true) {
        // 0
        PV0iz = as_type<int>(-(as_type<float>(R4i.w)) + as_type<float>(R3i.y));
        PV0iw = as_type<int>(-(as_type<float>(R4i.w)) + as_type<float>(R3i.x));
        // 1
        PV1iy =
            as_type<int>(max(as_type<float>(PV0iz), -(as_type<float>(PV0iz))));
        PV1iz =
            as_type<int>(max(as_type<float>(PV0iw), -(as_type<float>(PV0iw))));
        // 2
        PV0ix = as_type<int>(as_type<float>(R4i.x) + -(as_type<float>(PV1iy)));
        PV0iy = as_type<int>(as_type<float>(R4i.x) + -(as_type<float>(PV1iz)));
        // 3
        R123i.x = ((as_type<float>(PV0iy) >= 0.0) ? (as_type<int>(1.0)) : (0));
        R123i.w = ((as_type<float>(PV0ix) >= 0.0) ? (as_type<int>(1.0)) : (0));
        // 4
        PV0ix = as_type<int>(
            mul_nonIEEE(as_type<float>(R5i.x), as_type<float>(R123i.x)));
        PV0ix = as_type<int>(as_type<float>(PV0ix) * 4.0);
        PV0iy = as_type<int>(
            mul_nonIEEE(as_type<float>(R3i.w), as_type<float>(R123i.x)));
        PV0iy = as_type<int>(as_type<float>(PV0iy) * 4.0);
        PV0iz = as_type<int>(
            mul_nonIEEE(as_type<float>(R3i.w), as_type<float>(R123i.w)));
        PV0iz = as_type<int>(as_type<float>(PV0iz) * 4.0);
        PV0iw = as_type<int>(
            mul_nonIEEE(as_type<float>(R5i.x), as_type<float>(R123i.w)));
        PV0iw = as_type<int>(as_type<float>(PV0iw) * 4.0);
        // 5
        R1i.x = as_type<int>(as_type<float>(R1i.x) + -(as_type<float>(PV0iw)));
        R1i.y = as_type<int>(as_type<float>(R1i.y) + -(as_type<float>(PV0iz)));
        R1i.z = as_type<int>(as_type<float>(R2i.z) + as_type<float>(PV0ix));
        R1i.w = as_type<int>(as_type<float>(R2i.w) + as_type<float>(PV0iy));
    }
    if (activeMaskStackC[2] == true) {
        R2i.w = as_type<int>(tex1.sample(samplr1, float2(as_type<float>(R1i.x),
                                                         as_type<float>(R1i.y)))
                                 .x);
        R2i.z = as_type<int>(tex1.sample(samplr1, float2(as_type<float>(R1i.z),
                                                         as_type<float>(R1i.w)))
                                 .x);
    }
    if (activeMaskStackC[2] == true) {
        // 0
        PV0iy = as_type<int>(-(as_type<float>(R4i.w)) + as_type<float>(R2i.z));
        PV0iz = as_type<int>(-(as_type<float>(R4i.w)) + as_type<float>(R2i.w));
        // 1
        PV1ix =
            as_type<int>(max(as_type<float>(PV0iy), -(as_type<float>(PV0iy))));
        PV1iy =
            as_type<int>(max(as_type<float>(PV0iz), -(as_type<float>(PV0iz))));
        // 2
        PV0ix = as_type<int>(as_type<float>(R4i.x) + -(as_type<float>(PV1iy)));
        PV0iw = as_type<int>(as_type<float>(R4i.x) + -(as_type<float>(PV1ix)));
        // 3
        R123i.z = ((as_type<float>(PV0iw) >= 0.0) ? (as_type<int>(1.0)) : (0));
        R123i.w = ((as_type<float>(PV0ix) >= 0.0) ? (as_type<int>(1.0)) : (0));
        // 4
        PV0ix = as_type<int>(
            mul_nonIEEE(as_type<float>(R5i.x), as_type<float>(R123i.w)));
        PV0iy = as_type<int>(
            mul_nonIEEE(as_type<float>(R5i.x), as_type<float>(R123i.z)));
        PV0iz = as_type<int>(
            mul_nonIEEE(as_type<float>(R3i.w), as_type<float>(R123i.z)));
        PV0iw = as_type<int>(
            mul_nonIEEE(as_type<float>(R3i.w), as_type<float>(R123i.w)));
        // 5
        R5i.x = as_type<int>(-(as_type<float>(PV0ix)) * 8.0 +
                             as_type<float>(R1i.x));
        R5i.y = as_type<int>(-(as_type<float>(PV0iw)) * 8.0 +
                             as_type<float>(R1i.y));
        R1i.z =
            as_type<int>(as_type<float>(PV0iy) * 8.0 + as_type<float>(R1i.z));
        R1i.w =
            as_type<int>(as_type<float>(PV0iz) * 8.0 + as_type<float>(R1i.w));
    }
    if (activeMaskStackC[2] == true) {
        R1i.y = as_type<int>(tex1.sample(samplr1, float2(as_type<float>(R5i.x),
                                                         as_type<float>(R5i.y)))
                                 .x);
        R1i.x = as_type<int>(tex1.sample(samplr1, float2(as_type<float>(R1i.z),
                                                         as_type<float>(R1i.w)))
                                 .x);
    }
    if (activeMaskStackC[2] == true) {
        // 0
        PV0ix = as_type<int>(-(as_type<float>(R4i.w)) + as_type<float>(R1i.x));
        PV0iy = as_type<int>(as_type<float>(R0i.x) + -(as_type<float>(R5i.x)));
        PV0iz = as_type<int>(-(as_type<float>(R4i.w)) + as_type<float>(R1i.y));
        R126i.w =
            as_type<int>(as_type<float>(R0i.y) + -(as_type<float>(R5i.y)));
        PS0i = as_type<int>(-(as_type<float>(R0i.x)) + as_type<float>(R1i.z));
        // 1
        PV1ix = as_type<int>(-(as_type<float>(R0i.y)) + as_type<float>(R1i.w));
        PV1iy = as_type<int>(
            mul_nonIEEE(as_type<float>(R5i.w), as_type<float>(PS0i)));
        PV1iz = ((0.0 > as_type<float>(PV0iz)) ? -1 : 0);
        PV1iw = as_type<int>(
            mul_nonIEEE(as_type<float>(R5i.w), as_type<float>(PV0iy)));
        PS1i = ((0.0 > as_type<float>(PV0ix)) ? -1 : 0);
        // 2
        R123i.x = as_type<int>(
            mul_nonIEEE(as_type<float>(R4i.z), as_type<float>(PV1ix)) +
            as_type<float>(PV1iy));
        R127i.y = (PV1iz != R2i.x) ? -1 : 0;
        R123i.z = as_type<int>(
            mul_nonIEEE(as_type<float>(R4i.z), as_type<float>(R126i.w)) +
            as_type<float>(PV1iw));
        R126i.w = (PS1i != R2i.x) ? -1 : 0;
        // 3
        PV1ix = as_type<int>(as_type<float>(R123i.z) + as_type<float>(R123i.x));
        R126i.y =
            as_type<int>(min(as_type<float>(R123i.z), as_type<float>(R123i.x)));
        PV1iz = ((as_type<float>(R123i.x) > as_type<float>(R123i.z)) ? -1 : 0);
        // 4
        R127i.y = ((PV1iz == 0) ? (R126i.w) : (R127i.y));
        PS0i = as_type<int>(1.0 / as_type<float>(PV1ix));
        // 5
        PV1iz = as_type<int>(as_type<float>(R126i.y) * as_type<float>(PS0i));
        // 6
        PV0iy = as_type<int>(-(as_type<float>(PV1iz)) + 0.5);
        // 7
        R123i.x = ((R127i.y == 0) ? (0) : (PV0iy));
        // 8
        PV0iw =
            as_type<int>(max(as_type<float>(R3i.z), as_type<float>(R123i.x)));
        // 9
        PV1iz = as_type<int>(
            mul_nonIEEE(as_type<float>(R6i.x), as_type<float>(PV0iw)));
        // 10
        R0i.x = as_type<int>(
            mul_nonIEEE(as_type<float>(R4i.z), as_type<float>(PV1iz)) +
            as_type<float>(R0i.x));
        R0i.y = as_type<int>(
            mul_nonIEEE(as_type<float>(R5i.w), as_type<float>(PV1iz)) +
            as_type<float>(R0i.y));
    }
    if (activeMaskStackC[2] == true) {
        R3i.xyzw =
            as_type<int4>(tex0.sample(samplr0, float2(as_type<float>(R0i.x),
                                                      as_type<float>(R0i.y)))
                              .xyzw);
    }
    activeMaskStackC[1] =
        activeMaskStack[0] == true && activeMaskStackC[0] == true;
    // export
    out.passPixelColor0 =
        as_type<float4>(float4(as_type<float>(R3i.x), as_type<float>(R3i.y),
                               as_type<float>(R3i.z), as_type<float>(R3i.w)));
#endif
    return out;
}
)BOTW_MSL_END"},
{0x148fc9c3111cf218ULL, 0x0000000000000000ULL, GraphicPack2::GP_SHADER_TYPE::VERTEX, R"BOTW_MSL_END(#include <metal_stdlib>
using namespace metal;

// shader 148fc9c3111cf218
// Shader dumped from BotW 208, using Cemu 2193a8c
// Based on ea9a49a6185cf1e5_0000000000000000_vs.txt
// Used for: Fix Reflection

constant float resXScale = float($width) / float($gameWidth);
constant float resYScale = float($height) / float($gameHeight);

#define SET_POSITION(_v)                                                       \
    out.position = _v;                                                         \
    out.position.z = (out.position.z + out.position.w) / 2.0
// start of shader inputs/outputs, predetermined by Cemu. Do not touch
struct SupportBuffer {
    int4 remapped[2];
};

struct VertexIn {
    uint4 attrDataSem0 [[attribute(0)]];
};

struct VertexOut {
    float4 position [[position]] [[invariant]];
    float4 passParameterSem0 [[user(locn0)]] [[center_no_perspective]];
    float4 passParameterSem1 [[user(locn1)]] [[center_no_perspective]];
    float4 passParameterSem2 [[user(locn2)]] [[center_no_perspective]];
    float4 passParameterSem3 [[user(locn3)]] [[center_no_perspective]];
};

// end of shader inputs/outputs
template <typename TextureT, typename CoordT>
float sampleCompareEmulate(TextureT tex, sampler samplr, CoordT coord,
                           float compareValue) {
    return compareValue < tex.sample(samplr, coord).x ? 1.0 : 0.0;
}
template <typename TextureT, typename CoordT>
float2 textureCalculateLod(TextureT tex, sampler samplr, CoordT coord) {
    float lod = tex.calculate_unclamped_lod(samplr, coord);
    return float2(floor(lod), fract(lod));
}
int clampFI32(int v) {
    if (v == 0x7FFFFFFF)
        return as_type<int>(1.0);
    else if (v == 0xFFFFFFFF)
        return as_type<int>(0.0);
    return as_type<int>(clamp(as_type<float>(v), 0.0, 1.0));
}
float mul_nonIEEE(float a, float b) {
    if (a == 0.0 || b == 0.0)
        return 0.0;
    return a * b;
}
vertex VertexOut main0(uint vid [[vertex_id]], uint iid [[instance_id]],
                       VertexIn in [[stage_in]],
                       constant SupportBuffer &supportBuffer [[buffer(0)]]) {
    VertexOut out;
    float4 R0f = float4(0.0);
    float4 R1f = float4(0.0);
    float4 R2f = float4(0.0);
    float4 R3f = float4(0.0);
    float4 R123f = float4(0.0);
    float4 R126f = float4(0.0);
    float4 R127f = float4(0.0);
    uint4 attrDecoder;
    float backupReg0f, backupReg1f, backupReg2f, backupReg3f, backupReg4f;
    float PV0fx = 0.0, PV0fy = 0.0, PV0fz = 0.0, PV0fw = 0.0, PV1fx = 0.0,
          PV1fy = 0.0, PV1fz = 0.0, PV1fw = 0.0;
    float PS0f = 0.0, PS1f = 0.0;
    float4 tempf = float4(0.0);
    float tempResultf;
    int tempResulti;
    int4 ARi = int4(0);
    bool predResult = true;
    R0f = float4(vid, 0, 0, iid);
    attrDecoder.xyz = in.attrDataSem0.xyz;
    attrDecoder.xyz =
        (attrDecoder.xyz >> 24) | ((attrDecoder.xyz >> 8) & 0xFF00) |
        ((attrDecoder.xyz << 8) & 0xFF0000) | ((attrDecoder.xyz << 24));
    attrDecoder.w = 0;
    R1f = float4(
        as_type<float>(int(attrDecoder.x)), as_type<float>(int(attrDecoder.y)),
        as_type<float>(int(attrDecoder.z)), as_type<float>(0x3f800000));
    // 0
    PV0fx = (0.0 > R1f.y) ? 1.0 : 0.0;
    PV0fy = (0.0 > R1f.x) ? 1.0 : 0.0;
    PV0fz = (R1f.y > 0.0) ? 1.0 : 0.0;
    PV0fw = (R1f.x > 0.0) ? 1.0 : 0.0;
    R1f.z = 0.0;
    // 1
    R1f.x = PV0fw + -(PV0fy);
    R1f.y = PV0fz + -(PV0fx);
    R1f.w = 1.0;
    // 2
    PV0fx = R1f.x;
    PV0fx /= 2.0;
    R127f.y = R1f.x * 1.0;
    PV0fz = -(R1f.y);
    PV0fz /= 2.0;
    R127f.w = R1f.y * -(1.0);
    R127f.x = R1f.x * 1.0;
    // 3
    R126f.x =
        as_type<float>(supportBuffer.remapped[0].y) * 2.0 / resYScale + R127f.w;
    R0f.y = PV0fz + 0.5;
    R123f.z =
        as_type<float>(supportBuffer.remapped[0].x) * 2.0 / resXScale + R127f.y;
    PV1fw = R127f.w;
    R0f.x = PV0fx + 0.5;
    // 4
    R2f.x =
        mul_nonIEEE(R127f.x, -(as_type<float>(supportBuffer.remapped[1].x))) +
        -(as_type<float>(supportBuffer.remapped[1].z));
    R2f.y =
        mul_nonIEEE(R127f.w, -(as_type<float>(supportBuffer.remapped[1].y))) +
        -(as_type<float>(supportBuffer.remapped[1].w));
    R0f.z =
        mul_nonIEEE(-(as_type<float>(supportBuffer.remapped[1].x)), R123f.z) +
        -(as_type<float>(supportBuffer.remapped[1].z));
    R0f.w = mul_nonIEEE(-(as_type<float>(supportBuffer.remapped[1].y)), PV1fw) +
            -(as_type<float>(supportBuffer.remapped[1].w));
    R3f.x =
        mul_nonIEEE(R127f.y, -(as_type<float>(supportBuffer.remapped[1].x))) +
        -(as_type<float>(supportBuffer.remapped[1].z));
    // 5
    R3f.y =
        mul_nonIEEE(R126f.x, -(as_type<float>(supportBuffer.remapped[1].y))) +
        -(as_type<float>(supportBuffer.remapped[1].w));
    // export
    SET_POSITION(float4(R1f.x, R1f.y, R1f.z, R1f.w));
    // export
    out.passParameterSem0 = float4(R0f.x, R0f.y, R0f.z, R0f.z);
    // export
    out.passParameterSem1 = float4(R2f.x, R2f.y, R2f.z, R2f.z);
    // export
    out.passParameterSem2 = float4(R0f.z, R0f.w, R0f.z, R0f.z);
    // export
    out.passParameterSem3 = float4(R3f.x, R3f.y, R3f.z, R3f.z);
    return out;
}
)BOTW_MSL_END"},
{0x1e69944c6036cda3ULL, 0x0000000000000000ULL, GraphicPack2::GP_SHADER_TYPE::VERTEX, R"BOTW_MSL_END(#include <metal_stdlib>
using namespace metal;

// shader 1e69944c6036cda3
// Shader dumped from BotW 208, using Cemu 2193a8c
// Based on f69e84515ae56e70_0000000000000000_vs.txt or
// 771e24915acbb074_0000000000000000_vs.txt Used for: Vertical Bloom or
// Reflection Blur

constant float resYScale = float($height) / float($gameHeight);

#define SET_POSITION(_v)                                                       \
    out.position = _v;                                                         \
    out.position.z = (out.position.z + out.position.w) / 2.0
// start of shader inputs/outputs, predetermined by Cemu. Do not touch
struct SupportBuffer {
    int4 remapped[1];
};

struct VertexIn {
    uint4 attrDataSem0 [[attribute(0)]];
    uint4 attrDataSem1 [[attribute(1)]];
};

struct VertexOut {
    float4 position [[position]] [[invariant]];
    float4 passParameterSem0 [[user(locn0)]];
    float4 passParameterSem1 [[user(locn1)]];
};

// end of shader inputs/outputs
template <typename TextureT, typename CoordT>
float sampleCompareEmulate(TextureT tex, sampler samplr, CoordT coord,
                           float compareValue) {
    return compareValue < tex.sample(samplr, coord).x ? 1.0 : 0.0;
}
template <typename TextureT, typename CoordT>
float2 textureCalculateLod(TextureT tex, sampler samplr, CoordT coord) {
    float lod = tex.calculate_unclamped_lod(samplr, coord);
    return float2(floor(lod), fract(lod));
}
int clampFI32(int v) {
    if (v == 0x7FFFFFFF)
        return as_type<int>(1.0);
    else if (v == 0xFFFFFFFF)
        return as_type<int>(0.0);
    return as_type<int>(clamp(as_type<float>(v), 0.0, 1.0));
}
float mul_nonIEEE(float a, float b) {
    if (a == 0.0 || b == 0.0)
        return 0.0;
    return a * b;
}
vertex VertexOut main0(uint vid [[vertex_id]], uint iid [[instance_id]],
                       VertexIn in [[stage_in]],
                       constant SupportBuffer &supportBuffer [[buffer(0)]]) {
    VertexOut out;
    float4 R0f = float4(0.0);
    float4 R1f = float4(0.0);
    float4 R2f = float4(0.0);
    float4 R127f = float4(0.0);
    uint4 attrDecoder;
    float backupReg0f, backupReg1f, backupReg2f, backupReg3f, backupReg4f;
    float PV0fx = 0.0, PV0fy = 0.0, PV0fz = 0.0, PV0fw = 0.0, PV1fx = 0.0,
          PV1fy = 0.0, PV1fz = 0.0, PV1fw = 0.0;
    float PS0f = 0.0, PS1f = 0.0;
    float4 tempf = float4(0.0);
    float tempResultf;
    int tempResulti;
    int4 ARi = int4(0);
    bool predResult = true;
    R0f = float4(vid, 0, 0, iid);
    attrDecoder.xyz = in.attrDataSem0.xyz;
    attrDecoder.xyz =
        (attrDecoder.xyz >> 24) | ((attrDecoder.xyz >> 8) & 0xFF00) |
        ((attrDecoder.xyz << 8) & 0xFF0000) | ((attrDecoder.xyz << 24));
    attrDecoder.w = 0;
    R1f = float4(
        as_type<float>(int(attrDecoder.x)), as_type<float>(int(attrDecoder.y)),
        as_type<float>(int(attrDecoder.z)), as_type<float>(0x3f800000));
    attrDecoder.xy = in.attrDataSem1.xy;
    attrDecoder.xy = (attrDecoder.xy >> 24) | ((attrDecoder.xy >> 8) & 0xFF00) |
                     ((attrDecoder.xy << 8) & 0xFF0000) |
                     ((attrDecoder.xy << 24));
    attrDecoder.z = 0;
    attrDecoder.w = 0;
    R2f = float4(as_type<float>(int(attrDecoder.x)),
                 as_type<float>(int(attrDecoder.y)), as_type<float>(0),
                 as_type<float>(0x3f800000));
    // 0
    R1f.x = R1f.x;
    R1f.x *= 2.0;
    R1f.y = R1f.y;
    R1f.y *= 2.0;
    R1f.z = 0.0;
    R1f.w = 1.0;
    PS0f = as_type<float>(supportBuffer.remapped[0].y) / resYScale *
           as_type<float>(0x3fb13a93);
    // 1
    R127f.x = as_type<float>(supportBuffer.remapped[0].y) / resYScale *
              as_type<float>(0x404ec4f0);
    R0f.y = R2f.y + -(PS0f);
    R2f.z = R2f.y + PS0f;
    R2f.w = R2f.y;
    R0f.x = R2f.x;
    // 2
    R0f.z = R2f.z;
    R0f.w = R2f.w;
    R2f.z = R2f.y + R127f.x;
    // 3
    backupReg0f = R2f.y;
    R2f.y = backupReg0f + -(R127f.x);
    R2f.w = backupReg0f;
    // export
    SET_POSITION(float4(R1f.x, R1f.y, R1f.z, R1f.w));
    // export
    out.passParameterSem0 = float4(R0f.x, R0f.y, R0f.z, R0f.w);
    // export
    out.passParameterSem1 = float4(R2f.x, R2f.y, R2f.z, R2f.w);
    return out;
}
)BOTW_MSL_END"},
{0x2633e3bef936262bULL, 0x0000000000000000ULL, GraphicPack2::GP_SHADER_TYPE::VERTEX, R"BOTW_MSL_END(#include <metal_stdlib>
using namespace metal;

// shader 2633e3bef936262b
// Shader dumped from BotW 208, using Cemu 2193a8c
// Based on fc3e63a2007625f8_0000000000000000_vs.txt
// Used For: Vertical Menu Blur

constant float resYScale = float($height) / float($gameHeight);

#define SET_POSITION(_v)                                                       \
    out.position = _v;                                                         \
    out.position.z = (out.position.z + out.position.w) / 2.0
// start of shader inputs/outputs, predetermined by Cemu. Do not touch
struct SupportBuffer {
    int4 remapped[5];
};

struct VertexIn {
    uint4 attrDataSem0 [[attribute(0)]];
};

struct VertexOut {
    float4 position [[position]] [[invariant]];
    float4 passParameterSem0 [[user(locn0)]];
};

// end of shader inputs/outputs
template <typename TextureT, typename CoordT>
float sampleCompareEmulate(TextureT tex, sampler samplr, CoordT coord,
                           float compareValue) {
    return compareValue < tex.sample(samplr, coord).x ? 1.0 : 0.0;
}
template <typename TextureT, typename CoordT>
float2 textureCalculateLod(TextureT tex, sampler samplr, CoordT coord) {
    float lod = tex.calculate_unclamped_lod(samplr, coord);
    return float2(floor(lod), fract(lod));
}
int clampFI32(int v) {
    if (v == 0x7FFFFFFF)
        return as_type<int>(1.0);
    else if (v == 0xFFFFFFFF)
        return as_type<int>(0.0);
    return as_type<int>(clamp(as_type<float>(v), 0.0, 1.0));
}
float mul_nonIEEE(float a, float b) {
    if (a == 0.0 || b == 0.0)
        return 0.0;
    return a * b;
}
vertex VertexOut main0(uint vid [[vertex_id]], uint iid [[instance_id]],
                       VertexIn in [[stage_in]],
                       constant SupportBuffer &supportBuffer [[buffer(0)]]) {
    VertexOut out;
    float4 R0f = float4(0.0);
    float4 R1f = float4(0.0);
    float4 R126f = float4(0.0);
    float4 R127f = float4(0.0);
    uint4 attrDecoder;
    float backupReg0f, backupReg1f, backupReg2f, backupReg3f, backupReg4f;
    float PV0fx = 0.0, PV0fy = 0.0, PV0fz = 0.0, PV0fw = 0.0, PV1fx = 0.0,
          PV1fy = 0.0, PV1fz = 0.0, PV1fw = 0.0;
    float PS0f = 0.0, PS1f = 0.0;
    float4 tempf = float4(0.0);
    float tempResultf;
    int tempResulti;
    int4 ARi = int4(0);
    bool predResult = true;
    R0f = float4(vid, 0, 0, iid);
    attrDecoder.xyz = in.attrDataSem0.xyz;
    attrDecoder.xyz =
        (attrDecoder.xyz >> 24) | ((attrDecoder.xyz >> 8) & 0xFF00) |
        ((attrDecoder.xyz << 8) & 0xFF0000) | ((attrDecoder.xyz << 24));
    attrDecoder.w = 0;
    R1f = float4(
        as_type<float>(int(attrDecoder.x)), as_type<float>(int(attrDecoder.y)),
        as_type<float>(int(attrDecoder.z)), as_type<float>(0x3f800000));
    // 0
    R127f.x = -(R1f.y);
    R127f.y = (R1f.x > 0.0) ? 1.0 : 0.0;
    R127f.y /= 2.0;
    R126f.z = (0.0 > R1f.x) ? 1.0 : 0.0;
    R126f.z /= 2.0;
    R127f.w = 1.0;
    R126f.x = as_type<float>(supportBuffer.remapped[0].w) *
              (as_type<float>(0x3fae8a72) / resYScale);
    // 1
    R0f.x = dot(float4(R1f.x, R1f.y, R1f.z, R127f.w),
                float4(as_type<float>(supportBuffer.remapped[1].x),
                       as_type<float>(supportBuffer.remapped[1].y),
                       as_type<float>(supportBuffer.remapped[1].z),
                       as_type<float>(supportBuffer.remapped[1].w)));
    PV1fy = R0f.x;
    PV1fz = R0f.x;
    PV1fw = R0f.x;
    R127f.z = (R127f.x > 0.0) ? 1.0 : 0.0;
    R127f.z /= 2.0;
    // 2
    tempf.x = dot(float4(R1f.x, R1f.y, R1f.z, R127f.w),
                  float4(as_type<float>(supportBuffer.remapped[2].x),
                         as_type<float>(supportBuffer.remapped[2].y),
                         as_type<float>(supportBuffer.remapped[2].z),
                         as_type<float>(supportBuffer.remapped[2].w)));
    PV0fx = tempf.x;
    R0f.y = tempf.x;
    PV0fz = tempf.x;
    PV0fw = tempf.x;
    PS0f = (0.0 > R127f.x) ? 1.0 : 0.0;
    PS0f /= 2.0;
    // 3
    tempf.x = dot(float4(R1f.x, R1f.y, R1f.z, R127f.w),
                  float4(as_type<float>(supportBuffer.remapped[3].x),
                         as_type<float>(supportBuffer.remapped[3].y),
                         as_type<float>(supportBuffer.remapped[3].z),
                         as_type<float>(supportBuffer.remapped[3].w)));
    PV1fx = tempf.x;
    PV1fy = tempf.x;
    R0f.z = tempf.x;
    PV1fw = tempf.x;
    R127f.z = R127f.z + -(PS0f);
    // 4
    tempf.x = dot(float4(R1f.x, R1f.y, R1f.z, R127f.w),
                  float4(as_type<float>(supportBuffer.remapped[4].x),
                         as_type<float>(supportBuffer.remapped[4].y),
                         as_type<float>(supportBuffer.remapped[4].z),
                         as_type<float>(supportBuffer.remapped[4].w)));
    PV0fx = tempf.x;
    PV0fy = tempf.x;
    PV0fz = tempf.x;
    R0f.w = tempf.x;
    PS0f = R127f.y + -(R126f.z);
    // 5
    R1f.x = PS0f + 0.5;
    PV1fy = R127f.z + 0.5;
    // 6
    R1f.y = PV1fy + -(R126f.x);
    R1f.z = PV1fy + R126f.x;
    R1f.w = PV1fy;
    // export
    SET_POSITION(float4(R0f.x, R0f.y, R0f.z, R0f.w));
    // export
    out.passParameterSem0 = float4(R1f.x, R1f.y, R1f.z, R1f.w);
    return out;
}
)BOTW_MSL_END"},
{0x2baf9a8f53ba0bc9ULL, 0x0000000000000000ULL, GraphicPack2::GP_SHADER_TYPE::VERTEX, R"BOTW_MSL_END(#include <metal_stdlib>
using namespace metal;

// shader 2baf9a8f53ba0bc9
// Shader dumped from BotW 208, using Cemu 2193a8c
// Based on 01ba1a725afa0b96_0000000000000000_vs.txt
// Night star (size)

constant float resScale = $height / 720.0;

#define SET_POSITION(_v)                                                       \
    out.position = _v;                                                         \
    out.position.z = (out.position.z + out.position.w) / 2.0
// start of shader inputs/outputs, predetermined by Cemu. Do not touch
struct SupportBuffer {};

struct UBuff1 {
    float4 d[4096];
};

struct UBuff2 {
    float4 d[8];
};

struct VertexIn {
    uint4 attrDataSem0 [[attribute(0)]];
};

struct VertexOut {
    float4 position [[position]] [[invariant]];
    float pointSize [[point_size]];
    float4 passParameterSem1 [[user(locn0)]];
    float4 passParameterSem2 [[user(locn1)]];
};

// end of shader inputs/outputs
template <typename TextureT, typename CoordT>
float sampleCompareEmulate(TextureT tex, sampler samplr, CoordT coord,
                           float compareValue) {
    return compareValue < tex.sample(samplr, coord).x ? 1.0 : 0.0;
}
template <typename TextureT, typename CoordT>
float2 textureCalculateLod(TextureT tex, sampler samplr, CoordT coord) {
    float lod = tex.calculate_unclamped_lod(samplr, coord);
    return float2(floor(lod), fract(lod));
}
int clampFI32(int v) {
    if (v == 0x7FFFFFFF)
        return as_type<int>(1.0);
    else if (v == 0xFFFFFFFF)
        return as_type<int>(0.0);
    return as_type<int>(clamp(as_type<float>(v), 0.0, 1.0));
}
float mul_nonIEEE(float a, float b) {
    if (a == 0.0 || b == 0.0)
        return 0.0;
    return a * b;
}
vertex VertexOut main0(uint vid [[vertex_id]], uint iid [[instance_id]],
                       VertexIn in [[stage_in]],
                       constant UBuff1 &ubuff1 [[buffer(0)]],
                       constant UBuff2 &ubuff2 [[buffer(1)]]) {
    VertexOut out;
    int4 R0i = int4(0);
    int4 R1i = int4(0);
    int4 R2i = int4(0);
    int4 R3i = int4(0);
    uint4 attrDecoder;
    int backupReg0i, backupReg1i, backupReg2i, backupReg3i, backupReg4i;
    int PV0ix = 0, PV0iy = 0, PV0iz = 0, PV0iw = 0, PV1ix = 0, PV1iy = 0,
        PV1iz = 0, PV1iw = 0;
    int PS0i = 0, PS1i = 0;
    int4 tempi = int4(0);
    float tempResultf;
    int tempResulti;
    int4 ARi = int4(0);
    bool predResult = true;
    R0i = int4(vid, 0, 0, iid);
    attrDecoder.xyz = in.attrDataSem0.xyz;
    attrDecoder.xyz =
        (attrDecoder.xyz >> 24) | ((attrDecoder.xyz >> 8) & 0xFF00) |
        ((attrDecoder.xyz << 8) & 0xFF0000) | ((attrDecoder.xyz << 24));
    attrDecoder.w = 0;
    R1i = int4(int(attrDecoder.x), int(attrDecoder.y), int(attrDecoder.z),
               0x3f800000);
    // 0
    R1i.x = as_type<int>(ubuff2.d[7].z);
    R0i.z = R0i.x << int(1);
    R0i.w = int(0x3f800000);
    // 1
    R0i.y = R0i.z + int(1);
    R3i.xyzw = as_type<int4>(ubuff1.d[R0i.z].xyzw);
    R2i.xyzw = as_type<int4>(ubuff1.d[R0i.y].xyzw);
    // export
    out.pointSize = (float4(as_type<float>(R1i.x), 0, 0, 0)).x * resScale;
    // 0
    R1i.x = as_type<int>(dot(
        float4(as_type<float>(R3i.x), as_type<float>(R3i.y),
               as_type<float>(R3i.z), as_type<float>(R0i.w)),
        float4(ubuff2.d[0].x, ubuff2.d[0].y, ubuff2.d[0].z, ubuff2.d[0].w)));
    PV0iy = R1i.x;
    PV0iz = R1i.x;
    PV0iw = R1i.x;
    R2i.w =
        as_type<int>(mul_nonIEEE(as_type<float>(R3i.w), as_type<float>(R2i.w)));
    // 1
    tempi.x = as_type<int>(dot(
        float4(as_type<float>(R3i.x), as_type<float>(R3i.y),
               as_type<float>(R3i.z), as_type<float>(R0i.w)),
        float4(ubuff2.d[1].x, ubuff2.d[1].y, ubuff2.d[1].z, ubuff2.d[1].w)));
    PV1ix = tempi.x;
    R1i.y = tempi.x;
    PV1iz = tempi.x;
    PV1iw = tempi.x;
    R3i.w = as_type<int>(float(R0i.x));
    // 2
    tempi.x = as_type<int>(dot(
        float4(as_type<float>(R3i.x), as_type<float>(R3i.y),
               as_type<float>(R3i.z), as_type<float>(R0i.w)),
        float4(ubuff2.d[2].x, ubuff2.d[2].y, ubuff2.d[2].z, ubuff2.d[2].w)));
    PV0ix = tempi.x;
    PV0iy = tempi.x;
    R1i.z = tempi.x;
    PV0iw = tempi.x;
    // 3
    tempi.x = as_type<int>(dot(
        float4(as_type<float>(R3i.x), as_type<float>(R3i.y),
               as_type<float>(R3i.z), as_type<float>(R0i.w)),
        float4(ubuff2.d[3].x, ubuff2.d[3].y, ubuff2.d[3].z, ubuff2.d[3].w)));
    PV1ix = tempi.x;
    PV1iy = tempi.x;
    PV1iz = tempi.x;
    R1i.w = tempi.x;
    // export
    SET_POSITION(float4(as_type<float>(R1i.x), as_type<float>(R1i.y),
                        as_type<float>(R1i.z), as_type<float>(R1i.w)));
    // export
    out.passParameterSem1 =
        float4(as_type<float>(R2i.x), as_type<float>(R2i.y),
               as_type<float>(R2i.z), as_type<float>(R2i.w));
    // export
    out.passParameterSem2 =
        float4(as_type<float>(R3i.x), as_type<float>(R3i.y),
               as_type<float>(R3i.z), as_type<float>(R3i.w));
    return out;
}
)BOTW_MSL_END"},
{0x2f308d3f1f06de8cULL, 0x0000000000000000ULL, GraphicPack2::GP_SHADER_TYPE::VERTEX, R"BOTW_MSL_END(#include <metal_stdlib>
using namespace metal;

// shader 2f308d3f1f06de8c
// Shader dumped from BotW 208, using Cemu 2193a8c
// Based on 0bcd653c18367d59_0000000000000000_vs.txt
// Used for: Restoring the native BotW Anti-Aliasing implementation

constant float resXScale = float($width) / float($gameWidth);
constant float resYScale = float($height) / float($gameHeight);

#define SET_POSITION(_v)                                                       \
    out.position = _v;                                                         \
    out.position.z = (out.position.z + out.position.w) / 2.0
// start of shader inputs/outputs, predetermined by Cemu. Do not touch
struct SupportBuffer {
    int4 remapped[5];
};

struct VertexIn {
    uint4 attrDataSem0 [[attribute(0)]];
    uint4 attrDataSem1 [[attribute(1)]];
};

struct VertexOut {
    float4 position [[position]] [[invariant]];
    float4 passParameterSem4 [[user(locn3)]];
    float4 passParameterSem0 [[user(locn0)]];
    float4 passParameterSem1 [[user(locn1)]];
    float4 passParameterSem3 [[user(locn2)]];
    float4 passParameterSem6 [[user(locn4)]];
};

// end of shader inputs/outputs
template <typename TextureT, typename CoordT>
float sampleCompareEmulate(TextureT tex, sampler samplr, CoordT coord,
                           float compareValue) {
    return compareValue < tex.sample(samplr, coord).x ? 1.0 : 0.0;
}
template <typename TextureT, typename CoordT>
float2 textureCalculateLod(TextureT tex, sampler samplr, CoordT coord) {
    float lod = tex.calculate_unclamped_lod(samplr, coord);
    return float2(floor(lod), fract(lod));
}
int clampFI32(int v) {
    if (v == 0x7FFFFFFF)
        return as_type<int>(1.0);
    else if (v == 0xFFFFFFFF)
        return as_type<int>(0.0);
    return as_type<int>(clamp(as_type<float>(v), 0.0, 1.0));
}
float mul_nonIEEE(float a, float b) {
    if (a == 0.0 || b == 0.0)
        return 0.0;
    return a * b;
}
vertex VertexOut main0(uint vid [[vertex_id]], uint iid [[instance_id]],
                       VertexIn in [[stage_in]],
                       constant SupportBuffer &supportBuffer [[buffer(0)]],
                       texture2d<float> tex0 [[texture(0)]],
                       sampler samplr0 [[sampler(0)]]) {
    VertexOut out;
    int4 R0i = int4(0);
    int4 R1i = int4(0);
    int4 R2i = int4(0);
    int4 R3i = int4(0);
    int4 R4i = int4(0);
    int4 R5i = int4(0);
    int4 R6i = int4(0);
    int4 R7i = int4(0);
    int4 R123i = int4(0);
    int4 R124i = int4(0);
    int4 R125i = int4(0);
    int4 R126i = int4(0);
    int4 R127i = int4(0);
    uint4 attrDecoder;
    int backupReg0i, backupReg1i, backupReg2i, backupReg3i, backupReg4i;
    int PV0ix = 0, PV0iy = 0, PV0iz = 0, PV0iw = 0, PV1ix = 0, PV1iy = 0,
        PV1iz = 0, PV1iw = 0;
    int PS0i = 0, PS1i = 0;
    int4 tempi = int4(0);
    float tempResultf;
    int tempResulti;
    int4 ARi = int4(0);
    bool predResult = true;
    R0i = int4(vid, 0, 0, iid);
    attrDecoder.xyzw = in.attrDataSem0.xyzw;
    attrDecoder = ((attrDecoder >> 8) & 0xFF) | ((attrDecoder << 8) & 0xFF00);
    attrDecoder.xyzw = as_type<uint4>(
        float4(float2(as_type<half2>(attrDecoder.x | (attrDecoder.y << 16))),
               float2(as_type<half2>(attrDecoder.z | (attrDecoder.w << 16)))));
    R1i = int4(int(attrDecoder.x), int(attrDecoder.y), int(attrDecoder.z),
               int(attrDecoder.w));
    attrDecoder.x = in.attrDataSem1.x;
    attrDecoder.x = (attrDecoder.x >> 24) | ((attrDecoder.x >> 8) & 0xFF00) |
                    ((attrDecoder.x << 8) & 0xFF0000) | ((attrDecoder.x << 24));
    attrDecoder.y = 0;
    attrDecoder.z = 0;
    attrDecoder.w = 0;
    attrDecoder.xyzw =
        uint4((attrDecoder.x >> 0) & 0x3FF, (attrDecoder.x >> 10) & 0x3FF,
              (attrDecoder.x >> 20) & 0x3FF, (attrDecoder.x >> 30) & 0x3);
    if ((attrDecoder.x & 0x200) != 0)
        attrDecoder.x |= 0xFFFFFC00;
    if ((attrDecoder.y & 0x200) != 0)
        attrDecoder.y |= 0xFFFFFC00;
    if ((attrDecoder.z & 0x200) != 0)
        attrDecoder.z |= 0xFFFFFC00;
    attrDecoder.x = as_type<uint>(max(float(int(attrDecoder.x)) / 511.0, -1.0));
    attrDecoder.y = as_type<uint>(max(float(int(attrDecoder.y)) / 511.0, -1.0));
    attrDecoder.z = as_type<uint>(max(float(int(attrDecoder.z)) / 511.0, -1.0));
    attrDecoder.w = as_type<uint>(float(attrDecoder.w));
    R2i = int4(int(attrDecoder.x), int(attrDecoder.y), int(attrDecoder.z),
               int(attrDecoder.w));
    // 0
    PV0ix = (R0i.x == int(1)) ? -1 : 0;
    R127i.y = int(0x3f800000);
    PV0iz = int(0x3f800000);
    PV0iw = int(0x40400000);
    R127i.z = int(0xbf800000);
    // 1
    R123i.x = ((PV0ix == 0) ? (PV0iz) : (int(0xc0400000)));
    R123i.y = ((PV0ix == 0) ? (PV0iw) : (int(0xbf800000)));
    R0i.z = 0;
    PV1iw = as_type<int>(as_type<float>(supportBuffer.remapped[0].z) *
                         as_type<float>(0x3b808081));
    R2i.w = int(0x3f800000);
    // 2
    R2i.x = ((R0i.x == 0) ? (R127i.z) : (R123i.y));
    R2i.y = ((R0i.x == 0) ? (R127i.y) : (R123i.x));
    R2i.z = as_type<int>(as_type<float>(PV1iw) + -(0.5));
    R2i.z = as_type<int>(as_type<float>(R2i.z) * 2.0);
    R0i.w = int(0x3d2aaaab);
    R0i.x = int(0x3e000000);
    // 3
    R5i.x = as_type<int>(as_type<float>(R2i.x) * 0.5 + 0.5);
    R0i.y = 0;
    PV1iz = as_type<int>(-(as_type<float>(R2i.y)));
    R4i.w = supportBuffer.remapped[1].z;
    R4i.w = as_type<int>(as_type<float>(R4i.w) / 2.0);
    R3i.x = as_type<int>(-(as_type<float>(supportBuffer.remapped[2].z)));
    R3i.x = as_type<int>(as_type<float>(R3i.x) / 2.0);
    // 4
    R6i.x = as_type<int>(-(as_type<float>(supportBuffer.remapped[3].x)));
    R6i.x = as_type<int>(as_type<float>(R6i.x) * 2.0);
    R5i.y = as_type<int>(as_type<float>(PV1iz) * 0.5 + 0.5);
    R7i.z = R5i.x;
    R3i.w = R2i.x;
    R3i.w = as_type<int>(as_type<float>(R3i.w) / 2.0);
    R6i.y = supportBuffer.remapped[3].y;
    R6i.y = as_type<int>(as_type<float>(R6i.y) * 2.0);
    R1i.w = as_type<int>(tex0.sample(samplr0, float2(as_type<float>(R0i.w),
                                                     as_type<float>(R0i.z)))
                             .x);
    R1i.xyz = as_type<int3>(tex0.sample(samplr0, float2(as_type<float>(R0i.x),
                                                        as_type<float>(R0i.y)))
                                .xyz);
    // export
    SET_POSITION(float4(as_type<float>(R2i.x), as_type<float>(R2i.y),
                        as_type<float>(R2i.z), as_type<float>(R2i.w)));
    // export
    // skipped export to semanticId 255
    // 0
    R126i.x = as_type<int>(as_type<float>(R3i.x) + -(0.5));
    R125i.y = as_type<int>(as_type<float>(R4i.w) + -(0.5));
    PV0iz = as_type<int>(-(as_type<float>(R2i.y)));
    PV0iz = as_type<int>(as_type<float>(PV0iz) / 2.0);
    R7i.w = R5i.y;
    R127i.y = as_type<int>(-(as_type<float>(R6i.x)));
    R127i.y = as_type<int>(as_type<float>(R127i.y) / 2.0);
    // 1
    R127i.x = as_type<int>(-(as_type<float>(R6i.y)));
    R127i.x = as_type<int>(as_type<float>(R127i.x) / 2.0);
    R126i.y = as_type<int>(as_type<float>(R3i.w) + as_type<float>(R4i.w));
    PV1iz =
        as_type<int>(as_type<float>(R7i.w) +
                     as_type<float>(supportBuffer.remapped[4].w) / resYScale);
    PV1iw = as_type<int>(as_type<float>(PV0iz) + as_type<float>(R3i.x));
    R127i.w =
        as_type<int>(as_type<float>(R7i.z) +
                     as_type<float>(supportBuffer.remapped[4].z) / resXScale);
    // 2
    backupReg0i = R127i.y;
    R125i.x = as_type<int>(as_type<float>(PV1iz) + as_type<float>(R126i.x));
    R127i.y = as_type<int>(as_type<float>(R5i.x) + as_type<float>(R125i.y));
    R126i.z =
        as_type<int>(mul_nonIEEE(as_type<float>(R6i.y), as_type<float>(PV1iw)));
    R126i.w =
        as_type<int>(mul_nonIEEE(as_type<float>(R6i.x), as_type<float>(R4i.w)) +
                     as_type<float>(backupReg0i));
    R125i.w =
        as_type<int>(as_type<float>(supportBuffer.remapped[4].z) / resXScale);
    // 3
    backupReg0i = R127i.x;
    R127i.x = as_type<int>(
        -(as_type<float>(supportBuffer.remapped[4].w) / resYScale));
    R126i.y = as_type<int>(
        mul_nonIEEE(as_type<float>(R6i.x), as_type<float>(R126i.y)));
    R127i.z =
        as_type<int>(mul_nonIEEE(as_type<float>(R6i.y), as_type<float>(R3i.x)) +
                     as_type<float>(backupReg0i));
    R124i.w =
        as_type<int>(as_type<float>(supportBuffer.remapped[4].x) * resXScale);
    R124i.w = as_type<int>(as_type<float>(R124i.w) / 2.0);
    R125i.z =
        as_type<int>(as_type<float>(supportBuffer.remapped[4].y) * resYScale);
    R125i.z = as_type<int>(as_type<float>(R125i.z) / 2.0);
    // 4
    backupReg0i = R126i.x;
    backupReg1i = R127i.y;
    R126i.x =
        as_type<int>(as_type<float>(supportBuffer.remapped[4].w) / resYScale);
    R126i.x = as_type<int>(as_type<float>(R126i.x) * 2.0);
    R127i.y =
        as_type<int>(as_type<float>(supportBuffer.remapped[4].z) / resXScale);
    R127i.y = as_type<int>(as_type<float>(R127i.y) * 2.0);
    R124i.z = as_type<int>(as_type<float>(R5i.y) + as_type<float>(backupReg0i));
    R127i.w = as_type<int>(as_type<float>(R127i.w) + as_type<float>(R125i.y));
    R4i.w = as_type<int>(
        mul_nonIEEE(as_type<float>(R6i.x), as_type<float>(backupReg1i)));
    // 5
    backupReg0i = R127i.z;
    R2i.x = as_type<int>(as_type<float>(R125i.w) * 1.5);
    R2i.y = as_type<int>(as_type<float>(R127i.x) * 1.5);
    R127i.z = as_type<int>(
        mul_nonIEEE(as_type<float>(R6i.y), as_type<float>(R125i.x)));
    R6i.w = backupReg0i;
    R6i.z = R126i.w;
    // 6
    R3i.x = as_type<int>(
        mul_nonIEEE(as_type<float>(R5i.x), as_type<float>(R124i.w)) + 0.5);
    R3i.y = as_type<int>(
        mul_nonIEEE(as_type<float>(R5i.y), as_type<float>(R125i.z)) + 0.5);
    R5i.z = R126i.y;
    R5i.w = R126i.z;
    R3i.z = R127i.y;
    // 7
    R4i.x = as_type<int>(
        mul_nonIEEE(as_type<float>(R6i.x), as_type<float>(R127i.w)));
    R4i.y = as_type<int>(
        mul_nonIEEE(as_type<float>(R6i.y), as_type<float>(R124i.z)));
    R4i.z = R4i.w;
    R3i.w = R126i.x;
    R4i.w = R127i.z;
    // export
    out.passParameterSem4 =
        float4(as_type<float>(R3i.x), as_type<float>(R3i.y),
               as_type<float>(R3i.z), as_type<float>(R3i.w));
    // export
    // skipped export to semanticId 255
    // export
    out.passParameterSem0 =
        float4(as_type<float>(R5i.x), as_type<float>(R5i.y),
               as_type<float>(R5i.z), as_type<float>(R5i.w));
    // export
    out.passParameterSem1 =
        float4(as_type<float>(R6i.x), as_type<float>(R6i.y),
               as_type<float>(R6i.z), as_type<float>(R6i.w));
    // export
    out.passParameterSem3 =
        float4(as_type<float>(R2i.x), as_type<float>(R2i.y),
               as_type<float>(R2i.z), as_type<float>(R2i.z));
    // export
    out.passParameterSem6 =
        float4(as_type<float>(R7i.x), as_type<float>(R7i.x),
               as_type<float>(R7i.z), as_type<float>(R7i.w));
    return out;
}
)BOTW_MSL_END"},
{0x30c44ffef434ce5eULL, 0x0000000000000000ULL, GraphicPack2::GP_SHADER_TYPE::VERTEX, R"BOTW_MSL_END(#include <metal_stdlib>
using namespace metal;

// shader 30c44ffef434ce5e
// Shader dumped from BotW 208, using Cemu 2193a8c
// Based on 0d6127fbed646d2b_0000000000000000_vs.txt
// Used for: Fixing Ambient-Occlusion

constant float resXScale = float($width) / float($gameWidth);
constant float resYScale = float($height) / float($gameHeight);

#define SET_POSITION(_v)                                                       \
    out.position = _v;                                                         \
    out.position.z = (out.position.z + out.position.w) / 2.0
// start of shader inputs/outputs, predetermined by Cemu. Do not touch
struct SupportBuffer {
    int4 remapped[5];
};

struct VertexIn {
    uint4 attrDataSem0 [[attribute(0)]];
    uint4 attrDataSem1 [[attribute(1)]];
};

struct VertexOut {
    float4 position [[position]] [[invariant]];
    float4 passParameterSem0 [[user(locn0)]];
    float4 passParameterSem5 [[user(locn1)]];
    float4 passParameterSem6 [[user(locn2)]];
};

// end of shader inputs/outputs
template <typename TextureT, typename CoordT>
float sampleCompareEmulate(TextureT tex, sampler samplr, CoordT coord,
                           float compareValue) {
    return compareValue < tex.sample(samplr, coord).x ? 1.0 : 0.0;
}
template <typename TextureT, typename CoordT>
float2 textureCalculateLod(TextureT tex, sampler samplr, CoordT coord) {
    float lod = tex.calculate_unclamped_lod(samplr, coord);
    return float2(floor(lod), fract(lod));
}
int clampFI32(int v) {
    if (v == 0x7FFFFFFF)
        return as_type<int>(1.0);
    else if (v == 0xFFFFFFFF)
        return as_type<int>(0.0);
    return as_type<int>(clamp(as_type<float>(v), 0.0, 1.0));
}
float mul_nonIEEE(float a, float b) {
    if (a == 0.0 || b == 0.0)
        return 0.0;
    return a * b;
}
vertex VertexOut main0(uint vid [[vertex_id]], uint iid [[instance_id]],
                       VertexIn in [[stage_in]],
                       constant SupportBuffer &supportBuffer [[buffer(0)]],
                       texture2d<float> tex0 [[texture(0)]],
                       sampler samplr0 [[sampler(0)]]) {
    VertexOut out;
    int4 R0i = int4(0);
    int4 R1i = int4(0);
    int4 R2i = int4(0);
    int4 R3i = int4(0);
    int4 R4i = int4(0);
    int4 R123i = int4(0);
    int4 R124i = int4(0);
    int4 R125i = int4(0);
    int4 R126i = int4(0);
    int4 R127i = int4(0);
    uint4 attrDecoder;
    int backupReg0i, backupReg1i, backupReg2i, backupReg3i, backupReg4i;
    int PV0ix = 0, PV0iy = 0, PV0iz = 0, PV0iw = 0, PV1ix = 0, PV1iy = 0,
        PV1iz = 0, PV1iw = 0;
    int PS0i = 0, PS1i = 0;
    int4 tempi = int4(0);
    float tempResultf;
    int tempResulti;
    int4 ARi = int4(0);
    bool predResult = true;
    R0i = int4(vid, 0, 0, iid);
    attrDecoder.xyzw = in.attrDataSem0.xyzw;
    attrDecoder = ((attrDecoder >> 8) & 0xFF) | ((attrDecoder << 8) & 0xFF00);
    attrDecoder.xyzw = as_type<uint4>(
        float4(float2(as_type<half2>(attrDecoder.x | (attrDecoder.y << 16))),
               float2(as_type<half2>(attrDecoder.z | (attrDecoder.w << 16)))));
    R1i = int4(int(attrDecoder.x), int(attrDecoder.y), int(attrDecoder.z),
               int(attrDecoder.w));
    attrDecoder.x = in.attrDataSem1.x;
    attrDecoder.x = (attrDecoder.x >> 24) | ((attrDecoder.x >> 8) & 0xFF00) |
                    ((attrDecoder.x << 8) & 0xFF0000) | ((attrDecoder.x << 24));
    attrDecoder.y = 0;
    attrDecoder.z = 0;
    attrDecoder.w = 0;
    attrDecoder.xyzw =
        uint4((attrDecoder.x >> 0) & 0x3FF, (attrDecoder.x >> 10) & 0x3FF,
              (attrDecoder.x >> 20) & 0x3FF, (attrDecoder.x >> 30) & 0x3);
    if ((attrDecoder.x & 0x200) != 0)
        attrDecoder.x |= 0xFFFFFC00;
    if ((attrDecoder.y & 0x200) != 0)
        attrDecoder.y |= 0xFFFFFC00;
    if ((attrDecoder.z & 0x200) != 0)
        attrDecoder.z |= 0xFFFFFC00;
    attrDecoder.x = as_type<uint>(max(float(int(attrDecoder.x)) / 511.0, -1.0));
    attrDecoder.y = as_type<uint>(max(float(int(attrDecoder.y)) / 511.0, -1.0));
    attrDecoder.z = as_type<uint>(max(float(int(attrDecoder.z)) / 511.0, -1.0));
    attrDecoder.w = as_type<uint>(float(attrDecoder.w));
    R2i = int4(int(attrDecoder.x), int(attrDecoder.y), int(attrDecoder.z),
               int(attrDecoder.w));
    // 0
    PV0ix = int(0x3f800000);
    PV0iy = int(0x40400000);
    PV0iz = (R0i.x == int(1)) ? -1 : 0;
    R127i.w = int(0x3f800000);
    R127i.x = int(0xbf800000);
    // 1
    R3i.x = supportBuffer.remapped[0].z;
    R3i.x = as_type<int>(as_type<float>(R3i.x) / 2.0);
    PV1iy = as_type<int>(as_type<float>(supportBuffer.remapped[1].z) *
                         as_type<float>(0x3b808081));
    R123i.z = ((PV0iz == 0) ? (PV0ix) : (int(0xc0400000)));
    R123i.w = ((PV0iz == 0) ? (PV0iy) : (int(0xbf800000)));
    R2i.w = int(0x3f800000);
    // 2
    R2i.x = ((R0i.x == 0) ? (R127i.x) : (R123i.w));
    R2i.y = ((R0i.x == 0) ? (R127i.w) : (R123i.z));
    R2i.z = as_type<int>(as_type<float>(PV1iy) + -(0.5));
    R2i.z = as_type<int>(as_type<float>(R2i.z) * 2.0);
    R0i.w = int(0x3d2aaaab);
    R0i.y = 0;
    // 3
    R1i.x = int(0x3e000000);
    R1i.y = 0;
    R1i.z = as_type<int>(-(as_type<float>(R2i.y)));
    R1i.w = as_type<int>(-(as_type<float>(supportBuffer.remapped[2].z)));
    R1i.w = as_type<int>(as_type<float>(R1i.w) / 2.0);
    R4i.x = as_type<int>(as_type<float>(R2i.x) * 0.5 + 0.5);
    R0i.w = as_type<int>(tex0.sample(samplr0, float2(as_type<float>(R0i.w),
                                                     as_type<float>(R0i.y)))
                             .x);
    R0i.xyz = as_type<int3>(tex0.sample(samplr0, float2(as_type<float>(R1i.x),
                                                        as_type<float>(R1i.y)))
                                .xyz);
    // export
    SET_POSITION(float4(as_type<float>(R2i.x), as_type<float>(R2i.y),
                        as_type<float>(R2i.z), as_type<float>(R2i.w)));
    // export
    // skipped export to semanticId 255
    // 0
    R126i.x = as_type<int>(as_type<float>(R3i.x) + -(0.5));
    R4i.y = as_type<int>(as_type<float>(R1i.z) * 0.5 + 0.5);
    PV0iz = as_type<int>(-(as_type<float>(R2i.y)));
    PV0iz = as_type<int>(as_type<float>(PV0iz) / 2.0);
    PV0iw = R2i.x;
    PV0iw = as_type<int>(as_type<float>(PV0iw) / 2.0);
    R125i.z = as_type<int>(as_type<float>(R1i.w) + -(0.5));
    // 1
    R123i.x = as_type<int>(as_type<float>(supportBuffer.remapped[3].w) * 2.0 /
                               resYScale +
                           as_type<float>(R4i.y));
    R126i.y = as_type<int>(-(as_type<float>(supportBuffer.remapped[4].x)));
    R126i.y = as_type<int>(as_type<float>(R126i.y) * 2.0);
    R1i.z = supportBuffer.remapped[4].y;
    R1i.z = as_type<int>(as_type<float>(R1i.z) * 2.0);
    R127i.w = as_type<int>(as_type<float>(PV0iw) + as_type<float>(R3i.x));
    R127i.z = as_type<int>(as_type<float>(PV0iz) + as_type<float>(R1i.w));
    // 2
    R127i.x =
        as_type<int>(as_type<float>(supportBuffer.remapped[3].y) * resYScale);
    R127i.x = as_type<int>(as_type<float>(R127i.x) / 2.0);
    PV0iy =
        as_type<int>(as_type<float>(supportBuffer.remapped[3].x) * resXScale);
    PV0iy = as_type<int>(as_type<float>(PV0iy) / 2.0);
    R126i.z = as_type<int>(as_type<float>(supportBuffer.remapped[3].z) * 2.0 /
                               resXScale +
                           as_type<float>(R4i.x));
    R126i.w = as_type<int>(as_type<float>(R4i.x) + as_type<float>(R126i.x));
    R124i.z = as_type<int>(as_type<float>(R123i.x) + as_type<float>(R125i.z));
    // 3
    R125i.x = as_type<int>(
        mul_nonIEEE(as_type<float>(R1i.z), as_type<float>(R127i.z)));
    R127i.y = as_type<int>(
        mul_nonIEEE(as_type<float>(R126i.y), as_type<float>(R127i.w)));
    R127i.z = as_type<int>(
        -(as_type<float>(supportBuffer.remapped[3].w) / resYScale) *
            as_type<float>(0x3c23d70a) +
        as_type<float>(R4i.y));
    R123i.w = as_type<int>(
        -(as_type<float>(supportBuffer.remapped[3].z) / resXScale) *
            as_type<float>(0x3c23d70a) +
        as_type<float>(R4i.x));
    R127i.w =
        as_type<int>(mul_nonIEEE(as_type<float>(R4i.x), as_type<float>(PV0iy)));
    // 4
    PV0ix = as_type<int>(as_type<float>(R4i.y) + as_type<float>(R125i.z));
    PV0iy = as_type<int>(as_type<float>(R126i.z) + as_type<float>(R126i.x));
    R126i.z = as_type<int>(
        mul_nonIEEE(as_type<float>(R4i.y), as_type<float>(R127i.x)));
    R126i.w = as_type<int>(
        mul_nonIEEE(as_type<float>(R126i.y), as_type<float>(R126i.w)));
    R3i.z = R123i.w;
    // 5
    R1i.x = as_type<int>(
        mul_nonIEEE(as_type<float>(R126i.y), as_type<float>(PV0iy)));
    R1i.y =
        as_type<int>(mul_nonIEEE(as_type<float>(R1i.z), as_type<float>(PV0ix)));
    PV1iz = as_type<int>(
        mul_nonIEEE(as_type<float>(R1i.z), as_type<float>(R124i.z)));
    R3i.w = R127i.z;
    R4i.z = R127i.y;
    // 6
    R3i.x = as_type<int>(as_type<float>(R127i.w) * 0.25);
    R3i.y = as_type<int>(as_type<float>(R126i.z) * 0.25);
    R1i.z = R126i.w;
    R4i.w = R125i.x;
    R1i.w = PV1iz;
    // export
    out.passParameterSem0 =
        float4(as_type<float>(R4i.x), as_type<float>(R4i.y),
               as_type<float>(R4i.z), as_type<float>(R4i.w));
    // export
    out.passParameterSem5 =
        float4(as_type<float>(R1i.x), as_type<float>(R1i.y),
               as_type<float>(R1i.z), as_type<float>(R1i.w));
    // export
    out.passParameterSem6 =
        float4(as_type<float>(R3i.x), as_type<float>(R3i.y),
               as_type<float>(R3i.z), as_type<float>(R3i.w));
    return out;
}
)BOTW_MSL_END"},
{0x35910f7af2d6ad8bULL, 0x0000000000000000ULL, GraphicPack2::GP_SHADER_TYPE::VERTEX, R"BOTW_MSL_END(#include <metal_stdlib>
using namespace metal;

// shader 35910f7af2d6ad8b
// Shader dumped from BotW 208, using Cemu 2193a8c
// Based on 0b9b8f5dfa16ad58_0000000000000000_vs.txt
// Used For: Horizontal Menu Blur

constant float resXScale = float($width) / float($gameWidth);

#define SET_POSITION(_v)                                                       \
    out.position = _v;                                                         \
    out.position.z = (out.position.z + out.position.w) / 2.0
// start of shader inputs/outputs, predetermined by Cemu. Do not touch
struct SupportBuffer {
    int4 remapped[5];
};

struct VertexIn {
    uint4 attrDataSem0 [[attribute(0)]];
};

struct VertexOut {
    float4 position [[position]] [[invariant]];
    float4 passParameterSem0 [[user(locn0)]];
};

// end of shader inputs/outputs
template <typename TextureT, typename CoordT>
float sampleCompareEmulate(TextureT tex, sampler samplr, CoordT coord,
                           float compareValue) {
    return compareValue < tex.sample(samplr, coord).x ? 1.0 : 0.0;
}
template <typename TextureT, typename CoordT>
float2 textureCalculateLod(TextureT tex, sampler samplr, CoordT coord) {
    float lod = tex.calculate_unclamped_lod(samplr, coord);
    return float2(floor(lod), fract(lod));
}
int clampFI32(int v) {
    if (v == 0x7FFFFFFF)
        return as_type<int>(1.0);
    else if (v == 0xFFFFFFFF)
        return as_type<int>(0.0);
    return as_type<int>(clamp(as_type<float>(v), 0.0, 1.0));
}
float mul_nonIEEE(float a, float b) {
    if (a == 0.0 || b == 0.0)
        return 0.0;
    return a * b;
}
vertex VertexOut main0(uint vid [[vertex_id]], uint iid [[instance_id]],
                       VertexIn in [[stage_in]],
                       constant SupportBuffer &supportBuffer [[buffer(0)]]) {
    VertexOut out;
    float4 R0f = float4(0.0);
    float4 R1f = float4(0.0);
    float4 R126f = float4(0.0);
    float4 R127f = float4(0.0);
    uint4 attrDecoder;
    float backupReg0f, backupReg1f, backupReg2f, backupReg3f, backupReg4f;
    float PV0fx = 0.0, PV0fy = 0.0, PV0fz = 0.0, PV0fw = 0.0, PV1fx = 0.0,
          PV1fy = 0.0, PV1fz = 0.0, PV1fw = 0.0;
    float PS0f = 0.0, PS1f = 0.0;
    float4 tempf = float4(0.0);
    float tempResultf;
    int tempResulti;
    int4 ARi = int4(0);
    bool predResult = true;
    R0f = float4(vid, 0, 0, iid);
    attrDecoder.xyz = in.attrDataSem0.xyz;
    attrDecoder.xyz =
        (attrDecoder.xyz >> 24) | ((attrDecoder.xyz >> 8) & 0xFF00) |
        ((attrDecoder.xyz << 8) & 0xFF0000) | ((attrDecoder.xyz << 24));
    attrDecoder.w = 0;
    R1f = float4(
        as_type<float>(int(attrDecoder.x)), as_type<float>(int(attrDecoder.y)),
        as_type<float>(int(attrDecoder.z)), as_type<float>(0x3f800000));
    // 0
    R127f.x = (R1f.x > 0.0) ? 1.0 : 0.0;
    R127f.x /= 2.0;
    R127f.y = -(R1f.y);
    R127f.z = (0.0 > R1f.x) ? 1.0 : 0.0;
    R127f.z /= 2.0;
    R127f.w = 1.0;
    R126f.x = as_type<float>(supportBuffer.remapped[0].z) *
              (as_type<float>(0x3fae8a72) / resXScale);
    // 1
    R0f.x = dot(float4(R1f.x, R1f.y, R1f.z, R127f.w),
                float4(as_type<float>(supportBuffer.remapped[1].x),
                       as_type<float>(supportBuffer.remapped[1].y),
                       as_type<float>(supportBuffer.remapped[1].z),
                       as_type<float>(supportBuffer.remapped[1].w)));
    PV1fy = R0f.x;
    PV1fz = R0f.x;
    PV1fw = R0f.x;
    R126f.w = (R127f.y > 0.0) ? 1.0 : 0.0;
    R126f.w /= 2.0;
    // 2
    tempf.x = dot(float4(R1f.x, R1f.y, R1f.z, R127f.w),
                  float4(as_type<float>(supportBuffer.remapped[2].x),
                         as_type<float>(supportBuffer.remapped[2].y),
                         as_type<float>(supportBuffer.remapped[2].z),
                         as_type<float>(supportBuffer.remapped[2].w)));
    PV0fx = tempf.x;
    R0f.y = tempf.x;
    PV0fz = tempf.x;
    PV0fw = tempf.x;
    R127f.y = (0.0 > R127f.y) ? 1.0 : 0.0;
    R127f.y /= 2.0;
    // 3
    tempf.x = dot(float4(R1f.x, R1f.y, R1f.z, R127f.w),
                  float4(as_type<float>(supportBuffer.remapped[3].x),
                         as_type<float>(supportBuffer.remapped[3].y),
                         as_type<float>(supportBuffer.remapped[3].z),
                         as_type<float>(supportBuffer.remapped[3].w)));
    PV1fx = tempf.x;
    PV1fy = tempf.x;
    R0f.z = tempf.x;
    PV1fw = tempf.x;
    R127f.x = R127f.x + -(R127f.z);
    // 4
    tempf.x = dot(float4(R1f.x, R1f.y, R1f.z, R127f.w),
                  float4(as_type<float>(supportBuffer.remapped[4].x),
                         as_type<float>(supportBuffer.remapped[4].y),
                         as_type<float>(supportBuffer.remapped[4].z),
                         as_type<float>(supportBuffer.remapped[4].w)));
    PV0fx = tempf.x;
    PV0fy = tempf.x;
    PV0fz = tempf.x;
    R0f.w = tempf.x;
    PS0f = R126f.w + -(R127f.y);
    // 5
    PV1fy = PS0f + 0.5;
    PV1fz = R127f.x + 0.5;
    // 6
    R1f.x = PV1fy;
    R1f.y = PV1fz + -(R126f.x);
    R1f.z = PV1fz + R126f.x;
    R1f.w = PV1fz;
    // export
    SET_POSITION(float4(R0f.x, R0f.y, R0f.z, R0f.w));
    // export
    out.passParameterSem0 = float4(R1f.x, R1f.y, R1f.z, R1f.w);
    return out;
}
)BOTW_MSL_END"},
{0x3bea584604ecfe8dULL, 0x0000000000000000ULL, GraphicPack2::GP_SHADER_TYPE::VERTEX, R"BOTW_MSL_END(#include <metal_stdlib>
using namespace metal;

// shader 3bea584604ecfe8d
// Shader dumped from BotW 208, using Cemu 2193a8c
// Based on 141f484aff9b9f5a_0000000000000000_vs.txt
// Used for: Anti-Aliasing Color Mix

constant float resXScale = float($width) / float($gameWidth);
constant float resYScale = float($height) / float($gameHeight);

#define SET_POSITION(_v)                                                       \
    out.position = _v;                                                         \
    out.position.z = (out.position.z + out.position.w) / 2.0
// start of shader inputs/outputs, predetermined by Cemu. Do not touch
struct SupportBuffer {
    int4 remapped[3];
};

struct VertexIn {
    uint4 attrDataSem0 [[attribute(0)]];
    uint4 attrDataSem1 [[attribute(1)]];
};

struct VertexOut {
    float4 position [[position]] [[invariant]];
    float4 passParameterSem2 [[user(locn0)]];
};

// end of shader inputs/outputs
template <typename TextureT, typename CoordT>
float sampleCompareEmulate(TextureT tex, sampler samplr, CoordT coord,
                           float compareValue) {
    return compareValue < tex.sample(samplr, coord).x ? 1.0 : 0.0;
}
template <typename TextureT, typename CoordT>
float2 textureCalculateLod(TextureT tex, sampler samplr, CoordT coord) {
    float lod = tex.calculate_unclamped_lod(samplr, coord);
    return float2(floor(lod), fract(lod));
}
int clampFI32(int v) {
    if (v == 0x7FFFFFFF)
        return as_type<int>(1.0);
    else if (v == 0xFFFFFFFF)
        return as_type<int>(0.0);
    return as_type<int>(clamp(as_type<float>(v), 0.0, 1.0));
}
float mul_nonIEEE(float a, float b) {
    if (a == 0.0 || b == 0.0)
        return 0.0;
    return a * b;
}
vertex VertexOut main0(uint vid [[vertex_id]], uint iid [[instance_id]],
                       VertexIn in [[stage_in]],
                       constant SupportBuffer &supportBuffer [[buffer(0)]]) {
    VertexOut out;
    float4 R0f = float4(0.0);
    float4 R1f = float4(0.0);
    float4 R2f = float4(0.0);
    float4 R123f = float4(0.0);
    uint4 attrDecoder;
    float backupReg0f, backupReg1f, backupReg2f, backupReg3f, backupReg4f;
    float PV0fx = 0.0, PV0fy = 0.0, PV0fz = 0.0, PV0fw = 0.0, PV1fx = 0.0,
          PV1fy = 0.0, PV1fz = 0.0, PV1fw = 0.0;
    float PS0f = 0.0, PS1f = 0.0;
    float4 tempf = float4(0.0);
    float tempResultf;
    int tempResulti;
    int4 ARi = int4(0);
    bool predResult = true;
    R0f = float4(vid, 0, 0, iid);
    attrDecoder.xyz = in.attrDataSem0.xyz;
    attrDecoder.xyz =
        (attrDecoder.xyz >> 24) | ((attrDecoder.xyz >> 8) & 0xFF00) |
        ((attrDecoder.xyz << 8) & 0xFF0000) | ((attrDecoder.xyz << 24));
    attrDecoder.w = 0;
    R1f = float4(
        as_type<float>(int(attrDecoder.x)), as_type<float>(int(attrDecoder.y)),
        as_type<float>(int(attrDecoder.z)), as_type<float>(0x3f800000));
    attrDecoder.xy = in.attrDataSem1.xy;
    attrDecoder.xy = (attrDecoder.xy >> 24) | ((attrDecoder.xy >> 8) & 0xFF00) |
                     ((attrDecoder.xy << 8) & 0xFF0000) |
                     ((attrDecoder.xy << 24));
    attrDecoder.z = 0;
    attrDecoder.w = 0;
    R2f = float4(as_type<float>(int(attrDecoder.x)),
                 as_type<float>(int(attrDecoder.y)), as_type<float>(0),
                 as_type<float>(0x3f800000));
    // 0
    R1f.x = R1f.x;
    R1f.x *= 2.0;
    R1f.y = R1f.y;
    R1f.y *= 2.0;
    R1f.z = as_type<float>(supportBuffer.remapped[0].x);
    R1f.w = 1.0;
    PS0f = as_type<float>(supportBuffer.remapped[1].x) + 0.5;
    // 1
    R123f.x = mul_nonIEEE(-(PS0f), as_type<float>(supportBuffer.remapped[2].y) /
                                       resYScale) +
              R2f.y;
    R123f.y = mul_nonIEEE(-(PS0f), as_type<float>(supportBuffer.remapped[2].x) /
                                       resXScale) +
              R2f.x;
    // 2
    R2f.z = R123f.y;
    R2f.w = R123f.x;
    // export
    SET_POSITION(float4(R1f.x, R1f.y, R1f.z, R1f.w));
    // export
    out.passParameterSem2 = float4(R2f.x, R2f.y, R2f.z, R2f.w);
    return out;
}
)BOTW_MSL_END"},
{0x3d7806915fa6143dULL, 0x0000000000000000ULL, GraphicPack2::GP_SHADER_TYPE::VERTEX, R"BOTW_MSL_END(#include <metal_stdlib>
using namespace metal;

// shader 3d7806915fa6143d
// Shader dumped from BotW 208, using Cemu 2193a8c
// Based on 1a14de8e58d5b30a_0000000000000000_vs.txt
// Used for: Restoring the native BotW Anti-Aliasing implementation

constant float resXScale = float($width) / float($gameWidth);
constant float resYScale = float($height) / float($gameHeight);

#define SET_POSITION(_v)                                                       \
    out.position = _v;                                                         \
    out.position.z = (out.position.z + out.position.w) / 2.0
// start of shader inputs/outputs, predetermined by Cemu. Do not touch
struct SupportBuffer {
    int4 remapped[5];
};

struct VertexIn {
    uint4 attrDataSem0 [[attribute(0)]];
    uint4 attrDataSem1 [[attribute(1)]];
};

struct VertexOut {
    float4 position [[position]] [[invariant]];
    float4 passParameterSem2 [[user(locn2)]];
    float4 passParameterSem4 [[user(locn3)]];
    float4 passParameterSem0 [[user(locn0)]];
    float4 passParameterSem1 [[user(locn1)]];
    float4 passParameterSem6 [[user(locn4)]];
};

// end of shader inputs/outputs
template <typename TextureT, typename CoordT>
float sampleCompareEmulate(TextureT tex, sampler samplr, CoordT coord,
                           float compareValue) {
    return compareValue < tex.sample(samplr, coord).x ? 1.0 : 0.0;
}
template <typename TextureT, typename CoordT>
float2 textureCalculateLod(TextureT tex, sampler samplr, CoordT coord) {
    float lod = tex.calculate_unclamped_lod(samplr, coord);
    return float2(floor(lod), fract(lod));
}
int clampFI32(int v) {
    if (v == 0x7FFFFFFF)
        return as_type<int>(1.0);
    else if (v == 0xFFFFFFFF)
        return as_type<int>(0.0);
    return as_type<int>(clamp(as_type<float>(v), 0.0, 1.0));
}
float mul_nonIEEE(float a, float b) {
    if (a == 0.0 || b == 0.0)
        return 0.0;
    return a * b;
}
vertex VertexOut main0(uint vid [[vertex_id]], uint iid [[instance_id]],
                       VertexIn in [[stage_in]],
                       constant SupportBuffer &supportBuffer [[buffer(0)]],
                       texture2d<float> tex0 [[texture(0)]],
                       sampler samplr0 [[sampler(0)]]) {
    VertexOut out;
    int4 R0i = int4(0);
    int4 R1i = int4(0);
    int4 R2i = int4(0);
    int4 R3i = int4(0);
    int4 R4i = int4(0);
    int4 R5i = int4(0);
    int4 R123i = int4(0);
    int4 R124i = int4(0);
    int4 R125i = int4(0);
    int4 R126i = int4(0);
    int4 R127i = int4(0);
    uint4 attrDecoder;
    int backupReg0i, backupReg1i, backupReg2i, backupReg3i, backupReg4i;
    int PV0ix = 0, PV0iy = 0, PV0iz = 0, PV0iw = 0, PV1ix = 0, PV1iy = 0,
        PV1iz = 0, PV1iw = 0;
    int PS0i = 0, PS1i = 0;
    int4 tempi = int4(0);
    float tempResultf;
    int tempResulti;
    int4 ARi = int4(0);
    bool predResult = true;
    R0i = int4(vid, 0, 0, iid);
    attrDecoder.xyzw = in.attrDataSem0.xyzw;
    attrDecoder = ((attrDecoder >> 8) & 0xFF) | ((attrDecoder << 8) & 0xFF00);
    attrDecoder.xyzw = as_type<uint4>(
        float4(float2(as_type<half2>(attrDecoder.x | (attrDecoder.y << 16))),
               float2(as_type<half2>(attrDecoder.z | (attrDecoder.w << 16)))));
    R1i = int4(int(attrDecoder.x), int(attrDecoder.y), int(attrDecoder.z),
               int(attrDecoder.w));
    attrDecoder.x = in.attrDataSem1.x;
    attrDecoder.x = (attrDecoder.x >> 24) | ((attrDecoder.x >> 8) & 0xFF00) |
                    ((attrDecoder.x << 8) & 0xFF0000) | ((attrDecoder.x << 24));
    attrDecoder.y = 0;
    attrDecoder.z = 0;
    attrDecoder.w = 0;
    attrDecoder.xyzw =
        uint4((attrDecoder.x >> 0) & 0x3FF, (attrDecoder.x >> 10) & 0x3FF,
              (attrDecoder.x >> 20) & 0x3FF, (attrDecoder.x >> 30) & 0x3);
    if ((attrDecoder.x & 0x200) != 0)
        attrDecoder.x |= 0xFFFFFC00;
    if ((attrDecoder.y & 0x200) != 0)
        attrDecoder.y |= 0xFFFFFC00;
    if ((attrDecoder.z & 0x200) != 0)
        attrDecoder.z |= 0xFFFFFC00;
    attrDecoder.x = as_type<uint>(max(float(int(attrDecoder.x)) / 511.0, -1.0));
    attrDecoder.y = as_type<uint>(max(float(int(attrDecoder.y)) / 511.0, -1.0));
    attrDecoder.z = as_type<uint>(max(float(int(attrDecoder.z)) / 511.0, -1.0));
    attrDecoder.w = as_type<uint>(float(attrDecoder.w));
    R2i = int4(int(attrDecoder.x), int(attrDecoder.y), int(attrDecoder.z),
               int(attrDecoder.w));
    // 0
    PV0ix = (R0i.x == int(1)) ? -1 : 0;
    R127i.y = int(0x3f800000);
    PV0iz = int(0x3f800000);
    PV0iw = int(0x40400000);
    R127i.z = int(0xbf800000);
    // 1
    R123i.x = ((PV0ix == 0) ? (PV0iz) : (int(0xc0400000)));
    R123i.y = ((PV0ix == 0) ? (PV0iw) : (int(0xbf800000)));
    R0i.z = 0;
    PV1iw = as_type<int>(as_type<float>(supportBuffer.remapped[0].z) *
                         as_type<float>(0x3b808081));
    R2i.w = int(0x3f800000);
    // 2
    R2i.x = ((R0i.x == 0) ? (R127i.z) : (R123i.y));
    R2i.y = ((R0i.x == 0) ? (R127i.y) : (R123i.x));
    R2i.z = as_type<int>(as_type<float>(PV1iw) + -(0.5));
    R2i.z = as_type<int>(as_type<float>(R2i.z) * 2.0);
    R0i.w = int(0x3ec00000);
    R0i.x = int(0x3eeaaaab);
    // 3
    R1i.x = int(0x3d2aaaab);
    R0i.y = 0;
    R1i.z = 0;
    R1i.w = as_type<int>(-(as_type<float>(R2i.y)));
    R5i.x = as_type<int>(as_type<float>(R2i.x) * 0.5 + 0.5);
    R3i.xyz = as_type<int3>(tex0.sample(samplr0, float2(as_type<float>(R0i.w),
                                                        as_type<float>(R0i.z)))
                                .xyz);
    R0i.xyz = as_type<int3>(tex0.sample(samplr0, float2(as_type<float>(R0i.x),
                                                        as_type<float>(R0i.y)))
                                .xyz);
    R4i.w = as_type<int>(tex0.sample(samplr0, float2(as_type<float>(R1i.x),
                                                     as_type<float>(R1i.z)))
                             .y);
    // export
    SET_POSITION(float4(as_type<float>(R2i.x), as_type<float>(R2i.y),
                        as_type<float>(R2i.z), as_type<float>(R2i.w)));
    // 0
    R127i.x = supportBuffer.remapped[1].z;
    R127i.x = as_type<int>(as_type<float>(R127i.x) / 2.0);
    R5i.y = as_type<int>(as_type<float>(R1i.w) * 0.5 + 0.5);
    R126i.z = as_type<int>(-(as_type<float>(supportBuffer.remapped[2].z)));
    R126i.z = as_type<int>(as_type<float>(R126i.z) / 2.0);
    R127i.w = R2i.x;
    R127i.w = as_type<int>(as_type<float>(R127i.w) / 2.0);
    R1i.z = R5i.x;
    // 1
    backupReg0i = R2i.y;
    R2i.x = as_type<int>(-(as_type<float>(supportBuffer.remapped[3].x)));
    R2i.x = as_type<int>(as_type<float>(R2i.x) * 2.0);
    R2i.y = supportBuffer.remapped[3].y;
    R2i.y = as_type<int>(as_type<float>(R2i.y) * 2.0);
    R127i.z = as_type<int>(-(as_type<float>(backupReg0i)));
    R127i.z = as_type<int>(as_type<float>(R127i.z) / 2.0);
    R1i.w = R5i.y;
    R126i.y = as_type<int>(as_type<float>(R127i.x) + -(0.5));
    // 2
    backupReg0i = R127i.w;
    R126i.x = as_type<int>(as_type<float>(R126i.z) + -(0.5));
    PV0iy = as_type<int>(-(as_type<float>(R2i.x)));
    PV0iy = as_type<int>(as_type<float>(PV0iy) / 2.0);
    PV0iz =
        as_type<int>(as_type<float>(R1i.w) +
                     as_type<float>(supportBuffer.remapped[4].w) / resYScale);
    R127i.w = as_type<int>(-(as_type<float>(R2i.y)));
    R127i.w = as_type<int>(as_type<float>(R127i.w) / 2.0);
    R127i.y =
        as_type<int>(as_type<float>(backupReg0i) + as_type<float>(R127i.x));
    // 3
    PV1ix = as_type<int>(as_type<float>(R127i.z) + as_type<float>(R126i.z));
    R125i.y = as_type<int>(as_type<float>(R5i.x) + as_type<float>(R126i.y));
    R127i.z = as_type<int>(as_type<float>(PV0iz) + as_type<float>(R126i.x));
    R126i.w =
        as_type<int>(as_type<float>(R1i.z) +
                     as_type<float>(supportBuffer.remapped[4].z) / resXScale);
    R125i.w = as_type<int>(
        mul_nonIEEE(as_type<float>(R2i.x), as_type<float>(R127i.x)) +
        as_type<float>(PV0iy));
    // 4
    R127i.x =
        as_type<int>(mul_nonIEEE(as_type<float>(R2i.y), as_type<float>(PV1ix)));
    R127i.y = as_type<int>(
        mul_nonIEEE(as_type<float>(R2i.x), as_type<float>(R127i.y)));
    R126i.z = as_type<int>(
        mul_nonIEEE(as_type<float>(R2i.y), as_type<float>(R126i.z)) +
        as_type<float>(R127i.w));
    R127i.w =
        as_type<int>(as_type<float>(supportBuffer.remapped[4].x) * resXScale);
    R127i.w = as_type<int>(as_type<float>(R127i.w) / 2.0);
    R125i.z =
        as_type<int>(as_type<float>(supportBuffer.remapped[4].y) * resYScale);
    R125i.z = as_type<int>(as_type<float>(R125i.z) / 2.0);
    // 5
    backupReg0i = R126i.x;
    backupReg1i = R126i.y;
    R126i.x =
        as_type<int>(as_type<float>(supportBuffer.remapped[4].w) / resYScale);
    R126i.x = as_type<int>(as_type<float>(R126i.x) * 2.0);
    R126i.y =
        as_type<int>(as_type<float>(supportBuffer.remapped[4].z) / resXScale);
    R126i.y = as_type<int>(as_type<float>(R126i.y) * 2.0);
    R124i.z = as_type<int>(as_type<float>(R5i.y) + as_type<float>(backupReg0i));
    R126i.w =
        as_type<int>(as_type<float>(R126i.w) + as_type<float>(backupReg1i));
    R124i.w = as_type<int>(
        mul_nonIEEE(as_type<float>(R2i.x), as_type<float>(R125i.y)));
    // 6
    backupReg0i = R125i.w;
    PV0ix = as_type<int>(-(as_type<float>(R3i.z)) + as_type<float>(R0i.z));
    PV0iy = as_type<int>(-(as_type<float>(R3i.y)) + as_type<float>(R0i.y));
    PV0iz = as_type<int>(-(as_type<float>(R3i.x)) + as_type<float>(R0i.x));
    R125i.w = as_type<int>(
        mul_nonIEEE(as_type<float>(R2i.y), as_type<float>(R127i.z)));
    R2i.z = backupReg0i;
    // 7
    R4i.x =
        as_type<int>(mul_nonIEEE(as_type<float>(PV0iz), as_type<float>(R5i.y)) +
                     as_type<float>(R3i.x));
    R4i.y =
        as_type<int>(mul_nonIEEE(as_type<float>(PV0iy), as_type<float>(R5i.y)) +
                     as_type<float>(R3i.y));
    R4i.z =
        as_type<int>(mul_nonIEEE(as_type<float>(PV0ix), as_type<float>(R5i.y)) +
                     as_type<float>(R3i.z));
    R2i.w = R126i.z;
    R5i.z = R127i.y;
    // 8
    R3i.x = as_type<int>(
        mul_nonIEEE(as_type<float>(R5i.x), as_type<float>(R127i.w)) + 0.5);
    R3i.y = as_type<int>(
        mul_nonIEEE(as_type<float>(R5i.y), as_type<float>(R125i.z)) + 0.5);
    R3i.z = R126i.y;
    R5i.w = R127i.x;
    R3i.w = R126i.x;
    // 9
    R0i.x = as_type<int>(
        mul_nonIEEE(as_type<float>(R2i.x), as_type<float>(R126i.w)));
    R0i.y = as_type<int>(
        mul_nonIEEE(as_type<float>(R2i.y), as_type<float>(R124i.z)));
    R0i.z = R124i.w;
    R0i.w = R125i.w;
    // export
    out.passParameterSem2 =
        float4(as_type<float>(R4i.x), as_type<float>(R4i.y),
               as_type<float>(R4i.z), as_type<float>(R4i.w));
    // export
    out.passParameterSem4 =
        float4(as_type<float>(R3i.x), as_type<float>(R3i.y),
               as_type<float>(R3i.z), as_type<float>(R3i.w));
    // export
    // skipped export to semanticId 255
    // export
    out.passParameterSem0 =
        float4(as_type<float>(R5i.x), as_type<float>(R5i.y),
               as_type<float>(R5i.z), as_type<float>(R5i.w));
    // export
    out.passParameterSem1 =
        float4(as_type<float>(R2i.x), as_type<float>(R2i.y),
               as_type<float>(R2i.z), as_type<float>(R2i.w));
    // export
    out.passParameterSem6 =
        float4(as_type<float>(R1i.x), as_type<float>(R1i.x),
               as_type<float>(R1i.z), as_type<float>(R1i.w));
    return out;
}
)BOTW_MSL_END"},
{0x3d7aeadf35ccf970ULL, 0x0000000000000000ULL, GraphicPack2::GP_SHADER_TYPE::VERTEX, R"BOTW_MSL_END(#include <metal_stdlib>
using namespace metal;

// shader 3d7aeadf35ccf970
// Shader dumped from BotW 208, using Cemu 2193a8c
// Based on 15afdae4307b9a3d_0000000000000000_vs.txt
// Used for: Vertical Ambient-Occlusion Blur

constant float resYScale = float($height) / float($gameHeight);

#define SET_POSITION(_v)                                                       \
    out.position = _v;                                                         \
    out.position.z = (out.position.z + out.position.w) / 2.0
// start of shader inputs/outputs, predetermined by Cemu. Do not touch
struct SupportBuffer {
    int4 remapped[1];
};

struct VertexIn {
    uint4 attrDataSem0 [[attribute(0)]];
    uint4 attrDataSem1 [[attribute(1)]];
};

struct VertexOut {
    float4 position [[position]] [[invariant]];
    float4 passParameterSem0 [[user(locn0)]];
};

// end of shader inputs/outputs
template <typename TextureT, typename CoordT>
float sampleCompareEmulate(TextureT tex, sampler samplr, CoordT coord,
                           float compareValue) {
    return compareValue < tex.sample(samplr, coord).x ? 1.0 : 0.0;
}
template <typename TextureT, typename CoordT>
float2 textureCalculateLod(TextureT tex, sampler samplr, CoordT coord) {
    float lod = tex.calculate_unclamped_lod(samplr, coord);
    return float2(floor(lod), fract(lod));
}
int clampFI32(int v) {
    if (v == 0x7FFFFFFF)
        return as_type<int>(1.0);
    else if (v == 0xFFFFFFFF)
        return as_type<int>(0.0);
    return as_type<int>(clamp(as_type<float>(v), 0.0, 1.0));
}
float mul_nonIEEE(float a, float b) {
    if (a == 0.0 || b == 0.0)
        return 0.0;
    return a * b;
}
vertex VertexOut main0(uint vid [[vertex_id]], uint iid [[instance_id]],
                       VertexIn in [[stage_in]],
                       constant SupportBuffer &supportBuffer [[buffer(0)]]) {
    VertexOut out;
    float4 R0f = float4(0.0);
    float4 R1f = float4(0.0);
    float4 R2f = float4(0.0);
    uint4 attrDecoder;
    float backupReg0f, backupReg1f, backupReg2f, backupReg3f, backupReg4f;
    float PV0fx = 0.0, PV0fy = 0.0, PV0fz = 0.0, PV0fw = 0.0, PV1fx = 0.0,
          PV1fy = 0.0, PV1fz = 0.0, PV1fw = 0.0;
    float PS0f = 0.0, PS1f = 0.0;
    float4 tempf = float4(0.0);
    float tempResultf;
    int tempResulti;
    int4 ARi = int4(0);
    bool predResult = true;
    R0f = float4(vid, 0, 0, iid);
    attrDecoder.xyz = in.attrDataSem0.xyz;
    attrDecoder.xyz =
        (attrDecoder.xyz >> 24) | ((attrDecoder.xyz >> 8) & 0xFF00) |
        ((attrDecoder.xyz << 8) & 0xFF0000) | ((attrDecoder.xyz << 24));
    attrDecoder.w = 0;
    R1f = float4(
        as_type<float>(int(attrDecoder.x)), as_type<float>(int(attrDecoder.y)),
        as_type<float>(int(attrDecoder.z)), as_type<float>(0x3f800000));
    attrDecoder.xy = in.attrDataSem1.xy;
    attrDecoder.xy = (attrDecoder.xy >> 24) | ((attrDecoder.xy >> 8) & 0xFF00) |
                     ((attrDecoder.xy << 8) & 0xFF0000) |
                     ((attrDecoder.xy << 24));
    attrDecoder.z = 0;
    attrDecoder.w = 0;
    R2f = float4(as_type<float>(int(attrDecoder.x)),
                 as_type<float>(int(attrDecoder.y)), as_type<float>(0),
                 as_type<float>(0x3f800000));
    // 0
    R1f.x = R1f.x;
    R1f.x *= 2.0;
    R1f.y = R1f.y;
    R1f.y *= 2.0;
    R1f.z = -(1.0);
    R1f.w = 1.0;
    R0f.x = R2f.x;
    // 1
    R0f.y = R2f.y;
    R2f.z = R2f.y + as_type<float>(supportBuffer.remapped[0].y) / resYScale;
    R2f.w = R2f.y;
    R2f.y = R2f.y + -(as_type<float>(supportBuffer.remapped[0].y) / resYScale);
    // export
    SET_POSITION(float4(R1f.x, R1f.y, R1f.z, R1f.w));
    // export
    out.passParameterSem0 = float4(R2f.x, R2f.y, R2f.z, R2f.w);
    // export
    // skipped export to semanticId 255
    return out;
}
)BOTW_MSL_END"},
{0x4a8f1fff48ea26e2ULL, 0x0000000000000000ULL, GraphicPack2::GP_SHADER_TYPE::VERTEX, R"BOTW_MSL_END(#include <metal_stdlib>
using namespace metal;

// shader 3d7aeadf35ccf970
// Shader dumped from BotW 208, using Cemu 2193a8c
// Based on 22c410044398c7af_0000000000000000_vs.txt
// Fixed radius blur
// shadow pass blur v

constant float resScale = $shadowRes;

#define SET_POSITION(_v)                                                       \
    out.position = _v;                                                         \
    out.position.z = (out.position.z + out.position.w) / 2.0
// start of shader inputs/outputs, predetermined by Cemu. Do not touch
struct SupportBuffer {
    int4 remapped[1];
};

struct VertexIn {
    uint4 attrDataSem0 [[attribute(0)]];
    uint4 attrDataSem1 [[attribute(1)]];
};

struct VertexOut {
    float4 position [[position]] [[invariant]];
    float4 passParameterSem0 [[user(locn0)]];
};

// end of shader inputs/outputs
template <typename TextureT, typename CoordT>
float sampleCompareEmulate(TextureT tex, sampler samplr, CoordT coord,
                           float compareValue) {
    return compareValue < tex.sample(samplr, coord).x ? 1.0 : 0.0;
}
template <typename TextureT, typename CoordT>
float2 textureCalculateLod(TextureT tex, sampler samplr, CoordT coord) {
    float lod = tex.calculate_unclamped_lod(samplr, coord);
    return float2(floor(lod), fract(lod));
}
int clampFI32(int v) {
    if (v == 0x7FFFFFFF)
        return as_type<int>(1.0);
    else if (v == 0xFFFFFFFF)
        return as_type<int>(0.0);
    return as_type<int>(clamp(as_type<float>(v), 0.0, 1.0));
}
float mul_nonIEEE(float a, float b) {
    if (a == 0.0 || b == 0.0)
        return 0.0;
    return a * b;
}
vertex VertexOut main0(uint vid [[vertex_id]], uint iid [[instance_id]],
                       VertexIn in [[stage_in]],
                       constant SupportBuffer &supportBuffer [[buffer(0)]]) {
    VertexOut out;
    float4 R0f = float4(0.0);
    float4 R1f = float4(0.0);
    float4 R2f = float4(0.0);
    uint4 attrDecoder;
    float backupReg0f, backupReg1f, backupReg2f, backupReg3f, backupReg4f;
    float PV0fx = 0.0, PV0fy = 0.0, PV0fz = 0.0, PV0fw = 0.0, PV1fx = 0.0,
          PV1fy = 0.0, PV1fz = 0.0, PV1fw = 0.0;
    float PS0f = 0.0, PS1f = 0.0;
    float4 tempf = float4(0.0);
    float tempResultf;
    int tempResulti;
    int4 ARi = int4(0);
    bool predResult = true;
    R0f = float4(vid, 0, 0, iid);
    attrDecoder.xyz = in.attrDataSem0.xyz;
    attrDecoder.xyz =
        (attrDecoder.xyz >> 24) | ((attrDecoder.xyz >> 8) & 0xFF00) |
        ((attrDecoder.xyz << 8) & 0xFF0000) | ((attrDecoder.xyz << 24));
    attrDecoder.w = 0;
    R1f = float4(
        as_type<float>(int(attrDecoder.x)), as_type<float>(int(attrDecoder.y)),
        as_type<float>(int(attrDecoder.z)), as_type<float>(0x3f800000));
    attrDecoder.xy = in.attrDataSem1.xy;
    attrDecoder.xy = (attrDecoder.xy >> 24) | ((attrDecoder.xy >> 8) & 0xFF00) |
                     ((attrDecoder.xy << 8) & 0xFF0000) |
                     ((attrDecoder.xy << 24));
    attrDecoder.z = 0;
    attrDecoder.w = 0;
    R2f = float4(as_type<float>(int(attrDecoder.x)),
                 as_type<float>(int(attrDecoder.y)), as_type<float>(0),
                 as_type<float>(0x3f800000));
    // 0
    R1f.x = R1f.x;
    R1f.x *= 2.0;
    R1f.y = R1f.y;
    R1f.y *= 2.0;
    R1f.z = -(1.0);
    R1f.w = 1.0;
    PS0f = as_type<float>(supportBuffer.remapped[0].y) / resScale *
           as_type<float>(0x3fae8a72);
    // 1
    R0f.x = R2f.x;
    R0f.y = R2f.y;
    R2f.z = R2f.y + PS0f;
    R2f.w = R2f.y;
    R2f.y = R2f.y + -(PS0f);
    // export
    SET_POSITION(float4(R1f.x, R1f.y, R1f.z, R1f.w));
    // export
    out.passParameterSem0 = float4(R2f.x, R2f.y, R2f.z, R2f.w);
    // export
    // skipped export to semanticId 255
    return out;
}
)BOTW_MSL_END"},
{0x51fa659db827a3a7ULL, 0x0000000000000000ULL, GraphicPack2::GP_SHADER_TYPE::VERTEX, R"BOTW_MSL_END(#include <metal_stdlib>
using namespace metal;

// shader 51fa659db827a3a7
// Shader dumped from BotW 208, using Cemu 2193a8c
// Based on 2a2f55a2b2d64474_0000000000000000_vs.txt
// Fixed radius blur
// shadow pass blur

constant float resScale = $shadowRes;

#define SET_POSITION(_v)                                                       \
    out.position = _v;                                                         \
    out.position.z = (out.position.z + out.position.w) / 2.0
// start of shader inputs/outputs, predetermined by Cemu. Do not touch
struct SupportBuffer {
    int4 remapped[1];
};

struct VertexIn {
    uint4 attrDataSem0 [[attribute(0)]];
    uint4 attrDataSem1 [[attribute(1)]];
};

struct VertexOut {
    float4 position [[position]] [[invariant]];
    float4 passParameterSem0 [[user(locn0)]];
};

// end of shader inputs/outputs
template <typename TextureT, typename CoordT>
float sampleCompareEmulate(TextureT tex, sampler samplr, CoordT coord,
                           float compareValue) {
    return compareValue < tex.sample(samplr, coord).x ? 1.0 : 0.0;
}
template <typename TextureT, typename CoordT>
float2 textureCalculateLod(TextureT tex, sampler samplr, CoordT coord) {
    float lod = tex.calculate_unclamped_lod(samplr, coord);
    return float2(floor(lod), fract(lod));
}
int clampFI32(int v) {
    if (v == 0x7FFFFFFF)
        return as_type<int>(1.0);
    else if (v == 0xFFFFFFFF)
        return as_type<int>(0.0);
    return as_type<int>(clamp(as_type<float>(v), 0.0, 1.0));
}
float mul_nonIEEE(float a, float b) {
    if (a == 0.0 || b == 0.0)
        return 0.0;
    return a * b;
}
vertex VertexOut main0(uint vid [[vertex_id]], uint iid [[instance_id]],
                       VertexIn in [[stage_in]],
                       constant SupportBuffer &supportBuffer [[buffer(0)]]) {
    VertexOut out;
    float4 R0f = float4(0.0);
    float4 R1f = float4(0.0);
    float4 R2f = float4(0.0);
    float4 R3f = float4(0.0);
    float4 R127f = float4(0.0);
    uint4 attrDecoder;
    float backupReg0f, backupReg1f, backupReg2f, backupReg3f, backupReg4f;
    float PV0fx = 0.0, PV0fy = 0.0, PV0fz = 0.0, PV0fw = 0.0, PV1fx = 0.0,
          PV1fy = 0.0, PV1fz = 0.0, PV1fw = 0.0;
    float PS0f = 0.0, PS1f = 0.0;
    float4 tempf = float4(0.0);
    float tempResultf;
    int tempResulti;
    int4 ARi = int4(0);
    bool predResult = true;
    R0f = float4(vid, 0, 0, iid);
    attrDecoder.xyz = in.attrDataSem0.xyz;
    attrDecoder.xyz =
        (attrDecoder.xyz >> 24) | ((attrDecoder.xyz >> 8) & 0xFF00) |
        ((attrDecoder.xyz << 8) & 0xFF0000) | ((attrDecoder.xyz << 24));
    attrDecoder.w = 0;
    R1f = float4(
        as_type<float>(int(attrDecoder.x)), as_type<float>(int(attrDecoder.y)),
        as_type<float>(int(attrDecoder.z)), as_type<float>(0x3f800000));
    attrDecoder.xy = in.attrDataSem1.xy;
    attrDecoder.xy = (attrDecoder.xy >> 24) | ((attrDecoder.xy >> 8) & 0xFF00) |
                     ((attrDecoder.xy << 8) & 0xFF0000) |
                     ((attrDecoder.xy << 24));
    attrDecoder.z = 0;
    attrDecoder.w = 0;
    R2f = float4(as_type<float>(int(attrDecoder.x)),
                 as_type<float>(int(attrDecoder.y)), as_type<float>(0),
                 as_type<float>(0x3f800000));
    // 0
    R1f.x = R1f.x;
    R1f.x *= 2.0;
    R1f.y = R1f.y;
    R1f.y *= 2.0;
    R1f.z = -(1.0);
    R1f.w = 1.0;
    R127f.x = as_type<float>(supportBuffer.remapped[0].x) / resScale *
              as_type<float>(0x3fae8a72);
    // 1
    R3f.x = R2f.x;
    R3f.y = R2f.y;
    R0f.z = R2f.x + R127f.x;
    R0f.w = R2f.x;
    R0f.x = R2f.y;
    // 2
    R0f.y = R2f.x + -(R127f.x);
    // export
    SET_POSITION(float4(R1f.x, R1f.y, R1f.z, R1f.w));
    // export
    out.passParameterSem0 = float4(R0f.x, R0f.y, R0f.z, R0f.w);
    // export
    // skipped export to semanticId 255
    return out;
}
)BOTW_MSL_END"},
{0x6212836042496393ULL, 0x0000000000000000ULL, GraphicPack2::GP_SHADER_TYPE::VERTEX, R"BOTW_MSL_END(#include <metal_stdlib>
using namespace metal;

// shader 6212836042496393
// Shader dumped from BotW 208, using Cemu 2193a8c
// Based on 381d034349896360_0000000000000000_vs.txt
// Used for: Horizontal Volumetric Light Blur

constant float resXScale = float($width) / float($gameWidth);

#define SET_POSITION(_v)                                                       \
    out.position = _v;                                                         \
    out.position.z = (out.position.z + out.position.w) / 2.0
// start of shader inputs/outputs, predetermined by Cemu. Do not touch
struct SupportBuffer {
    int4 remapped[5];
};

struct VertexIn {
    uint4 attrDataSem0 [[attribute(0)]];
};

struct VertexOut {
    float4 position [[position]] [[invariant]];
    float4 passParameterSem0 [[user(locn0)]];
};

// end of shader inputs/outputs
template <typename TextureT, typename CoordT>
float sampleCompareEmulate(TextureT tex, sampler samplr, CoordT coord,
                           float compareValue) {
    return compareValue < tex.sample(samplr, coord).x ? 1.0 : 0.0;
}
template <typename TextureT, typename CoordT>
float2 textureCalculateLod(TextureT tex, sampler samplr, CoordT coord) {
    float lod = tex.calculate_unclamped_lod(samplr, coord);
    return float2(floor(lod), fract(lod));
}
int clampFI32(int v) {
    if (v == 0x7FFFFFFF)
        return as_type<int>(1.0);
    else if (v == 0xFFFFFFFF)
        return as_type<int>(0.0);
    return as_type<int>(clamp(as_type<float>(v), 0.0, 1.0));
}
float mul_nonIEEE(float a, float b) {
    if (a == 0.0 || b == 0.0)
        return 0.0;
    return a * b;
}
vertex VertexOut main0(uint vid [[vertex_id]], uint iid [[instance_id]],
                       VertexIn in [[stage_in]],
                       constant SupportBuffer &supportBuffer [[buffer(0)]]) {
    VertexOut out;
    float4 R0f = float4(0.0);
    float4 R1f = float4(0.0);
    float4 R126f = float4(0.0);
    float4 R127f = float4(0.0);
    uint4 attrDecoder;
    float backupReg0f, backupReg1f, backupReg2f, backupReg3f, backupReg4f;
    float PV0fx = 0.0, PV0fy = 0.0, PV0fz = 0.0, PV0fw = 0.0, PV1fx = 0.0,
          PV1fy = 0.0, PV1fz = 0.0, PV1fw = 0.0;
    float PS0f = 0.0, PS1f = 0.0;
    float4 tempf = float4(0.0);
    float tempResultf;
    int tempResulti;
    int4 ARi = int4(0);
    bool predResult = true;
    R0f = float4(vid, 0, 0, iid);
    attrDecoder.xyz = in.attrDataSem0.xyz;
    attrDecoder.xyz =
        (attrDecoder.xyz >> 24) | ((attrDecoder.xyz >> 8) & 0xFF00) |
        ((attrDecoder.xyz << 8) & 0xFF0000) | ((attrDecoder.xyz << 24));
    attrDecoder.w = 0;
    R1f = float4(
        as_type<float>(int(attrDecoder.x)), as_type<float>(int(attrDecoder.y)),
        as_type<float>(int(attrDecoder.z)), as_type<float>(0x3f800000));
    // 0
    R127f.x = -(R1f.y);
    R127f.y = (R1f.x > 0.0) ? 1.0 : 0.0;
    R127f.y /= 2.0;
    R127f.z = (0.0 > R1f.x) ? 1.0 : 0.0;
    R127f.z /= 2.0;
    R127f.w = 1.0;
    // 1
    R0f.x = dot(float4(R1f.x, R1f.y, R1f.z, R127f.w),
                float4(as_type<float>(supportBuffer.remapped[0].x),
                       as_type<float>(supportBuffer.remapped[0].y),
                       as_type<float>(supportBuffer.remapped[0].z),
                       as_type<float>(supportBuffer.remapped[0].w)));
    PV1fy = R0f.x;
    PV1fz = R0f.x;
    PV1fw = R0f.x;
    R126f.z = (R127f.x > 0.0) ? 1.0 : 0.0;
    R126f.z /= 2.0;
    // 2
    tempf.x = dot(float4(R1f.x, R1f.y, R1f.z, R127f.w),
                  float4(as_type<float>(supportBuffer.remapped[1].x),
                         as_type<float>(supportBuffer.remapped[1].y),
                         as_type<float>(supportBuffer.remapped[1].z),
                         as_type<float>(supportBuffer.remapped[1].w)));
    PV0fx = tempf.x;
    R0f.y = tempf.x;
    PV0fz = tempf.x;
    PV0fw = tempf.x;
    R127f.x = (0.0 > R127f.x) ? 1.0 : 0.0;
    R127f.x /= 2.0;
    // 3
    tempf.x = dot(float4(R1f.x, R1f.y, R1f.z, R127f.w),
                  float4(as_type<float>(supportBuffer.remapped[2].x),
                         as_type<float>(supportBuffer.remapped[2].y),
                         as_type<float>(supportBuffer.remapped[2].z),
                         as_type<float>(supportBuffer.remapped[2].w)));
    PV1fx = tempf.x;
    PV1fy = tempf.x;
    R0f.z = tempf.x;
    PV1fw = tempf.x;
    R126f.w = R127f.y + -(R127f.z);
    // 4
    tempf.x = dot(float4(R1f.x, R1f.y, R1f.z, R127f.w),
                  float4(as_type<float>(supportBuffer.remapped[3].x),
                         as_type<float>(supportBuffer.remapped[3].y),
                         as_type<float>(supportBuffer.remapped[3].z),
                         as_type<float>(supportBuffer.remapped[3].w)));
    PV0fx = tempf.x;
    PV0fy = tempf.x;
    PV0fz = tempf.x;
    R0f.w = tempf.x;
    PS0f = R126f.z + -(R127f.x);
    // 5
    PV1fx = PS0f + 0.5;
    PV1fy = R126f.w + 0.5;
    // 6
    R1f.x = PV1fx;
    R1f.y = -(as_type<float>(supportBuffer.remapped[4].z)) * 0.5 / resXScale +
            PV1fy;
    R1f.z =
        as_type<float>(supportBuffer.remapped[4].z) * 0.5 / resXScale + PV1fy;
    R1f.w = PV1fy;
    // export
    SET_POSITION(float4(R0f.x, R0f.y, R0f.z, R0f.w));
    // export
    out.passParameterSem0 = float4(R1f.x, R1f.y, R1f.z, R1f.w);
    return out;
}
)BOTW_MSL_END"},
{0x6a84889d491efb52ULL, 0x0000000000000000ULL, GraphicPack2::GP_SHADER_TYPE::VERTEX, R"BOTW_MSL_END(#include <metal_stdlib>
using namespace metal;

// shader 6a84889d491efb52
// Shader dumped from BotW 208, using Cemu 2193a8c
// Based on 4721609a424e9a1f_0000000000000000_vs.txt
// Used for: Restoring the native BotW Anti-Aliasing implementation

constant float resXScale = float($width) / float($gameWidth);
constant float resYScale = float($height) / float($gameHeight);

#define SET_POSITION(_v)                                                       \
    out.position = _v;                                                         \
    out.position.z = (out.position.z + out.position.w) / 2.0
// start of shader inputs/outputs, predetermined by Cemu. Do not touch
struct SupportBuffer {
    int4 remapped[5];
};

struct VertexIn {
    uint4 attrDataSem0 [[attribute(0)]];
    uint4 attrDataSem1 [[attribute(1)]];
};

struct VertexOut {
    float4 position [[position]] [[invariant]];
    float4 passParameterSem4 [[user(locn1)]];
    float4 passParameterSem0 [[user(locn0)]];
    float4 passParameterSem6 [[user(locn2)]];
};

// end of shader inputs/outputs
template <typename TextureT, typename CoordT>
float sampleCompareEmulate(TextureT tex, sampler samplr, CoordT coord,
                           float compareValue) {
    return compareValue < tex.sample(samplr, coord).x ? 1.0 : 0.0;
}
template <typename TextureT, typename CoordT>
float2 textureCalculateLod(TextureT tex, sampler samplr, CoordT coord) {
    float lod = tex.calculate_unclamped_lod(samplr, coord);
    return float2(floor(lod), fract(lod));
}
int clampFI32(int v) {
    if (v == 0x7FFFFFFF)
        return as_type<int>(1.0);
    else if (v == 0xFFFFFFFF)
        return as_type<int>(0.0);
    return as_type<int>(clamp(as_type<float>(v), 0.0, 1.0));
}
float mul_nonIEEE(float a, float b) {
    if (a == 0.0 || b == 0.0)
        return 0.0;
    return a * b;
}
vertex VertexOut main0(uint vid [[vertex_id]], uint iid [[instance_id]],
                       VertexIn in [[stage_in]],
                       constant SupportBuffer &supportBuffer [[buffer(0)]],
                       texture2d<float> tex0 [[texture(0)]],
                       sampler samplr0 [[sampler(0)]]) {
    VertexOut out;
    int4 R0i = int4(0);
    int4 R1i = int4(0);
    int4 R2i = int4(0);
    int4 R3i = int4(0);
    int4 R4i = int4(0);
    int4 R5i = int4(0);
    int4 R6i = int4(0);
    int4 R123i = int4(0);
    int4 R124i = int4(0);
    int4 R125i = int4(0);
    int4 R126i = int4(0);
    int4 R127i = int4(0);
    uint4 attrDecoder;
    int backupReg0i, backupReg1i, backupReg2i, backupReg3i, backupReg4i;
    int PV0ix = 0, PV0iy = 0, PV0iz = 0, PV0iw = 0, PV1ix = 0, PV1iy = 0,
        PV1iz = 0, PV1iw = 0;
    int PS0i = 0, PS1i = 0;
    int4 tempi = int4(0);
    float tempResultf;
    int tempResulti;
    int4 ARi = int4(0);
    bool predResult = true;
    R0i = int4(vid, 0, 0, iid);
    attrDecoder.xyzw = in.attrDataSem0.xyzw;
    attrDecoder = ((attrDecoder >> 8) & 0xFF) | ((attrDecoder << 8) & 0xFF00);
    attrDecoder.xyzw = as_type<uint4>(
        float4(float2(as_type<half2>(attrDecoder.x | (attrDecoder.y << 16))),
               float2(as_type<half2>(attrDecoder.z | (attrDecoder.w << 16)))));
    R1i = int4(int(attrDecoder.x), int(attrDecoder.y), int(attrDecoder.z),
               int(attrDecoder.w));
    attrDecoder.x = in.attrDataSem1.x;
    attrDecoder.x = (attrDecoder.x >> 24) | ((attrDecoder.x >> 8) & 0xFF00) |
                    ((attrDecoder.x << 8) & 0xFF0000) | ((attrDecoder.x << 24));
    attrDecoder.y = 0;
    attrDecoder.z = 0;
    attrDecoder.w = 0;
    attrDecoder.xyzw =
        uint4((attrDecoder.x >> 0) & 0x3FF, (attrDecoder.x >> 10) & 0x3FF,
              (attrDecoder.x >> 20) & 0x3FF, (attrDecoder.x >> 30) & 0x3);
    if ((attrDecoder.x & 0x200) != 0)
        attrDecoder.x |= 0xFFFFFC00;
    if ((attrDecoder.y & 0x200) != 0)
        attrDecoder.y |= 0xFFFFFC00;
    if ((attrDecoder.z & 0x200) != 0)
        attrDecoder.z |= 0xFFFFFC00;
    attrDecoder.x = as_type<uint>(max(float(int(attrDecoder.x)) / 511.0, -1.0));
    attrDecoder.y = as_type<uint>(max(float(int(attrDecoder.y)) / 511.0, -1.0));
    attrDecoder.z = as_type<uint>(max(float(int(attrDecoder.z)) / 511.0, -1.0));
    attrDecoder.w = as_type<uint>(float(attrDecoder.w));
    R2i = int4(int(attrDecoder.x), int(attrDecoder.y), int(attrDecoder.z),
               int(attrDecoder.w));
    // 0
    PV0ix = (R0i.x == int(1)) ? -1 : 0;
    R127i.y = int(0x3f800000);
    PV0iz = int(0x3f800000);
    PV0iw = int(0x40400000);
    R127i.z = int(0xbf800000);
    // 1
    PV1ix = as_type<int>(as_type<float>(supportBuffer.remapped[0].z) *
                         as_type<float>(0x3b808081));
    R123i.y = ((PV0ix == 0) ? (PV0iw) : (int(0xbf800000)));
    R0i.z = 0;
    R123i.w = ((PV0ix == 0) ? (PV0iz) : (int(0xc0400000)));
    R1i.w = int(0x3f800000);
    // 2
    R1i.x = ((R0i.x == 0) ? (R127i.z) : (R123i.y));
    R1i.y = ((R0i.x == 0) ? (R127i.y) : (R123i.w));
    R1i.z = as_type<int>(as_type<float>(PV1ix) + -(0.5));
    R1i.z = as_type<int>(as_type<float>(R1i.z) * 2.0);
    R0i.w = int(0x3d2aaaab);
    R0i.x = int(0x3e000000);
    // 3
    R5i.x = as_type<int>(as_type<float>(R1i.x) * 0.5 + 0.5);
    R0i.y = 0;
    PV1iz = as_type<int>(-(as_type<float>(R1i.y)));
    R4i.w = supportBuffer.remapped[1].z;
    R4i.w = as_type<int>(as_type<float>(R4i.w) / 2.0);
    R4i.x = as_type<int>(-(as_type<float>(supportBuffer.remapped[2].z)));
    R4i.x = as_type<int>(as_type<float>(R4i.x) / 2.0);
    // 4
    R3i.x = as_type<int>(-(as_type<float>(R1i.y)));
    R3i.x = as_type<int>(as_type<float>(R3i.x) / 2.0);
    R5i.y = as_type<int>(as_type<float>(PV1iz) * 0.5 + 0.5);
    R6i.z = R5i.x;
    R3i.w = R1i.x;
    R3i.w = as_type<int>(as_type<float>(R3i.w) / 2.0);
    R3i.y = as_type<int>(as_type<float>(R4i.w) + -(0.5));
    R2i.w = as_type<int>(tex0.sample(samplr0, float2(as_type<float>(R0i.w),
                                                     as_type<float>(R0i.z)))
                             .x);
    R2i.xyz = as_type<int3>(tex0.sample(samplr0, float2(as_type<float>(R0i.x),
                                                        as_type<float>(R0i.y)))
                                .xyz);
    // export
    SET_POSITION(float4(as_type<float>(R1i.x), as_type<float>(R1i.y),
                        as_type<float>(R1i.z), as_type<float>(R1i.w)));
    // export
    // skipped export to semanticId 255
    // 0
    R127i.x = as_type<int>(as_type<float>(R4i.x) + -(0.5));
    R2i.y = as_type<int>(-(as_type<float>(supportBuffer.remapped[3].x)));
    R2i.y = as_type<int>(as_type<float>(R2i.y) * 2.0);
    R127i.z = supportBuffer.remapped[3].y;
    R127i.z = as_type<int>(as_type<float>(R127i.z) * 2.0);
    R6i.w = R5i.y;
    PS0i = as_type<int>(as_type<float>(R3i.w) + as_type<float>(R4i.w));
    // 1
    PV1ix = as_type<int>(as_type<float>(R3i.x) + as_type<float>(R4i.x));
    R126i.y = as_type<int>(as_type<float>(R5i.x) + as_type<float>(R3i.y));
    PV1iz =
        as_type<int>(as_type<float>(R6i.w) +
                     as_type<float>(supportBuffer.remapped[4].w) / resYScale);
    R127i.w =
        as_type<int>(as_type<float>(R6i.z) +
                     as_type<float>(supportBuffer.remapped[4].z) / resXScale);
    R127i.y =
        as_type<int>(mul_nonIEEE(as_type<float>(R2i.y), as_type<float>(PS0i)));
    // 2
    PV0ix = as_type<int>(as_type<float>(PV1iz) + as_type<float>(R127i.x));
    R125i.y = as_type<int>(
        mul_nonIEEE(as_type<float>(R127i.z), as_type<float>(PV1ix)));
    R126i.z =
        as_type<int>(as_type<float>(supportBuffer.remapped[4].y) * resYScale);
    R126i.z = as_type<int>(as_type<float>(R126i.z) / 2.0);
    R126i.w =
        as_type<int>(as_type<float>(supportBuffer.remapped[4].x) * resXScale);
    R126i.w = as_type<int>(as_type<float>(R126i.w) / 2.0);
    R124i.y =
        as_type<int>(as_type<float>(supportBuffer.remapped[4].z) / resXScale);
    R124i.y = as_type<int>(as_type<float>(R124i.y) * 2.0);
    // 3
    backupReg0i = R127i.x;
    backupReg1i = R127i.y;
    R127i.x =
        as_type<int>(as_type<float>(supportBuffer.remapped[4].w) / resYScale);
    R127i.x = as_type<int>(as_type<float>(R127i.x) * 2.0);
    R127i.y = as_type<int>(
        mul_nonIEEE(as_type<float>(R127i.z), as_type<float>(PV0ix)));
    PV1iz = as_type<int>(as_type<float>(R5i.y) + as_type<float>(backupReg0i));
    R127i.w = as_type<int>(as_type<float>(R127i.w) + as_type<float>(R3i.y));
    R5i.z = backupReg1i;
    // 4
    R3i.x = as_type<int>(
        mul_nonIEEE(as_type<float>(R5i.x), as_type<float>(R126i.w)) + 0.5);
    R4i.y = as_type<int>(
        mul_nonIEEE(as_type<float>(R127i.z), as_type<float>(PV1iz)));
    PV0iw = as_type<int>(
        mul_nonIEEE(as_type<float>(R2i.y), as_type<float>(R126i.y)));
    R5i.w = R125i.y;
    // 5
    R4i.x = as_type<int>(
        mul_nonIEEE(as_type<float>(R2i.y), as_type<float>(R127i.w)));
    R3i.y = as_type<int>(
        mul_nonIEEE(as_type<float>(R5i.y), as_type<float>(R126i.z)) + 0.5);
    R3i.z = R124i.y;
    R3i.w = R127i.x;
    R4i.z = PV0iw;
    // 6
    R4i.w = R127i.y;
    // export
    out.passParameterSem4 =
        float4(as_type<float>(R3i.x), as_type<float>(R3i.y),
               as_type<float>(R3i.z), as_type<float>(R3i.w));
    // export
    // skipped export to semanticId 255
    // export
    out.passParameterSem0 =
        float4(as_type<float>(R5i.x), as_type<float>(R5i.y),
               as_type<float>(R5i.z), as_type<float>(R5i.w));
    // export
    out.passParameterSem6 =
        float4(as_type<float>(R6i.x), as_type<float>(R6i.x),
               as_type<float>(R6i.z), as_type<float>(R6i.w));
    return out;
}
)BOTW_MSL_END"},
{0x6c824cdb33572219ULL, 0x0000000000000000ULL, GraphicPack2::GP_SHADER_TYPE::VERTEX, R"BOTW_MSL_END(#include <metal_stdlib>
using namespace metal;

// shader 3d7aeadf35ccf970
// Shader dumped from BotW 208, using Cemu 2193a8c
// Based on 44b73ce02e05c2e6_0000000000000000_vs.txt
// Used for: Vertical Self-Shadowing Mask Fix Blur

constant float resYScale = float($height) / float($gameHeight);

#define SET_POSITION(_v)                                                       \
    out.position = _v;                                                         \
    out.position.z = (out.position.z + out.position.w) / 2.0
// start of shader inputs/outputs, predetermined by Cemu. Do not touch
struct SupportBuffer {
    int4 remapped[1];
};

struct VertexIn {
    uint4 attrDataSem0 [[attribute(0)]];
    uint4 attrDataSem1 [[attribute(1)]];
};

struct VertexOut {
    float4 position [[position]] [[invariant]];
    float4 passParameterSem0 [[user(locn0)]];
    float4 passParameterSem1 [[user(locn1)]];
};

// end of shader inputs/outputs
template <typename TextureT, typename CoordT>
float sampleCompareEmulate(TextureT tex, sampler samplr, CoordT coord,
                           float compareValue) {
    return compareValue < tex.sample(samplr, coord).x ? 1.0 : 0.0;
}
template <typename TextureT, typename CoordT>
float2 textureCalculateLod(TextureT tex, sampler samplr, CoordT coord) {
    float lod = tex.calculate_unclamped_lod(samplr, coord);
    return float2(floor(lod), fract(lod));
}
int clampFI32(int v) {
    if (v == 0x7FFFFFFF)
        return as_type<int>(1.0);
    else if (v == 0xFFFFFFFF)
        return as_type<int>(0.0);
    return as_type<int>(clamp(as_type<float>(v), 0.0, 1.0));
}
float mul_nonIEEE(float a, float b) {
    if (a == 0.0 || b == 0.0)
        return 0.0;
    return a * b;
}
vertex VertexOut main0(uint vid [[vertex_id]], uint iid [[instance_id]],
                       VertexIn in [[stage_in]],
                       constant SupportBuffer &supportBuffer [[buffer(0)]]) {
    VertexOut out;
    float4 R0f = float4(0.0);
    float4 R1f = float4(0.0);
    float4 R2f = float4(0.0);
    float4 R3f = float4(0.0);
    float4 R126f = float4(0.0);
    float4 R127f = float4(0.0);
    uint4 attrDecoder;
    float backupReg0f, backupReg1f, backupReg2f, backupReg3f, backupReg4f;
    float PV0fx = 0.0, PV0fy = 0.0, PV0fz = 0.0, PV0fw = 0.0, PV1fx = 0.0,
          PV1fy = 0.0, PV1fz = 0.0, PV1fw = 0.0;
    float PS0f = 0.0, PS1f = 0.0;
    float4 tempf = float4(0.0);
    float tempResultf;
    int tempResulti;
    int4 ARi = int4(0);
    bool predResult = true;
    R0f = float4(vid, 0, 0, iid);
    attrDecoder.xyz = in.attrDataSem0.xyz;
    attrDecoder.xyz =
        (attrDecoder.xyz >> 24) | ((attrDecoder.xyz >> 8) & 0xFF00) |
        ((attrDecoder.xyz << 8) & 0xFF0000) | ((attrDecoder.xyz << 24));
    attrDecoder.w = 0;
    R1f = float4(
        as_type<float>(int(attrDecoder.x)), as_type<float>(int(attrDecoder.y)),
        as_type<float>(int(attrDecoder.z)), as_type<float>(0x3f800000));
    attrDecoder.xy = in.attrDataSem1.xy;
    attrDecoder.xy = (attrDecoder.xy >> 24) | ((attrDecoder.xy >> 8) & 0xFF00) |
                     ((attrDecoder.xy << 8) & 0xFF0000) |
                     ((attrDecoder.xy << 24));
    attrDecoder.z = 0;
    attrDecoder.w = 0;
    R2f = float4(as_type<float>(int(attrDecoder.x)),
                 as_type<float>(int(attrDecoder.y)), as_type<float>(0),
                 as_type<float>(0x3f800000));
    // 0
    R1f.x = R1f.x;
    R1f.x *= 2.0;
    R1f.y = R1f.y;
    R1f.y *= 2.0;
    R1f.z = as_type<float>(0xbf5fdfe0);
    R1f.w = 1.0;
    R127f.x = as_type<float>(supportBuffer.remapped[0].y) *
              as_type<float>(0x3fb13a93) / resYScale;
    // 1
    R126f.x = as_type<float>(supportBuffer.remapped[0].y) *
              as_type<float>(0x404ec4f0) / resYScale;
    R3f.y = R2f.y;
    R3f.z = R2f.x;
    R2f.w = R2f.y;
    R2f.z = R2f.y + R127f.x;
    // 2
    R0f.x = R2f.x;
    R0f.y = R2f.y + -(R127f.x);
    R0f.z = R2f.z;
    R0f.w = R2f.w;
    R2f.z = R2f.y + R126f.x;
    // 3
    backupReg0f = R2f.y;
    R2f.y = backupReg0f + -(R126f.x);
    R2f.w = backupReg0f;
    // export
    SET_POSITION(float4(R1f.x, R1f.y, R1f.z, R1f.w));
    // export
    out.passParameterSem0 = float4(R0f.x, R0f.y, R0f.z, R0f.w);
    // export
    out.passParameterSem1 = float4(R2f.x, R2f.y, R2f.z, R2f.w);
    // export
    // skipped export to semanticId 255
    return out;
}
)BOTW_MSL_END"},
{0x7cd338ce4c6ea935ULL, 0xff060c183060c54bULL, GraphicPack2::GP_SHADER_TYPE::PIXEL, R"BOTW_MSL_END(#include <metal_stdlib>
using namespace metal;

// shader 7cd338ce4c6ea935
// Shader dumped from BotW 208, using Cemu 2193a8c
// Based on 75a85b0cbcab764b_0000000000000000_vs.txt
// Used for: Ultrawide cutscenes (when enabled)

constant float aspectRatio =
    float(($gameWidth / $gameHeight) / ($width / $height));
constant float reverseAspectRatio =
    float(($width / $height) / ($gameWidth / $gameHeight));
#define ultrawideHUDMode $ultrawideHUDMode

#define SET_POSITION(_v)                                                       \
    out.position = _v;                                                         \
    out.position.z = (out.position.z + out.position.w) / 2.0
// start of shader inputs/outputs, predetermined by Cemu. Do not touch
struct SupportBuffer {
    int4 remapped[1];
    float2 fragCoordScale;
};

#define GET_FRAGCOORD()                                                        \
    float4(in.position.xy *supportBuffer.fragCoordScale.xy, in.position.z,     \
           1.0 / in.position.w)
struct FragmentIn {
    float4 position [[position]];
    float4 passParameterSem0 [[user(locn0)]];
};

struct FragmentOut {
    float4 passPixelColor0 [[color(0)]];
};

// end of shader inputs/outputs
template <typename TextureT, typename CoordT>
float sampleCompareEmulate(TextureT tex, sampler samplr, CoordT coord,
                           float compareValue) {
    return compareValue < tex.sample(samplr, coord).x ? 1.0 : 0.0;
}
template <typename TextureT, typename CoordT>
float2 textureCalculateLod(TextureT tex, sampler samplr, CoordT coord) {
    float lod = tex.calculate_unclamped_lod(samplr, coord);
    return float2(floor(lod), fract(lod));
}
int clampFI32(int v) {
    if (v == 0x7FFFFFFF)
        return as_type<int>(1.0);
    else if (v == 0xFFFFFFFF)
        return as_type<int>(0.0);
    return as_type<int>(clamp(as_type<float>(v), 0.0, 1.0));
}
float mul_nonIEEE(float a, float b) {
    if (a == 0.0 || b == 0.0)
        return 0.0;
    return a * b;
}
fragment FragmentOut main0(FragmentIn in [[stage_in]],
                           float2 pointCoord [[point_coord]],
                           bool frontFacing [[front_facing]],
                           constant SupportBuffer &supportBuffer [[buffer(0)]],
                           texture2d<float> tex0 [[texture(0)]],
                           sampler samplr0 [[sampler(0)]]) {
    FragmentOut out;
    float4 R0f = float4(0.0);
    float4 R127f = float4(0.0);
    float backupReg0f, backupReg1f, backupReg2f, backupReg3f, backupReg4f;
    float PV0fx = 0.0, PV0fy = 0.0, PV0fz = 0.0, PV0fw = 0.0, PV1fx = 0.0,
          PV1fy = 0.0, PV1fz = 0.0, PV1fw = 0.0;
    float PS0f = 0.0, PS1f = 0.0;
    float4 tempf = float4(0.0);
    float tempResultf;
    int tempResulti;
    int4 ARi = int4(0);
    bool predResult = true;
    R0f = in.passParameterSem0;

#if (ultrawideHUDMode != 0)
    float uv_x = 0.5 + (R0f.x - 0.5) / aspectRatio;
    R0f.xyzw = (tex0.sample(samplr0, float2(uv_x, R0f.y)).xyzw);
#else
    R0f.xyzw = (tex0.sample(samplr0, float2(R0f.x, R0f.y)).xyzw);
#endif
    // 0
    tempResultf = max(0.0, R0f.x);
    tempResultf = log2(tempResultf);
    if (isinf(tempResultf) == true)
        tempResultf = -3.40282347E+38F;
    PS0f = tempResultf;
    // 1
    R127f.z = mul_nonIEEE(PS0f, as_type<float>(supportBuffer.remapped[0].x));
    tempResultf = max(0.0, R0f.y);
    tempResultf = log2(tempResultf);
    if (isinf(tempResultf) == true)
        tempResultf = -3.40282347E+38F;
    PS1f = tempResultf;
    // 2
    R127f.w = mul_nonIEEE(PS1f, as_type<float>(supportBuffer.remapped[0].x));
    tempResultf = max(0.0, R0f.z);
    tempResultf = log2(tempResultf);
    if (isinf(tempResultf) == true)
        tempResultf = -3.40282347E+38F;
    PS0f = tempResultf;
    // 3
    R127f.x = mul_nonIEEE(PS0f, as_type<float>(supportBuffer.remapped[0].x));
    PS1f = exp2(R127f.z);
    // 4
    R0f.x = PS1f;
    PS0f = exp2(R127f.w);
    // 5
    R0f.y = PS0f;
    PS1f = exp2(R127f.x);
    // 6
    R0f.z = PS1f;
    // export

#if (ultrawideHUDMode != 0)
    bool isBorder = uv_x < 0 || uv_x > 1.0;
    out.passPixelColor0 = as_type<float4>(isBorder ? float4(0.0) : R0f.xyzw);
#else
    out.passPixelColor0 = as_type<float4>(float4(R0f.x, R0f.y, R0f.z, R0f.w));
#endif
    return out;
}
)BOTW_MSL_END"},
{0x83e271cc453e2d32ULL, 0x0000000000000000ULL, GraphicPack2::GP_SHADER_TYPE::VERTEX, R"BOTW_MSL_END(#include <metal_stdlib>
using namespace metal;

// shader 83e271cc453e2d32
// Shader dumped from BotW 208, using Cemu 2193a8c
// Based on 5c1761d13feccdff_0000000000000000_vs.txt
// Used for: Vertical+Horizontal Fullscreen Blur

constant float resXScale = float($width) / float($gameWidth);
constant float resYScale = float($height) / float($gameHeight);

#define SET_POSITION(_v)                                                       \
    out.position = _v;                                                         \
    out.position.z = (out.position.z + out.position.w) / 2.0
// start of shader inputs/outputs, predetermined by Cemu. Do not touch
struct SupportBuffer {
    int4 remapped[1];
};

struct VertexIn {
    uint4 attrDataSem0 [[attribute(0)]];
    uint4 attrDataSem1 [[attribute(1)]];
};

struct VertexOut {
    float4 position [[position]] [[invariant]];
    float4 passParameterSem3 [[user(locn0)]];
};

// end of shader inputs/outputs
template <typename TextureT, typename CoordT>
float sampleCompareEmulate(TextureT tex, sampler samplr, CoordT coord,
                           float compareValue) {
    return compareValue < tex.sample(samplr, coord).x ? 1.0 : 0.0;
}
template <typename TextureT, typename CoordT>
float2 textureCalculateLod(TextureT tex, sampler samplr, CoordT coord) {
    float lod = tex.calculate_unclamped_lod(samplr, coord);
    return float2(floor(lod), fract(lod));
}
int clampFI32(int v) {
    if (v == 0x7FFFFFFF)
        return as_type<int>(1.0);
    else if (v == 0xFFFFFFFF)
        return as_type<int>(0.0);
    return as_type<int>(clamp(as_type<float>(v), 0.0, 1.0));
}
float mul_nonIEEE(float a, float b) {
    if (a == 0.0 || b == 0.0)
        return 0.0;
    return a * b;
}
vertex VertexOut main0(uint vid [[vertex_id]], uint iid [[instance_id]],
                       VertexIn in [[stage_in]],
                       constant SupportBuffer &supportBuffer [[buffer(0)]]) {
    VertexOut out;
    float4 R0f = float4(0.0);
    float4 R1f = float4(0.0);
    float4 R2f = float4(0.0);
    uint4 attrDecoder;
    float backupReg0f, backupReg1f, backupReg2f, backupReg3f, backupReg4f;
    float PV0fx = 0.0, PV0fy = 0.0, PV0fz = 0.0, PV0fw = 0.0, PV1fx = 0.0,
          PV1fy = 0.0, PV1fz = 0.0, PV1fw = 0.0;
    float PS0f = 0.0, PS1f = 0.0;
    float4 tempf = float4(0.0);
    float tempResultf;
    int tempResulti;
    int4 ARi = int4(0);
    bool predResult = true;
    R0f = float4(vid, 0, 0, iid);
    attrDecoder.xyz = in.attrDataSem0.xyz;
    attrDecoder.xyz =
        (attrDecoder.xyz >> 24) | ((attrDecoder.xyz >> 8) & 0xFF00) |
        ((attrDecoder.xyz << 8) & 0xFF0000) | ((attrDecoder.xyz << 24));
    attrDecoder.w = 0;
    R1f = float4(
        as_type<float>(int(attrDecoder.x)), as_type<float>(int(attrDecoder.y)),
        as_type<float>(int(attrDecoder.z)), as_type<float>(0x3f800000));
    attrDecoder.xy = in.attrDataSem1.xy;
    attrDecoder.xy = (attrDecoder.xy >> 24) | ((attrDecoder.xy >> 8) & 0xFF00) |
                     ((attrDecoder.xy << 8) & 0xFF0000) |
                     ((attrDecoder.xy << 24));
    attrDecoder.z = 0;
    attrDecoder.w = 0;
    R2f = float4(as_type<float>(int(attrDecoder.x)),
                 as_type<float>(int(attrDecoder.y)), as_type<float>(0),
                 as_type<float>(0x3f800000));
    // 0
    R1f.x = R1f.x;
    R1f.x *= 2.0;
    R1f.y = R1f.y;
    R1f.y *= 2.0;
    R1f.z = -1.0;
    R1f.w = 1.0;
    PS0f = R2f.x + -(as_type<float>(supportBuffer.remapped[0].x) / resXScale);
    // 1
    PV1fx = R2f.y + -(as_type<float>(supportBuffer.remapped[0].y) / resYScale);
    R2f.y = R2f.y + as_type<float>(supportBuffer.remapped[0].y) / resYScale;
    R2f.z = PS0f;
    R2f.x = R2f.x + as_type<float>(supportBuffer.remapped[0].x) / resXScale;
    // 2
    R2f.w = PV1fx;
    // export
    SET_POSITION(float4(R1f.x, R1f.y, R1f.z, R1f.w));
    // export
    out.passParameterSem3 = float4(R2f.x, R2f.y, R2f.z, R2f.w);
    return out;
}
)BOTW_MSL_END"},
{0x84626b0942fd6495ULL, 0x0000000000000000ULL, GraphicPack2::GP_SHADER_TYPE::VERTEX, R"BOTW_MSL_END(#include <metal_stdlib>
using namespace metal;

// shader 84626b0942fd6495
// Shader dumped from BotW 208, using Cemu 2193a8c
// Based on 5c975b0e3dac0562_0000000000000000_vs.txt
// Used for: Horizontal Bloom Blur

constant float resXScale = float($width) / float($gameWidth);

#define SET_POSITION(_v)                                                       \
    out.position = _v;                                                         \
    out.position.z = (out.position.z + out.position.w) / 2.0
// start of shader inputs/outputs, predetermined by Cemu. Do not touch
struct SupportBuffer {
    int4 remapped[1];
};

struct VertexIn {
    uint4 attrDataSem0 [[attribute(0)]];
    uint4 attrDataSem1 [[attribute(1)]];
};

struct VertexOut {
    float4 position [[position]] [[invariant]];
    float4 passParameterSem1 [[user(locn1)]];
    float4 passParameterSem0 [[user(locn0)]];
};

// end of shader inputs/outputs
template <typename TextureT, typename CoordT>
float sampleCompareEmulate(TextureT tex, sampler samplr, CoordT coord,
                           float compareValue) {
    return compareValue < tex.sample(samplr, coord).x ? 1.0 : 0.0;
}
template <typename TextureT, typename CoordT>
float2 textureCalculateLod(TextureT tex, sampler samplr, CoordT coord) {
    float lod = tex.calculate_unclamped_lod(samplr, coord);
    return float2(floor(lod), fract(lod));
}
int clampFI32(int v) {
    if (v == 0x7FFFFFFF)
        return as_type<int>(1.0);
    else if (v == 0xFFFFFFFF)
        return as_type<int>(0.0);
    return as_type<int>(clamp(as_type<float>(v), 0.0, 1.0));
}
float mul_nonIEEE(float a, float b) {
    if (a == 0.0 || b == 0.0)
        return 0.0;
    return a * b;
}
vertex VertexOut main0(uint vid [[vertex_id]], uint iid [[instance_id]],
                       VertexIn in [[stage_in]],
                       constant SupportBuffer &supportBuffer [[buffer(0)]]) {
    VertexOut out;
    float4 R0f = float4(0.0);
    float4 R1f = float4(0.0);
    float4 R2f = float4(0.0);
    uint4 attrDecoder;
    float backupReg0f, backupReg1f, backupReg2f, backupReg3f, backupReg4f;
    float PV0fx = 0.0, PV0fy = 0.0, PV0fz = 0.0, PV0fw = 0.0, PV1fx = 0.0,
          PV1fy = 0.0, PV1fz = 0.0, PV1fw = 0.0;
    float PS0f = 0.0, PS1f = 0.0;
    float4 tempf = float4(0.0);
    float tempResultf;
    int tempResulti;
    int4 ARi = int4(0);
    bool predResult = true;
    R0f = float4(vid, 0, 0, iid);
    attrDecoder.xyz = in.attrDataSem0.xyz;
    attrDecoder.xyz =
        (attrDecoder.xyz >> 24) | ((attrDecoder.xyz >> 8) & 0xFF00) |
        ((attrDecoder.xyz << 8) & 0xFF0000) | ((attrDecoder.xyz << 24));
    attrDecoder.w = 0;
    R1f = float4(
        as_type<float>(int(attrDecoder.x)), as_type<float>(int(attrDecoder.y)),
        as_type<float>(int(attrDecoder.z)), as_type<float>(0x3f800000));
    attrDecoder.xy = in.attrDataSem1.xy;
    attrDecoder.xy = (attrDecoder.xy >> 24) | ((attrDecoder.xy >> 8) & 0xFF00) |
                     ((attrDecoder.xy << 8) & 0xFF0000) |
                     ((attrDecoder.xy << 24));
    attrDecoder.z = 0;
    attrDecoder.w = 0;
    R2f = float4(as_type<float>(int(attrDecoder.x)),
                 as_type<float>(int(attrDecoder.y)), as_type<float>(0),
                 as_type<float>(0x3f800000));
    // 0
    R1f.x = R1f.x;
    R1f.x *= 2.0;
    R1f.y = R1f.y;
    R1f.y *= 2.0;
    R1f.z = 0.0;
    R1f.w = 1.0;
    PS0f = as_type<float>(supportBuffer.remapped[0].x) / resXScale *
           as_type<float>(0x3fb13a93);
    // 1
    PV1fx = as_type<float>(supportBuffer.remapped[0].x) / resXScale *
            as_type<float>(0x404ec4f0);
    R0f.y = R2f.x + -(PS0f);
    R0f.z = R2f.x + PS0f;
    R0f.w = R2f.x;
    R0f.x = R2f.y;
    // 2
    backupReg0f = R2f.x;
    R2f.x = R0f.x;
    R2f.y = backupReg0f + -(PV1fx);
    R2f.z = backupReg0f + PV1fx;
    R2f.w = backupReg0f;
    // export
    SET_POSITION(float4(R1f.x, R1f.y, R1f.z, R1f.w));
    // export
    out.passParameterSem1 = float4(R2f.x, R2f.y, R2f.z, R2f.w);
    // export
    out.passParameterSem0 = float4(R0f.x, R0f.y, R0f.z, R0f.w);
    return out;
}
)BOTW_MSL_END"},
{0x88133ee405eaae28ULL, 0x70b38325d41178c1ULL, GraphicPack2::GP_SHADER_TYPE::PIXEL, R"BOTW_MSL_END(#include <metal_stdlib>
using namespace metal;

// shader 88133ee405eaae28
// Shader dumped from BotW 208, using Cemu 2193a8c
// Based on 88133ee405eaae28_000003c000009269_ps.txt
// Shadow 2x2 box blur fix

constant float resScale = $shadowRes;

#define SET_POSITION(_v)                                                       \
    out.position = _v;                                                         \
    out.position.z = (out.position.z + out.position.w) / 2.0
// start of shader inputs/outputs, predetermined by Cemu. Do not touch
struct SupportBuffer {
    float2 fragCoordScale;
};

struct UBuff1 {
    float4 d[4096];
};

struct UBuff6 {
    float4 d[54];
};

struct UBuff10 {
    float4 d[4];
};

#define GET_FRAGCOORD()                                                        \
    float4(in.position.xy *supportBuffer.fragCoordScale.xy, in.position.z,     \
           1.0 / in.position.w)
struct FragmentIn {
    float4 position [[position]];
    float4 passParameterSem0 [[user(locn0)]];
    float4 passParameterSem5 [[user(locn1)]];
    float4 passParameterSem6 [[user(locn2)]];
};

struct FragmentOut {
    float4 passPixelColor5 [[color(5)]];
};

// end of shader inputs/outputs
template <typename TextureT, typename CoordT>
float sampleCompareEmulate(TextureT tex, sampler samplr, CoordT coord,
                           float compareValue) {
    return compareValue < tex.sample(samplr, coord).x ? 1.0 : 0.0;
}
template <typename TextureT, typename CoordT>
float2 textureCalculateLod(TextureT tex, sampler samplr, CoordT coord) {
    float lod = tex.calculate_unclamped_lod(samplr, coord);
    return float2(floor(lod), fract(lod));
}
int clampFI32(int v) {
    if (v == 0x7FFFFFFF)
        return as_type<int>(1.0);
    else if (v == 0xFFFFFFFF)
        return as_type<int>(0.0);
    return as_type<int>(clamp(as_type<float>(v), 0.0, 1.0));
}
float mul_nonIEEE(float a, float b) {
    if (a == 0.0 || b == 0.0)
        return 0.0;
    return a * b;
}
fragment FragmentOut
main0(FragmentIn in [[stage_in]], float2 pointCoord [[point_coord]],
      bool frontFacing [[front_facing]],
      constant SupportBuffer &supportBuffer [[buffer(0)]],
      constant UBuff1 &ubuff1 [[buffer(1)]],
      constant UBuff6 &ubuff6 [[buffer(2)]],
      constant UBuff10 &ubuff10 [[buffer(3)]],
      texture2d<float> tex0 [[texture(0)]], sampler samplr0 [[sampler(0)]],
      texture2d<float> tex1 [[texture(1)]], sampler samplr1 [[sampler(1)]],
      texture2d<float> tex3 [[texture(2)]], sampler samplr3 [[sampler(2)]],
      texture2d<float> tex6 [[texture(3)]], sampler samplr6 [[sampler(3)]],
      depth2d_array<float> tex8 [[texture(4)]], sampler samplr8 [[sampler(4)]],
      texture2d<float> tex15 [[texture(5)]], sampler samplr15 [[sampler(5)]]) {
    FragmentOut out;
    int4 R0i = int4(0);
    int4 R1i = int4(0);
    int4 R2i = int4(0);
    int4 R3i = int4(0);
    int4 R4i = int4(0);
    int4 R5i = int4(0);
    int4 R6i = int4(0);
    int4 R7i = int4(0);
    int4 R8i = int4(0);
    int4 R9i = int4(0);
    int4 R10i = int4(0);
    int4 R11i = int4(0);
    int4 R12i = int4(0);
    int4 R13i = int4(0);
    int4 R14i = int4(0);
    int4 R15i = int4(0);
    int4 R16i = int4(0);
    int4 R17i = int4(0);
    int4 R122i = int4(0);
    int4 R123i = int4(0);
    int4 R124i = int4(0);
    int4 R125i = int4(0);
    int4 R126i = int4(0);
    int4 R127i = int4(0);
    int backupReg0i, backupReg1i, backupReg2i, backupReg3i, backupReg4i;
    int PV0ix = 0, PV0iy = 0, PV0iz = 0, PV0iw = 0, PV1ix = 0, PV1iy = 0,
        PV1iz = 0, PV1iw = 0;
    int PS0i = 0, PS1i = 0;
    int4 tempi = int4(0);
    float tempResultf;
    int tempResulti;
    int4 ARi = int4(0);
    bool predResult = true;
    bool activeMaskStack[3];
    bool activeMaskStackC[4];
    activeMaskStack[0] = false;
    activeMaskStack[1] = false;
    activeMaskStackC[0] = false;
    activeMaskStackC[1] = false;
    activeMaskStackC[2] = false;
    activeMaskStack[0] = true;
    activeMaskStackC[0] = true;
    activeMaskStackC[1] = true;
    R0i = as_type<int4>(in.passParameterSem0);
    R1i = as_type<int4>(in.passParameterSem5);
    R2i = as_type<int4>(in.passParameterSem6);
    if (activeMaskStackC[1] == true) {
        R3i.w = as_type<int>(tex6.sample(samplr6, float2(as_type<float>(R0i.x),
                                                         as_type<float>(R0i.y)))
                                 .x);
        R7i.xzw = as_type<int3>(
            tex6.gather(samplr6, float2(0.0001) + float2(as_type<float>(R0i.x),
                                                         as_type<float>(R0i.y)))
                .xzw);
        R4i.xyzw =
            as_type<int4>(tex3.sample(samplr3, float2(as_type<float>(R2i.z),
                                                      as_type<float>(R2i.w)))
                              .xyzw);
        R2i.xy = as_type<int2>(
            tex0.sample(samplr0,
                        float2(as_type<float>(R2i.x), as_type<float>(R2i.y)),
                        level(0.0))
                .xw);
    }
    if (activeMaskStackC[1] == true) {
        // 0
        R123i.x =
            as_type<int>(mul_nonIEEE(ubuff1.d[16].x, as_type<float>(R7i.w)) +
                         ubuff1.d[14].x);
        R127i.y =
            as_type<int>(mul_nonIEEE(ubuff1.d[16].x, as_type<float>(R7i.z)) +
                         ubuff1.d[14].x);
        R127i.z =
            as_type<int>(mul_nonIEEE(ubuff1.d[16].x, as_type<float>(R7i.x)) +
                         ubuff1.d[14].x);
        R123i.w =
            as_type<int>(mul_nonIEEE(as_type<float>(R3i.w), ubuff1.d[16].x) +
                         ubuff1.d[14].x);
        R127i.x = as_type<int>(as_type<float>(R4i.x) * 2.0 + -(1.0));
        // 1
        R3i.x = as_type<int>(
            mul_nonIEEE(as_type<float>(R0i.z), -(as_type<float>(R123i.w))));
        R8i.y = as_type<int>(
            mul_nonIEEE(as_type<float>(R0i.w), -(as_type<float>(R123i.w))));
        R12i.z = as_type<int>(-(as_type<float>(R123i.x)));
        R127i.w = as_type<int>(as_type<float>(R4i.y) * 2.0 + -(1.0));
        R125i.z = as_type<int>(as_type<float>(R4i.z) * 2.0 + -(1.0));
        // 2
        PV0ix = as_type<int>(-(as_type<float>(R127i.y)));
        PV0iy = as_type<int>(
            mul_nonIEEE(as_type<float>(R1i.y), -(as_type<float>(R127i.y))));
        PV0iz = as_type<int>(
            mul_nonIEEE(as_type<float>(R1i.x), -(as_type<float>(R127i.y))));
        PV0iw = as_type<int>(
            mul_nonIEEE(as_type<float>(R1i.z), -(as_type<float>(R127i.z))));
        R126i.z = as_type<int>(
            mul_nonIEEE(as_type<float>(R1i.w), -(as_type<float>(R127i.z))));
        // 3
        R125i.x =
            as_type<int>(-(as_type<float>(R3i.x)) + as_type<float>(PV0iz));
        R127i.y = as_type<int>(-(as_type<float>(R127i.z)));
        R124i.z =
            as_type<int>(-(as_type<float>(R12i.z)) + as_type<float>(PV0ix));
        R125i.w =
            as_type<int>(-(as_type<float>(R8i.y)) + as_type<float>(PV0iy));
        R126i.y =
            as_type<int>(-(as_type<float>(R3i.x)) + as_type<float>(PV0iw));
        // 4
        backupReg0i = R126i.z;
        tempi.x = as_type<int>(
            dot(float4(as_type<float>(R3i.x), as_type<float>(R8i.y),
                       as_type<float>(R12i.z), as_type<float>(0x80000000)),
                float4(as_type<float>(R3i.x), as_type<float>(R8i.y),
                       as_type<float>(R12i.z), 0.0)));
        PV0ix = tempi.x;
        PV0iy = tempi.x;
        R126i.z = tempi.x;
        PV0iw = tempi.x;
        R126i.x = as_type<int>(-(as_type<float>(R8i.y)) +
                               as_type<float>(backupReg0i));
        // 5
        tempi.x = as_type<int>(
            dot(float4(as_type<float>(R127i.x), as_type<float>(R127i.w),
                       as_type<float>(R125i.z), as_type<float>(0x80000000)),
                float4(as_type<float>(R127i.x), as_type<float>(R127i.w),
                       as_type<float>(R125i.z), 0.0)));
        PV1ix = tempi.x;
        PV1iy = tempi.x;
        R127i.z = tempi.x;
        PV1iw = tempi.x;
        R126i.w =
            as_type<int>(-(as_type<float>(R12i.z)) + as_type<float>(R127i.y));
        // 6
        backupReg0i = R126i.z;
        PV0ix =
            as_type<int>(mul_nonIEEE(as_type<float>(R3i.x), ubuff6.d[43].x));
        R127i.y = as_type<int>(
            mul_nonIEEE(as_type<float>(R124i.z), as_type<float>(R126i.y)));
        R126i.z = as_type<int>(
            mul_nonIEEE(as_type<float>(R125i.w), as_type<float>(R126i.w)));
        R124i.w = as_type<int>(
            mul_nonIEEE(as_type<float>(R125i.x), as_type<float>(R126i.x)));
        R125i.y = as_type<int>(sqrt(as_type<float>(backupReg0i)));
        // 7
        backupReg0i = R127i.z;
        R124i.x =
            as_type<int>(mul_nonIEEE(as_type<float>(R3i.x), ubuff6.d[45].x));
        R124i.y = as_type<int>(as_type<float>(R4i.w) * 255.0);
        R127i.z = as_type<int>(as_type<float>(R2i.x) * 2.0 + -(1.0));
        R4i.w =
            as_type<int>(mul_nonIEEE(as_type<float>(R8i.y), ubuff6.d[43].y) +
                         as_type<float>(PV0ix));
        tempResultf = 1.0 / sqrt(as_type<float>(backupReg0i));
        PS1i = as_type<int>(tempResultf);
        // 8
        backupReg0i = R125i.z;
        R4i.x = as_type<int>(
            mul_nonIEEE(as_type<float>(R127i.x), as_type<float>(PS1i)));
        R4i.y = as_type<int>(
            mul_nonIEEE(as_type<float>(R127i.w), as_type<float>(PS1i)));
        R125i.z = as_type<int>(mul_nonIEEE(ubuff1.d[18].y, ubuff1.d[18].z));
        R127i.w = as_type<int>(-(as_type<float>(R12i.z)) *
                               as_type<float>(0x3d4ccccd));
        R2i.z = as_type<int>(
            mul_nonIEEE(as_type<float>(backupReg0i), as_type<float>(PS1i)));
        // 9
        R1i.x = as_type<int>(
            mul_nonIEEE(-(as_type<float>(R126i.x)), as_type<float>(R124i.z)) +
            as_type<float>(R126i.z));
        R3i.y = as_type<int>(
            mul_nonIEEE(-(as_type<float>(R126i.w)), as_type<float>(R125i.x)) +
            as_type<float>(R127i.y));
        R126i.z = as_type<int>(as_type<float>(R2i.y) * 2.0 + -(1.0));
        R123i.w =
            as_type<int>(mul_nonIEEE(as_type<float>(R8i.y), ubuff6.d[45].y) +
                         as_type<float>(R124i.x));
        R126i.w = as_type<int>(
            mul_nonIEEE(as_type<float>(R125i.z), as_type<float>(R127i.z)));
        // 10
        backupReg0i = R126i.y;
        R126i.x =
            as_type<int>(mul_nonIEEE(as_type<float>(R12i.z), ubuff6.d[45].z) +
                         as_type<float>(R123i.w));
        R126i.y =
            as_type<int>(mul_nonIEEE(as_type<float>(R12i.z), ubuff6.d[43].z) +
                         as_type<float>(R4i.w));
        R1i.z = as_type<int>(mul_nonIEEE(-(as_type<float>(backupReg0i)),
                                         as_type<float>(R125i.w)) +
                             as_type<float>(R124i.w));
        R125i.w = R4i.x;
        R125i.w = as_type<int>(as_type<float>(R125i.w) * 2.0);
        R124i.z = R4i.y;
        R124i.z = as_type<int>(as_type<float>(R124i.z) * 2.0);
        // 11
        R124i.x = as_type<int>(
            dot(float4(-(as_type<float>(R1i.x)), -(as_type<float>(R3i.y)),
                       -(as_type<float>(R1i.z)), as_type<float>(0x80000000)),
                float4(-(as_type<float>(R1i.x)), -(as_type<float>(R3i.y)),
                       -(as_type<float>(R1i.z)), 0.0)));
        PV1iy = R124i.x;
        PV1iz = R124i.x;
        PV1iw = R124i.x;
        R2i.x = as_type<int>(-(ubuff6.d[53].w) * as_type<float>(0x3d4ccccd) +
                             as_type<float>(R127i.w));
        R2i.x = clampFI32(R2i.x);
        // 12
        R125i.x = as_type<int>(
            mul_nonIEEE(as_type<float>(R125i.z), -(as_type<float>(R126i.z))));
        R127i.y =
            as_type<int>(as_type<float>(R126i.z) * as_type<float>(0xbb6fe5d7));
        PV0iz =
            as_type<int>(as_type<float>(R126i.w) * as_type<float>(0x3ca30589));
        PV0iw =
            as_type<int>(as_type<float>(R126i.z) * as_type<float>(0x3ca30589));
        R126i.z =
            as_type<int>(as_type<float>(R126i.w) * as_type<float>(0xbb6fe5d7));
        // 13
        R7i.x = as_type<int>(ubuff6.d[43].w + as_type<float>(R126i.y));
        R1i.y =
            as_type<int>(as_type<float>(R127i.z) * as_type<float>(0x3b02da3b) +
                         as_type<float>(PV0iw));
        R3i.z = as_type<int>(
            mul_nonIEEE(-(as_type<float>(R12i.z)), ubuff1.d[17].y));
        R1i.w = int(0x3f800000);
        R6i.x =
            as_type<int>(as_type<float>(R125i.x) * as_type<float>(0x3b02da3b) +
                         as_type<float>(PV0iz));
        // 14
        R5i.x =
            as_type<int>(as_type<float>(R125i.x) * as_type<float>(0x3d156fb9) +
                         as_type<float>(R126i.z));
        R5i.y =
            as_type<int>(as_type<float>(R127i.z) * as_type<float>(0x3d156fb9) +
                         as_type<float>(R127i.y));
        R13i.z = as_type<int>(ubuff6.d[45].w + as_type<float>(R126i.x));
        R12i.w = as_type<int>(-(ubuff6.d[53].z) * as_type<float>(0x3d4ccccd) +
                              as_type<float>(R127i.w));
        R12i.w = clampFI32(R12i.w);
        PS0i = as_type<int>(1.0 / as_type<float>(R125i.y));
        // 15
        backupReg0i = R124i.y;
        R126i.x = as_type<int>(
            mul_nonIEEE(as_type<float>(R3i.x), as_type<float>(PS0i)));
        R124i.y = as_type<int>(
            mul_nonIEEE(as_type<float>(R8i.y), as_type<float>(PS0i)));
        PV1iz = as_type<int>(
            mul_nonIEEE(as_type<float>(R12i.z), as_type<float>(PS0i)));
        PS1i = int(as_type<float>(backupReg0i));
        // 16
        tempi.x = as_type<int>(
            dot(float4(as_type<float>(R4i.x), as_type<float>(R4i.y),
                       as_type<float>(R2i.z), as_type<float>(0x80000000)),
                float4(as_type<float>(R126i.x), as_type<float>(R124i.y),
                       as_type<float>(PV1iz), 0.0)));
        PV0ix = tempi.x;
        PV0iy = tempi.x;
        PV0iz = tempi.x;
        PV0iw = tempi.x;
        PS0i = PS1i & int(1);
        // 17
        R12i.x = as_type<int>(
            mul_nonIEEE(-(as_type<float>(PV0ix)), as_type<float>(R125i.w)) +
            as_type<float>(R126i.x));
        R7i.y = as_type<int>(
            mul_nonIEEE(-(as_type<float>(PV0ix)), as_type<float>(R124i.z)) +
            as_type<float>(R124i.y));
        R13i.w = as_type<int>(as_type<float>(PV0ix) + 1.0);
        R13i.w = clampFI32(R13i.w);
        R2i.y = ((PS0i == 0) ? (0) : (int(0x3f800000)));
        // 18
        tempResultf = 1.0 / sqrt(as_type<float>(R124i.x));
        R2i.w = as_type<int>(tempResultf);
    }
    if (activeMaskStackC[1] == true) {
        activeMaskStack[1] = activeMaskStack[0];
        activeMaskStackC[2] = activeMaskStackC[1];
        // 0
        PV0ix = as_type<int>(ubuff10.d[2].z + 1.0);
        R9i.y = as_type<int>(mul_nonIEEE(ubuff6.d[4].y, ubuff10.d[2].w));
        R8i.x = as_type<int>(mul_nonIEEE(ubuff6.d[4].x, ubuff10.d[2].w));
        // 1
        R10i.z = as_type<int>(mul_nonIEEE(ubuff6.d[4].z, ubuff10.d[2].w));
        R14i.w = as_type<int>(-(as_type<float>(R2i.x)) + as_type<float>(PV0ix));
        R14i.w = clampFI32(R14i.w);
        R4i.w = 0;
        // 2
        predResult = (1.0 > as_type<float>(R12i.w));
        activeMaskStack[1] = predResult;
        activeMaskStackC[2] = predResult == true && activeMaskStackC[1] == true;
    } else {
        activeMaskStack[1] = false;
        activeMaskStackC[2] = false;
    }
    if (activeMaskStackC[2] == true) {
        // 0
        R13i.x = as_type<int>(as_type<float>(R0i.x) + as_type<float>(R6i.x));
        R13i.y = as_type<int>(as_type<float>(R0i.y) + as_type<float>(R1i.y));
        R14i.x = as_type<int>(as_type<float>(R0i.x) + -(as_type<float>(R6i.x)));
        // 1
        R15i.x = as_type<int>(as_type<float>(R0i.x) + as_type<float>(R5i.x));
        R14i.y = as_type<int>(as_type<float>(R0i.y) + -(as_type<float>(R1i.y)));
        R15i.y = as_type<int>(as_type<float>(R0i.y) + as_type<float>(R5i.y));
        // 2
        R16i.x = as_type<int>(as_type<float>(R0i.x) + -(as_type<float>(R5i.x)));
        R16i.y = as_type<int>(as_type<float>(R0i.y) + -(as_type<float>(R5i.y)));
    }
    if (activeMaskStackC[2] == true) {
        // 0
        PV0ix = as_type<int>(as_type<float>(R4i.x) * 1.5);
        PV0iy = as_type<int>(as_type<float>(R4i.y) * 1.5);
        PV0iz = ((-(as_type<float>(R12i.z)) > ubuff1.d[58].x) ? -1 : 0);
        R127i.w = ((-(as_type<float>(R12i.z)) > ubuff1.d[58].y) ? -1 : 0);
        PS0i = as_type<int>(as_type<float>(R2i.z) * 1.5);
        // 1
        backupReg0i = R1i.z;
        R1i.x = as_type<int>(
            mul_nonIEEE(-(as_type<float>(R1i.x)), as_type<float>(R2i.w)) +
            as_type<float>(PV0ix));
        PV1iy = PV0iz & int(1);
        R1i.z = as_type<int>(
            mul_nonIEEE(-(as_type<float>(R3i.y)), as_type<float>(R2i.w)) +
            as_type<float>(PV0iy));
        R1i.w = as_type<int>(
            mul_nonIEEE(-(as_type<float>(backupReg0i)), as_type<float>(R2i.w)) +
            as_type<float>(PS0i));
        R122i.x = as_type<int>(as_type<float>(R3i.z) * 0.25 + 1.0);
        // 2
        backupReg0i = R2i.y;
        R2i.x = as_type<int>(ubuff6.d[53].y / resScale);
        R2i.x = as_type<int>(as_type<float>(R2i.x) / 2.0);
        R2i.y = as_type<int>(ubuff6.d[53].x / resScale);
        R2i.y = as_type<int>(as_type<float>(R2i.y) / 2.0);
        R3i.z = as_type<int>(
            mul_nonIEEE(as_type<float>(backupReg0i), as_type<float>(R122i.x)));
        R3i.w = PV1iy - R127i.w;
        PS0i = as_type<int>(
            mul_nonIEEE(as_type<float>(R1i.w), as_type<float>(R1i.w)));
        // 3
        R123i.x = as_type<int>(
            mul_nonIEEE(as_type<float>(R1i.z), as_type<float>(R1i.z)) +
            as_type<float>(PS0i));
        R3i.y = as_type<int>(-(as_type<float>(R2i.x)));
        R7i.z = as_type<int>(-(as_type<float>(R2i.y)));
        PV1iw = R3i.w << int(0x2);
        R1i.y = as_type<int>(float(R3i.w));
        // 4
        R0i.x = PV1iw + int(0x2c);
        R0i.y = PV1iw + int(0x2b);
        R0i.z = PV1iw + int(0x2a);
        R0i.w = PV1iw + int(0x2d);
        R122i.x = as_type<int>(
            mul_nonIEEE(as_type<float>(R1i.x), as_type<float>(R1i.x)) +
            as_type<float>(R123i.x));
        // 5
        backupReg0i = R2i.z;
        tempi.x = as_type<int>(dot(
            float4(as_type<float>(R4i.x), as_type<float>(R4i.y),
                   as_type<float>(backupReg0i), as_type<float>(backupReg0i)),
            float4(-(as_type<float>(R8i.x)), -(as_type<float>(R9i.y)),
                   -(as_type<float>(R10i.z)), -(as_type<float>(R4i.w)))));
        tempi.x = clampFI32(tempi.x);
        PV1ix = tempi.x;
        PV1iy = tempi.x;
        R2i.z = tempi.x;
        PV1iw = tempi.x;
        tempResultf = 1.0 / sqrt(as_type<float>(R122i.x));
        R2i.w = as_type<int>(tempResultf);
    }
    if (activeMaskStackC[2] == true) {
        R4i.xyzw = as_type<int4>(ubuff1.d[R0i.w].xyzw);
        R5i.xyzw = as_type<int4>(ubuff1.d[R0i.x].xyzw);
        R6i.xyzw = as_type<int4>(ubuff1.d[R0i.z].xyzw);
        R0i.xyzw = as_type<int4>(ubuff1.d[R0i.y].xyzw);
    }
    if (activeMaskStackC[2] == true) {
        // 0
        R127i.x = as_type<int>(
            mul_nonIEEE(as_type<float>(R1i.x), as_type<float>(R2i.w)));
        R127i.y = as_type<int>(
            mul_nonIEEE(as_type<float>(R1i.z), as_type<float>(R2i.w)));
        R127i.z = as_type<int>(
            mul_nonIEEE(as_type<float>(R1i.w), as_type<float>(R2i.w)));
        R127i.w = R2i.x;
        R8i.z = as_type<int>(rint(as_type<float>(R1i.y)));
        // 1
        tempi.x = as_type<int>(
            dot(float4(as_type<float>(R8i.x), as_type<float>(R9i.y),
                       as_type<float>(R10i.z), as_type<float>(0x80000000)),
                float4(as_type<float>(R127i.x), as_type<float>(R127i.y),
                       as_type<float>(R127i.z), 0.0)));
        PV1ix = tempi.x;
        PV1iy = tempi.x;
        PV1iz = tempi.x;
        PV1iw = tempi.x;
        R9i.z = R8i.z;
        // 2
        R123i.x = as_type<int>(
            mul_nonIEEE(-(as_type<float>(R10i.z)), as_type<float>(PV1ix)) +
            as_type<float>(R127i.z));
        R123i.y = as_type<int>(
            mul_nonIEEE(-(as_type<float>(R9i.y)), as_type<float>(PV1ix)) +
            as_type<float>(R127i.y));
        R123i.z = as_type<int>(
            mul_nonIEEE(-(as_type<float>(PV1ix)), as_type<float>(PV1ix)) + 1.0);
        R123i.w = as_type<int>(
            mul_nonIEEE(-(as_type<float>(R8i.x)), as_type<float>(PV1ix)) +
            as_type<float>(R127i.x));
        R10i.z = R8i.z;
        // 3
        R127i.x = as_type<int>(
            mul_nonIEEE(as_type<float>(R3i.z), as_type<float>(R123i.w)));
        R127i.y = as_type<int>(
            mul_nonIEEE(as_type<float>(R3i.z), as_type<float>(R123i.y)));
        R127i.z = as_type<int>(
            mul_nonIEEE(as_type<float>(R3i.z), as_type<float>(R123i.x)));
        PV1iw = as_type<int>(
            mul_nonIEEE(as_type<float>(R123i.z), as_type<float>(R2i.z)));
        R11i.z = R8i.z;
        // 4
        PV0ix =
            as_type<int>(as_type<float>(PV1iw) * as_type<float>(0x3da1ff2e));
        // 5
        R17i.x = as_type<int>(
            mul_nonIEEE(as_type<float>(R127i.x), as_type<float>(PV0ix)) +
            as_type<float>(R3i.x));
        R12i.y = as_type<int>(
            mul_nonIEEE(as_type<float>(R127i.y), as_type<float>(PV0ix)) +
            as_type<float>(R8i.y));
        R12i.z = as_type<int>(
            mul_nonIEEE(as_type<float>(R127i.z), as_type<float>(PV0ix)) +
            as_type<float>(R12i.z));
        // 6
        PV0ix = as_type<int>(
            mul_nonIEEE(as_type<float>(R17i.x), as_type<float>(R5i.x)));
        R127i.y = as_type<int>(
            mul_nonIEEE(as_type<float>(R17i.x), as_type<float>(R6i.x)));
        PV0iz = as_type<int>(
            mul_nonIEEE(as_type<float>(R17i.x), as_type<float>(R4i.x)));
        // 7
        R123i.x = as_type<int>(
            mul_nonIEEE(as_type<float>(R12i.y), as_type<float>(R5i.y)) +
            as_type<float>(PV0ix));
        R123i.y = as_type<int>(
            mul_nonIEEE(as_type<float>(R12i.y), as_type<float>(R4i.y)) +
            as_type<float>(PV0iz));
        R127i.z = as_type<int>(
            mul_nonIEEE(as_type<float>(R17i.x), as_type<float>(R0i.x)));
        // 8
        backupReg0i = R123i.x;
        R123i.x = as_type<int>(
            mul_nonIEEE(as_type<float>(R12i.z), as_type<float>(R4i.z)) +
            as_type<float>(R123i.y));
        R123i.y = as_type<int>(
            mul_nonIEEE(as_type<float>(R12i.z), as_type<float>(R5i.z)) +
            as_type<float>(backupReg0i));
        R123i.z = as_type<int>(
            mul_nonIEEE(as_type<float>(R12i.y), as_type<float>(R6i.y)) +
            as_type<float>(R127i.y));
        // 9
        R127i.x = as_type<int>(as_type<float>(R5i.w) + as_type<float>(R123i.y));
        R123i.y = as_type<int>(
            mul_nonIEEE(as_type<float>(R12i.y), as_type<float>(R0i.y)) +
            as_type<float>(R127i.z));
        R123i.z = as_type<int>(
            mul_nonIEEE(as_type<float>(R12i.z), as_type<float>(R6i.z)) +
            as_type<float>(R123i.z));
        PV1iw = as_type<int>(as_type<float>(R4i.w) + as_type<float>(R123i.x));
        // 10
        R123i.x = as_type<int>(
            mul_nonIEEE(as_type<float>(R12i.z), as_type<float>(R0i.z)) +
            as_type<float>(R123i.y));
        PV0iw = as_type<int>(as_type<float>(R6i.w) + as_type<float>(R123i.z));
        R127i.z = as_type<int>(1.0 / as_type<float>(PV1iw));
        // 11
        PV1ix = as_type<int>(as_type<float>(R0i.w) + as_type<float>(R123i.x));
        R127i.y = as_type<int>(
            mul_nonIEEE(as_type<float>(PV0iw), as_type<float>(R127i.z)));
        PV1iw = as_type<int>(
            mul_nonIEEE(as_type<float>(R127i.x), as_type<float>(R127i.z)));
        // 12
        R127i.x = as_type<int>(
            mul_nonIEEE(as_type<float>(PV1ix), as_type<float>(R127i.z)));
        PV0iz =
            as_type<int>(as_type<float>(PV1iw) + as_type<float>(0xbb03126f));
        R8i.x = as_type<int>(ubuff6.d[53].x / resScale * 0.5 +
                             as_type<float>(R127i.y));
        // 13
        R126i.x =
            as_type<int>(-(as_type<float>(R1i.y)) * as_type<float>(0x3a03126f) +
                         as_type<float>(PV0iz));
        R126i.x = clampFI32(R126i.x);
        R8i.y = as_type<int>(ubuff6.d[53].y / resScale * 0.5 +
                             as_type<float>(R127i.x));
        R9i.x = as_type<int>(as_type<float>(R127i.y) + as_type<float>(R2i.y));
        // 14
        R10i.x = as_type<int>(as_type<float>(R127i.y) + as_type<float>(R7i.z));
        R9i.y = as_type<int>(as_type<float>(R127i.x) + as_type<float>(R3i.y));
        R8i.w = R126i.x;
        R9i.w = R126i.x;
        // 15
        R11i.x = as_type<int>(-(ubuff6.d[53].x) / resScale * 0.5 +
                              as_type<float>(R127i.y));
        R10i.y =
            as_type<int>(as_type<float>(R127i.x) + as_type<float>(R127i.w));
        R10i.w = R126i.x;
        R11i.y = as_type<int>(-(ubuff6.d[53].y) / resScale * 0.5 +
                              as_type<float>(R127i.x));
        // 16
        R11i.w = R126i.x;
    }
    if (activeMaskStackC[2] == true) {
        R8i.z = as_type<int>(
            float4(tex8.sample_compare(
                       samplr8,
                       float2(as_type<float>(R8i.x), as_type<float>(R8i.y)),
                       uint(rint(as_type<float>(R8i.z))),
                       as_type<float>(R8i.w)))
                .x);
        R9i.y = as_type<int>(
            float4(tex8.sample_compare(
                       samplr8,
                       float2(as_type<float>(R9i.x), as_type<float>(R9i.y)),
                       uint(rint(as_type<float>(R9i.z))),
                       as_type<float>(R9i.w)))
                .x);
        R10i.x = as_type<int>(
            float4(tex8.sample_compare(
                       samplr8,
                       float2(as_type<float>(R10i.x), as_type<float>(R10i.y)),
                       uint(rint(as_type<float>(R10i.z))),
                       as_type<float>(R10i.w)))
                .x);
        R11i.w = as_type<int>(
            float4(tex8.sample_compare(
                       samplr8,
                       float2(as_type<float>(R11i.x), as_type<float>(R11i.y)),
                       uint(rint(as_type<float>(R11i.z))),
                       as_type<float>(R11i.w)))
                .x);
    }
    if (activeMaskStackC[2] == true) {
        activeMaskStack[2] = activeMaskStack[1];
        activeMaskStackC[3] = activeMaskStackC[2];
        // 0
        PV0iw = as_type<int>(as_type<float>(R8i.z) + as_type<float>(R9i.y));
        PV0iw = as_type<int>(as_type<float>(PV0iw) / 2.0);
        // 1
        R123i.y =
            as_type<int>(as_type<float>(R10i.x) * 0.5 + as_type<float>(PV0iw));
        // 2
        R4i.z = as_type<int>(
            (as_type<float>(R11i.w) * 0.5 + as_type<float>(R123i.y)) / 2.0);
        // 3
        PV1ix = ((1.0 > as_type<float>(R4i.z)) ? -1 : 0);
        // 4
        R0i.w = ((R3i.w > 0) ? (PV1ix) : (0));
        // 5
        predResult = (R0i.w != 0);
        activeMaskStack[2] = predResult;
        activeMaskStackC[3] = predResult == true && activeMaskStackC[2] == true;
    } else {
        activeMaskStack[2] = false;
        activeMaskStackC[3] = false;
    }
    if (activeMaskStackC[3] == true) {
        // 0
        PV0iz = int(-1) + R3i.w;
        // 1
        PV1iy = PV0iz << int(0x2);
        R4i.x = as_type<int>(float(PV0iz));
        // 2
        R0i.x = PV1iy + int(0x2a);
        R0i.y = PV1iy + int(0x2d);
        R0i.z = PV1iy + int(0x2c);
        R0i.w = PV1iy + int(0x2b);
        R5i.z = as_type<int>(rint(as_type<float>(R4i.x)));
    }
    if (activeMaskStackC[3] == true) {
        R1i.xyzw = as_type<int4>(ubuff1.d[R0i.y].xyzw);
        R2i.xyzw = as_type<int4>(ubuff1.d[R0i.z].xyzw);
        R3i.xyzw = as_type<int4>(ubuff1.d[R0i.x].xyzw);
        R0i.xyzw = as_type<int4>(ubuff1.d[R0i.w].xyzw);
    }
    if (activeMaskStackC[3] == true) {
        // 0
        PV0ix = as_type<int>(
            mul_nonIEEE(as_type<float>(R17i.x), as_type<float>(R1i.x)));
        PV0iy = as_type<int>(
            mul_nonIEEE(as_type<float>(R17i.x), as_type<float>(R2i.x)));
        // 1
        R123i.x = as_type<int>(
            mul_nonIEEE(as_type<float>(R12i.y), as_type<float>(R2i.y)) +
            as_type<float>(PV0iy));
        PV1iy = as_type<int>(
            mul_nonIEEE(as_type<float>(R17i.x), as_type<float>(R3i.x)));
        PV1iz = as_type<int>(
            mul_nonIEEE(as_type<float>(R17i.x), as_type<float>(R0i.x)));
        R123i.w = as_type<int>(
            mul_nonIEEE(as_type<float>(R12i.y), as_type<float>(R1i.y)) +
            as_type<float>(PV0ix));
        // 2
        R123i.x = as_type<int>(
            mul_nonIEEE(as_type<float>(R12i.z), as_type<float>(R2i.z)) +
            as_type<float>(R123i.x));
        R123i.y = as_type<int>(
            mul_nonIEEE(as_type<float>(R12i.y), as_type<float>(R0i.y)) +
            as_type<float>(PV1iz));
        R123i.z = as_type<int>(
            mul_nonIEEE(as_type<float>(R12i.z), as_type<float>(R1i.z)) +
            as_type<float>(R123i.w));
        R123i.w = as_type<int>(
            mul_nonIEEE(as_type<float>(R12i.y), as_type<float>(R3i.y)) +
            as_type<float>(PV1iy));
        // 3
        R127i.x = as_type<int>(as_type<float>(R2i.w) + as_type<float>(R123i.x));
        PV1iy = as_type<int>(as_type<float>(R1i.w) + as_type<float>(R123i.z));
        R123i.z = as_type<int>(
            mul_nonIEEE(as_type<float>(R12i.z), as_type<float>(R3i.z)) +
            as_type<float>(R123i.w));
        R123i.w = as_type<int>(
            mul_nonIEEE(as_type<float>(R12i.z), as_type<float>(R0i.z)) +
            as_type<float>(R123i.y));
        // 4
        PV0ix = as_type<int>(as_type<float>(R0i.w) + as_type<float>(R123i.w));
        PV0iy = as_type<int>(as_type<float>(R3i.w) + as_type<float>(R123i.z));
        PS0i = as_type<int>(1.0 / as_type<float>(PV1iy));
        // 5
        R5i.x = as_type<int>(
            mul_nonIEEE(as_type<float>(PV0iy), as_type<float>(PS0i)));
        R5i.y = as_type<int>(
            mul_nonIEEE(as_type<float>(PV0ix), as_type<float>(PS0i)));
        PV1iw = as_type<int>(
            mul_nonIEEE(as_type<float>(R127i.x), as_type<float>(PS0i)));
        // 6
        PV0iy =
            as_type<int>(as_type<float>(PV1iw) + as_type<float>(0xbb03126f));
        // 7
        R5i.w =
            as_type<int>(-(as_type<float>(R4i.x)) * as_type<float>(0x3a03126f) +
                         as_type<float>(PV0iy));
        R5i.w = clampFI32(R5i.w);
    }
    if (activeMaskStackC[3] == true) {
        R5i.z = as_type<int>(
            float4(tex8.sample_compare(
                       samplr8,
                       float2(as_type<float>(R5i.x), as_type<float>(R5i.y)),
                       uint(rint(as_type<float>(R5i.z))),
                       as_type<float>(R5i.w)))
                .x);
    }
    if (activeMaskStackC[3] == true) {
        // 0
        R4i.z = as_type<int>(
            mul_nonIEEE(as_type<float>(R4i.z), as_type<float>(R5i.z)));
    }
    activeMaskStackC[2] =
        activeMaskStack[1] == true && activeMaskStackC[1] == true;
    if (activeMaskStackC[2] == true) {
        R0i.x =
            as_type<int>(tex6.sample(samplr6, float2(as_type<float>(R15i.x),
                                                     as_type<float>(R15i.y)))
                             .x);
        R0i.y =
            as_type<int>(tex6.sample(samplr6, float2(as_type<float>(R16i.x),
                                                     as_type<float>(R16i.y)))
                             .x);
        R1i.x =
            as_type<int>(tex6.sample(samplr6, float2(as_type<float>(R13i.x),
                                                     as_type<float>(R13i.y)))
                             .x);
        R1i.y =
            as_type<int>(tex6.sample(samplr6, float2(as_type<float>(R14i.x),
                                                     as_type<float>(R14i.y)))
                             .x);
    }
    if (activeMaskStackC[2] == true) {
        // 0
        R127i.x =
            as_type<int>(as_type<float>(R7i.w) * as_type<float>(0x3f7eb852));
        PV0iy = as_type<int>(ubuff1.d[16].x * 20.0);
        R124i.z = as_type<int>(-(as_type<float>(R4i.z)) + 1.0);
        R127i.w = as_type<int>(ubuff1.d[16].x * as_type<float>(0x3f555555));
        // 1
        PV1ix =
            as_type<int>(as_type<float>(R127i.x) + -(as_type<float>(R0i.y)));
        PV1iy =
            as_type<int>(as_type<float>(R127i.x) + -(as_type<float>(R0i.x)));
        R127i.z =
            as_type<int>(as_type<float>(PV0iy) * as_type<float>(0x3f8ba8d6));
        R127i.z = as_type<int>(as_type<float>(R127i.z) / 2.0);
        PV1iw =
            as_type<int>(as_type<float>(PV0iy) * as_type<float>(0x3fbc4580));
        PV1iw = as_type<int>(as_type<float>(PV1iw) / 2.0);
        R126i.z =
            as_type<int>(-(as_type<float>(R1i.x)) + as_type<float>(R127i.x));
        // 2
        backupReg0i = R127i.x;
        R127i.x = as_type<int>(
            mul_nonIEEE(-(as_type<float>(R127i.w)), as_type<float>(PV1iy)) +
            1.0);
        R127i.x = clampFI32(R127i.x);
        R127i.y = as_type<int>(
            mul_nonIEEE(as_type<float>(PV1iw), as_type<float>(PV1ix)) + 0.5);
        R127i.y = clampFI32(R127i.y);
        R125i.z = as_type<int>(
            mul_nonIEEE(as_type<float>(PV1iw), as_type<float>(PV1iy)) + 0.5);
        R125i.z = clampFI32(R125i.z);
        R126i.w = as_type<int>(-(as_type<float>(R1i.y)) +
                               as_type<float>(backupReg0i));
        R125i.w = as_type<int>(
            mul_nonIEEE(-(as_type<float>(R127i.w)), as_type<float>(PV1ix)) +
            1.0);
        R125i.w = clampFI32(R125i.w);
        // 3
        R126i.x = as_type<int>(
            mul_nonIEEE(as_type<float>(R127i.z), as_type<float>(R126i.z)) +
            0.5);
        R126i.x = clampFI32(R126i.x);
        PV1iy = as_type<int>(0.5 + -(as_type<float>(R127i.y)));
        PV1iz = as_type<int>(0.5 + -(as_type<float>(R125i.z)));
        R124i.w = as_type<int>(
            mul_nonIEEE(as_type<float>(R127i.z), as_type<float>(R126i.w)) +
            0.5);
        R124i.w = clampFI32(R124i.w);
        R0i.w = as_type<int>(
            mul_nonIEEE(-(as_type<float>(R127i.w)), as_type<float>(R126i.z)) +
            1.0);
        R0i.w = clampFI32(R0i.w);
        // 4
        backupReg0i = R127i.w;
        PV0ix = as_type<int>(0.5 + -(as_type<float>(R126i.x)));
        R126i.y = as_type<int>(
            mul_nonIEEE(as_type<float>(PV1iz), as_type<float>(R127i.x)) + 0.5);
        PV0iz = as_type<int>(0.5 + -(as_type<float>(R124i.w)));
        R127i.w = as_type<int>(
            mul_nonIEEE(as_type<float>(PV1iy), as_type<float>(R125i.w)) + 0.5);
        R125i.y = as_type<int>(mul_nonIEEE(-(as_type<float>(backupReg0i)),
                                           as_type<float>(R126i.w)) +
                               1.0);
        R125i.y = clampFI32(R125i.y);
        // 5
        PV1ix =
            as_type<int>(as_type<float>(R127i.y) + -(as_type<float>(R126i.y)));
        R127i.y = as_type<int>(
            mul_nonIEEE(as_type<float>(PV0iz), as_type<float>(R125i.y)) + 0.5);
        PV1iz =
            as_type<int>(as_type<float>(R125i.z) + -(as_type<float>(R127i.w)));
        R126i.w = as_type<int>(
            mul_nonIEEE(as_type<float>(PV0ix), as_type<float>(R0i.w)) + 0.5);
        R3i.w = as_type<int>(
            mul_nonIEEE(as_type<float>(R124i.z), as_type<float>(R12i.w)) +
            as_type<float>(R4i.z));
        // 6
        R123i.x = as_type<int>(
            mul_nonIEEE(as_type<float>(PV1iz), as_type<float>(R127i.x)) +
            as_type<float>(R127i.w));
        PV0iy =
            as_type<int>(as_type<float>(R126i.x) + -(as_type<float>(R127i.y)));
        PV0iz =
            as_type<int>(as_type<float>(R124i.w) + -(as_type<float>(R126i.w)));
        R123i.w = as_type<int>(
            mul_nonIEEE(as_type<float>(PV1ix), as_type<float>(R125i.w)) +
            as_type<float>(R126i.y));
        // 7
        PV1ix =
            as_type<int>(as_type<float>(R123i.x) * as_type<float>(0x3e35e743));
        PV1iy =
            as_type<int>(as_type<float>(R123i.w) * as_type<float>(0x3e35e743));
        R123i.z = as_type<int>(
            mul_nonIEEE(as_type<float>(PV0iz), as_type<float>(R125i.y)) +
            as_type<float>(R126i.w));
        R123i.w = as_type<int>(
            mul_nonIEEE(as_type<float>(PV0iy), as_type<float>(R0i.w)) +
            as_type<float>(R127i.y));
        // 8
        R123i.x =
            as_type<int>(as_type<float>(R123i.w) * as_type<float>(0x3e825397) +
                         as_type<float>(PV1ix));
        R123i.w =
            as_type<int>(as_type<float>(R123i.z) * as_type<float>(0x3e825397) +
                         as_type<float>(PV1iy));
        // 9
        PV1iz = as_type<int>(as_type<float>(R123i.x) + as_type<float>(R123i.w));
        // 10
        PV0iy =
            as_type<int>(as_type<float>(PV1iz) + as_type<float>(0xbedd476b));
        // 11
        PV1ix = as_type<int>(as_type<float>(PV0iy) * 6.0);
        PV1ix = clampFI32(PV1ix);
        // 12
        R1i.w = as_type<int>(-(as_type<float>(PV1ix)) + 1.0);
    }
    activeMaskStack[1] = activeMaskStack[1] == false;
    activeMaskStackC[2] =
        activeMaskStack[1] == true && activeMaskStackC[1] == true;
    if (activeMaskStackC[2] == true) {
        // 0
        R3i.w = R1i.w;
    }
    activeMaskStackC[1] =
        activeMaskStack[0] == true && activeMaskStackC[0] == true;
    if (activeMaskStackC[1] == true) {
        // 0
        R0i.x = as_type<int>(
            as_type<float>(R7i.x) * as_type<float>(0x38d1b717) + 0.5);
        R0i.y = as_type<int>(
            as_type<float>(R13i.z) * as_type<float>(0x3903126f) + 0.5);
        PV0iz = as_type<int>(-(as_type<float>(R7i.y)));
        PV0iz = as_type<int>(as_type<float>(PV0iz) / 2.0);
        PV0iw = R12i.x;
        PV0iw = as_type<int>(as_type<float>(PV0iw) / 2.0);
        R2i.z = R1i.w;
        // 1
        R1i.x = as_type<int>(as_type<float>(PV0iw) + 0.5);
        R1i.y = as_type<int>(as_type<float>(PV0iz) + 0.5);
        R2i.w = int(0x3f800000);
    }
    if (activeMaskStackC[1] == true) {
        R0i.xyz = as_type<int3>(
            tex15
                .sample(samplr15,
                        float2(as_type<float>(R0i.x), as_type<float>(R0i.y)),
                        level(0.0))
                .xyz);
        R1i.y = as_type<int>(
            tex1.sample(samplr1,
                        float2(as_type<float>(R1i.x), as_type<float>(R1i.y)),
                        level(0.0))
                .x);
    }
    if (activeMaskStackC[1] == true) {
        // 0
        tempi.x = as_type<int>(
            dot(float4(as_type<float>(R0i.x), as_type<float>(R0i.y),
                       as_type<float>(R0i.z), as_type<float>(0x80000000)),
                float4(ubuff6.d[42].x, ubuff6.d[42].y, ubuff6.d[42].z, 0.0)));
        PV0ix = tempi.x;
        PV0iy = tempi.x;
        PV0iz = tempi.x;
        PV0iw = tempi.x;
        R2i.y = as_type<int>(
            mul_nonIEEE(as_type<float>(R1i.y), -(as_type<float>(R13i.w))) +
            as_type<float>(R1i.y));
        // 1
        PV1iw = as_type<int>(
            mul_nonIEEE(as_type<float>(PV0ix), as_type<float>(PV0ix)));
        // 2
        R127i.z = as_type<int>(
            mul_nonIEEE(as_type<float>(PV1iw), as_type<float>(PV1iw)));
        // 3
        PV1iy =
            as_type<int>(as_type<float>(R3i.w) + -(as_type<float>(R127i.z)));
        // 4
        R123i.x = as_type<int>(
            mul_nonIEEE(as_type<float>(PV1iy), as_type<float>(R14i.w)) +
            as_type<float>(R127i.z));
        // 5
        R2i.x =
            as_type<int>(mul_nonIEEE(as_type<float>(R123i.x), ubuff10.d[3].z) +
                         ubuff10.d[1].y);
        R2i.x = clampFI32(R2i.x);
    }
    // export
    out.passPixelColor5 =
        as_type<float4>(float4(as_type<float>(R2i.x), as_type<float>(R2i.y),
                               as_type<float>(R2i.z), as_type<float>(R2i.w)));
    return out;
}
)BOTW_MSL_END"},
{0x9d736b07c1fcd57eULL, 0x0000000000000000ULL, GraphicPack2::GP_SHADER_TYPE::VERTEX, R"BOTW_MSL_END(#include <metal_stdlib>
using namespace metal;

// shader 9d736b07c1fcd57e
// Shader dumped from BotW 208, using Cemu 2193a8c
// Based on 75a85b0cbcab764b_0000000000000000_vs.txt
// Used for: Horizontal Reflection Blur

constant float resXScale = float($width) / float($gameWidth);

#define SET_POSITION(_v)                                                       \
    out.position = _v;                                                         \
    out.position.z = (out.position.z + out.position.w) / 2.0
// start of shader inputs/outputs, predetermined by Cemu. Do not touch
struct SupportBuffer {
    int4 remapped[1];
};

struct VertexIn {
    uint4 attrDataSem0 [[attribute(0)]];
    uint4 attrDataSem1 [[attribute(1)]];
};

struct VertexOut {
    float4 position [[position]] [[invariant]];
    float4 passParameterSem1 [[user(locn1)]];
    float4 passParameterSem0 [[user(locn0)]];
};

// end of shader inputs/outputs
template <typename TextureT, typename CoordT>
float sampleCompareEmulate(TextureT tex, sampler samplr, CoordT coord,
                           float compareValue) {
    return compareValue < tex.sample(samplr, coord).x ? 1.0 : 0.0;
}
template <typename TextureT, typename CoordT>
float2 textureCalculateLod(TextureT tex, sampler samplr, CoordT coord) {
    float lod = tex.calculate_unclamped_lod(samplr, coord);
    return float2(floor(lod), fract(lod));
}
int clampFI32(int v) {
    if (v == 0x7FFFFFFF)
        return as_type<int>(1.0);
    else if (v == 0xFFFFFFFF)
        return as_type<int>(0.0);
    return as_type<int>(clamp(as_type<float>(v), 0.0, 1.0));
}
float mul_nonIEEE(float a, float b) {
    if (a == 0.0 || b == 0.0)
        return 0.0;
    return a * b;
}
vertex VertexOut main0(uint vid [[vertex_id]], uint iid [[instance_id]],
                       VertexIn in [[stage_in]],
                       constant SupportBuffer &supportBuffer [[buffer(0)]]) {
    VertexOut out;
    float4 R0f = float4(0.0);
    float4 R1f = float4(0.0);
    float4 R2f = float4(0.0);
    uint4 attrDecoder;
    float backupReg0f, backupReg1f, backupReg2f, backupReg3f, backupReg4f;
    float PV0fx = 0.0, PV0fy = 0.0, PV0fz = 0.0, PV0fw = 0.0, PV1fx = 0.0,
          PV1fy = 0.0, PV1fz = 0.0, PV1fw = 0.0;
    float PS0f = 0.0, PS1f = 0.0;
    float4 tempf = float4(0.0);
    float tempResultf;
    int tempResulti;
    int4 ARi = int4(0);
    bool predResult = true;
    R0f = float4(vid, 0, 0, iid);
    attrDecoder.xyz = in.attrDataSem0.xyz;
    attrDecoder.xyz =
        (attrDecoder.xyz >> 24) | ((attrDecoder.xyz >> 8) & 0xFF00) |
        ((attrDecoder.xyz << 8) & 0xFF0000) | ((attrDecoder.xyz << 24));
    attrDecoder.w = 0;
    R1f = float4(
        as_type<float>(int(attrDecoder.x)), as_type<float>(int(attrDecoder.y)),
        as_type<float>(int(attrDecoder.z)), as_type<float>(0x3f800000));
    attrDecoder.xy = in.attrDataSem1.xy;
    attrDecoder.xy = (attrDecoder.xy >> 24) | ((attrDecoder.xy >> 8) & 0xFF00) |
                     ((attrDecoder.xy << 8) & 0xFF0000) |
                     ((attrDecoder.xy << 24));
    attrDecoder.z = 0;
    attrDecoder.w = 0;
    R2f = float4(as_type<float>(int(attrDecoder.x)),
                 as_type<float>(int(attrDecoder.y)), as_type<float>(0),
                 as_type<float>(0x3f800000));
    // 0
    R1f.x = R1f.x;
    R1f.x *= 2.0;
    R1f.y = R1f.y;
    R1f.y *= 2.0;
    R1f.z = 0.0;
    R1f.w = 1.0;
    PS0f = as_type<float>(supportBuffer.remapped[0].x) / resXScale *
           as_type<float>(0x3fb13a93);
    // 1
    PV1fx = as_type<float>(supportBuffer.remapped[0].x) / resXScale *
            as_type<float>(0x404ec4f0);
    R0f.y = R2f.x + -(PS0f);
    R0f.z = R2f.x + PS0f;
    R0f.w = R2f.x;
    R0f.x = R2f.y;
    // 2
    backupReg0f = R2f.x;
    R2f.x = R0f.x;
    R2f.y = backupReg0f + -(PV1fx);
    R2f.z = backupReg0f + PV1fx;
    R2f.w = backupReg0f;
    // export
    SET_POSITION(float4(R1f.x, R1f.y, R1f.z, R1f.w));
    // export
    out.passParameterSem1 = float4(R2f.x, R2f.y, R2f.z, R2f.w);
    // export
    out.passParameterSem0 = float4(R0f.x, R0f.y, R0f.z, R0f.w);
    return out;
}
)BOTW_MSL_END"},
{0x9ee9348c601d0fa7ULL, 0x0000000000000000ULL, GraphicPack2::GP_SHADER_TYPE::VERTEX, R"BOTW_MSL_END(#include <metal_stdlib>
using namespace metal;

// shader 9ee9348c601d0fa7
// Shader dumped from BotW 208, using Cemu 2193a8c
// Based on f69e84515ae56e70_0000000000000000_vs.txt or
// 771e24915acbb074_0000000000000000_vs.txt Used for: Vertical Bloom or
// Reflection Blur

constant float resYScale = float($height) / float($gameHeight);

#define SET_POSITION(_v)                                                       \
    out.position = _v;                                                         \
    out.position.z = (out.position.z + out.position.w) / 2.0
// start of shader inputs/outputs, predetermined by Cemu. Do not touch
struct SupportBuffer {
    int4 remapped[1];
};

struct VertexIn {
    uint4 attrDataSem0 [[attribute(0)]];
    uint4 attrDataSem1 [[attribute(1)]];
};

struct VertexOut {
    float4 position [[position]] [[invariant]];
    float4 passParameterSem0 [[user(locn0)]];
    float4 passParameterSem1 [[user(locn1)]];
};

// end of shader inputs/outputs
template <typename TextureT, typename CoordT>
float sampleCompareEmulate(TextureT tex, sampler samplr, CoordT coord,
                           float compareValue) {
    return compareValue < tex.sample(samplr, coord).x ? 1.0 : 0.0;
}
template <typename TextureT, typename CoordT>
float2 textureCalculateLod(TextureT tex, sampler samplr, CoordT coord) {
    float lod = tex.calculate_unclamped_lod(samplr, coord);
    return float2(floor(lod), fract(lod));
}
int clampFI32(int v) {
    if (v == 0x7FFFFFFF)
        return as_type<int>(1.0);
    else if (v == 0xFFFFFFFF)
        return as_type<int>(0.0);
    return as_type<int>(clamp(as_type<float>(v), 0.0, 1.0));
}
float mul_nonIEEE(float a, float b) {
    if (a == 0.0 || b == 0.0)
        return 0.0;
    return a * b;
}
vertex VertexOut main0(uint vid [[vertex_id]], uint iid [[instance_id]],
                       VertexIn in [[stage_in]],
                       constant SupportBuffer &supportBuffer [[buffer(0)]]) {
    VertexOut out;
    float4 R0f = float4(0.0);
    float4 R1f = float4(0.0);
    float4 R2f = float4(0.0);
    float4 R127f = float4(0.0);
    uint4 attrDecoder;
    float backupReg0f, backupReg1f, backupReg2f, backupReg3f, backupReg4f;
    float PV0fx = 0.0, PV0fy = 0.0, PV0fz = 0.0, PV0fw = 0.0, PV1fx = 0.0,
          PV1fy = 0.0, PV1fz = 0.0, PV1fw = 0.0;
    float PS0f = 0.0, PS1f = 0.0;
    float4 tempf = float4(0.0);
    float tempResultf;
    int tempResulti;
    int4 ARi = int4(0);
    bool predResult = true;
    R0f = float4(vid, 0, 0, iid);
    attrDecoder.xyz = in.attrDataSem0.xyz;
    attrDecoder.xyz =
        (attrDecoder.xyz >> 24) | ((attrDecoder.xyz >> 8) & 0xFF00) |
        ((attrDecoder.xyz << 8) & 0xFF0000) | ((attrDecoder.xyz << 24));
    attrDecoder.w = 0;
    R1f = float4(
        as_type<float>(int(attrDecoder.x)), as_type<float>(int(attrDecoder.y)),
        as_type<float>(int(attrDecoder.z)), as_type<float>(0x3f800000));
    attrDecoder.xy = in.attrDataSem1.xy;
    attrDecoder.xy = (attrDecoder.xy >> 24) | ((attrDecoder.xy >> 8) & 0xFF00) |
                     ((attrDecoder.xy << 8) & 0xFF0000) |
                     ((attrDecoder.xy << 24));
    attrDecoder.z = 0;
    attrDecoder.w = 0;
    R2f = float4(as_type<float>(int(attrDecoder.x)),
                 as_type<float>(int(attrDecoder.y)), as_type<float>(0),
                 as_type<float>(0x3f800000));
    // 0
    R1f.x = R1f.x;
    R1f.x *= 2.0;
    R1f.y = R1f.y;
    R1f.y *= 2.0;
    R1f.z = 0.0;
    R1f.w = 1.0;
    PS0f = as_type<float>(supportBuffer.remapped[0].y) / resYScale *
           as_type<float>(0x3fb13a93);
    // 1
    R127f.x = as_type<float>(supportBuffer.remapped[0].y) / resYScale *
              as_type<float>(0x404ec4f0);
    R0f.y = R2f.y + -(PS0f);
    R2f.z = R2f.y + PS0f;
    R2f.w = R2f.y;
    R0f.x = R2f.x;
    // 2
    R0f.z = R2f.z;
    R0f.w = R2f.w;
    R2f.z = R2f.y + R127f.x;
    // 3
    backupReg0f = R2f.y;
    R2f.y = backupReg0f + -(R127f.x);
    R2f.w = backupReg0f;
    // export
    SET_POSITION(float4(R1f.x, R1f.y, R1f.z, R1f.w));
    // export
    out.passParameterSem0 = float4(R0f.x, R0f.y, R0f.z, R0f.w);
    // export
    out.passParameterSem1 = float4(R2f.x, R2f.y, R2f.z, R2f.w);
    return out;
}
)BOTW_MSL_END"},
{0xabe0a6676dc1640cULL, 0x0000000000000000ULL, GraphicPack2::GP_SHADER_TYPE::VERTEX, R"BOTW_MSL_END(#include <metal_stdlib>
using namespace metal;

// shader abe0a6676dc1640c
// Shader dumped from BotW 208, using Cemu 2193a8c
// Based on 75a85b0cbcab764b_0000000000000000_vs.txt
// Used for: Vertical Volumetric Light Blur
// volumetric light blur v

constant float resYScale = float($height) / float($gameHeight);

#define SET_POSITION(_v)                                                       \
    out.position = _v;                                                         \
    out.position.z = (out.position.z + out.position.w) / 2.0
// start of shader inputs/outputs, predetermined by Cemu. Do not touch
struct SupportBuffer {
    int4 remapped[5];
};

struct VertexIn {
    uint4 attrDataSem0 [[attribute(0)]];
};

struct VertexOut {
    float4 position [[position]] [[invariant]];
    float4 passParameterSem0 [[user(locn0)]];
};

// end of shader inputs/outputs
template <typename TextureT, typename CoordT>
float sampleCompareEmulate(TextureT tex, sampler samplr, CoordT coord,
                           float compareValue) {
    return compareValue < tex.sample(samplr, coord).x ? 1.0 : 0.0;
}
template <typename TextureT, typename CoordT>
float2 textureCalculateLod(TextureT tex, sampler samplr, CoordT coord) {
    float lod = tex.calculate_unclamped_lod(samplr, coord);
    return float2(floor(lod), fract(lod));
}
int clampFI32(int v) {
    if (v == 0x7FFFFFFF)
        return as_type<int>(1.0);
    else if (v == 0xFFFFFFFF)
        return as_type<int>(0.0);
    return as_type<int>(clamp(as_type<float>(v), 0.0, 1.0));
}
float mul_nonIEEE(float a, float b) {
    if (a == 0.0 || b == 0.0)
        return 0.0;
    return a * b;
}
vertex VertexOut main0(uint vid [[vertex_id]], uint iid [[instance_id]],
                       VertexIn in [[stage_in]],
                       constant SupportBuffer &supportBuffer [[buffer(0)]]) {
    VertexOut out;
    float4 R0f = float4(0.0);
    float4 R1f = float4(0.0);
    float4 R126f = float4(0.0);
    float4 R127f = float4(0.0);
    uint4 attrDecoder;
    float backupReg0f, backupReg1f, backupReg2f, backupReg3f, backupReg4f;
    float PV0fx = 0.0, PV0fy = 0.0, PV0fz = 0.0, PV0fw = 0.0, PV1fx = 0.0,
          PV1fy = 0.0, PV1fz = 0.0, PV1fw = 0.0;
    float PS0f = 0.0, PS1f = 0.0;
    float4 tempf = float4(0.0);
    float tempResultf;
    int tempResulti;
    int4 ARi = int4(0);
    bool predResult = true;
    R0f = float4(vid, 0, 0, iid);
    attrDecoder.xyz = in.attrDataSem0.xyz;
    attrDecoder.xyz =
        (attrDecoder.xyz >> 24) | ((attrDecoder.xyz >> 8) & 0xFF00) |
        ((attrDecoder.xyz << 8) & 0xFF0000) | ((attrDecoder.xyz << 24));
    attrDecoder.w = 0;
    R1f = float4(
        as_type<float>(int(attrDecoder.x)), as_type<float>(int(attrDecoder.y)),
        as_type<float>(int(attrDecoder.z)), as_type<float>(0x3f800000));
    // 0
    R127f.x = -(R1f.y);
    R126f.y = (0.0 > R1f.x) ? 1.0 : 0.0;
    R126f.y /= 2.0;
    R127f.z = (R1f.x > 0.0) ? 1.0 : 0.0;
    R127f.z /= 2.0;
    R127f.w = 1.0;
    // 1
    R0f.x = dot(float4(R1f.x, R1f.y, R1f.z, R127f.w),
                float4(as_type<float>(supportBuffer.remapped[0].x),
                       as_type<float>(supportBuffer.remapped[0].y),
                       as_type<float>(supportBuffer.remapped[0].z),
                       as_type<float>(supportBuffer.remapped[0].w)));
    PV1fy = R0f.x;
    PV1fz = R0f.x;
    PV1fw = R0f.x;
    R127f.y = (R127f.x > 0.0) ? 1.0 : 0.0;
    R127f.y /= 2.0;
    // 2
    tempf.x = dot(float4(R1f.x, R1f.y, R1f.z, R127f.w),
                  float4(as_type<float>(supportBuffer.remapped[1].x),
                         as_type<float>(supportBuffer.remapped[1].y),
                         as_type<float>(supportBuffer.remapped[1].z),
                         as_type<float>(supportBuffer.remapped[1].w)));
    PV0fx = tempf.x;
    R0f.y = tempf.x;
    PV0fz = tempf.x;
    PV0fw = tempf.x;
    PS0f = (0.0 > R127f.x) ? 1.0 : 0.0;
    PS0f /= 2.0;
    // 3
    tempf.x = dot(float4(R1f.x, R1f.y, R1f.z, R127f.w),
                  float4(as_type<float>(supportBuffer.remapped[2].x),
                         as_type<float>(supportBuffer.remapped[2].y),
                         as_type<float>(supportBuffer.remapped[2].z),
                         as_type<float>(supportBuffer.remapped[2].w)));
    PV1fx = tempf.x;
    PV1fy = tempf.x;
    R0f.z = tempf.x;
    PV1fw = tempf.x;
    R127f.y = R127f.y + -(PS0f);
    // 4
    tempf.x = dot(float4(R1f.x, R1f.y, R1f.z, R127f.w),
                  float4(as_type<float>(supportBuffer.remapped[3].x),
                         as_type<float>(supportBuffer.remapped[3].y),
                         as_type<float>(supportBuffer.remapped[3].z),
                         as_type<float>(supportBuffer.remapped[3].w)));
    PV0fx = tempf.x;
    PV0fy = tempf.x;
    PV0fz = tempf.x;
    R0f.w = tempf.x;
    PS0f = R127f.z + -(R126f.y);
    // 5
    PV1fx = R127f.y + 0.5;
    R1f.x = PS0f + 0.5;
    // 6
    R1f.y = -(as_type<float>(supportBuffer.remapped[4].w)) * 0.5 / resYScale +
            PV1fx;
    R1f.z =
        as_type<float>(supportBuffer.remapped[4].w) * 0.5 / resYScale + PV1fx;
    R1f.w = PV1fx;
    // export
    SET_POSITION(float4(R0f.x, R0f.y, R0f.z, R0f.w));
    // export
    out.passParameterSem0 = float4(R1f.x, R1f.y, R1f.z, R1f.w);
    return out;
}
)BOTW_MSL_END"},
{0xb4a729584b6188eaULL, 0xff060c54c2a0897bULL, GraphicPack2::GP_SHADER_TYPE::PIXEL, R"BOTW_MSL_END(#include <metal_stdlib>
using namespace metal;

// shader b4a729584b6188ea
// Shader dumped from BotW 208, using Cemu 2193a8c
// Based on b4a729584b6188ea_0000000000001e49_ps.txt
// Used for: Restoring the native BotW Anti-Aliasing implementation for distant
// trees

constant float resXScale = float($width) / float($gameWidth);
constant float resYScale = float($height) / float($gameHeight);
#define fxaa $fxaa

#define SET_POSITION(_v)                                                       \
    out.position = _v;                                                         \
    out.position.z = (out.position.z + out.position.w) / 2.0
// start of shader inputs/outputs, predetermined by Cemu. Do not touch
struct SupportBuffer {
    int4 remapped[3];
    float2 fragCoordScale;
};

#define GET_FRAGCOORD()                                                        \
    float4(in.position.xy *supportBuffer.fragCoordScale.xy, in.position.z,     \
           1.0 / in.position.w)
struct FragmentIn {
    float4 position [[position]];
    float4 passParameterSem0 [[user(locn0)]];
};

struct FragmentOut {
    float4 passPixelColor0 [[color(0)]];
};

// end of shader inputs/outputs
template <typename TextureT, typename CoordT>
float sampleCompareEmulate(TextureT tex, sampler samplr, CoordT coord,
                           float compareValue) {
    return compareValue < tex.sample(samplr, coord).x ? 1.0 : 0.0;
}
template <typename TextureT, typename CoordT>
float2 textureCalculateLod(TextureT tex, sampler samplr, CoordT coord) {
    float lod = tex.calculate_unclamped_lod(samplr, coord);
    return float2(floor(lod), fract(lod));
}
int clampFI32(int v) {
    if (v == 0x7FFFFFFF)
        return as_type<int>(1.0);
    else if (v == 0xFFFFFFFF)
        return as_type<int>(0.0);
    return as_type<int>(clamp(as_type<float>(v), 0.0, 1.0));
}
float mul_nonIEEE(float a, float b) {
    if (a == 0.0 || b == 0.0)
        return 0.0;
    return a * b;
}
fragment FragmentOut
main0(FragmentIn in [[stage_in]], float2 pointCoord [[point_coord]],
      bool frontFacing [[front_facing]],
      constant SupportBuffer &supportBuffer [[buffer(0)]],
      texture2d<float> tex0 [[texture(0)]], sampler samplr0 [[sampler(0)]],
      texture2d<float> tex1 [[texture(1)]], sampler samplr1 [[sampler(1)]],
      texture2d<float> tex2 [[texture(2)]], sampler samplr2 [[sampler(2)]]) {
    FragmentOut out;
#if fxaa == 0
    out.passPixelColor0 = tex0.sample(samplr0, in.passParameterSem0.xy);
#else
    float4 R0f = float4(0.0);
    float4 R1f = float4(0.0);
    float4 R2f = float4(0.0);
    float4 R123f = float4(0.0);
    float4 R127f = float4(0.0);
    float backupReg0f, backupReg1f, backupReg2f, backupReg3f, backupReg4f;
    float PV0fx = 0.0, PV0fy = 0.0, PV0fz = 0.0, PV0fw = 0.0, PV1fx = 0.0,
          PV1fy = 0.0, PV1fz = 0.0, PV1fw = 0.0;
    float PS0f = 0.0, PS1f = 0.0;
    float4 tempf = float4(0.0);
    float tempResultf;
    int tempResulti;
    int4 ARi = int4(0);
    bool predResult = true;
    R0f = in.passParameterSem0;
    R2f.w = (tex1.sample(samplr1, float2(R0f.x, R0f.y)).x);
    R1f.xyzw =
        (tex2.gather(samplr2, float2(0.0001) + float2(R0f.x, R0f.y)).xyzw);
    // 0
    PV0fx = -(R2f.w) + R1f.y;
    PV0fy = -(R2f.w) + R1f.x;
    PV0fz = -(R2f.w) + R1f.w;
    PV0fw = -(R2f.w) + R1f.z;
    PS0f = R2f.w + as_type<float>(supportBuffer.remapped[0].x);
    // 1
    PV1fx = max(PV0fx, -(PV0fx));
    PV1fy = max(PV0fy, -(PV0fy));
    PV1fz = max(PV0fz, -(PV0fz));
    PV1fw = max(PV0fw, -(PV0fw));
    PS1f = 1.0 / PS0f;
    // 2
    PV0fx = PV1fy + -(PV1fw);
    PV0fy = -(PV1fx) + PV1fz;
    R127f.z = as_type<float>(supportBuffer.remapped[1].w) * PS1f;
    R127f.w = fract(R0f.z);
    R127f.y = fract(R0f.w);
    // 3
    PV1fz = PV0fy + -(PV0fx);
    PV1fw = PV0fy + PV0fx;
    // 4
    R123f.x = mul_nonIEEE(R127f.z, PV1fz) + R127f.y;
    R123f.x = clamp(R123f.x, 0.0, 1.0);
    R123f.y = mul_nonIEEE(R127f.z, PV1fw) + R127f.w;
    R123f.y = clamp(R123f.y, 0.0, 1.0);
    // 5
    PV1fz = -(R127f.y) + R123f.x;
    PV1fw = -(R127f.w) + R123f.y;
    // 6
    R0f.x = mul_nonIEEE(PV1fw, as_type<float>(supportBuffer.remapped[2].z) /
                                   resXScale) +
            R0f.x;
    R0f.y = mul_nonIEEE(PV1fz, as_type<float>(supportBuffer.remapped[2].w) /
                                   resYScale) +
            R0f.y;
    R0f.xyzw = (tex0.sample(samplr0, float2(R0f.x, R0f.y)).xyzw);
    // export
    out.passPixelColor0 = as_type<float4>(float4(R0f.x, R0f.y, R0f.z, R0f.w));
#endif
    return out;
}
)BOTW_MSL_END"},
{0xbbbc7becd5d4f26eULL, 0x0000000000000000ULL, GraphicPack2::GP_SHADER_TYPE::VERTEX, R"BOTW_MSL_END(#include <metal_stdlib>
using namespace metal;

// shader bbbc7becd5d4f26e
// Shader dumped from BotW 208, using Cemu 2193a8c
// Based on 5c1761d13feccdff_0000000000000000_vs.txt
// Used for: Vertical+Horizontal Fullscreen Blur

constant float resXScale = float($width) / float($gameWidth);

#define SET_POSITION(_v)                                                       \
    out.position = _v;                                                         \
    out.position.z = (out.position.z + out.position.w) / 2.0
// start of shader inputs/outputs, predetermined by Cemu. Do not touch
struct SupportBuffer {
    int4 remapped[1];
};

struct VertexIn {
    uint4 attrDataSem0 [[attribute(0)]];
    uint4 attrDataSem1 [[attribute(1)]];
};

struct VertexOut {
    float4 position [[position]] [[invariant]];
    float4 passParameterSem1 [[user(locn1)]];
    float4 passParameterSem0 [[user(locn0)]];
};

// end of shader inputs/outputs
template <typename TextureT, typename CoordT>
float sampleCompareEmulate(TextureT tex, sampler samplr, CoordT coord,
                           float compareValue) {
    return compareValue < tex.sample(samplr, coord).x ? 1.0 : 0.0;
}
template <typename TextureT, typename CoordT>
float2 textureCalculateLod(TextureT tex, sampler samplr, CoordT coord) {
    float lod = tex.calculate_unclamped_lod(samplr, coord);
    return float2(floor(lod), fract(lod));
}
int clampFI32(int v) {
    if (v == 0x7FFFFFFF)
        return as_type<int>(1.0);
    else if (v == 0xFFFFFFFF)
        return as_type<int>(0.0);
    return as_type<int>(clamp(as_type<float>(v), 0.0, 1.0));
}
float mul_nonIEEE(float a, float b) {
    if (a == 0.0 || b == 0.0)
        return 0.0;
    return a * b;
}
vertex VertexOut main0(uint vid [[vertex_id]], uint iid [[instance_id]],
                       VertexIn in [[stage_in]],
                       constant SupportBuffer &supportBuffer [[buffer(0)]]) {
    VertexOut out;
    float4 R0f = float4(0.0);
    float4 R1f = float4(0.0);
    float4 R2f = float4(0.0);
    float4 R3f = float4(0.0);
    float4 R4f = float4(0.0);
    float4 R127f = float4(0.0);
    uint4 attrDecoder;
    float backupReg0f, backupReg1f, backupReg2f, backupReg3f, backupReg4f;
    float PV0fx = 0.0, PV0fy = 0.0, PV0fz = 0.0, PV0fw = 0.0, PV1fx = 0.0,
          PV1fy = 0.0, PV1fz = 0.0, PV1fw = 0.0;
    float PS0f = 0.0, PS1f = 0.0;
    float4 tempf = float4(0.0);
    float tempResultf;
    int tempResulti;
    int4 ARi = int4(0);
    bool predResult = true;
    R0f = float4(vid, 0, 0, iid);
    attrDecoder.xyz = in.attrDataSem0.xyz;
    attrDecoder.xyz =
        (attrDecoder.xyz >> 24) | ((attrDecoder.xyz >> 8) & 0xFF00) |
        ((attrDecoder.xyz << 8) & 0xFF0000) | ((attrDecoder.xyz << 24));
    attrDecoder.w = 0;
    R1f = float4(
        as_type<float>(int(attrDecoder.x)), as_type<float>(int(attrDecoder.y)),
        as_type<float>(int(attrDecoder.z)), as_type<float>(0x3f800000));
    attrDecoder.xy = in.attrDataSem1.xy;
    attrDecoder.xy = (attrDecoder.xy >> 24) | ((attrDecoder.xy >> 8) & 0xFF00) |
                     ((attrDecoder.xy << 8) & 0xFF0000) |
                     ((attrDecoder.xy << 24));
    attrDecoder.z = 0;
    attrDecoder.w = 0;
    R2f = float4(as_type<float>(int(attrDecoder.x)),
                 as_type<float>(int(attrDecoder.y)), as_type<float>(0),
                 as_type<float>(0x3f800000));
    // 0
    R1f.x = R1f.x;
    R1f.x *= 2.0;
    R1f.y = R1f.y;
    R1f.y *= 2.0;
    R1f.z = as_type<float>(0xbf5fdfe0);
    R1f.w = 1.0;
    R127f.y = as_type<float>(supportBuffer.remapped[0].x) *
              as_type<float>(0x3fb13a93) / resXScale;
    // 1
    R127f.x = as_type<float>(supportBuffer.remapped[0].x) *
              as_type<float>(0x404ec4f0) / resXScale;
    R4f.y = R2f.y;
    R4f.z = R2f.x;
    R3f.w = R2f.x;
    R3f.x = R2f.y;
    // 2
    R0f.x = R3f.x;
    R3f.y = R2f.x + -(R127f.y);
    R3f.z = R2f.x + R127f.y;
    R0f.w = R2f.x;
    R0f.y = R2f.x + -(R127f.x);
    // 3
    R0f.z = R2f.x + R127f.x;
    // export
    SET_POSITION(float4(R1f.x, R1f.y, R1f.z, R1f.w));
    // export
    out.passParameterSem1 = float4(R0f.x, R0f.y, R0f.z, R0f.w);
    // export
    out.passParameterSem0 = float4(R3f.x, R3f.y, R3f.z, R3f.w);
    // export
    // skipped export to semanticId 255
    return out;
}
)BOTW_MSL_END"},
{0xc92c1c4c0a2fb839ULL, 0xff060c54c2a0897bULL, GraphicPack2::GP_SHADER_TYPE::PIXEL, R"BOTW_MSL_END(#include <metal_stdlib>
using namespace metal;

// shader c92c1c4c0a2fb839
// Shader dumped from BotW 208, using Cemu 2193a8c
// Based on c92c1c4c0a2fb839_0000000000001e49_ps.txt
// Used for: Camera Depth of Field Blur

constant float resXScale = float($width) / float($gameWidth);
constant float resYScale = float($height) / float($gameHeight);
#define SET_POSITION(_v)                                                       \
    out.position = _v;                                                         \
    out.position.z = (out.position.z + out.position.w) / 2.0
// start of shader inputs/outputs, predetermined by Cemu. Do not touch
struct SupportBuffer {
    int4 remapped[5];
    float2 fragCoordScale;
};

#define GET_FRAGCOORD()                                                        \
    float4(in.position.xy *supportBuffer.fragCoordScale.xy, in.position.z,     \
           1.0 / in.position.w)
struct FragmentIn {
    float4 position [[position]];
    float4 passParameterSem2 [[user(locn0)]];
};

struct FragmentOut {
    float4 passPixelColor0 [[color(0)]];
};

// end of shader inputs/outputs
template <typename TextureT, typename CoordT>
float sampleCompareEmulate(TextureT tex, sampler samplr, CoordT coord,
                           float compareValue) {
    return compareValue < tex.sample(samplr, coord).x ? 1.0 : 0.0;
}
template <typename TextureT, typename CoordT>
float2 textureCalculateLod(TextureT tex, sampler samplr, CoordT coord) {
    float lod = tex.calculate_unclamped_lod(samplr, coord);
    return float2(floor(lod), fract(lod));
}
int clampFI32(int v) {
    if (v == 0x7FFFFFFF)
        return as_type<int>(1.0);
    else if (v == 0xFFFFFFFF)
        return as_type<int>(0.0);
    return as_type<int>(clamp(as_type<float>(v), 0.0, 1.0));
}
float mul_nonIEEE(float a, float b) {
    if (a == 0.0 || b == 0.0)
        return 0.0;
    return a * b;
}
fragment FragmentOut
main0(FragmentIn in [[stage_in]], float2 pointCoord [[point_coord]],
      bool frontFacing [[front_facing]],
      constant SupportBuffer &supportBuffer [[buffer(0)]],
      texture2d<float> tex0 [[texture(0)]], sampler samplr0 [[sampler(0)]],
      texture2d<float> tex1 [[texture(1)]], sampler samplr1 [[sampler(1)]],
      texture2d<float> tex2 [[texture(2)]], sampler samplr2 [[sampler(2)]]) {
    FragmentOut out;
    int4 R0i = int4(0);
    int4 R1i = int4(0);
    int4 R2i = int4(0);
    int4 R3i = int4(0);
    int4 R4i = int4(0);
    int4 R123i = int4(0);
    int4 R127i = int4(0);
    int backupReg0i, backupReg1i, backupReg2i, backupReg3i, backupReg4i;
    int PV0ix = 0, PV0iy = 0, PV0iz = 0, PV0iw = 0, PV1ix = 0, PV1iy = 0,
        PV1iz = 0, PV1iw = 0;
    int PS0i = 0, PS1i = 0;
    int4 tempi = int4(0);
    float tempResultf;
    int tempResulti;
    int4 ARi = int4(0);
    bool predResult = true;
    bool activeMaskStack[2];
    bool activeMaskStackC[3];
    activeMaskStack[0] = false;
    activeMaskStackC[0] = false;
    activeMaskStackC[1] = false;
    activeMaskStack[0] = true;
    activeMaskStackC[0] = true;
    activeMaskStackC[1] = true;
    R0i = as_type<int4>(in.passParameterSem2);
    if (activeMaskStackC[1] == true) {
        R1i.w = as_type<int>(tex0.sample(samplr0, float2(as_type<float>(R0i.x),
                                                         as_type<float>(R0i.y)))
                                 .x);
        R1i.z = as_type<int>(tex2.sample(samplr2, float2(as_type<float>(R0i.x),
                                                         as_type<float>(R0i.y)))
                                 .x);
    }
    if (activeMaskStackC[1] == true) {
        // 0
        R123i.x = as_type<int>(
            mul_nonIEEE(as_type<float>(R1i.w),
                        as_type<float>(supportBuffer.remapped[0].x)) +
            as_type<float>(supportBuffer.remapped[1].x));
        R123i.x = clampFI32(R123i.x);
        // 1
        R0i.z =
            as_type<int>(max(as_type<float>(R1i.z), as_type<float>(R123i.x)));
        // 2
        R1i.y = ((as_type<float>(R0i.z) != 0.0) ? -1 : 0);
        // 3
        if ((R1i.y == 0))
            discard_fragment();
    }
    if (activeMaskStackC[1] == true) {
        activeMaskStack[1] = activeMaskStack[0];
        activeMaskStackC[2] = activeMaskStackC[1];
        // 0
        predResult = (R1i.y != 0);
        activeMaskStack[1] = predResult;
        activeMaskStackC[2] = predResult == true && activeMaskStackC[1] == true;
    } else {
        activeMaskStack[1] = false;
        activeMaskStackC[2] = false;
    }
    if (activeMaskStackC[2] == true) {
        // 0
        PV0iw =
            as_type<int>(as_type<float>(supportBuffer.remapped[2].y) + -(1.0));
        // 1
        R123i.z = as_type<int>(
            mul_nonIEEE(as_type<float>(PV0iw), as_type<float>(R0i.z)) + 1.0);
        // 2
        tempResultf = max(0.0, as_type<float>(R123i.z));
        tempResultf = log2(tempResultf);
        PS0i = as_type<int>(tempResultf);
        // 3
        R127i.x = as_type<int>(as_type<float>(PS0i) + -(1.0));
        // 4
        R2i.z = R127i.x;
        R1i.w = R127i.x;
        PS0i = as_type<int>(exp2(as_type<float>(R127i.x)));
        // 5
        PV1iz = as_type<int>(as_type<float>(PS0i) +
                             as_type<float>(supportBuffer.remapped[3].w));
        R3i.w = R127i.x;
        R4i.w = R127i.x;
        // 6
        backupReg0i = R127i.x;
        R127i.x = as_type<int>(
            mul_nonIEEE(as_type<float>(PV1iz),
                        as_type<float>(supportBuffer.remapped[4].y)));
        PV0iy = as_type<int>(mul_nonIEEE(
            as_type<float>(PV1iz),
            as_type<float>(supportBuffer.remapped[4].x) / resXScale));
        R127i.z = as_type<int>(mul_nonIEEE(
            as_type<float>(PV1iz),
            as_type<float>(supportBuffer.remapped[4].w) / resYScale));
        R127i.w = as_type<int>(
            mul_nonIEEE(as_type<float>(PV1iz),
                        as_type<float>(supportBuffer.remapped[4].z)));
        R2i.w = as_type<int>(as_type<float>(backupReg0i) + 1.0);
        R2i.w = clampFI32(R2i.w);
        // 7
        R1i.x = as_type<int>(as_type<float>(R0i.x) + as_type<float>(PV0iy));
        R1i.y = as_type<int>(as_type<float>(R0i.y) + as_type<float>(R127i.x));
        R3i.z = as_type<int>(as_type<float>(R0i.x) + as_type<float>(R127i.w));
        R2i.x = as_type<int>(as_type<float>(R0i.x) + -(as_type<float>(PV0iy)));
        // 8
        R3i.x = as_type<int>(as_type<float>(R0i.y) + as_type<float>(R127i.z));
        R2i.y =
            as_type<int>(as_type<float>(R0i.y) + -(as_type<float>(R127i.x)));
        R4i.z =
            as_type<int>(as_type<float>(R0i.x) + -(as_type<float>(R127i.w)));
        R4i.y =
            as_type<int>(as_type<float>(R0i.y) + -(as_type<float>(R127i.z)));
    }
    if (activeMaskStackC[2] == true) {
        R1i.xyz = as_type<int3>(
            tex1.sample(samplr1,
                        float2(as_type<float>(R1i.x), as_type<float>(R1i.y)),
                        level(as_type<float>(R1i.w)))
                .xyz);
        R2i.xyz = as_type<int3>(
            tex1.sample(samplr1,
                        float2(as_type<float>(R2i.x), as_type<float>(R2i.y)),
                        level(as_type<float>(R2i.z)))
                .xyz);
        R3i.xyz = as_type<int3>(
            tex1.sample(samplr1,
                        float2(as_type<float>(R3i.z), as_type<float>(R3i.x)),
                        level(as_type<float>(R3i.w)))
                .xyz);
        R4i.xyz = as_type<int3>(
            tex1.sample(samplr1,
                        float2(as_type<float>(R4i.z), as_type<float>(R4i.y)),
                        level(as_type<float>(R4i.w)))
                .xyz);
    }
    if (activeMaskStackC[2] == true) {
        // 0
        PV0iy = as_type<int>(as_type<float>(R1i.z) + as_type<float>(R2i.z));
        PV0iy = as_type<int>(as_type<float>(PV0iy) / 2.0);
        PV0iz = as_type<int>(as_type<float>(R1i.y) + as_type<float>(R2i.y));
        PV0iz = as_type<int>(as_type<float>(PV0iz) / 2.0);
        PV0iw = as_type<int>(as_type<float>(R1i.x) + as_type<float>(R2i.x));
        PV0iw = as_type<int>(as_type<float>(PV0iw) / 2.0);
        // 1
        R123i.x =
            as_type<int>(as_type<float>(R3i.x) * 0.5 + as_type<float>(PV0iw));
        R123i.z =
            as_type<int>(as_type<float>(R3i.z) * 0.5 + as_type<float>(PV0iy));
        R123i.w =
            as_type<int>(as_type<float>(R3i.y) * 0.5 + as_type<float>(PV0iz));
        // 2
        R2i.x = as_type<int>(
            (as_type<float>(R4i.x) * 0.5 + as_type<float>(R123i.x)) / 2.0);
        R2i.y = as_type<int>(
            (as_type<float>(R4i.y) * 0.5 + as_type<float>(R123i.w)) / 2.0);
        R2i.z = as_type<int>(
            (as_type<float>(R4i.z) * 0.5 + as_type<float>(R123i.z)) / 2.0);
    }
    activeMaskStackC[1] =
        activeMaskStack[0] == true && activeMaskStackC[0] == true;
    // export
    out.passPixelColor0 =
        as_type<float4>(float4(as_type<float>(R2i.x), as_type<float>(R2i.y),
                               as_type<float>(R2i.z), as_type<float>(R2i.w)));
    return out;
}
)BOTW_MSL_END"},
{0xcb0e6e8cbec4502aULL, 0xff060c183060c54bULL, GraphicPack2::GP_SHADER_TYPE::PIXEL, R"BOTW_MSL_END(// shader cb0e6e8cbec4502a
#include <metal_stdlib>
using namespace metal;
#define SET_POSITION(_v)                                                       \
    out.position = _v;                                                         \
    out.position.z = (out.position.z + out.position.w) / 2.0
// start of shader inputs/outputs, predetermined by Cemu. Do not touch
struct SupportBuffer {
    float2 fragCoordScale;
};

#define GET_FRAGCOORD()                                                        \
    float4(in.position.xy *supportBuffer.fragCoordScale.xy, in.position.z,     \
           1.0 / in.position.w)
struct FragmentIn {
    float4 position [[position]];
    float4 passParameterSem3 [[user(locn0)]];
};

struct FragmentOut {
    float4 passPixelColor0 [[color(0)]];
};

// end of shader inputs/outputs
template <typename TextureT, typename CoordT>
float sampleCompareEmulate(TextureT tex, sampler samplr, CoordT coord,
                           float compareValue) {
    return compareValue < tex.sample(samplr, coord).x ? 1.0 : 0.0;
}
template <typename TextureT, typename CoordT>
float2 textureCalculateLod(TextureT tex, sampler samplr, CoordT coord) {
    float lod = tex.calculate_unclamped_lod(samplr, coord);
    return float2(floor(lod), fract(lod));
}
int clampFI32(int v) {
    if (v == 0x7FFFFFFF)
        return as_type<int>(1.0);
    else if (v == 0xFFFFFFFF)
        return as_type<int>(0.0);
    return as_type<int>(clamp(as_type<float>(v), 0.0, 1.0));
}
float mul_nonIEEE(float a, float b) {
    if (a == 0.0 || b == 0.0)
        return 0.0;
    return a * b;
}

#define disableDOF $disableDOF

#if (disableDOF == 0) // Keep DOF Enabled

fragment FragmentOut main0(FragmentIn in [[stage_in]],
                           float2 pointCoord [[point_coord]],
                           bool frontFacing [[front_facing]],
                           constant SupportBuffer &supportBuffer [[buffer(0)]],
                           texture2d<float> tex0 [[texture(0)]],
                           sampler samplr0 [[sampler(0)]]) {
    int radius = int(rint(2.0 / supportBuffer.fragCoordScale.y));
    float2 resolution = float2(tex0.get_width(), tex0.get_height());

    float2 center = (in.passParameterSem3.xy + in.passParameterSem3.zw) / 2.0;
    float3 result = float3(0.0);
    float count = 0.0;
    for (int x = 1 - radius; x <= radius - 1; x += 2) {
        for (int y = 1 - radius; y <= radius - 1; y += 2) {
            if (length(float2(x, y)) <= radius) {
                result +=
                    tex0.sample(samplr0, center + float2(x, y) / resolution)
                        .xyz;
                count += 1.0;
            }
        }
    }

    return {float4(result / count, 1.0)};
}

#elif (disableDOF == 1) // Disable DOF

fragment FragmentOut main0(FragmentIn in [[stage_in]],
                           float2 pointCoord [[point_coord]],
                           bool frontFacing [[front_facing]],
                           constant SupportBuffer &supportBuffer [[buffer(0)]],
                           texture2d<float> tex0 [[texture(0)]],
                           sampler samplr0 [[sampler(0)]]) {
    return {tex0.sample(samplr0, in.passParameterSem3)};
}

#endif
)BOTW_MSL_END"},
{0xdeb3faf15678dcf5ULL, 0x0000000000000000ULL, GraphicPack2::GP_SHADER_TYPE::VERTEX, R"BOTW_MSL_END(#include <metal_stdlib>
using namespace metal;

// shader deb3faf15678dcf5
// Shader dumped from BotW 208, using Cemu 2193a8c
// Based on bb50d2ee4fa87bc2_0000000000000000_vs.txt
// Used for: Horizontal+Vertical Combat Targeting Blur

constant float resXScale = float($width) / float($gameWidth);
constant float resYScale = float($height) / float($gameHeight);

#define SET_POSITION(_v)                                                       \
    out.position = _v;                                                         \
    out.position.z = (out.position.z + out.position.w) / 2.0
// start of shader inputs/outputs, predetermined by Cemu. Do not touch
struct SupportBuffer {
    int4 remapped[5];
};

struct VertexIn {
    uint4 attrDataSem0 [[attribute(0)]];
    uint4 attrDataSem1 [[attribute(1)]];
};

struct VertexOut {
    float4 position [[position]] [[invariant]];
    float4 passParameterSem2 [[user(locn1)]];
    float4 passParameterSem0 [[user(locn0)]];
    float4 passParameterSem6 [[user(locn2)]];
};

// end of shader inputs/outputs
template <typename TextureT, typename CoordT>
float sampleCompareEmulate(TextureT tex, sampler samplr, CoordT coord,
                           float compareValue) {
    return compareValue < tex.sample(samplr, coord).x ? 1.0 : 0.0;
}
template <typename TextureT, typename CoordT>
float2 textureCalculateLod(TextureT tex, sampler samplr, CoordT coord) {
    float lod = tex.calculate_unclamped_lod(samplr, coord);
    return float2(floor(lod), fract(lod));
}
int clampFI32(int v) {
    if (v == 0x7FFFFFFF)
        return as_type<int>(1.0);
    else if (v == 0xFFFFFFFF)
        return as_type<int>(0.0);
    return as_type<int>(clamp(as_type<float>(v), 0.0, 1.0));
}
float mul_nonIEEE(float a, float b) {
    if (a == 0.0 || b == 0.0)
        return 0.0;
    return a * b;
}
vertex VertexOut main0(uint vid [[vertex_id]], uint iid [[instance_id]],
                       VertexIn in [[stage_in]],
                       constant SupportBuffer &supportBuffer [[buffer(0)]],
                       texture2d<float> tex0 [[texture(0)]],
                       sampler samplr0 [[sampler(0)]]) {
    VertexOut out;
    int4 R0i = int4(0);
    int4 R1i = int4(0);
    int4 R2i = int4(0);
    int4 R3i = int4(0);
    int4 R4i = int4(0);
    int4 R123i = int4(0);
    int4 R125i = int4(0);
    int4 R126i = int4(0);
    int4 R127i = int4(0);
    uint4 attrDecoder;
    int backupReg0i, backupReg1i, backupReg2i, backupReg3i, backupReg4i;
    int PV0ix = 0, PV0iy = 0, PV0iz = 0, PV0iw = 0, PV1ix = 0, PV1iy = 0,
        PV1iz = 0, PV1iw = 0;
    int PS0i = 0, PS1i = 0;
    int4 tempi = int4(0);
    float tempResultf;
    int tempResulti;
    int4 ARi = int4(0);
    bool predResult = true;
    R0i = int4(vid, 0, 0, iid);
    attrDecoder.xyzw = in.attrDataSem0.xyzw;
    attrDecoder = ((attrDecoder >> 8) & 0xFF) | ((attrDecoder << 8) & 0xFF00);
    attrDecoder.xyzw = as_type<uint4>(
        float4(float2(as_type<half2>(attrDecoder.x | (attrDecoder.y << 16))),
               float2(as_type<half2>(attrDecoder.z | (attrDecoder.w << 16)))));
    R1i = int4(int(attrDecoder.x), int(attrDecoder.y), int(attrDecoder.z),
               int(attrDecoder.w));
    attrDecoder.x = in.attrDataSem1.x;
    attrDecoder.x = (attrDecoder.x >> 24) | ((attrDecoder.x >> 8) & 0xFF00) |
                    ((attrDecoder.x << 8) & 0xFF0000) | ((attrDecoder.x << 24));
    attrDecoder.y = 0;
    attrDecoder.z = 0;
    attrDecoder.w = 0;
    attrDecoder.xyzw =
        uint4((attrDecoder.x >> 0) & 0x3FF, (attrDecoder.x >> 10) & 0x3FF,
              (attrDecoder.x >> 20) & 0x3FF, (attrDecoder.x >> 30) & 0x3);
    if ((attrDecoder.x & 0x200) != 0)
        attrDecoder.x |= 0xFFFFFC00;
    if ((attrDecoder.y & 0x200) != 0)
        attrDecoder.y |= 0xFFFFFC00;
    if ((attrDecoder.z & 0x200) != 0)
        attrDecoder.z |= 0xFFFFFC00;
    attrDecoder.x = as_type<uint>(max(float(int(attrDecoder.x)) / 511.0, -1.0));
    attrDecoder.y = as_type<uint>(max(float(int(attrDecoder.y)) / 511.0, -1.0));
    attrDecoder.z = as_type<uint>(max(float(int(attrDecoder.z)) / 511.0, -1.0));
    attrDecoder.w = as_type<uint>(float(attrDecoder.w));
    R2i = int4(int(attrDecoder.x), int(attrDecoder.y), int(attrDecoder.z),
               int(attrDecoder.w));
    // 0
    PV0ix = int(0x3f800000);
    PV0iy = int(0x40400000);
    PV0iz = (R0i.x == int(1)) ? -1 : 0;
    R127i.w = int(0x3f800000);
    R127i.x = int(0xbf800000);
    // 1
    PV1ix = as_type<int>(as_type<float>(supportBuffer.remapped[0].z) *
                         as_type<float>(0x3b808081));
    R0i.y = 0;
    R123i.z = ((PV0iz == 0) ? (PV0ix) : (int(0xc0400000)));
    R123i.w = ((PV0iz == 0) ? (PV0iy) : (int(0xbf800000)));
    R2i.w = int(0x3f800000);
    // 2
    R2i.x = ((R0i.x == 0) ? (R127i.x) : (R123i.w));
    R2i.y = ((R0i.x == 0) ? (R127i.w) : (R123i.z));
    R2i.z = as_type<int>(as_type<float>(PV1ix) + -(0.5));
    R2i.z = as_type<int>(as_type<float>(R2i.z) * 2.0);
    R0i.w = int(0x3d2aaaab);
    R1i.x = int(0x3e000000);
    // 3
    R3i.x = supportBuffer.remapped[1].z;
    R3i.x = as_type<int>(as_type<float>(R3i.x) / 2.0);
    R1i.y = 0;
    R1i.z = as_type<int>(-(as_type<float>(R2i.y)));
    R1i.w = as_type<int>(-(as_type<float>(supportBuffer.remapped[2].z)));
    R1i.w = as_type<int>(as_type<float>(R1i.w) / 2.0);
    R4i.x = as_type<int>(as_type<float>(R2i.x) * 0.5 + 0.5);
    R0i.w = as_type<int>(tex0.sample(samplr0, float2(as_type<float>(R0i.w),
                                                     as_type<float>(R0i.y)))
                             .x);
    R0i.xyz = as_type<int3>(tex0.sample(samplr0, float2(as_type<float>(R1i.x),
                                                        as_type<float>(R1i.y)))
                                .xyz);
    // export
    SET_POSITION(float4(as_type<float>(R2i.x), as_type<float>(R2i.y),
                        as_type<float>(R2i.z), as_type<float>(R2i.w)));
    // export
    out.passParameterSem2 =
        float4(as_type<float>(R0i.x), as_type<float>(R0i.y),
               as_type<float>(R0i.z), as_type<float>(R0i.w));
    // 0
    R127i.x = as_type<int>(as_type<float>(R3i.x) + -(0.5));
    R4i.y = as_type<int>(as_type<float>(R1i.z) * 0.5 + 0.5);
    PV0iz = as_type<int>(-(as_type<float>(R2i.y)));
    PV0iz = as_type<int>(as_type<float>(PV0iz) / 2.0);
    PV0iw = R2i.x;
    PV0iw = as_type<int>(as_type<float>(PV0iw) / 2.0);
    R126i.x = as_type<int>(as_type<float>(R1i.w) + -(0.5));
    // 1
    R125i.x = supportBuffer.remapped[3].y;
    R125i.x = as_type<int>(as_type<float>(R125i.x) * 2.0);
    R126i.y = as_type<int>(-(as_type<float>(supportBuffer.remapped[3].x)));
    R126i.y = as_type<int>(as_type<float>(R126i.y) * 2.0);
    R123i.z = as_type<int>(as_type<float>(supportBuffer.remapped[4].w) /
                               resYScale * 2.0 +
                           as_type<float>(R4i.y));
    PV1iw = as_type<int>(as_type<float>(PV0iw) + as_type<float>(R3i.x));
    R127i.z = as_type<int>(as_type<float>(PV0iz) + as_type<float>(R1i.w));
    // 2
    PV0ix = as_type<int>(as_type<float>(R123i.z) + as_type<float>(R126i.x));
    PV0iy = as_type<int>(as_type<float>(R4i.x) + as_type<float>(R127i.x));
    R126i.z = as_type<int>(
        -(as_type<float>(supportBuffer.remapped[4].w) / resYScale) *
            as_type<float>(0x3c23d70a) +
        as_type<float>(R4i.y));
    R123i.w = as_type<int>(as_type<float>(supportBuffer.remapped[4].z) /
                               resXScale * 2.0 +
                           as_type<float>(R4i.x));
    R127i.y = as_type<int>(
        mul_nonIEEE(as_type<float>(R126i.y), as_type<float>(PV1iw)));
    // 3
    backupReg0i = R127i.x;
    R127i.x = as_type<int>(
        mul_nonIEEE(as_type<float>(R125i.x), as_type<float>(R127i.z)));
    R123i.y = as_type<int>(
        -(as_type<float>(supportBuffer.remapped[4].z) / resXScale) *
            as_type<float>(0x3c23d70a) +
        as_type<float>(R4i.x));
    R127i.z = as_type<int>(
        mul_nonIEEE(as_type<float>(R126i.y), as_type<float>(PV0iy)));
    R127i.w =
        as_type<int>(as_type<float>(R123i.w) + as_type<float>(backupReg0i));
    R125i.z = as_type<int>(
        mul_nonIEEE(as_type<float>(R125i.x), as_type<float>(PV0ix)));
    // 4
    R3i.x = R123i.y;
    PV0iz = as_type<int>(as_type<float>(R4i.y) + as_type<float>(R126i.x));
    R3i.w = R126i.z;
    R4i.z = R127i.y;
    // 5
    R1i.x = as_type<int>(
        mul_nonIEEE(as_type<float>(R126i.y), as_type<float>(R127i.w)));
    R1i.y = as_type<int>(
        mul_nonIEEE(as_type<float>(R125i.x), as_type<float>(PV0iz)));
    R1i.z = R127i.z;
    R4i.w = R127i.x;
    R1i.w = R125i.z;
    // export
    out.passParameterSem0 =
        float4(as_type<float>(R4i.x), as_type<float>(R4i.y),
               as_type<float>(R4i.z), as_type<float>(R4i.w));
    // export
    // skipped export to semanticId 255
    // export
    out.passParameterSem6 =
        float4(as_type<float>(R3i.x), as_type<float>(R3i.x),
               as_type<float>(R3i.x), as_type<float>(R3i.w));
    return out;
}
)BOTW_MSL_END"},
{0xed56253a182b7014ULL, 0x0000000000000000ULL, GraphicPack2::GP_SHADER_TYPE::VERTEX, R"BOTW_MSL_END(#include <metal_stdlib>
using namespace metal;

// shader ed56253a182b7014
// Shader dumped from BotW 208, using Cemu 2193a8c
// Based on c9f2fd37115b0ee1_0000000000000000_vs.txt
// Used for: Horizontal+Vertical Combat Targeting Blur

constant float resXScale = float($width) / float($gameWidth);
constant float resYScale = float($height) / float($gameHeight);

#define SET_POSITION(_v)                                                       \
    out.position = _v;                                                         \
    out.position.z = (out.position.z + out.position.w) / 2.0
// start of shader inputs/outputs, predetermined by Cemu. Do not touch
struct SupportBuffer {
    int4 remapped[5];
};

struct VertexIn {
    uint4 attrDataSem0 [[attribute(0)]];
    uint4 attrDataSem1 [[attribute(1)]];
};

struct VertexOut {
    float4 position [[position]] [[invariant]];
    float4 passParameterSem0 [[user(locn0)]];
    float4 passParameterSem2 [[user(locn1)]];
    float4 passParameterSem6 [[user(locn2)]];
};

// end of shader inputs/outputs
template <typename TextureT, typename CoordT>
float sampleCompareEmulate(TextureT tex, sampler samplr, CoordT coord,
                           float compareValue) {
    return compareValue < tex.sample(samplr, coord).x ? 1.0 : 0.0;
}
template <typename TextureT, typename CoordT>
float2 textureCalculateLod(TextureT tex, sampler samplr, CoordT coord) {
    float lod = tex.calculate_unclamped_lod(samplr, coord);
    return float2(floor(lod), fract(lod));
}
int clampFI32(int v) {
    if (v == 0x7FFFFFFF)
        return as_type<int>(1.0);
    else if (v == 0xFFFFFFFF)
        return as_type<int>(0.0);
    return as_type<int>(clamp(as_type<float>(v), 0.0, 1.0));
}
float mul_nonIEEE(float a, float b) {
    if (a == 0.0 || b == 0.0)
        return 0.0;
    return a * b;
}
vertex VertexOut main0(uint vid [[vertex_id]], uint iid [[instance_id]],
                       VertexIn in [[stage_in]],
                       constant SupportBuffer &supportBuffer [[buffer(0)]],
                       texture2d<float> tex0 [[texture(0)]],
                       sampler samplr0 [[sampler(0)]]) {
    VertexOut out;
    int4 R0i = int4(0);
    int4 R1i = int4(0);
    int4 R2i = int4(0);
    int4 R3i = int4(0);
    int4 R4i = int4(0);
    int4 R5i = int4(0);
    int4 R123i = int4(0);
    int4 R124i = int4(0);
    int4 R125i = int4(0);
    int4 R126i = int4(0);
    int4 R127i = int4(0);
    uint4 attrDecoder;
    int backupReg0i, backupReg1i, backupReg2i, backupReg3i, backupReg4i;
    int PV0ix = 0, PV0iy = 0, PV0iz = 0, PV0iw = 0, PV1ix = 0, PV1iy = 0,
        PV1iz = 0, PV1iw = 0;
    int PS0i = 0, PS1i = 0;
    int4 tempi = int4(0);
    float tempResultf;
    int tempResulti;
    int4 ARi = int4(0);
    bool predResult = true;
    R0i = int4(vid, 0, 0, iid);
    attrDecoder.xyzw = in.attrDataSem0.xyzw;
    attrDecoder = ((attrDecoder >> 8) & 0xFF) | ((attrDecoder << 8) & 0xFF00);
    attrDecoder.xyzw = as_type<uint4>(
        float4(float2(as_type<half2>(attrDecoder.x | (attrDecoder.y << 16))),
               float2(as_type<half2>(attrDecoder.z | (attrDecoder.w << 16)))));
    R1i = int4(int(attrDecoder.x), int(attrDecoder.y), int(attrDecoder.z),
               int(attrDecoder.w));
    attrDecoder.x = in.attrDataSem1.x;
    attrDecoder.x = (attrDecoder.x >> 24) | ((attrDecoder.x >> 8) & 0xFF00) |
                    ((attrDecoder.x << 8) & 0xFF0000) | ((attrDecoder.x << 24));
    attrDecoder.y = 0;
    attrDecoder.z = 0;
    attrDecoder.w = 0;
    attrDecoder.xyzw =
        uint4((attrDecoder.x >> 0) & 0x3FF, (attrDecoder.x >> 10) & 0x3FF,
              (attrDecoder.x >> 20) & 0x3FF, (attrDecoder.x >> 30) & 0x3);
    if ((attrDecoder.x & 0x200) != 0)
        attrDecoder.x |= 0xFFFFFC00;
    if ((attrDecoder.y & 0x200) != 0)
        attrDecoder.y |= 0xFFFFFC00;
    if ((attrDecoder.z & 0x200) != 0)
        attrDecoder.z |= 0xFFFFFC00;
    attrDecoder.x = as_type<uint>(max(float(int(attrDecoder.x)) / 511.0, -1.0));
    attrDecoder.y = as_type<uint>(max(float(int(attrDecoder.y)) / 511.0, -1.0));
    attrDecoder.z = as_type<uint>(max(float(int(attrDecoder.z)) / 511.0, -1.0));
    attrDecoder.w = as_type<uint>(float(attrDecoder.w));
    R2i = int4(int(attrDecoder.x), int(attrDecoder.y), int(attrDecoder.z),
               int(attrDecoder.w));
    // 0
    PV0ix = int(0x3f800000);
    PV0iy = int(0x40400000);
    PV0iz = (R0i.x == int(1)) ? -1 : 0;
    R127i.w = int(0x3f800000);
    R127i.x = int(0xbf800000);
    // 1
    R3i.x = supportBuffer.remapped[0].z;
    R3i.x = as_type<int>(as_type<float>(R3i.x) / 2.0);
    PV1iy = as_type<int>(as_type<float>(supportBuffer.remapped[1].z) *
                         as_type<float>(0x3b808081));
    R123i.z = ((PV0iz == 0) ? (PV0ix) : (int(0xc0400000)));
    R123i.w = ((PV0iz == 0) ? (PV0iy) : (int(0xbf800000)));
    R2i.w = int(0x3f800000);
    // 2
    R2i.x = ((R0i.x == 0) ? (R127i.x) : (R123i.w));
    R2i.y = ((R0i.x == 0) ? (R127i.w) : (R123i.z));
    R2i.z = as_type<int>(as_type<float>(PV1iy) + -(0.5));
    R2i.z = as_type<int>(as_type<float>(R2i.z) * 2.0);
    R0i.w = int(0x3ec00000);
    R0i.y = 0;
    // 3
    R1i.x = int(0x3eeaaaab);
    R1i.y = 0;
    R1i.z = int(0x3d2aaaab);
    R1i.w = 0;
    R0i.x = as_type<int>(-(as_type<float>(R2i.y)));
    R5i.xyz = as_type<int3>(tex0.sample(samplr0, float2(as_type<float>(R0i.w),
                                                        as_type<float>(R0i.y)))
                                .xyz);
    R4i.xyz = as_type<int3>(tex0.sample(samplr0, float2(as_type<float>(R1i.x),
                                                        as_type<float>(R1i.y)))
                                .xyz);
    R1i.w = as_type<int>(tex0.sample(samplr0, float2(as_type<float>(R1i.z),
                                                     as_type<float>(R1i.w)))
                             .y);
    // export
    SET_POSITION(float4(as_type<float>(R2i.x), as_type<float>(R2i.y),
                        as_type<float>(R2i.z), as_type<float>(R2i.w)));
    // 0
    R127i.x = as_type<int>(-(as_type<float>(supportBuffer.remapped[2].z)));
    R127i.x = as_type<int>(as_type<float>(R127i.x) / 2.0);
    R0i.y = as_type<int>(as_type<float>(R0i.x) * 0.5 + 0.5);
    R127i.z = as_type<int>(-(as_type<float>(R2i.y)));
    R127i.z = as_type<int>(as_type<float>(R127i.z) / 2.0);
    R127i.w = R2i.x;
    R127i.w = as_type<int>(as_type<float>(R127i.w) / 2.0);
    R0i.x = as_type<int>(as_type<float>(R2i.x) * 0.5 + 0.5);
    // 1
    R2i.x = as_type<int>(-(as_type<float>(supportBuffer.remapped[3].x)));
    R2i.x = as_type<int>(as_type<float>(R2i.x) * 2.0);
    R2i.y = supportBuffer.remapped[3].y;
    R2i.y = as_type<int>(as_type<float>(R2i.y) * 2.0);
    R123i.z = as_type<int>(as_type<float>(supportBuffer.remapped[4].w) /
                               resYScale * 2.0 +
                           as_type<float>(R0i.y));
    R126i.w = as_type<int>(as_type<float>(R3i.x) + -(0.5));
    R125i.x = as_type<int>(as_type<float>(R127i.x) + -(0.5));
    // 2
    backupReg0i = R127i.z;
    R126i.x = as_type<int>(-(as_type<float>(R2i.y)));
    R126i.x = as_type<int>(as_type<float>(R126i.x) / 2.0);
    PV0iy = as_type<int>(-(as_type<float>(R2i.x)));
    PV0iy = as_type<int>(as_type<float>(PV0iy) / 2.0);
    R127i.z = as_type<int>(as_type<float>(R127i.w) + as_type<float>(R3i.x));
    R127i.w =
        as_type<int>(as_type<float>(backupReg0i) + as_type<float>(R127i.x));
    R126i.z = as_type<int>(as_type<float>(R123i.z) + as_type<float>(R125i.x));
    // 3
    R124i.x = as_type<int>(as_type<float>(R0i.x) + as_type<float>(R126i.w));
    R127i.y = as_type<int>(
        -(as_type<float>(supportBuffer.remapped[4].z) / resXScale) *
            as_type<float>(0x3c23d70a) +
        as_type<float>(R0i.x));
    R125i.z = as_type<int>(
        -(as_type<float>(supportBuffer.remapped[4].w) / resYScale) *
            as_type<float>(0x3c23d70a) +
        as_type<float>(R0i.y));
    R123i.w = as_type<int>(as_type<float>(supportBuffer.remapped[4].z) /
                               resXScale * 2.0 +
                           as_type<float>(R0i.x));
    R125i.w =
        as_type<int>(mul_nonIEEE(as_type<float>(R2i.x), as_type<float>(R3i.x)) +
                     as_type<float>(PV0iy));
    // 4
    backupReg0i = R127i.x;
    R127i.x = as_type<int>(
        mul_nonIEEE(as_type<float>(R2i.y), as_type<float>(R127i.w)));
    R126i.y = as_type<int>(
        mul_nonIEEE(as_type<float>(R2i.x), as_type<float>(R127i.z)));
    R127i.z = as_type<int>(
        mul_nonIEEE(as_type<float>(R2i.y), as_type<float>(backupReg0i)) +
        as_type<float>(R126i.x));
    R127i.w = as_type<int>(as_type<float>(R123i.w) + as_type<float>(R126i.w));
    R124i.w = as_type<int>(
        mul_nonIEEE(as_type<float>(R2i.y), as_type<float>(R126i.z)));
    // 5
    backupReg0i = R125i.z;
    PV1ix = as_type<int>(-(as_type<float>(R5i.z)) + as_type<float>(R4i.z));
    PV1iy = as_type<int>(-(as_type<float>(R5i.y)) + as_type<float>(R4i.y));
    R125i.z = as_type<int>(as_type<float>(R0i.y) + as_type<float>(R125i.x));
    R126i.w = as_type<int>(
        mul_nonIEEE(as_type<float>(R2i.x), as_type<float>(R124i.x)));
    R3i.w = backupReg0i;
    // 6
    R3i.x = R127i.y;
    R1i.y =
        as_type<int>(mul_nonIEEE(as_type<float>(PV1iy), as_type<float>(R0i.y)) +
                     as_type<float>(R5i.y));
    PV0iz = as_type<int>(-(as_type<float>(R5i.x)) + as_type<float>(R4i.x));
    R2i.w = R127i.z;
    R1i.z =
        as_type<int>(mul_nonIEEE(as_type<float>(PV1ix), as_type<float>(R0i.y)) +
                     as_type<float>(R5i.z));
    // 7
    R1i.x =
        as_type<int>(mul_nonIEEE(as_type<float>(PV0iz), as_type<float>(R0i.y)) +
                     as_type<float>(R5i.x));
    R5i.y = as_type<int>(
        mul_nonIEEE(as_type<float>(R2i.y), as_type<float>(R125i.z)));
    R2i.z = R125i.w;
    R0i.w = R127i.x;
    R0i.z = R126i.y;
    // 8
    R5i.x = as_type<int>(
        mul_nonIEEE(as_type<float>(R2i.x), as_type<float>(R127i.w)));
    R5i.z = R126i.w;
    R5i.w = R124i.w;
    // export
    out.passParameterSem0 =
        float4(as_type<float>(R0i.x), as_type<float>(R0i.y),
               as_type<float>(R0i.z), as_type<float>(R0i.w));
    // export
    out.passParameterSem2 =
        float4(as_type<float>(R1i.x), as_type<float>(R1i.y),
               as_type<float>(R1i.z), as_type<float>(R1i.w));
    // export
    // skipped export to semanticId 255
    // export
    // skipped export to semanticId 255
    // export
    out.passParameterSem6 =
        float4(as_type<float>(R3i.x), as_type<float>(R3i.x),
               as_type<float>(R3i.x), as_type<float>(R3i.w));
    return out;
}
)BOTW_MSL_END"},
{0xf14bb57cd5c9cb77ULL, 0xff060c183153097bULL, GraphicPack2::GP_SHADER_TYPE::PIXEL, R"BOTW_MSL_END(#include <metal_stdlib>
using namespace metal;

// shader f14bb57cd5c9cb77
// Shader dumped from BotW 208, using Cemu 2193a8c
// Based on f14bb57cd5c9cb77_00000000000003c9_ps.txt
// Used for: Removing/Restoring the native BotW World Anti-Aliasing
// implementation

constant float resX = float($width) / float($gameWidth);
constant float resY = float($height) / float($gameHeight);
#define fxaa $fxaa

#define SET_POSITION(_v)                                                       \
    out.position = _v;                                                         \
    out.position.z = (out.position.z + out.position.w) / 2.0
// start of shader inputs/outputs, predetermined by Cemu. Do not touch
struct SupportBuffer {
    int4 remapped[4];
    float2 fragCoordScale;
};

#define GET_FRAGCOORD()                                                        \
    float4(in.position.xy *supportBuffer.fragCoordScale.xy, in.position.z,     \
           1.0 / in.position.w)
struct FragmentIn {
    float4 position [[position]];
    float4 passParameterSem2 [[user(locn0)]];
};

struct FragmentOut {
    float4 passPixelColor0 [[color(0)]];
};

// end of shader inputs/outputs
template <typename TextureT, typename CoordT>
float sampleCompareEmulate(TextureT tex, sampler samplr, CoordT coord,
                           float compareValue) {
    return compareValue < tex.sample(samplr, coord).x ? 1.0 : 0.0;
}
template <typename TextureT, typename CoordT>
float2 textureCalculateLod(TextureT tex, sampler samplr, CoordT coord) {
    float lod = tex.calculate_unclamped_lod(samplr, coord);
    return float2(floor(lod), fract(lod));
}
int clampFI32(int v) {
    if (v == 0x7FFFFFFF)
        return as_type<int>(1.0);
    else if (v == 0xFFFFFFFF)
        return as_type<int>(0.0);
    return as_type<int>(clamp(as_type<float>(v), 0.0, 1.0));
}
float mul_nonIEEE(float a, float b) {
    if (a == 0.0 || b == 0.0)
        return 0.0;
    return a * b;
}
fragment FragmentOut main0(FragmentIn in [[stage_in]],
                           float2 pointCoord [[point_coord]],
                           bool frontFacing [[front_facing]],
                           constant SupportBuffer &supportBuffer [[buffer(0)]],
                           texture2d<float> tex0 [[texture(0)]],
                           sampler samplr0 [[sampler(0)]],
                           texture2d<float> tex1 [[texture(1)]],
                           sampler samplr1 [[sampler(1)]]) {
    FragmentOut out;
#if fxaa == 0
    out.passPixelColor0 = tex0.sample(samplr0, in.passParameterSem2.xy);
#else
    float4 R0f = float4(0.0);
    float4 R1f = float4(0.0);
    float4 R2f = float4(0.0);
    float4 R3f = float4(0.0);
    float4 R123f = float4(0.0);
    float4 R126f = float4(0.0);
    float4 R127f = float4(0.0);
    float backupReg0f, backupReg1f, backupReg2f, backupReg3f, backupReg4f;
    float PV0fx = 0.0, PV0fy = 0.0, PV0fz = 0.0, PV0fw = 0.0, PV1fx = 0.0,
          PV1fy = 0.0, PV1fz = 0.0, PV1fw = 0.0;
    float PS0f = 0.0, PS1f = 0.0;
    float4 tempf = float4(0.0);
    float tempResultf;
    int tempResulti;
    int4 ARi = int4(0);
    bool predResult = true;
    bool activeMaskStack[2];
    bool activeMaskStackC[3];
    activeMaskStack[0] = false;
    activeMaskStackC[0] = false;
    activeMaskStackC[1] = false;
    activeMaskStack[0] = true;
    activeMaskStackC[0] = true;
    activeMaskStackC[1] = true;
    R0f = in.passParameterSem2;
    if (activeMaskStackC[1] == true) {
        R1f.xyzw =
            (tex1.gather(samplr1, float2(0.0001) + float2(R0f.x, R0f.y)).wzxy);
        R2f.xyzw = (tex0.sample(samplr0, float2(R0f.x, R0f.y)).xyzw);
    }
    if (activeMaskStackC[1] == true) {
        activeMaskStack[1] = activeMaskStack[0];
        activeMaskStackC[2] = activeMaskStackC[1];
        // 0
        PV0fx = R1f.w + -(R1f.x);
        PV0fy = R1f.z + -(R1f.y);
        PV0fz = mul_nonIEEE(R2f.x, as_type<float>(supportBuffer.remapped[0].x));
        R127f.w = min(R1f.z, R1f.x);
        R127f.x = min(R1f.w, R1f.y);
        // 1
        R123f.x =
            mul_nonIEEE(R2f.y,
                        as_type<float>(supportBuffer.remapped[0].y) * resX) +
            PV0fz;
        PV1fy = max(R1f.z, R1f.x);
        R127f.z = PV0fy + -(PV0fx);
        R126f.w = PV0fy + PV0fx;
        PS1f = max(R1f.w, R1f.y);
        // 2
        PV0fx = max(R127f.z, -(R127f.z));
        PV0fy = max(R126f.w, -(R126f.w));
        R123f.z =
            mul_nonIEEE(R2f.z, as_type<float>(supportBuffer.remapped[0].z)) +
            R123f.x;
        PV0fw = min(R127f.w, R127f.x);
        PS0f = max(PV1fy, PS1f);
        // 3
        PV1fx = mul_nonIEEE(PS0f, as_type<float>(supportBuffer.remapped[1].x));
        PV1fy = max(R123f.z, PS0f);
        PV1fz = min(R123f.z, PV0fw);
        PV1fw = min(PV0fy, PV0fx);
        // 4
        R1f.x = -(PV1fz) + PV1fy;
        R0f.z = max(PV1fx, as_type<float>(supportBuffer.remapped[1].y));
        PS0f = 1.0 / PV1fw;
        // 5
        PV1fx = mul_nonIEEE(R127f.z, PS0f);
        PV1fy = mul_nonIEEE(R126f.w, PS0f);
        // 6
        PV0fz = max(PV1fx, -(as_type<float>(supportBuffer.remapped[2].y)));
        PV0fw = max(PV1fy, -(as_type<float>(supportBuffer.remapped[2].y)));
        // 7
        R3f.x = min(PV0fw, as_type<float>(supportBuffer.remapped[2].y));
        R1f.y = min(PV0fz, as_type<float>(supportBuffer.remapped[2].y));
        // 8
        predResult = (R1f.x > R0f.z);
        activeMaskStack[1] = predResult;
        activeMaskStackC[2] = predResult == true && activeMaskStackC[1] == true;
    } else {
        activeMaskStack[1] = false;
        activeMaskStackC[2] = false;
    }
    if (activeMaskStackC[2] == true) {
        // 0
        backupReg0f = R3f.x;
        R3f.x =
            mul_nonIEEE(backupReg0f,
                        as_type<float>(supportBuffer.remapped[3].x) / resX) +
            R0f.x;
        R3f.y = mul_nonIEEE(R1f.y, as_type<float>(supportBuffer.remapped[3].y) /
                                       resY) +
                R0f.y;
        R1f.x =
            mul_nonIEEE(backupReg0f,
                        -(as_type<float>(supportBuffer.remapped[3].x) / resX)) +
            R0f.x;
        // 1
        R1f.y =
            mul_nonIEEE(R1f.y,
                        -(as_type<float>(supportBuffer.remapped[3].y) / resY)) +
            R0f.y;
    }
    if (activeMaskStackC[2] == true) {
        R0f.xyzw = (tex0.sample(samplr0, float2(R3f.x, R3f.y)).xyzw);
        R1f.xyzw = (tex0.sample(samplr0, float2(R1f.x, R1f.y)).xyzw);
    }
    if (activeMaskStackC[2] == true) {
        // 0
        R127f.x = R0f.w + R1f.w;
        R127f.x /= 2.0;
        R127f.y = R0f.z + R1f.z;
        R127f.y /= 2.0;
        R127f.z = R0f.y + R1f.y;
        R127f.z /= 2.0;
        R127f.w = R0f.x + R1f.x;
        R127f.w /= 2.0;
        // 1
        PV1fx = R2f.w + -(R127f.x);
        PV1fy = R2f.z + -(R127f.y);
        PV1fz = R2f.y + -(R127f.z);
        PV1fw = R2f.x + -(R127f.w);
        // 2
        R2f.x = PV1fw * as_type<float>(0x3eb33333) + R127f.w;
        R2f.y = PV1fz * as_type<float>(0x3eb33333) + R127f.z;
        R2f.z = PV1fy * as_type<float>(0x3eb33333) + R127f.y;
        R2f.w = PV1fx * as_type<float>(0x3eb33333) + R127f.x;
    }
    activeMaskStackC[1] =
        activeMaskStack[0] == true && activeMaskStackC[0] == true;
    // export
    out.passPixelColor0 = as_type<float4>(float4(R2f.x, R2f.y, R2f.z, R2f.w));
#endif
    return out;
}
)BOTW_MSL_END"},
{0xf99a791bc92710c7ULL, 0x0000000000000000ULL, GraphicPack2::GP_SHADER_TYPE::VERTEX, R"BOTW_MSL_END(#include <metal_stdlib>
using namespace metal;

// shader f99a791bc92710c7
// Shader dumped from BotW 208, using Cemu 2193a8c
// Based on d1cf6920c3d5b194_0000000000000000_vs.txt
// Used for: Restoring the native BotW Anti-Aliasing implementation for distant
// trees

constant float resXScale = float($width) / float($gameWidth);
constant float resYScale = float($height) / float($gameHeight);

#define SET_POSITION(_v)                                                       \
    out.position = _v;                                                         \
    out.position.z = (out.position.z + out.position.w) / 2.0
// start of shader inputs/outputs, predetermined by Cemu. Do not touch
struct SupportBuffer {
    int4 remapped[1];
};

struct VertexIn {
    uint4 attrDataSem0 [[attribute(0)]];
    uint4 attrDataSem1 [[attribute(1)]];
};

struct VertexOut {
    float4 position [[position]] [[invariant]];
    float4 passParameterSem0 [[user(locn0)]];
};

// end of shader inputs/outputs
template <typename TextureT, typename CoordT>
float sampleCompareEmulate(TextureT tex, sampler samplr, CoordT coord,
                           float compareValue) {
    return compareValue < tex.sample(samplr, coord).x ? 1.0 : 0.0;
}
template <typename TextureT, typename CoordT>
float2 textureCalculateLod(TextureT tex, sampler samplr, CoordT coord) {
    float lod = tex.calculate_unclamped_lod(samplr, coord);
    return float2(floor(lod), fract(lod));
}
int clampFI32(int v) {
    if (v == 0x7FFFFFFF)
        return as_type<int>(1.0);
    else if (v == 0xFFFFFFFF)
        return as_type<int>(0.0);
    return as_type<int>(clamp(as_type<float>(v), 0.0, 1.0));
}
float mul_nonIEEE(float a, float b) {
    if (a == 0.0 || b == 0.0)
        return 0.0;
    return a * b;
}
vertex VertexOut main0(uint vid [[vertex_id]], uint iid [[instance_id]],
                       VertexIn in [[stage_in]],
                       constant SupportBuffer &supportBuffer [[buffer(0)]]) {
    VertexOut out;
    float4 R1f = float4(0.0);
    float4 R2f = float4(0.0);
    float4 R122f = float4(0.0);
    float4 R123f = float4(0.0);
    uint4 attrDecoder;
    float backupReg0f, backupReg1f, backupReg2f, backupReg3f, backupReg4f;
    float PV0fx = 0.0, PV0fy = 0.0, PV0fz = 0.0, PV0fw = 0.0, PV1fx = 0.0,
          PV1fy = 0.0, PV1fz = 0.0, PV1fw = 0.0;
    float PS0f = 0.0, PS1f = 0.0;
    float4 tempf = float4(0.0);
    float tempResultf;
    int tempResulti;
    int4 ARi = int4(0);
    bool predResult = true;
    attrDecoder.xyz = in.attrDataSem0.xyz;
    attrDecoder.xyz =
        (attrDecoder.xyz >> 24) | ((attrDecoder.xyz >> 8) & 0xFF00) |
        ((attrDecoder.xyz << 8) & 0xFF0000) | ((attrDecoder.xyz << 24));
    attrDecoder.w = 0;
    R1f = float4(
        as_type<float>(int(attrDecoder.x)), as_type<float>(int(attrDecoder.y)),
        as_type<float>(int(attrDecoder.z)), as_type<float>(0x3f800000));
    attrDecoder.xy = in.attrDataSem1.xy;
    attrDecoder.xy = (attrDecoder.xy >> 24) | ((attrDecoder.xy >> 8) & 0xFF00) |
                     ((attrDecoder.xy << 8) & 0xFF0000) |
                     ((attrDecoder.xy << 24));
    attrDecoder.z = 0;
    attrDecoder.w = 0;
    R2f = float4(as_type<float>(int(attrDecoder.x)),
                 as_type<float>(int(attrDecoder.y)), as_type<float>(0),
                 as_type<float>(0x3f800000));
    // 0
    R1f.x = R1f.x;
    R1f.x *= 2.0;
    R1f.y = R1f.y;
    R1f.y *= 2.0;
    R1f.z = 1.0;
    R123f.w = mul_nonIEEE(R2f.x, as_type<float>(supportBuffer.remapped[0].x) *
                                     resXScale) +
              0.5;
    R122f.x = mul_nonIEEE(R2f.y, as_type<float>(supportBuffer.remapped[0].y) *
                                     resYScale) +
              0.5;
    // 1
    R2f.z = R123f.w;
    R2f.w = R122f.x;
    // export
    SET_POSITION(float4(R1f.x, R1f.y, R1f.z, R1f.z));
    // export
    out.passParameterSem0 = float4(R2f.x, R2f.y, R2f.z, R2f.w);
    return out;
}
)BOTW_MSL_END"},

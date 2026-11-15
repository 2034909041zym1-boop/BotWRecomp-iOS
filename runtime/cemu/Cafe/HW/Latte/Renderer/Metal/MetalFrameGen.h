#pragma once

#include <Metal/Metal.hpp>
#include <cstdint>

class MetalFrameGen
{
public:
    explicit MetalFrameGen(MTL::Device* device);
    ~MetalFrameGen();

    MetalFrameGen(const MetalFrameGen&) = delete;
    MetalFrameGen& operator=(const MetalFrameGen&) = delete;

    // Captures srcTex into history and (if a previous frame exists and generate==true)
    // runs optical flow + interpolation into the output texture.
    // Returns true if an interpolated frame was generated and GetInterpTexture() is ready.
    bool Process(MTL::CommandBuffer* cmdBuf, MTL::Texture* srcTex, bool generate);

    MTL::Texture* GetInterpTexture() const { return m_interpView ? m_interpView : m_interpTex; }

    void ResetHistory();

private:
    bool EnsurePipelines();
    bool EnsureTextures(uint32_t w, uint32_t h, bool isSRGB);
    void ReleaseTextures();

    MTL::Device* m_device = nullptr;
    MTL::Library* m_library = nullptr;
    MTL::ComputePipelineState* m_psoCopy = nullptr;
    MTL::ComputePipelineState* m_psoLuma1 = nullptr;
    MTL::ComputePipelineState* m_psoLumaDown = nullptr;
    MTL::ComputePipelineState* m_psoFlowCoarse = nullptr;
    MTL::ComputePipelineState* m_psoFlowRefine = nullptr;
    MTL::ComputePipelineState* m_psoFlowSmooth = nullptr;
    MTL::ComputePipelineState* m_psoInterpolate = nullptr;
    MTL::SamplerState* m_linearSampler = nullptr;

    uint32_t m_w = 0;
    uint32_t m_h = 0;
    bool m_isSRGB = false;
    bool m_hasPrev = false;
    bool m_initFailed = false;

    // Ping-pong frame history (0 and 1), plus interpolated output
    MTL::Texture* m_frameTex[2] = {nullptr, nullptr};
    MTL::Texture* m_frameView[2] = {nullptr, nullptr};  // sRGB view if m_isSRGB, else nullptr
    MTL::Texture* m_luma1[2] = {nullptr, nullptr};
    MTL::Texture* m_luma2[2] = {nullptr, nullptr};
    MTL::Texture* m_luma3[2] = {nullptr, nullptr};
    int m_currIdx = 0;

    MTL::Texture* m_flow3 = nullptr;
    MTL::Texture* m_flow2 = nullptr;
    MTL::Texture* m_flowSmooth = nullptr;
    MTL::Texture* m_interpTex = nullptr;
    MTL::Texture* m_interpView = nullptr;
};

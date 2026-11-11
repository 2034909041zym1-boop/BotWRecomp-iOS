#include "Cafe/HW/Latte/Renderer/Metal/MetalLayerHandle.h"
#include "Cafe/HW/Latte/Renderer/Metal/MetalLayer.h"

#include "gui/interface/WindowSystem.h"

MetalLayerHandle::MetalLayerHandle(MTL::Device* device, const Vector2i& size, bool mainWindow)
{
    const auto& windowInfo = (mainWindow ? WindowSystem::GetWindowInfo().window_main : WindowSystem::GetWindowInfo().window_pad);

    m_layer = (CA::MetalLayer*)CreateMetalLayer(windowInfo.surface, m_layerScaleX, m_layerScaleY);
    m_layer->setDevice(device);
    m_layer->setDrawableSize(CGSize{(float)size.x * m_layerScaleX, (float)size.y * m_layerScaleY});
    m_layer->setFramebufferOnly(true);
}

MetalLayerHandle::~MetalLayerHandle()
{
    if (m_layer)
        m_layer->release();
}

void MetalLayerHandle::Resize(const Vector2i& size)
{
    m_layer->setDrawableSize(CGSize{(float)size.x * m_layerScaleX, (float)size.y * m_layerScaleY});
}

extern "C" bool botw_is_background();

bool MetalLayerHandle::AcquireDrawable()
{
    if (botw_is_background())
        return false;

    if (m_drawable)
        return true;

    m_drawable = m_layer->nextDrawable();
    if (!m_drawable)
    {
        cemuLog_log(LogType::Force, "layer {} failed to acquire next drawable", (void*)this);
        return false;
    }

    return true;
}

void MetalLayerHandle::PresentDrawable(MTL::CommandBuffer* commandBuffer)
{
    if (m_drawable)
    {
        commandBuffer->presentDrawable(m_drawable);
        m_drawable = nullptr;
    }
}

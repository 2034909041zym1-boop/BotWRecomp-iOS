#include "Cafe/HW/Latte/Renderer/Metal/MetalCommon.h"
#include "Cafe/HW/Latte/Renderer/Metal/MetalMemoryManager.h"
#include "Cafe/HW/Latte/Renderer/Metal/MetalVoidVertexPipeline.h"

#include "CafeSystem.h"
#include "Cemu/Logging/CemuLogging.h"
#include "Common/precompiled.h"
#include "HW/MMU/MMU.h"
#include "config/CemuConfig.h"

MetalMemoryManager::~MetalMemoryManager()
{
    if (m_bufferCache)
    {
        m_bufferCache->release();
    }
}

void* MetalMemoryManager::AcquireTextureUploadBuffer(size_t size)
{
    if (m_textureUploadBuffer.size() < size)
    {
        m_textureUploadBuffer.resize(size);
    }

    return m_textureUploadBuffer.data();
}

void MetalMemoryManager::ReleaseTextureUploadBuffer(uint8* mem)
{
    cemu_assert_debug(m_textureUploadBuffer.data() == mem);
	m_textureUploadBuffer.clear();
}

void MetalMemoryManager::InitBufferCache(size_t size)
{
    cemu_assert_debug(!m_bufferCache);

    m_metalBufferCacheMode = g_current_game_profile->GetBufferCacheMode();

    if (m_metalBufferCacheMode == MetalBufferCacheMode::Auto)
    {
        // TODO: do this for all unified memory systems?
        if (m_mtlr->IsAppleGPU())
        {
            switch (CafeSystem::GetForegroundTitleId())
            {
            // The Legend of Zelda: Wind Waker HD
            case 0x0005000010143600: // EUR
            case 0x0005000010143500: // USA
            case 0x0005000010143400: // JPN
                // TODO: use host instead?
                m_metalBufferCacheMode = MetalBufferCacheMode::DeviceShared;
                break;
            default:
                m_metalBufferCacheMode = MetalBufferCacheMode::DevicePrivate;
                break;
            }
        }
        else
        {
            m_metalBufferCacheMode = MetalBufferCacheMode::DevicePrivate;
        }
    }

    // First, try to import the host memory as a buffer
    if (m_metalBufferCacheMode == MetalBufferCacheMode::Host)
    {
        if (m_mtlr->HasUnifiedMemory())
        {
            m_importedMemBaseAddress = mmuRange_MEM2.getBase();
           	m_hostAllocationSize = mmuRange_MEM2.getSize();
            m_bufferCache = m_mtlr->GetDevice()->newBuffer(memory_getPointerFromVirtualOffset(m_importedMemBaseAddress), m_hostAllocationSize, MTL::ResourceStorageModeShared, nullptr);
            if (!m_bufferCache)
            {
                cemuLog_log(LogType::Force, "Failed to import host memory as a buffer, using device shared mode instead");
                m_metalBufferCacheMode = MetalBufferCacheMode::DeviceShared;
            }
        }
        else
        {
            cemuLog_log(LogType::Force, "Host buffer cache mode is only available on unified memory systems, using device shared mode instead");
            m_metalBufferCacheMode = MetalBufferCacheMode::DeviceShared;
        }
    }

    if (!m_bufferCache)
        m_bufferCache = m_mtlr->GetDevice()->newBuffer(size, (m_metalBufferCacheMode == MetalBufferCacheMode::DevicePrivate ? MTL::ResourceStorageModePrivate : MTL::ResourceStorageModeShared));

#ifdef CEMU_DEBUG_ASSERT
    m_bufferCache->setLabel(GetLabel("Buffer cache", m_bufferCache));
#endif
}

void MetalMemoryManager::UploadToBufferCache(const void* data, size_t offset, size_t size)
{
    cemu_assert_debug(m_metalBufferCacheMode != MetalBufferCacheMode::Host);
    cemu_assert_debug(m_bufferCache);
    cemu_assert_debug((offset + size) <= m_bufferCache->length());

    if (m_metalBufferCacheMode == MetalBufferCacheMode::DevicePrivate)
    {
        auto allocation = m_stagingAllocator.AllocateBufferMemory(size, 1);
        memcpy(allocation.memPtr, data, size);
        m_stagingAllocator.FlushReservation(allocation);

        if (m_mtlr->GetEncoderType() == MetalEncoderType::Render)
        {
            m_pendingBufferUploads.push_back({allocation.mtlBuffer, allocation.bufferOffset, offset, size});
        }
        else
        {
            if (!m_pendingBufferUploads.empty())
                FlushPendingBufferUploads();
            auto blitCommandEncoder = m_mtlr->GetBlitCommandEncoder();
            blitCommandEncoder->copyFromBuffer(allocation.mtlBuffer, allocation.bufferOffset, m_bufferCache, offset, size);
        }
    }
    else
    {
        memcpy((uint8*)m_bufferCache->contents() + offset, data, size);
    }
}

void MetalMemoryManager::FlushPendingBufferUploads()
{
    if (m_pendingBufferUploads.empty())
        return;

    auto uploads = std::move(m_pendingBufferUploads);
    m_pendingBufferUploads.clear();

    auto blitCommandEncoder = m_mtlr->GetBlitCommandEncoder();
    for (const auto& u : uploads)
    {
        blitCommandEncoder->copyFromBuffer(u.srcBuffer, u.srcOffset, m_bufferCache, u.dstOffset, u.size);
    }
}

bool MetalMemoryManager::HasUnsafePendingOverlap(size_t offset) const
{
    const PendingBufferUpload* matched = nullptr;
    for (auto it = m_pendingBufferUploads.rbegin(); it != m_pendingBufferUploads.rend(); ++it)
    {
        const auto& u = *it;
        if (!matched)
        {
            if (offset >= u.dstOffset && offset < u.dstOffset + u.size)
            {
                matched = &u;
                continue;
            }
            if (u.dstOffset > offset && (u.dstOffset - offset) < (4u * 1024u * 1024u))
                return true;
        }
        else
        {
            // If an older pending upload overlaps the matched range without being fully covered by it, flush to be safe.
            size_t mEnd = matched->dstOffset + matched->size;
            size_t uEnd = u.dstOffset + u.size;
            if (u.dstOffset < mEnd && uEnd > matched->dstOffset)
            {
                if (u.dstOffset < matched->dstOffset || uEnd > mEnd)
                    return true;
            }
        }
    }
    return false;
}

void MetalMemoryManager::ResolveBufferCacheBinding(size_t offset, MTL::Buffer*& outBuffer, size_t& outOffset) const
{
    for (auto it = m_pendingBufferUploads.rbegin(); it != m_pendingBufferUploads.rend(); ++it)
    {
        if (offset >= it->dstOffset && offset < it->dstOffset + it->size)
        {
            outBuffer = it->srcBuffer;
            outOffset = it->srcOffset + (offset - it->dstOffset);
            return;
        }
    }
    outBuffer = m_bufferCache;
    outOffset = offset;
}

void MetalMemoryManager::CopyBufferCache(size_t srcOffset, size_t dstOffset, size_t size)
{
    cemu_assert_debug(m_metalBufferCacheMode != MetalBufferCacheMode::Host);
    cemu_assert_debug(m_bufferCache);

    if (m_metalBufferCacheMode == MetalBufferCacheMode::DevicePrivate)
    {
        FlushPendingBufferUploads();
        m_mtlr->CopyBufferToBuffer(m_bufferCache, srcOffset, m_bufferCache, dstOffset, size, ALL_MTL_RENDER_STAGES, ALL_MTL_RENDER_STAGES);
    }
    else
        memcpy((uint8*)m_bufferCache->contents() + dstOffset, (uint8*)m_bufferCache->contents() + srcOffset, size);
}

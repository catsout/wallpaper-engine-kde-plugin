#include "StagingBuffer.hpp"
#include <algorithm>
#include <cstring>
#include "Util.hpp"
#include "Device.hpp"

using namespace wallpaper::vulkan;

#define CHECK_REF(ref, act)                                                  \
    if (! ref) {                                                             \
        LOG_ERROR("stage ref not available, index %d", ref.m_virtual_index); \
        { act; }                                                             \
    }

StagingBuffer::StagingBuffer(const Device& d, VkDeviceSize size, VkBufferUsageFlags usage)
    : m_device(d), m_size_step(size), m_usage(usage) {}
StagingBuffer::~StagingBuffer() {}

namespace
{
std::optional<VmaBufferParameters> CreateGpuBuffer(VmaAllocator allocator, VkBufferUsageFlags usage,
                                                   std::size_t size) {
    do {
        VmaBufferParameters buffer;
        VkBufferCreateInfo  ci {
             .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
             .pNext = nullptr,
             .size  = size,
             .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage,
        };
        buffer.req_size                  = ci.size;
        VmaAllocationCreateInfo vma_info = {};
        vma_info.usage                   = VMA_MEMORY_USAGE_GPU_ONLY;
        VVK_CHECK_ACT(break, vvk::CreateBuffer(allocator, ci, vma_info, buffer.handle));
        return buffer;
    } while (false);
    return std::nullopt;
}

void RecordCopyBuffer(const BufferParameters& dst_buf, const BufferParameters& src_buf,
                      vvk::CommandBuffer& cmd) {
    VkBufferCopy copy {
        .srcOffset = 0,
        .dstOffset = 0,
        .size      = src_buf.req_size,
    };
    cmd.CopyBuffer(src_buf.handle, dst_buf.handle, copy);

    VkBufferMemoryBarrier in_bar {
        .sType         = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .pNext         = nullptr,
        .srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_UNIFORM_READ_BIT,
        .buffer        = dst_buf.handle,
        .offset        = 0,
        .size          = VK_WHOLE_SIZE,
    };
    cmd.PipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT,
                        VK_PIPELINE_STAGE_VERTEX_INPUT_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
                            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                        VK_DEPENDENCY_BY_REGION_BIT,
                        in_bar);
}
} // namespace

StagingBuffer::VirtualBlock* StagingBuffer::newVirtualBlock(VkDeviceSize nsize) {
    auto it = std::find_if(m_virtual_blocks.begin(), m_virtual_blocks.end(), [nsize](auto& b) {
        return ! b.enabled && b.size >= nsize;
    });
    if (it == std::end(m_virtual_blocks)) {
        VkDeviceSize offset = m_virtual_blocks.empty()
                                  ? 0
                                  : m_virtual_blocks.back().offset + m_virtual_blocks.back().size;

        m_virtual_blocks.push_back({});
        it         = m_virtual_blocks.end() - 1;
        it->size   = nsize > m_size_step ? nsize : m_size_step;
        it->index  = (size_t)std::distance(m_virtual_blocks.begin(), it);
        it->offset = offset;
    }
    auto& block = *it;

    VmaVirtualBlockCreateInfo blockCreateInfo = {};
    blockCreateInfo.size                      = block.size;

    VVK_CHECK_ACT(return nullptr, vmaCreateVirtualBlock(&blockCreateInfo, &block.handle));
    block.enabled = true;

    LOG_INFO("new buffer block(%p), size: %d, index: %d / %d",
             this,
             block.size,
             block.index,
             m_virtual_blocks.size());
    return &block;
}
bool StagingBuffer::increaseBuf(VkDeviceSize nsize) {
    if (m_stage_raw == nullptr) {
        VVK_CHECK_BOOL_RE(mapStageBuf());
    }
    auto newsize = m_stage_buf.req_size + nsize;
    // do double copy
    std::vector<uint8_t> tmp;
    tmp.resize(newsize);
    memcpy(tmp.data(), m_stage_raw, m_stage_buf.req_size);

    m_stage_raw = nullptr;
    m_stage_buf.handle.UnMapMemory();
    m_stage_buf.handle = nullptr;

    if (! CreateStagingBuffer(m_device.vma_allocator(), newsize, m_stage_buf)) return false;
    VVK_CHECK_BOOL_RE(mapStageBuf());
    memcpy(m_stage_raw, tmp.data(), newsize);

    m_gpu_buf.handle = nullptr;
    LOG_INFO("increase buffer size: %d", nsize);
    return true;
}

bool StagingBuffer::allocate() {
    if (! CreateStagingBuffer(m_device.vma_allocator(), m_size_step, m_stage_buf)) return false;
    VVK_CHECK_BOOL_RE(m_stage_buf.handle.MapMemory(&m_stage_raw));
    auto* block = newVirtualBlock(m_size_step);
    return block != nullptr;
}

void StagingBuffer::destroy() {
    if (m_stage_raw != nullptr) {
        m_stage_buf.handle.UnMapMemory();
    }
    for (auto& block : m_virtual_blocks) {
        if (block.enabled) {
            vmaClearVirtualBlock(block.handle);
            vmaDestroyVirtualBlock(block.handle);
        }
    }
    m_virtual_blocks.clear();

    m_stage_buf = {};
    m_gpu_buf   = {};
}

bool StagingBuffer::allocateSubRef(VkDeviceSize size, StagingBufferRef& ref,
                                   VkDeviceSize alignment) {
    VkResult                       result;
    VmaVirtualAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.size                           = size;
    allocCreateInfo.alignment                      = alignment;

    VmaVirtualAllocation allocation;
    VkDeviceSize         offset;

    auto setRef = [&offset, &allocation, size](StagingBufferRef& ref, VirtualBlock& block) {
        ref.size   = size;
        ref.offset = offset + block.offset;

        ref.m_allocation    = allocation;
        ref.m_virtual_index = block.index;
    };

    for (auto& block : m_virtual_blocks) {
        if (block.enabled && block.size >= size) {
            if (auto res = vmaVirtualAllocate(block.handle, &allocCreateInfo, &allocation, &offset);
                res == VK_SUCCESS) {
                setRef(ref, block);
                return true;
            }
        }
    }

    auto  old_block_num = m_virtual_blocks.size();
    auto* p_block       = newVirtualBlock(size);
    if (p_block == nullptr) return false;

    auto& block = *p_block;
    if (old_block_num < m_virtual_blocks.size()) {
        if (! increaseBuf(block.size)) {
            auto& block = m_virtual_blocks.back();
            vmaClearVirtualBlock(block.handle);
            vmaDestroyVirtualBlock(block.handle);
            m_virtual_blocks.pop_back();
            LOG_ERROR("increase buf failed, pop_back block, current: %d", m_virtual_blocks.size());
            return false;
        }
    }
    VVK_CHECK_BOOL_RE(vmaVirtualAllocate(block.handle, &allocCreateInfo, &allocation, &offset));
    setRef(ref, block);
    return true;
}
void StagingBuffer::unallocateSubRef(const StagingBufferRef& ref) {
    CHECK_REF(ref, ;);
    if (ref.m_virtual_index < m_virtual_blocks.size()) {
        auto& block = m_virtual_blocks[ref.m_virtual_index];
        vmaVirtualFree(block.handle, ref.m_allocation);
        if (block.enabled && vmaIsVirtualBlockEmpty(block.handle)) {
            vmaDestroyVirtualBlock(block.handle);
            block.handle  = VK_NULL_HANDLE;
            block.enabled = false;
        }
    } else {
        LOG_ERROR("unallocate stagingbuffer failed: wrong index %d", ref.m_virtual_index);
    }
}

VkResult StagingBuffer::mapStageBuf() { return m_stage_buf.handle.MapMemory(&m_stage_raw); }

bool StagingBuffer::writeToBuf(const StagingBufferRef& ref, Span<uint8_t> data, size_t offset) {
    CHECK_REF(ref, return false);

    if (m_stage_raw == nullptr) {
        mapStageBuf();
    }
    VkDeviceSize size = std::min(ref.size - offset, data.size());
    uint8_t*     raw  = (uint8_t*)m_stage_raw;
    std::copy(data.begin(), data.begin() + size, raw + ref.offset + offset);
    return true;
}

bool StagingBuffer::fillBuf(const StagingBufferRef& ref, size_t offset, size_t size, uint8_t c) {
    CHECK_REF(ref, return false);

    if (m_stage_raw == nullptr) {
        mapStageBuf();
    }
    VkDeviceSize size_     = std::min(ref.size - offset, size);
    uint8_t*     raw       = (uint8_t*)m_stage_raw;
    uint8_t*     raw_begin = raw + ref.offset + offset;
    std::fill(raw_begin, raw_begin + size_, c);
    return true;
}

bool StagingBuffer::recordUpload(vvk::CommandBuffer& cmd) {
    if (! m_gpu_buf.handle) {
        if (auto opt = CreateGpuBuffer(m_device.vma_allocator(), m_usage, m_stage_buf.req_size);
            opt.has_value()) {
            m_gpu_buf = std::move(opt.value());
        } else
            return false;
    }
    if (m_stage_raw != nullptr) {
        m_stage_buf.handle.UnMapMemory();
        m_stage_raw = nullptr;
    }
    VVK_CHECK_BOOL_RE(vmaFlushAllocation(
        m_device.vma_allocator(), m_stage_buf.handle.Allocation(), 0, VK_WHOLE_SIZE));
    RecordCopyBuffer(m_gpu_buf, m_stage_buf, cmd);
    return true;
}

VkBuffer StagingBuffer::gpuBuf() const { return *m_gpu_buf.handle; }

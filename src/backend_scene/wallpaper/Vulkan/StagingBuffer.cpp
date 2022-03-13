#include "StagingBuffer.hpp"
#include <algorithm>
#include "Util.hpp"
#include "Device.hpp"

using namespace wallpaper::vulkan;

#define CHECK_REF(ref, act) if(!ref) {    \
    LOG_ERROR("stage ref not available, index %d", ref.m_virtual_index); \
    {act;}                                \
}

StagingBuffer::StagingBuffer(const Device& d, vk::DeviceSize size, vk::BufferUsageFlags usage):m_device(d),m_size_step(size),m_usage(usage) {}
StagingBuffer::~StagingBuffer() {
}

namespace {
vk::Result CreateGpuBuffer(VmaAllocator allocator, vk::BufferUsageFlags usage, std::size_t size, BufferParameters& buffer) {
	VkResult result;
	do {
		vk::BufferCreateInfo buffInfo;
		buffInfo.setSize(size)
			.setUsage(vk::BufferUsageFlagBits::eTransferDst | usage);
		buffer.req_size = buffInfo.size;
		VmaAllocationCreateInfo vmaallocInfo = {};
		vmaallocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		result = vmaCreateBuffer(allocator, (VkBufferCreateInfo *)&buffInfo, &vmaallocInfo,
						(VkBuffer *)&buffer.handle,
						&buffer.allocation,
						&buffer.allocationInfo);
	} while(false);
	return (vk::Result)result; 
}

void RecordCopyBuffer(BufferParameters &dst_buf, BufferParameters &src_buf, vk::CommandBuffer &cmd) {
    vk::BufferCopy copy(0, 0, src_buf.req_size);
    cmd.copyBuffer(src_buf.handle, dst_buf.handle, 1, &copy);

    vk::BufferMemoryBarrier in_bar;
    in_bar.setSrcAccessMask(vk::AccessFlagBits::eMemoryWrite)
        .setDstAccessMask(vk::AccessFlagBits::eVertexAttributeRead | vk::AccessFlagBits::eUniformRead)
        .setBuffer(dst_buf.handle)
        .setOffset(0)
        .setSize(VK_WHOLE_SIZE);
    cmd.pipelineBarrier(
        vk::PipelineStageFlagBits::eTransfer,
        vk::PipelineStageFlagBits::eVertexInput | vk::PipelineStageFlagBits::eVertexShader | 
        vk::PipelineStageFlagBits::eFragmentShader,
        vk::DependencyFlagBits::eByRegion,
        0, nullptr,
        1, &in_bar,
        0, nullptr);
}
}

StagingBuffer::VirtualBlock* StagingBuffer::newVirtualBlock(vk::DeviceSize nsize) {
    auto it = std::find_if(m_virtual_blocks.begin(), m_virtual_blocks.end(), [nsize](auto& b) { 
        return !b.enabled && nsize <= b.size; 
    });
    if(it == std::end(m_virtual_blocks)) {
        vk::DeviceSize offset = m_virtual_blocks.empty() 
            ? 0 
            : m_virtual_blocks.back().offset + m_virtual_blocks.back().size;

        m_virtual_blocks.push_back({});
        it = m_virtual_blocks.end() - 1;
        it->size = nsize > m_size_step ? nsize : m_size_step;
        it->index = std::distance(m_virtual_blocks.begin(), it);
        it->offset = offset;
    }
    auto& block = *it;

    VmaVirtualBlockCreateInfo blockCreateInfo = {};
    blockCreateInfo.size = block.size;

    VK_CHECK_RESULT_ACT(return nullptr, (vk::Result)vmaCreateVirtualBlock(&blockCreateInfo, &block.handle));
    block.enabled = true;

    LOG_INFO("new buffer block, size: %d, index: %d", block.size, block.index);
    return &block;
}
bool StagingBuffer::increaseBuf(vk::DeviceSize nsize) {
    if(m_stage_raw == nullptr) {
        VK_CHECK_RESULT_BOOL_RE(mapStageBuf());
    }
    auto newsize = m_stage_buf.req_size + nsize;
    // do double copy
    std::vector<uint8_t> tmp;
    tmp.resize(newsize);
    memcpy(tmp.data(), m_stage_raw, m_stage_buf.req_size);

    m_stage_raw = nullptr;
    vmaUnmapMemory(m_device.vma_allocator(), m_stage_buf.allocation);
    m_device.DestroyBuffer(m_stage_buf);

    if(!CreateStagingBuffer(m_device.vma_allocator(), newsize, m_stage_buf)) return false;
    VK_CHECK_RESULT_BOOL_RE(mapStageBuf());
    memcpy(m_stage_raw, tmp.data(), newsize);

    m_device.DestroyBuffer(m_gpu_buf);
    m_gpu_buf.handle = nullptr;

    LOG_INFO("increase buffer size: %d", nsize);
    return true;
}


bool StagingBuffer::allocate() {
    if(!CreateStagingBuffer(m_device.vma_allocator(), m_size_step, m_stage_buf)) return false;
    VK_CHECK_RESULT_BOOL_RE((vk::Result)vmaMapMemory(m_device.vma_allocator(), m_stage_buf.allocation, &m_stage_raw));

    auto* block = newVirtualBlock(m_size_step);
    return block != nullptr;
}


void StagingBuffer::destroy() {
    if(m_stage_raw != nullptr) {
        vmaUnmapMemory(m_device.vma_allocator(), m_stage_buf.allocation);
    }
    for(auto& block:m_virtual_blocks) {
        if(block.enabled) {
            vmaClearVirtualBlock(block.handle);
            vmaDestroyVirtualBlock(block.handle);
        }
    }
    m_virtual_blocks.clear();

    m_device.DestroyBuffer(m_stage_buf);
    m_device.DestroyBuffer(m_gpu_buf);
}

bool StagingBuffer::allocateSubRef(vk::DeviceSize size, StagingBufferRef& ref, vk::DeviceSize alignment) {
    vk::Result result;
    VmaVirtualAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.size = size;
    allocCreateInfo.alignment = alignment;

    VmaVirtualAllocation allocation;
    vk::DeviceSize offset;

    auto setRef = [&offset, &allocation, size](StagingBufferRef& ref, VirtualBlock& block) {
        ref.size = size;
        ref.offset = offset + block.offset;

        ref.m_allocation = allocation;
        ref.m_virtual_index = block.index;
    };

    for(auto& block:m_virtual_blocks) {
        if(!block.enabled || block.size < size) continue;

        auto res = (vk::Result)vmaVirtualAllocate(block.handle, &allocCreateInfo, &allocation, &offset);
        if(res == vk::Result::eSuccess) {
            setRef(ref, block);
            return true;
        }
    }

    auto old_block_num = m_virtual_blocks.size();
    auto* p_block = newVirtualBlock(size); 
    if(p_block == nullptr) return false;

    auto& block = *p_block;
    if(old_block_num < m_virtual_blocks.size()) {
        if(!increaseBuf(block.size)) { 
            auto& block = m_virtual_blocks.back();
            vmaClearVirtualBlock(block.handle);
            vmaDestroyVirtualBlock(block.handle);
            m_virtual_blocks.pop_back();
            return false;
        }
    }
    VK_CHECK_RESULT_BOOL_RE((vk::Result)vmaVirtualAllocate(block.handle, &allocCreateInfo, &allocation, &offset));
    setRef(ref, block);
    return true;
}
void StagingBuffer::unallocateSubRef(const StagingBufferRef& ref) {
    CHECK_REF(ref, ;);
    if(ref.m_virtual_index < m_virtual_blocks.size()) {
        auto& block = m_virtual_blocks[ref.m_virtual_index];
        vmaVirtualFree(block.handle, ref.m_allocation);
        if(vmaIsVirtualBlockEmpty(block.handle)) {
            vmaDestroyVirtualBlock(block.handle);
            block.handle = VK_NULL_HANDLE;
            block.enabled = false;
        }
    } else {
        LOG_ERROR("unallocate stagingbuffer failed: wrong index %d", ref.m_virtual_index);
    }
}


vk::Result StagingBuffer::mapStageBuf() {
    return (vk::Result)vmaMapMemory(m_device.vma_allocator(), m_stage_buf.allocation, &m_stage_raw);
}

bool StagingBuffer::writeToBuf(const StagingBufferRef& ref, Span<uint8_t> data, size_t offset) {
    CHECK_REF(ref, return false);

    if(m_stage_raw == nullptr) {
        mapStageBuf();
    }
    vk::DeviceSize size = std::min(ref.size - offset, (vk::DeviceSize)data.size());
    uint8_t* raw = (uint8_t*)m_stage_raw;
    std::memcpy(raw+ref.offset+offset, data.data(), size);
    return true;
}

bool StagingBuffer::recordUpload(vk::CommandBuffer& cmd) {
    if(!m_gpu_buf.handle) {
        VK_CHECK_RESULT_BOOL_RE(CreateGpuBuffer(m_device.vma_allocator(), m_usage, m_stage_buf.req_size, m_gpu_buf));
    }
    if(m_stage_raw != nullptr) {
        vmaUnmapMemory(m_device.vma_allocator(), m_stage_buf.allocation);
        m_stage_raw = nullptr;
    }
    VK_CHECK_RESULT_BOOL_RE((vk::Result)vmaFlushAllocation(m_device.vma_allocator(), m_stage_buf.allocation, 0, VK_WHOLE_SIZE));
    RecordCopyBuffer(m_gpu_buf, m_stage_buf, cmd);
    return true;
}

const vk::Buffer& StagingBuffer::gpuBuf() const { return m_gpu_buf.handle; }
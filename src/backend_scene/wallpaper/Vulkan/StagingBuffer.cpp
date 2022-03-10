#include "StagingBuffer.hpp"
#include <algorithm>
#include "Util.hpp"
#include "Device.hpp"

using namespace wallpaper::vulkan;


StagingBuffer::StagingBuffer(const Device& d, vk::DeviceSize size, vk::BufferUsageFlags usage):m_device(d),m_size(size),m_usage(usage) {}
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

bool StagingBuffer::allocate() {
    if(!CreateStagingBuffer(m_device.vma_allocator(), m_size, m_stage_buf)) return false;
    VK_CHECK_RESULT_BOOL_RE((vk::Result)vmaMapMemory(m_device.vma_allocator(), m_stage_buf.allocation, &m_stage_raw));
    VmaVirtualBlockCreateInfo blockCreateInfo = {};
    blockCreateInfo.size = m_size;
    VK_CHECK_RESULT_BOOL_RE((vk::Result)vmaCreateVirtualBlock(&blockCreateInfo, &m_virtual_block));
    return true;
}
void StagingBuffer::destroy() {
    if(m_stage_raw != nullptr) {
        vmaUnmapMemory(m_device.vma_allocator(), m_stage_buf.allocation);
    }
    vmaClearVirtualBlock(m_virtual_block);
    vmaDestroyVirtualBlock(m_virtual_block);

    m_device.DestroyBuffer(m_stage_buf);
    m_device.DestroyBuffer(m_gpu_buf);
}

bool StagingBuffer::allocateSubRef(vk::DeviceSize size, StagingBufferRef& ref, vk::DeviceSize alignment) {
    vk::Result result;
    VmaVirtualAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.size = size;
    allocCreateInfo.alignment = alignment;
    
    VK_CHECK_RESULT_BOOL_RE((vk::Result)vmaVirtualAllocate(m_virtual_block, &allocCreateInfo, &ref.allocation, &ref.offset));
    ref.size = size;
    return true;
}
void StagingBuffer::unallocateSubRef(const StagingBufferRef& ref) {
    vmaVirtualFree(m_virtual_block, ref.allocation);
}


vk::Result StagingBuffer::mapStageBuf() {
    return (vk::Result)vmaMapMemory(m_device.vma_allocator(), m_stage_buf.allocation, &m_stage_raw);
}

bool StagingBuffer::writeToBuf(const StagingBufferRef& ref, Span<uint8_t> data, size_t offset) {
    if(ref.allocation == VK_NULL_HANDLE) return false;
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
    return false;
}

const vk::Buffer& StagingBuffer::gpuBuf() const { return m_gpu_buf.handle; }
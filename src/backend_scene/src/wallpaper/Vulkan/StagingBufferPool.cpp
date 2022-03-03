#include "StagingBufferPool.hpp"

using namespace wallpaper::vulkan;


StagingBufferPool::StagingBufferPool(const Device& d):m_device(d) {}
StagingBufferPool::~StagingBufferPool() {}
#pragma once

#include <iterator>
#include <memory>
#include <vector>

#include "IBinaryStream.h"
#include "Utils/Logging.h"

namespace wallpaper
{
namespace fs
{

class MemBinaryStream : public IBinaryStream {
public:
    MemBinaryStream(std::vector<uint8_t>&& data): m_pos(0), m_data(std::move(data)) {}
    MemBinaryStream(IBinaryStream& f): m_pos(0) {
        m_data = std::vector<uint8_t>((usize)f.Size());
        f.Read(m_data.data(), m_data.size());
    }

    virtual ~MemBinaryStream() = default;

public:
    virtual usize Read(void* buffer, usize sizeInByte) {
        auto start_pos = m_data.begin() + m_pos;
        idx  moved     = moveForward((idx)sizeInByte);
        std::copy(start_pos, start_pos + moved, (uint8_t*)buffer);
        return (usize)moved;
    }
    virtual char* Gets(char* buffer, usize sizeStr) {
        Read(buffer, sizeStr);
        return buffer;
    }
    virtual idx  Tell() const { return m_pos; }
    virtual bool SeekSet(idx offset) {
        if (InArea(offset)) {
            m_pos = offset;
            return true;
        }
        return false;
    }
    virtual bool SeekCur(idx offset) {
        idx new_pos = m_pos + offset;
        if (InArea(new_pos)) {
            m_pos = new_pos;
            return true;
        }
        return false;
    }
    virtual bool SeekEnd(idx offset) {
        idx new_pos = Size() + offset;
        if (InArea(new_pos)) {
            m_pos = new_pos;
            return true;
        }
        return false;
    }
    virtual isize Size() const { return std::ssize(m_data); }

protected:
    virtual usize Write_impl(const void* buffer, usize sizeInByte) { return 0; }

private:
    bool InArea(idx pos) const noexcept { return pos >= 0 && pos <= Size(); }
    idx  moveForward(idx step) noexcept {
         idx end     = m_pos + step;
         end         = end > Size() ? Size() : end;
         idx stepped = end - m_pos;
         m_pos       = end;
         return stepped;
    };

    idx                  m_pos;
    std::vector<uint8_t> m_data;
};

} // namespace fs
} // namespace wallpaper

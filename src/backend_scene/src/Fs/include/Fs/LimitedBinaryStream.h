#pragma once
#include <memory>
#include <vector>
#include "IBinaryStream.h"

#include "Core/Literals.hpp"

namespace wallpaper
{
namespace fs
{

class LimitedBinaryStream : public IBinaryStream {
public:
    LimitedBinaryStream(std::shared_ptr<IBinaryStream> infs, idx start, isize size)
        : m_pos(0), m_start(start), m_end(start + size), m_infs(infs) {}
    virtual ~LimitedBinaryStream() = default;

private:
    bool CheckInArea(idx pos) const { return pos > 0 && pos <= Size(); }

    bool SeekInMPos(void) { return m_infs->SeekSet(m_start + m_pos); }
    bool SeekInPos(idx pos) {
        if (CheckInArea(pos)) {
            m_pos = pos;
            return SeekInMPos();
        }
        return false;
    }
    bool End() const { return m_pos < 0 || m_pos == Size(); };

protected:
    virtual usize Write_impl(const void*, usize) { return 0; }

public:
    virtual usize Read(void* buffer, usize sizeInByte) {
        if (End()) return 0;

        isize isizeInByte = (isize)sizeInByte;

        if (! CheckInArea(m_pos + isizeInByte)) {
            isizeInByte = Size() - m_pos;
        }
        SeekInMPos();
        m_pos += isizeInByte;

        return m_infs->Read(buffer, (usize)isizeInByte);
    }
    virtual char* Gets(char* buffer, usize sizeStr) {
        Read(buffer, sizeStr);
        return buffer;
    }
    virtual idx  Tell() const { return m_pos; }
    virtual bool SeekSet(idx offset) {
        idx pos = offset;
        return SeekInPos(pos);
    }
    virtual bool SeekCur(idx offset) {
        idx pos = m_pos + offset;
        return SeekInPos(pos);
    }
    virtual bool SeekEnd(idx offset) {
        idx pos = Size() - 1 - offset;
        return SeekInPos(pos);
    }
    virtual isize Size() const { return m_end - m_start; }

private:
    idx                            m_pos; // 0 < m_pos <= m_end - m_start
    const idx                      m_start;
    const idx                      m_end; // end if m_pos == m_end - m_start
    std::shared_ptr<IBinaryStream> m_infs;
};

} // namespace fs
} // namespace wallpaper

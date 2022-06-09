#pragma once
#include <memory>
#include "IBinaryStream.h"

namespace wallpaper 
{
namespace fs
{

class LimitedBinaryStream : public IBinaryStream {
public:
	LimitedBinaryStream(std::shared_ptr<IBinaryStream> infs, long start, long size)
		:m_infs(infs),
		 m_pos(0),
		 m_start(start),
		 m_end(start + size) {}
	virtual ~LimitedBinaryStream() = default;
private: 
	bool CheckInArea(long pos) const { return pos <= Size(); }
	bool SeekInPos(void) { return m_infs->SeekSet(m_start + m_pos); }
	bool SeekInPos(long pos) { 
		if(CheckInArea(pos)) {
			m_pos = pos;
			return m_infs->SeekSet(pos);
		}
		return false;
	}
	bool End() const { return m_pos == Size(); }; 

protected:
    virtual size_t Write_impl(const void* buffer, size_t sizeInByte) { return 0; }

public:
    virtual size_t Read(void* buffer, size_t sizeInByte) {
		if(End()) return 0;
		if(!CheckInArea(m_pos + sizeInByte)) {
			sizeInByte = Size() - m_pos;
		}
		SeekInPos();
		m_pos += sizeInByte;
		return m_infs->Read(buffer, sizeInByte);
	}
    virtual char* Gets(char* buffer, size_t sizeStr) { 
		Read(buffer, sizeStr);
		return buffer;
	}
    virtual long Tell() const { return m_pos; }
    virtual bool SeekSet(long offset) {
		long pos = offset;
		return SeekInPos(pos);
	}
    virtual bool SeekCur(long offset) {
		long pos = m_pos + offset;
		return SeekInPos(pos);
	}
    virtual bool SeekEnd(long offset) {
		long pos = Size() - 1 - offset;
		return SeekInPos(pos);
	}
    virtual std::size_t Size() const { return m_end - m_start; }
private:
	long m_pos; // 0 < m_pos <= m_end - m_start 
	const long m_start;
	const long m_end; // end if m_pos == m_end - m_start
	std::shared_ptr<IBinaryStream> m_infs;
};

}
}

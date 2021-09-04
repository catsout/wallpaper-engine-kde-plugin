#pragma once
#include <memory>
#include "IBinaryStream.h"
//#include <cassert>
//#include "../Log.h"

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
	bool CheckInArea(long pos) { return pos <= Size(); }
	bool SeekInPos(void) { return m_infs->SeekSet(m_start + m_pos); }
	bool SeekInPos(long pos) { 
		if(CheckInArea(pos)) {
			m_pos = pos;
			return m_infs->SeekSet(pos);
		}
		return false;
	}
public:
    virtual int Read(void* buffer, int sizeInByte) {
		if(CheckInArea(m_pos + sizeInByte)) {
			SeekInPos();
			m_pos += sizeInByte;
			return m_infs->Read(buffer, sizeInByte);
		}
		return 0;
	}
    virtual int Write(const void* buffer, int sizeInByte) { return 0; }
    virtual char* Gets(char* buffer, int sizeStr) { 
		Read(buffer, sizeStr);
		return buffer;
	}
    virtual long Tell() { return m_pos; }
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
    virtual std::size_t Size() { return m_end - m_start; }
private:
	long m_pos; // 0 < m_pos <= m_end - m_start 
	const long m_start;
	const long m_end; // end if m_pos == m_end - m_start
	std::shared_ptr<IBinaryStream> m_infs;
};

}
}
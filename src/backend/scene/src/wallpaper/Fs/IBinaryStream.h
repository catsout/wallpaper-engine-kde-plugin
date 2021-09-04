#pragma once
#include <cstdint>

namespace wallpaper
{
namespace fs
{

class IBinaryStream {
public:
	IBinaryStream() = default;
	virtual ~IBinaryStream() = default;

    float ReadFloat() {
        float x;
        Read(reinterpret_cast<char *>(&x), sizeof(x));
		return x;
    }

    int32_t ReadInt32() {
        int32_t x;
        Read(reinterpret_cast<char *>(&x), sizeof(x));
		return x;
    }

	uint32_t ReadUint32() {
        uint32_t x;
        Read(reinterpret_cast<char *>(&x), sizeof(x));
		return x;
	}

    std::string ReadAllStr() {
        std::string str;
        str.resize(Size());
        Read(str.data(), str.size());
        return str;
    }

	bool Rewind() {
		return SeekSet(0);
	}


public:
    virtual int Read(void* buffer, int sizeInByte) = 0;
    virtual int Write( const void* buffer, int sizeInByte) = 0;
    virtual char* Gets( char* buffer, int sizeStr) = 0;
    virtual long Tell() = 0;
    virtual bool SeekSet(long offset) = 0;
    virtual bool SeekCur(long offset) = 0;
    virtual bool SeekEnd(long offset) = 0;
    virtual std::size_t Size() = 0;
};

}
}
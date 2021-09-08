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
	IBinaryStream(const IBinaryStream&) = delete;
	IBinaryStream& operator=(const IBinaryStream&) = delete;
	IBinaryStream(IBinaryStream&&) = default;
	IBinaryStream& operator=(IBinaryStream&&) = default;


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
    virtual size_t Read(void* buffer, size_t sizeInByte) = 0;
    virtual size_t Write(const void* buffer, size_t sizeInByte) = 0;
    virtual char* Gets(char* buffer, size_t sizeStr) = 0;
    virtual long Tell() const = 0;
    virtual bool SeekSet(long offset) = 0;
    virtual bool SeekCur(long offset) = 0;
    virtual bool SeekEnd(long offset) = 0;
    virtual std::size_t Size() const = 0;
};

}
}
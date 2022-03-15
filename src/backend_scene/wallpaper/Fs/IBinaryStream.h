#pragma once
#include <cstdint>
#include <string>
#include "Bswap.hpp"
#include "Utils/NoCopyMove.hpp"

namespace wallpaper
{
namespace fs
{

class IBinaryStream : NoCopy,NoMove {
public:
    enum class ByteOrder {
        BigEndian, LittleEndian
    };

    constexpr static ByteOrder sys_byte_order {
#ifdef WP_BIG_ENDIAN
        ByteOrder::BigEndian
#else
        ByteOrder::LittleEndian
#endif
    };
private:
    template<typename T>
    T _ReadInt() {
        T x {0};
        if(Read(reinterpret_cast<char*>(&x), sizeof(x)) != sizeof(x)) {
            x = T{0};
        } else {
            if(!m_noswap) {
                x = bswap<T>(x);
            }
        }
        return x;
    }
public:
	IBinaryStream() = default;
	virtual ~IBinaryStream() = default;

    void SetByteOrder(ByteOrder order) {
        m_byte_order = order;
        m_noswap = order == sys_byte_order;
    }

    float ReadFloat() {
        float x {0};
        Read(reinterpret_cast<char *>(&x), sizeof(x));
		return x;
    }

    int64_t ReadInt64() { return _ReadInt<int64_t>(); }
    uint64_t ReadUint64() { return _ReadInt<uint64_t>(); }

    int32_t ReadInt32() { return _ReadInt<int32_t>(); }
    uint32_t ReadUint32() { return _ReadInt<uint32_t>(); }

    int16_t ReadInt16() { return _ReadInt<int16_t>(); }
    uint16_t ReadUint16() { return _ReadInt<uint16_t>(); }

    int8_t ReadInt8() { return _ReadInt<int8_t>(); }
    uint8_t ReadUint8() { return _ReadInt<uint8_t>(); }

    std::string ReadStr() {
        std::string str;
        char c;
        while(Read(&c, 1)) {
            if(c == '\0') break;
            str.push_back(c);
        }
        return str;
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
private:
    constexpr static ByteOrder default_byte_order {ByteOrder::LittleEndian};
    ByteOrder m_byte_order {default_byte_order};
    bool m_noswap {sys_byte_order == default_byte_order};
};

}
}
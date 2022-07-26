#pragma once
#include <cstdint>
#include <string>
#include <type_traits>
#include "Bswap.hpp"
#include "Utils/NoCopyMove.hpp"
#include "Core/Literals.hpp"

namespace wallpaper
{
namespace fs
{

class IBinaryStream : NoCopy, NoMove {
public:
    enum class ByteOrder
    {
        BigEndian,
        LittleEndian
    };

    constexpr static ByteOrder sys_byte_order {
#ifdef WP_BIG_ENDIAN
        ByteOrder::BigEndian
#else
        ByteOrder::LittleEndian
#endif
    };

protected:
    template<typename T>
    T _ReadInt() {
        T x { 0 };
        if (Read(reinterpret_cast<char*>(&x), sizeof(x)) != sizeof(x)) {
            x = T { 0 };
        } else {
            if (! m_noswap) {
                x = bswap<T>(x);
            }
        }
        return x;
    }
    template<typename T>
    bool _WriteInt(T x) {
        if (! m_noswap) {
            x = bswap<T>(x);
        }
        return Write_impl(reinterpret_cast<char*>(&x), sizeof(x)) != sizeof(x);
    }

public:
    IBinaryStream()          = default;
    virtual ~IBinaryStream() = default;

    void SetByteOrder(ByteOrder order) {
        m_byte_order = order;
        m_noswap     = order == sys_byte_order;
    }

    float ReadFloat() {
        float x { 0 };
        Read(reinterpret_cast<char*>(&x), sizeof(x));
        return x;
    }

    i64 ReadInt64() { return _ReadInt<i64>(); }
    u64 ReadUint64() { return _ReadInt<u64>(); }

    i32 ReadInt32() { return _ReadInt<i32>(); }
    u32 ReadUint32() { return _ReadInt<u32>(); }

    i16 ReadInt16() { return _ReadInt<i16>(); }
    u16 ReadUint16() { return _ReadInt<u16>(); }

    i8 ReadInt8() { return _ReadInt<i8>(); }
    u8 ReadUint8() { return _ReadInt<u8>(); }

    std::string ReadStr() {
        std::string str;
        char        c;
        while (Read(&c, 1)) {
            if (c == '\0') break;
            str.push_back(c);
        }
        return str;
    }

    std::string ReadAllStr() {
        std::string str;
        str.resize(Usize());
        Read(str.data(), std::size(str));
        return str;
    }

    bool Rewind() { return SeekSet(0); }

    usize Usize() const noexcept { return static_cast<usize>(Size()); }

public:
    virtual usize Read(void* buffer, usize sizeInByte) = 0;
    virtual char* Gets(char* buffer, usize sizeStr)    = 0;
    virtual idx   Tell() const                         = 0;
    virtual bool  SeekSet(idx offset)                  = 0;
    virtual bool  SeekCur(idx offset)                  = 0;
    virtual bool  SeekEnd(idx offset)                  = 0;

    virtual isize Size() const = 0;

protected:
    virtual usize Write_impl(const void* buffer, usize sizeInByte) = 0;

private:
    constexpr static ByteOrder default_byte_order { ByteOrder::LittleEndian };
    ByteOrder                  m_byte_order { default_byte_order };
    bool                       m_noswap { sys_byte_order == default_byte_order };
};

class IBinaryStreamW : IBinaryStream {
public:
    virtual ~IBinaryStreamW() = default;
    usize Write(const void* buffer, usize sizeInByte) { return Write_impl(buffer, sizeInByte); }
    i32   WriteInt32(i32 x) { return _WriteInt<i32>(x); }
    i32   WriteUint32(u32 x) { return _WriteInt<u32>(x); }
};

} // namespace fs
} // namespace wallpaper

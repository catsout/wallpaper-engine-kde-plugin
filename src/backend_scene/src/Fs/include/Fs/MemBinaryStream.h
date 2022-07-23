#pragma once

#include <memory>

#include "IBinaryStream.h"
#include "Utils/Logging.h"

namespace wallpaper
{
namespace fs
{

class MemBinaryStream : public IBinaryStream {
protected:

public:
    virtual ~MemBinaryStream() = default;
public:
    virtual size_t Read(void* buffer, size_t sizeInByte) = 0;
    virtual char* Gets(char* buffer, size_t sizeStr) = 0;
    virtual long Tell() const = 0;
    virtual bool SeekSet(long offset) = 0;
    virtual bool SeekCur(long offset) = 0;
    virtual bool SeekEnd(long offset) = 0;
    virtual std::size_t Size() const = 0;

private:

};

} // namespace fs
} // namespace wallpaper

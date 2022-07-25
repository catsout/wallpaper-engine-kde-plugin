#pragma once
#include <memory>
#include <cstdint>
#include <functional>
#include "Utils/Logging.h"
#include "Utils/NoCopyMove.hpp"

namespace wallpaper
{

namespace fs
{
class IBinaryStream;
}
namespace audio
{

class SoundStream : NoCopy, NoMove {
public:
    struct Desc {
        uint32_t channels;
        uint32_t sampleRate;
    };

public:
    SoundStream()          = default;
    virtual ~SoundStream() = default;

    virtual uint64_t NextPcmData(void* pData, uint32_t frameCount) = 0;
    virtual void     PassDesc(const Desc&)                         = 0;
};
std::unique_ptr<SoundStream> CreateSoundStream(std::shared_ptr<fs::IBinaryStream>,
                                               const SoundStream::Desc&);

class SoundManager : NoCopy, NoMove {
public:
    SoundManager();
    ~SoundManager();
    void MountStream(std::unique_ptr<SoundStream>&&);
    void UnMountAll();
    void Test(std::shared_ptr<fs::IBinaryStream>);
    bool Init();
    bool IsInited() const;
    void Play();
    void Pause();

    float Volume() const;
    bool  Muted() const;
    void  SetMuted(bool);
    void  SetVolume(float);

private:
    class impl;
    std::unique_ptr<impl> pImpl;
};
} // namespace audio
} // namespace wallpaper

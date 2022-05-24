#include "glExtra.hpp"
#include <glad/glad.h>
#include <stdio.h>
#include "Utils/Logging.h"

using namespace wallpaper;

#define CHECK_GL_ERROR_IF_DEBUG() CheckGlError(__SHORT_FILE__, __FUNCTION__, __LINE__);

namespace
{
inline char const* const GLErrorToStr(GLenum const err) noexcept {
#define Enum_GLError(glerr) \
    case glerr: return #glerr;

    switch (err) {
        // opengl 2
        Enum_GLError(GL_NO_ERROR);
        Enum_GLError(GL_INVALID_ENUM);
        Enum_GLError(GL_INVALID_VALUE);
        Enum_GLError(GL_INVALID_OPERATION);
        Enum_GLError(GL_OUT_OF_MEMORY);
        // opengl 3 errors (1)
        Enum_GLError(GL_INVALID_FRAMEBUFFER_OPERATION);
    default: return "Unknown GLError";
    }
#undef Enum_GLError
}

inline void CheckGlError(const char* file, const char* func, int line) {
    int err = glGetError();
    if (err != 0) {
        WallpaperLog(LOGLEVEL_ERROR, file, line, "%s(%d) at %s", GLErrorToStr(err), err, func);
    }
}
}

class GlExtra::impl {
public:
    bool                                       test;
    std::array<std::uint8_t, GL_UUID_SIZE_EXT> uuid;
};

GlExtra::GlExtra(): pImpl(std::make_unique<impl>()) {}
GlExtra::~GlExtra() {}

static std::array<std::uint8_t, GL_UUID_SIZE_EXT> getUUID() {
    int32_t num_device = 0;
    glGetIntegerv(GL_NUM_DEVICE_UUIDS_EXT, &num_device);

    GLubyte uuid[GL_UUID_SIZE_EXT] = { 0 };
    glGetUnsignedBytei_vEXT(GL_DEVICE_UUID_EXT, 0, uuid);
    std::array<std::uint8_t, GL_UUID_SIZE_EXT> result;
    std::copy(std::begin(uuid), std::end(uuid), result.begin());
    return result;
}

bool GlExtra::init(void* get_proc_address(const char*)) {
    do {
        if (inited) break;
        if (! gladLoadGLLoader((GLADloadproc)get_proc_address)) {
            LOG_ERROR("Failed to initialize GLAD");
            break;
        }
        LOG_INFO("gl: OpenGL version %d.%d loaded", GLVersion.major, GLVersion.minor);
        if (! (GLAD_GL_EXT_memory_object && GLAD_GL_EXT_semaphore)) {
            LOG_ERROR("EXT_memory_object not available");
            break;
        }
        pImpl->uuid = getUUID();

        std::string gl_verdor_name { (const char*)glGetString(GL_VENDOR) };
        LOG_INFO("gl: OpenGL vendor string: %s", gl_verdor_name.c_str());

        {
            int              num { 0 };
            std::vector<int> tex_tilings;
            glGetInternalformativ(GL_TEXTURE_2D, GL_RGBA8, GL_NUM_TILING_TYPES_EXT, 1, &num);
            if (num <= 0) {
                LOG_ERROR("can't get texture tiling support info");
                break;
            }
            num = std::min(num, 2);
            tex_tilings.resize(num);

            glGetInternalformativ(GL_TEXTURE_2D,
                                  GL_RGBA8,
                                  GL_TILING_TYPES_EXT,
                                  tex_tilings.size(),
                                  tex_tilings.data());
            CHECK_GL_ERROR_IF_DEBUG();

            bool support_optimal { false }, support_linear { false };
            for (auto& tiling : tex_tilings) {
                if (tiling == GL_OPTIMAL_TILING_EXT) {
                    support_optimal = true;
                } else if (tiling == GL_LINEAR_TILING_EXT) {
                    support_linear = true;
                }
            }
            if (! support_optimal && ! support_linear) {
                LOG_ERROR("no supported tiling mode");
                break;
            }

            if (support_optimal) {
                m_tiling = wallpaper::TexTiling::OPTIMAL;
            } else if (support_linear) {
                m_tiling = wallpaper::TexTiling::LINEAR;
            }

            // linear, fix for amd
            // https://gitlab.freedesktop.org/mesa/mesa/-/issues/2456
            if (support_linear && gl_verdor_name.find("AMD") != std::string::npos) {
                m_tiling = wallpaper::TexTiling::LINEAR;
            }

            if (m_tiling == wallpaper::TexTiling::OPTIMAL) {
                LOG_INFO("gl: external tex using optimal tiling");
            } else {
                LOG_INFO("gl: external tex using linear tiling");
            }
        }

        inited = true;
    } while (false);
    return inited;
}

Span<const std::uint8_t> GlExtra::uuid() const { return pImpl->uuid; }

TexTiling GlExtra::tiling() const { return m_tiling; }

uint GlExtra::genExTexture(ExHandle& handle) {
    uint memobject, tex;
    glCreateMemoryObjectsEXT(1, &memobject);
    glImportMemoryFdEXT(memobject, handle.size, GL_HANDLE_TYPE_OPAQUE_FD_EXT, handle.fd);
    CHECK_GL_ERROR_IF_DEBUG()

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_TILING_EXT,
        (m_tiling == TexTiling::OPTIMAL ? GL_OPTIMAL_TILING_EXT : GL_LINEAR_TILING_EXT));
    CHECK_GL_ERROR_IF_DEBUG()

    glTexStorageMem2DEXT(GL_TEXTURE_2D, 1, GL_RGBA8, handle.width, handle.height, memobject, 0);
    CHECK_GL_ERROR_IF_DEBUG()

    glBindTexture(GL_TEXTURE_2D, 0);
    handle.fd = -1;
    return tex;
}

void GlExtra::deleteTexture(uint tex) {
    glDeleteTextures(1, &tex);
    CHECK_GL_ERROR_IF_DEBUG();
}

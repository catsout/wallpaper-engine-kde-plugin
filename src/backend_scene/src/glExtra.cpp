#include "glExtra.hpp"
#include <glad/glad.h>
#include <stdio.h>

using namespace wallpaper;

#define __SHORT_FILE__ __FILE__
#if 1
#undef __SHORT_FILE__
#define __SHORT_FILE__ ({constexpr const char*const p = past_last_slash(__FILE__); p;})
static constexpr const char* const past_last_slash(const char* const path, const int pos=0, const int last_slash=0) {
	if(path[pos] == '\0') return &path[last_slash];
	if(path[pos] == '/') return past_last_slash(path, pos+1, pos+1);
	else return past_last_slash(path, pos+1, last_slash);
}
#endif 


#define CHECK_GL_ERROR_IF_DEBUG() CheckGlError(__SHORT_FILE__, __FUNCTION__, __LINE__);

inline char const* const GLErrorToStr(GLenum const err) noexcept {
#define Enum_GLError(glerr) case glerr: return #glerr;
  switch (err) {
    // opengl 2
	Enum_GLError(GL_NO_ERROR);
	Enum_GLError(GL_INVALID_ENUM);
	Enum_GLError(GL_INVALID_VALUE);
	Enum_GLError(GL_INVALID_OPERATION);
	Enum_GLError(GL_OUT_OF_MEMORY);
    // opengl 3 errors (1)
	Enum_GLError(GL_INVALID_FRAMEBUFFER_OPERATION);
    default:
      return "Unknown GLError";
  }
#undef Enum_GLError
}

inline void CheckGlError(const char* file, const char* func, int line) {
	int err = glGetError();
	if(err != 0)
		printf("%s(%d) at %s, line %d", GLErrorToStr(err), err, func, line);
}


class GlExtra::impl {
public:
    bool test;
    std::array<std::uint8_t, GL_UUID_SIZE_EXT> uuid;
};

GlExtra::GlExtra():pImpl(std::make_unique<impl>()) {}
GlExtra::~GlExtra() {}

static std::array<std::uint8_t, GL_UUID_SIZE_EXT> getUUID() {
    int32_t num_device = 0;
    glGetIntegerv(GL_NUM_DEVICE_UUIDS_EXT, &num_device);

    GLubyte uuid[GL_UUID_SIZE_EXT] = {0};
    glGetUnsignedBytei_vEXT(GL_DEVICE_UUID_EXT, 0, uuid);
    std::array<std::uint8_t, GL_UUID_SIZE_EXT> result;
    std::copy(std::begin(uuid), std::end(uuid), result.begin());
    return result;
}

bool GlExtra::init(void *get_proc_address(const char *)) {
    do {
        if(inited) break;
        if (!gladLoadGLLoader((GLADloadproc)get_proc_address)) {
            printf("Failed to initialize GLAD\n");
            break;
        }
        printf("OpenGL Version %d.%d loaded\n", GLVersion.major, GLVersion.minor);
        if(GLAD_GL_EXT_memory_object && GLAD_GL_EXT_semaphore) {
            printf("GL EXT loaded\n");
        }
        pImpl->uuid = getUUID();
        inited = true;
    } while(false);
    return inited;
}

void GlExtra::cmd() {
    printf("gl cmd\n");
}

Span<std::uint8_t> GlExtra::uuid() const {
    return pImpl->uuid;
}

uint GlExtra::genExTexture(ExHandle& handle) {
    uint memobject, tex;
    glCreateMemoryObjectsEXT(1, &memobject);
    glImportMemoryFdEXT(memobject, handle.size, GL_HANDLE_TYPE_OPAQUE_FD_EXT, handle.fd);
    CHECK_GL_ERROR_IF_DEBUG()
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    CHECK_GL_ERROR_IF_DEBUG()
    glTextureStorageMem2DEXT(tex, 1, GL_RGBA8, handle.width, handle.height, memobject, 0);
    CHECK_GL_ERROR_IF_DEBUG()
    handle.fd = -1;
    return tex;
}
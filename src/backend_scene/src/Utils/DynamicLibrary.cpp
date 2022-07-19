#include "DynamicLibrary.hpp"

#include "dlfcn.h"

namespace utils
{

DynamicLibrary::DynamicLibrary() = default;

DynamicLibrary::DynamicLibrary(const char* filename) { Open(filename); }

DynamicLibrary::~DynamicLibrary() { Close(); }

DynamicLibrary::DynamicLibrary(DynamicLibrary&& o) noexcept
    : handle(std::exchange(o.handle, nullptr)) {}

DynamicLibrary& DynamicLibrary::operator=(DynamicLibrary&& o) noexcept {
    Close();
    handle = std::exchange(o.handle, nullptr);
    return *this;
}

bool DynamicLibrary::Open(const char* filename) {
    handle = dlopen(filename, RTLD_NOW);
    return IsOpen();
}

bool DynamicLibrary::IsOpen() const { return handle != nullptr; }

void DynamicLibrary::Close() {
    if (IsOpen()) {
        dlclose(handle);
        handle = nullptr;
    }
}

void* DynamicLibrary::GetSymbolAddr(const char* name) const {
    return reinterpret_cast<void*>(dlsym(handle, name));
}

}; // namespace wallpaper

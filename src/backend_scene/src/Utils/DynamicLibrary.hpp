#pragma once

#include <utility>
#include "NoCopyMove.hpp"

namespace utils
{

class DynamicLibrary : NoCopy {
public:
    DynamicLibrary();
    ~DynamicLibrary();

    DynamicLibrary(const char* filename);

    DynamicLibrary(DynamicLibrary&& o) noexcept;
    DynamicLibrary& operator=(DynamicLibrary&& o) noexcept;

    bool IsOpen() const;
    bool Open(const char* filename);
    void Close();

    void* GetSymbolAddr(const char* name) const;
    
    // not using func deduction
    template<typename T>
    bool GetSymbol(const char* name, T& pfunc) const {
        pfunc = reinterpret_cast<T>(GetSymbolAddr(name));
        return pfunc != nullptr;
    }

private:
    void* handle { nullptr };
};
} // namespace wallpaper

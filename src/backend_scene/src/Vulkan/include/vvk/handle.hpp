#pragma once

#include <utility>
#include <cassert>

#include "Core/NoCopyMove.hpp"
#include "Utils/Logging.h"

namespace vvk
{
/// hold vulkan handle
template<typename Type, typename OwnerType, typename Dispatch>
class Handle : NoCopy {
public:
    using handle_type = Type;
    /// Construct a handle and hold it's ownership.
    explicit Handle(Type handle_, OwnerType owner_, const Dispatch& dld_) noexcept
        : handle { handle_ }, owner { owner_ }, dld { &dld_ } {}

    Handle() = default;
    Handle(std::nullptr_t) {}

    Handle(Handle&& rhs) noexcept
        : handle { std::exchange(rhs.handle, nullptr) }, owner { rhs.owner }, dld { rhs.dld } {}
    Handle& operator=(Handle&& rhs) noexcept {
        Release();
        handle = std::exchange(rhs.handle, nullptr);
        owner  = rhs.owner;
        dld    = rhs.dld;
        return *this;
    }

    /// Destroys the current handle if it existed.
    ~Handle() noexcept { Release(); }

    /// Destroys any held object.
    void reset() noexcept {
        Release();
        handle = nullptr;
    }

    /// Returns the address of the held object.
    /// Intended for Vulkan structures that expect a pointer to an array.
    const Type* address() const noexcept { return std::addressof(handle); }

    /// Returns the held Vulkan handle.
    const Type& operator*() const noexcept { return handle; }

    /// Returns true when there's a held object.
    explicit operator bool() const noexcept { return handle != nullptr; }

protected:
    Type            handle = nullptr;
    OwnerType       owner  = nullptr;
    const Dispatch* dld    = nullptr;

private:
    /// Destroys the held object if it exists.
    void Release() noexcept {
        if (handle) {
            Destroy(owner, handle, *dld);
        }
    }
};

struct NoOwner {};
struct NoOwnerLife {};

/// No owner
template<typename Type, typename Dispatch>
class Handle<Type, NoOwner, Dispatch> : NoCopy {
public:
    using handle_type = Type;
    /// Construct a handle and hold it's ownership.
    explicit Handle(Type handle_, const Dispatch& dld_) noexcept
        : handle { handle_ }, dld { &dld_ } {}

    Handle() = default;
    Handle(std::nullptr_t) {}

    Handle(Handle&& rhs) noexcept: handle { std::exchange(rhs.handle, nullptr) }, dld { rhs.dld } {}
    Handle& operator=(Handle&& rhs) noexcept {
        Release();
        handle = std::exchange(rhs.handle, nullptr);
        dld    = rhs.dld;
        return *this;
    }

    /// Destroys the current handle if it existed.
    ~Handle() noexcept { Release(); }

    /// Destroys any held object.
    void reset() noexcept {
        Release();
        handle = nullptr;
    }

    /// Returns the address of the held object.
    /// Intended for Vulkan structures that expect a pointer to an array.
    const Type* address() const noexcept { return std::addressof(handle); }

    /// Returns the held Vulkan handle.
    const Type& operator*() const noexcept { return handle; }

    /// Returns true when there's a held object.
    explicit operator bool() const noexcept { return handle != nullptr; }

protected:
    Type            handle = nullptr;
    const Dispatch* dld    = nullptr;

private:
    /// Destroys the held object if it exists.
    void Release() noexcept {
        if (handle) {
            Destroy(handle, *dld);
        }
    }
};

// No owner and no life -> no release
template<typename Type, typename Dispatch>
class Handle<Type, NoOwnerLife, Dispatch> {
public:
    using handle_type = Type;
    /// Construct a handle and hold it's ownership.
    explicit Handle(Type handle_, const Dispatch& dld_) noexcept
        : handle { handle_ }, dld { &dld_ } {}

    Handle()  = default;
    ~Handle() = default;

    Handle(std::nullptr_t) {}
    /// Destroys the current handle if it existed.

    /// Destroys any held object.
    void reset() noexcept { handle = nullptr; }

    /// Returns the address of the held object.
    /// Intended for Vulkan structures that expect a pointer to an array.
    const Type* address() const noexcept { return std::addressof(handle); }

    /// Returns the held Vulkan handle.
    const Type& operator*() const noexcept { return handle; }

    /// Returns true when there's a held object.
    explicit operator bool() const noexcept { return handle != nullptr; }

protected:
    Type            handle = nullptr;
    const Dispatch* dld    = nullptr;
};

} // namespace vvk

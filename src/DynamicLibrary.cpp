#include "DynamicLibrary.h"

#include <utility>

#include "Errors.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

DynamicLibrary::DynamicLibrary(const std::filesystem::path& path) {
    // Путь передаём как wide-строку, иначе не открываются папки с русскими буквами.
    HMODULE h = ::LoadLibraryW(path.wstring().c_str());
    if (h == nullptr)
        throw PluginError("cannot load library (WinAPI error " + std::to_string(::GetLastError()) +
                          ", file is not a valid DLL or has the wrong bitness)");
    handle_ = reinterpret_cast<void*>(h);
}

DynamicLibrary::~DynamicLibrary() { release(); }

DynamicLibrary::DynamicLibrary(DynamicLibrary&& other) noexcept
    : handle_(std::exchange(other.handle_, nullptr)) {}

DynamicLibrary& DynamicLibrary::operator=(DynamicLibrary&& other) noexcept {
    if (this != &other) {
        release();
        handle_ = std::exchange(other.handle_, nullptr);
    }
    return *this;
}

void* DynamicLibrary::symbol(const char* name) const {
    if (handle_ == nullptr) return nullptr;
    return reinterpret_cast<void*>(::GetProcAddress(static_cast<HMODULE>(handle_), name));
}

void DynamicLibrary::release() noexcept {
    if (handle_ == nullptr) return;
    ::FreeLibrary(static_cast<HMODULE>(handle_));
    handle_ = nullptr;
}

namespace platform {

const char* libraryExtension() { return ".dll"; }

std::filesystem::path executableDir() {
    wchar_t buf[MAX_PATH];
    const DWORD len = ::GetModuleFileNameW(nullptr, buf, MAX_PATH);
    if (len > 0 && len < MAX_PATH) return std::filesystem::path(buf).parent_path();

    std::error_code ec;
    return std::filesystem::current_path(ec);  // если не получилось, берём текущую папку
}

}  // namespace platform

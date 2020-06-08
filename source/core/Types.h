#pragma once

#include <cinttypes>
#include <stddef.h>
#include <memory>

using int8 = std::int8_t;
using int16 = std::int16_t;
using int32 = std::int32_t;
using int64 = std::int64_t;
using uint8 = std::uint8_t;
using uint16 = std::uint16_t;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;
using byte = uint8;
using uint = uint32;

using char8 = char8_t;
using char16 = char16_t;

// CHECK: properly convert with codecs / locales or wait for full C++20 support
[[nodiscard]] inline const char* U8(const char8_t* txt) noexcept
{
	return reinterpret_cast<const char*>(txt);
}

// TODO: move those

template<typename T, typename D = std::default_delete<T>>
using UniquePtr = std::unique_ptr<T, D>;

enum class TextureFiltering
{
	Nearest,
	Linear,
};

enum class MipmapFiltering
{
	Nearest,
	Linear,
	NoMipmap
};

// CHECK: (mirrored clamping etc)
enum class TextureWrapping
{
	ClampToEdge,
	MirroredRepeat,
	Repeat
};

enum class ImageFormat
{
	Unorm,
	Srgb,
	Hdr
};

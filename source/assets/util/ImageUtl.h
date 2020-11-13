#pragma once

namespace stbaux {

inline void LoadImage(
	const char* imagePath, ImageFormat& format, int32& width, int32& height, std::vector<byte>& dstData)
{
	auto isHdr = stbi_is_hdr(imagePath) == 1;


	void* data = nullptr;
	if (!isHdr) {
		data = stbi_load(imagePath, &width, &height, nullptr, STBI_rgb_alpha);
	}
	else {
		format = ImageFormat::Hdr;
		data = stbi_loadf(imagePath, &width, &height, nullptr, STBI_rgb_alpha);
	}

	const bool hasNotResult = !data || (width == 0) || (height == 0);

	CLOG_ABORT(hasNotResult, "Image loading failed, filepath: {}, data_empty: {} width: {} height: {}", imagePath,
		static_cast<bool>(data), width, height);

	// PERF: crappy std::vector initialization on resize,
	// to solve fork stb_image and pass preallocated pointer to loading functions
	size_t byteCount = (width * height * 4llu) * (isHdr ? sizeof(float) : sizeof(byte));
	dstData.resize(byteCount);

	memcpy(dstData.data(), data, byteCount);
	free(data);
}

inline void LoadImage(const char* imagePath, bool isHdr, byte* dstData)
{
	int32 width, height;
	void* data = nullptr;
	if (!isHdr) {
		data = stbi_load(imagePath, &width, &height, nullptr, STBI_rgb_alpha);
	}
	else {
		data = stbi_loadf(imagePath, &width, &height, nullptr, STBI_rgb_alpha);
	}

	const bool hasNotResult = !data || (width == 0) || (height == 0);

	CLOG_ABORT(hasNotResult, "Image loading failed, filepath: {}, data_empty: {} width: {} height: {}", imagePath,
		static_cast<bool>(data), width, height);

	// PERF: crappy std::vector initialization on resize,
	// to solve fork stb_image and pass preallocated pointer to loading functions
	size_t byteCount = (width * height * 4llu) * (isHdr ? sizeof(float) : sizeof(byte));

	memcpy(dstData, data, byteCount);
	free(data);
}

inline void GetImageResolution(const char* imagePath, int32& width, int32& height)
{
	int32 cmp;
	stbi_info(imagePath, &width, &height, &cmp);
}

inline bool IsHdr(const char* imagePath)
{
	return stbi_is_hdr(imagePath) == 1;
}

} // namespace stbaux

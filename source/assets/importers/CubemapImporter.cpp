#include "pch.h"
#include "CubemapImporter.h"

#include "assets/AssetImporterManager.h"
#include "assets/util/ImageUtl.h"
#include <nlohmann/json.hpp>

#include <fstream>

BasePodHandle CubemapImporter::Import(const fs::path& path)
{
	const auto finalPath = path.generic_string();

	std::ifstream inStream(uri::ToSystemPath(finalPath));

	CLOG_ABORT(!inStream.is_open(), "Unable to open cubemap file, path: {}", uri::ToSystemPath(finalPath));
	int32 firstChar = inStream.peek();

	CLOG_ABORT(firstChar == EOF, "Found empty cubemap file: {} No images loaded", uri::ToSystemPath(finalPath));

	nlohmann::json j;
	inStream >> j;

	std::string newFilePath;

	static std::unordered_map<std::string, int32> imageNames = {
		{ "front", Cubemap::Front },
		{ "back", Cubemap::Back },
		{ "up", Cubemap::Up },
		{ "down", Cubemap::Down },
		{ "right", Cubemap::Right },
		{ "left", Cubemap::Left },

	};

	auto& [handle, pod] = AssetImporterManager->CreateEntry<Cubemap>(
		path.generic_string(), path.filename().replace_extension().string());

	AssetImporterManager->PushPath(path.filename().replace_extension());

	bool firstLoaded = true;
	for (auto& [key, value] : imageNames) {
		std::string imagePath = j.value(key, "");

		CLOG_ABORT(imagePath.empty(), "Missing face path in cubemap: {}", uri::ToSystemPath(finalPath));

		auto finalImagePath = (path.parent_path() / imagePath).string();

		int32 width, height;
		stbaux::GetImageResolution(finalImagePath.c_str(), width, height);
		bool isHdr = stbaux::IsHdr(finalImagePath.c_str());

		CLOG_ABORT(width != height, "Cubemap face width/height missmatch: {}", uri::ToSystemPath(finalImagePath));

		ImageFormat format = isHdr ? ImageFormat::Hdr : ImageFormat::Srgb;
		if (firstLoaded) {
			pod->resolution = width;
			pod->format = format;
			firstLoaded = false;

			auto bytesPerPixel = pod->format == ImageFormat::Hdr ? 4u * 4u : 4u;
			auto size = pod->resolution * pod->resolution * bytesPerPixel * 6;

			pod->data.resize(size);
		}
		else {
			CLOG_ABORT(
				pod->resolution != width, "Cubemap faces resolution missmatch: {}", uri::ToSystemPath(finalPath));
			CLOG_ABORT(pod->format != format, "Cubemap faces format missmatch: {}", uri::ToSystemPath(finalPath));
		}


		size_t offset = value * width * height * 4llu * (isHdr ? sizeof(float) : sizeof(byte));
		stbaux::LoadImage(finalImagePath.c_str(), isHdr, pod->data.data() + offset);
	}

	AssetImporterManager->PopPath();

	return handle;
}

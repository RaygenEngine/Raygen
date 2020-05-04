#include "pch.h"
#include "CubemapImporter.h"

#include "assets/AssetImporterManager.h"

#include <fstream>

BasePodHandle CubemapImporter::Import(const fs::path& path)
{
	const auto finalPath = path.generic_string();

	std::ifstream inStream(uri::ToSystemPath(finalPath));

	CLOG_ABORT(!inStream.is_open(), "Unable to open string file, path: {}", uri::ToSystemPath(finalPath));
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

	auto& [handle, pod]
		= ImporterManager->CreateEntry<Cubemap>(path.generic_string(), path.filename().replace_extension().string());

	pod->faces.resize(imageNames.size());

	ImporterManager->PushPath(path.filename().replace_extension());

	bool firstLoaded = true;
	for (auto& [key, value] : imageNames) {
		std::string imagePath = j.value(key, "");

		CLOG_ABORT(imagePath.empty(), "Missing face path in cubemap: {}", uri::ToSystemPath(finalPath));

		auto finalImagePath = path.parent_path() / imagePath;
		pod->faces[value] = ImporterManager->ImportRequest<Image>(finalImagePath);

		auto face = pod->faces[value].Lock();

		CLOG_ABORT(
			face->width != face->height, "Cubemap face width/height missmatch: {}", uri::ToSystemPath(finalPath));

		if (face->format == ImageFormat::Unorm) {
			// WIP: force srgb for cubemaps
			const_cast<Image*>(face)->format = ImageFormat::Srgb;
		}

		if (firstLoaded) {
			pod->resolution = face->width;
			pod->format = face->format;
			firstLoaded = false;
		}
		else {
			CLOG_ABORT(
				pod->resolution != face->width, "Cubemap faces resolution missmatch: {}", uri::ToSystemPath(finalPath));
			CLOG_ABORT(pod->format != face->format, "Cubemap faces format missmatch: {}", uri::ToSystemPath(finalPath));
		}
	}

	ImporterManager->PopPath();

	return handle;
}

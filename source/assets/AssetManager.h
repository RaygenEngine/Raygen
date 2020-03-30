#pragma once
#include "assets/AssetImporterManager.h"
#include "assets/AssetPod.h"
#include "assets/AssetRegistry.h"
#include "assets/PodEntry.h"
#include "assets/PodHandle.h"
#include "assets/UriLibrary.h"
#include "engine/console/ConsoleVariable.h"
#include "engine/Engine.h"
#include "engine/Logger.h"
#include "reflection/PodReflection.h"

#include <future>

// DOC: OUTDATED, Please update.
// ASSET URI:
// Documentation of Asset URI convention: (DOC: ASSETS After refactor is finished)
//
// 4 types of assets: (first letter of the URI categorizes the type)
//
// * Default assets uri: "~X". ---> where X goes the UID index of this type (for debugging ONLY).
//
// * Runtime generated assets uri: "#flatGenPath" ---> flatGenPath needs to be a flat path (Maybe these won't stay in
//     final design and use just interal uid and handles without paths)
//
// * External disk file paths. "C:/Absolute/Disk/Path/x.gltf" OR (decide design) "../../relative/directory/x.gltf" these
//     paths can access "any" filesystem path to import from the user computer. This type of path MAY use additional
//     JSON data at the end of the path to show sub assets. eg: "../model.gltf{"mat"="gold"}". This path is what is
//     saved at the metadata field: originalImportLocation
//
// * Internal pod assets use the old URI system starting with "/" but CANNOT use json data and MUST resolve to a file
//     asset with .pod or .json with proper header metadata.
//


inline class AssetManager_ {
public:
	AssetManager_(const fs::path& workingDir = "assets/", const fs::path& defaultBinPath = "gen-data/");

	void Import(const fs::path& path) { ImporterManager->m_importerRegistry.ImportFile(path); }

	template<CONC(CAssetPod) T>
	[[nodiscard]] PodHandle<T> ImportAs(const fs::path& path)
	{
		return ImporterManager->m_importerRegistry.ImportFile<T>(path);
	}
} * AssetManager{};

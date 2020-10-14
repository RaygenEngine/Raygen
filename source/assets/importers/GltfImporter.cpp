#include "GltfImporter.h"

#include "assets/importers/gltf/GltfCache.h"
#include "assets/importers/gltf/GltfSceneToStaticMeshLoader.h"
#include "assets/importers/gltf/GltfSkinnedMeshLoader.h"

#include <tinygltf/tiny_gltf.h>

namespace tg = tinygltf;

BasePodHandle GltfImporter::Import(const fs::path& path)
{
	gltfutl::GltfCache cache{ path };

	BasePodHandle first;

	for (auto scene : cache.gltfData->scenes) {
		gltfutl::GltfSceneToStaticMeshLoader loader{ cache, scene };
		// return first assigned static
		if (!first.HasBeenAssigned()) {
			first = loader.GetLoadedPod();
		}
	}

	for (uint32 i = 0; auto skin : cache.gltfData->skins) {
		gltfutl::GltfSkinnedMeshLoader loader{ cache, i++, skin };
	}

	return first;
}

#pragma once
#include "core/MetaTemplates.h"
// When adding a pod, add it to both of these

#define ENGINE_POD_TYPES                                                                                               \
	GltfFilePod, ImagePod, MaterialPod, ModelPod, ShaderPod, StringPod, TexturePod, JsonDocPod, BinaryPod

// Can be macroed
struct GltfFilePod;
struct ImagePod;
struct MaterialPod;
struct ModelPod;
struct ShaderPod;
struct StringPod;
struct TexturePod;
struct JsonDocPod;
struct BinaryPod;


// Returns the default uid for this pod in the asset manager, all pod handles are initialized with the the valid
// "default"
template<typename PodType>
constexpr size_t GetDefaultPodUid()
{
	return index_of_type_v<PodType, ENGINE_POD_TYPES> + 1;
}

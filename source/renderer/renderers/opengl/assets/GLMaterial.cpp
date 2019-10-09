#include "pch/pch.h"

#include "renderer/renderers/opengl/assets/GLMaterial.h"
#include "asset/AssetManager.h"
#include "renderer/renderers/opengl/GLAssetManager.h"
#include "renderer/renderers/opengl/assets/GLTexture.h"

namespace OpenGL {
bool GLMaterial::Load()
{
	auto am = Engine::GetAssetManager();

	auto materialData = podHandle.Lock();

	auto glAm = GetGLAssetManager(this);

	baseColorTexture = glAm->GpuGetOrCreate<GLTexture>(materialData->baseColorTexture);
	metallicRoughnessTexture = glAm->GpuGetOrCreate<GLTexture>(materialData->metallicRoughnessTexture);
	occlusionTexture = glAm->GpuGetOrCreate<GLTexture>(materialData->occlusionTexture);
	normalTexture = glAm->GpuGetOrCreate<GLTexture>(materialData->normalTexture);
	emissiveTexture = glAm->GpuGetOrCreate<GLTexture>(materialData->emissiveTexture);

	return true;
}

} // namespace OpenGL

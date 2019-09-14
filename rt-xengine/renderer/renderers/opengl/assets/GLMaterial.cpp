#include "pch.h"

#include "renderer/renderers/opengl/test/GLTestRenderer.h"
#include "renderer/renderers/opengl/assets/GLMaterial.h"

namespace OpenGL
{
	bool GLMaterial::Load()
	{
		m_baseColorTexture = 
			GetGLAssetManager(this)->GetOrMakeFromPtr<GLTexture>(&m_materialData->baseColorTexture);
		
		m_occlusionMetallicRoughnessTexture = 
			GetGLAssetManager(this)->GetOrMakeFromPtr<GLTexture>(&m_materialData->occlusionMetallicRoughnessTexture);
		
		m_normalTexture = 
			GetGLAssetManager(this)->GetOrMakeFromPtr<GLTexture>(&m_materialData->normalTexture);

		m_emissiveTexture = 
			GetGLAssetManager(this)->GetOrMakeFromPtr<GLTexture>(&m_materialData->emissiveTexture);

		return true;
	}

}

#include "pch.h"

#include "renderer/renderers/opengl/GLAssetManager.h"
#include "renderer/renderers/opengl/GLAsset.h"

void OpenGL::GLAssetManager::Delete(uint64 cacheHash)
{
	auto it = m_assetMap.find(cacheHash);
	if (it != m_assetMap.end())
	{
		delete it->second;
	}
}
